  #include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include <string.h>
#include<malloc.h>
#include <math.h>
#include <unistd.h>
#include <stdbool.h>
#define DEST_SIZE 500

/**< -----------------------------------------------------------------------------------------------------------------------------------------/

/**< Functions declaration */
float *getarray(int* size_arr_main,char path[]);
float average (float a[],int size);
int is_bigger(float cmp_value,float cmp_arr[],int len_arr);
void slice_array (float dest_arr[],float src_arr[],int starting_location_dest,int starting_location_src,int nr_elements);
void write_peaks(char path_w[],int idx_peaks[],int n_peaks);
void define_range(int *cpm_range,int *low_range,int *high_range,int fs,int cpm,float std);
int average_2 (int a[],int size);

/**< -----------------------------------------------------------------------------------------------------------------------------------------/

/**< Global variables*/

    /**< Those are the only variables that need to be changed

        /**<Path variables: the name of the path where the data is stored, the name of the recording file and the file extension*/

char path[] = "C:\\Users\\Philippe\\Desktop\\USA Internship\\AMDP\\Recording data\\Rats (35 cpm)\\Initial recording";
char recording_file[] = "R1 postprandial-20_hZ";
char file_extension[] =".txt";
static float fs=20;  /**Specify the sampling frequency used to record the data. The usual range is (20-200 Hz) */


    /**<Peak detection parameters */
static int min_cpm = 35;
static int cpm = 40;  /**<Set up the average cpm for defining the detection range)*/
static int WS=1;  /**<Smoothing window size*/
static int PL=3;  /**<Positive lag (how many values to look at after the current sample)*/
static int l_scale=15;  /**< Optimal scale as defined by AMPD (how many values to look at before the current sample)*/
static int cpm_win_len = 5; /**< Length of the moving window for estimation of instant cpm*/
static float std = 0.1; /**< Standard deviation away from the average cpm for defining the adaptive detection range*/


int main()
{
        /**< --------------------------------------------------------(1)Initialization steps----------------------------------------------------------*/

    /**< (1.1) Paths */

    char path_r[DEST_SIZE]; /**< Create a string copy of the path for reading*/
    char path_w[DEST_SIZE]; /**< Create a string copy of the path for writing*/
    strcpy(path_r,path); /**< Copy the original path name to the reading path*/
    strcpy(path_w,path); /**< Copy the original path name to the writing path*/
    strcat(strcat(strcat(path_r,"\\"),recording_file),file_extension);  /**< Concatenate the recording file name to the reading path*/
    strcat(strcat(strcat(strcat(path_w,"\\"),recording_file),"_peaks_idx"),file_extension);  /**< Concatenate the recording file name (+ suffix) to the writing path*/

    /**< (1.2) Accessing data and allocating it to a dynamic memory*/

    int size_arr_main =0;  /**< Initalize pointer Array size*/
    float *ptr; /**< Initalize pointer to slow wave memory location that will be allocated in function "getarray"*/
    ptr = getarray(&size_arr_main,path_r); /**< "getarray" returns the pointer to the array and the array size (passed as an address argument)*/
    int N=size_arr_main/sizeof(ptr[0]);  /**< Number of data points*/

    /**< (1.3) Parameter definition */

         /**< (1.3.1) Detection range*/
    int cpm_range;  /**< Range of length cpm --> when no peak is detected, the missed peak will have a length of cpm from the last peak */
    int low_range; /**< Lower range of the detection range*/
    int high_range; /**< Higher range of the detection range*/
    define_range(&cpm_range,&low_range,&high_range,fs,cpm,std);

        /**< (1.3.2) Define different windows: smoothing, left and right hand side comparison and cpm window*/

    float arr_smooth[WS];      /**< Temporary array to store values that will be averaged (depending on size of WS) to iteratively smooth out the signal*/
    float cmpr_before[l_scale]; /**< Array for comparing the current datapoint to previous values (left hand side comparison)*/
    float cmpr_after[PL];  /**< Array for comparing the current datapoint to successive values(Right hand side comparison)*/
    int moving_window[cpm_win_len]; /**< Moving window of 5 elements to store the last 5 cpm values--> will store cpm values after 60 seconds of recording*/

        /**< (1.3.3) Define different booleans for left and right hand side comparison in addition to boolean to indicate whether a peak has been detected*/

    int bool_before;    /**< Boolean for left hand side comparison (is "1" when current value is bigger than all previous values up to l-scale)*/
    int bool_after;  /**< Boolean for right hand side comparison (is "1" when current value is bigger than all successive values up to PL)*/
    int bool_detected = 0; /**< Boolean that is activated when peak is detected (when both 'bool_before' and 'bool_after' are True)--> if no peak is detected within the detection range then this
                            boolean will be used to trigger missed peak section*/
    int bool_60s = 0; /**< Boolean that indicates that 60 seconds is passed*/

         /**< (1.3.4) Variable related to peaks*/

    int arr_len = (high_range+PL)*2;  /**< The array to store the values will have a length equals to the higher range of detection + Positive lag (compare values after current datapoint). It is mutiplied by 2 for safety factor since detection range changes depending on avg cpm*/
    int * detected_pks = malloc(1*sizeof(int)); /**< Dynamic memory allocation for storage of detected peaks (only for checking purposes, not needed in IPG implementation)*/
    int last_peak_idx=0;  /**< Store detected peaks */
    int last_peak_idx_cpy=0; /**< Copy that is used to calculate instant cpm (created a copy so it wont interfere with the missed peak index estimation since instant-CPM is calculated only at a distance between two consecutive "FOUND" peaks */
    int missed_peak; /**< Store missed peaks */
    int j=0;  /**< counter for temporary array */
    int pks_counter =0; /**< counter for  number of detected peaks */
    int missed_pks =0; /**< counter for  number of detected peaks */
    int early_pks=0;/**< counter for peak that are detected early */
    float arr_temp[arr_len];  /**< Temporary array to store data for each detection range cycle--> This array will be used to store value from memory/IPG and will re-initialize
        once the end peak is reached within a detection range (missed peak) or when a peak is detected  */
    for (int i=0;i<arr_len;i++){  /**< Initialize Temporary array with zero values*/
        arr_temp[i]=0;
    }

        /**< (1.3.5) Variable related to cpm*/
    int * inst_cpm_arr_60s = malloc(200*sizeof(int)); /**< Dynamic memory allocation for storage of cpm values up to 60 seconds)*/
    int cntr_cpm_60s=0;  /**< Count number of registred cpm up to 60 seconds for dynamic memory allocation*/
    int average_cpm=0;  /**< Average cpm caclulated from moving average cpm window --> used to regulate cpm */
    int inst_cpm; /**< Instanteous cpm for each new datapoint that is attached to peak (compare distance to last peak) */
    int cntr_continue =0; /**< Counter to continue iterating unitl next peak is detected when a peak is detected too early*/
    int cpm_arr_win[cpm_win_len]; /**< array to store cpm values. Used to calculate the average cpm*/
    int next_peak;  /**< Used to define which peak will be next peak when early detection occurs/


     /**< --------------------------------------------------------(2)Online peak detection algorithm----------------------------------------------------------*/


    /**< (2.1) Online peak detection algorithm */

    for (int i=0;i<N;i++){   /**< Outer most loop that will loop through the datapoints of slow wave stored in memory (in real
                                                    life, this has to be replaced with function that reads from electrodes)*/

        arr_temp[j] = ptr[i];     /**< Populate temporary array with slow wave data*/

//        When 60 seconds is reached, start calculating average cpm and redefine the detection range
            if (i==60*fs){

                slice_array(cpm_arr_win,inst_cpm_arr_60s,0,cntr_cpm_60s-cpm_win_len,cpm_win_len);
                average_cpm = round(average_2(cpm_arr_win,cpm_win_len));
                define_range(&cpm_range,&low_range,&high_range,fs,average_cpm,std);
                bool_60s=1;
            }

            if (i >= (WS-1)){       /**< If condition that is activated once enough datapoints are stored in the temporary array to perform smoothing operation */
            slice_array(arr_smooth,arr_temp,0,(j-(WS-1)),WS); /**< Copy datapoints to be smoothed to 'arr_smooth-*/
            arr_temp[j]=average(arr_smooth,WS);  /**< Update the current value with the smoothed value*/
            }

//        Counter to wait for next peak if peak is detected early
            if(cntr_continue>=1){
                cntr_continue-=1;

//        Next peak is reached
                if(cntr_continue==1){
                    slice_array(arr_temp,arr_temp,0,j+1,PL);
                    j=PL-1;
                    last_peak_idx=i;
                    inst_cpm=round(60 / ( (last_peak_idx - last_peak_idx_cpy) * (1 / fs)));
//       First 60 second store cpm values in an array
                    if (bool_60s==0){
                        inst_cpm_arr_60s[cntr_cpm_60s]=inst_cpm;
                        cntr_cpm_60s+=1;
                    }
//      After 60 seconds, store them in moving averate window and restimate the detection range and hte a
                    else{
                        slice_array(cpm_arr_win,cpm_arr_win,0,1,cpm_win_len-1);
                        cpm_arr_win[cpm_win_len-1]=inst_cpm;
                        average_cpm = round(average_2(cpm_arr_win,cpm_win_len));
                        if (average_cpm<min_cpm){
                                average_cpm=min_cpm;
                        }
                        define_range(&cpm_range,&low_range,&high_range,fs,average_cpm,std);
                    }
                last_peak_idx_cpy=last_peak_idx;
                printf("early_peak at %i \n",last_peak_idx_cpy);
                }

            goto next_iteration;
            }

        /**< Checking for peak within a range*/

            /**< This section is triggered when we are within the detection range
            IMPORTANT: Peaks are stored in their exact location (where they are estimated) but in fact there is a lag of PL that is taken into account
            in the algorithm (ex: peak detected and stored at index [140] but it is actually detected at the moment when
            the array is populated to value of [143] for a PL set to 3 which is a delay of 150 ms for a sampling frequency of 20 hz) */

        if ( (j >= low_range-PL) && (j < high_range+PL) ){  /**< Check if the temporary array has reach the detection range*/


         slice_array(cmpr_before,arr_temp,0,j-(l_scale+PL),l_scale);  /**< Copy values to be compared before the current value to 'cmpr_before' array */
         slice_array(cmpr_after,arr_temp,0,j-PL+1,PL);    /**< Copy values to be compared after the current value to 'cmpr_after' array*/
         float cmpr_value = arr_temp[j-PL]; /**< substract PL to accounts for delay*/

         bool_before = is_bigger(cmpr_value,cmpr_before,l_scale); /**< Check if current value is bigger than all the previous values up to l_scale*/
         bool_after = is_bigger(cmpr_value,cmpr_after,PL); /**< Check if current value is bigger than all the succeeding values up to PL*/


        /**< Found peak*/

         if( (bool_before == 1) && (bool_after == 1) ){


            last_peak_idx = i;  /**< Last peak index (subtract PL to account for delay)*/
            inst_cpm = (round(60 / ((last_peak_idx - last_peak_idx_cpy) * (1 / fs))));

            if(inst_cpm>cpm+cpm*std){

                next_peak= last_peak_idx_cpy+cpm_range;
                cntr_continue=next_peak-i+1;
                early_pks+=1;
                goto next_iteration;

            }

            if (bool_60s==0){
                inst_cpm_arr_60s[cntr_cpm_60s]=inst_cpm;
                cntr_cpm_60s+=1;

            }

            else{
                slice_array(cpm_arr_win,cpm_arr_win,0,1,cpm_win_len-1);
                cpm_arr_win[cpm_win_len-1]=inst_cpm;
                average_cpm = round(average_2(cpm_arr_win,cpm_win_len));
                if (average_cpm<min_cpm){
                        average_cpm=min_cpm;
                }
                define_range(&cpm_range,&low_range,&high_range,fs,average_cpm,std);
            }


            printf("Peak found ! at [%i] with value %f \n ",last_peak_idx,cmpr_value);
            detected_pks[pks_counter]=last_peak_idx; /**< Store peak in array*/
            pks_counter++;  /**<Increment peak counter*/
            int bool_detected = 1; /**<Prevents triggering of Missed Peak sub_section*/
            slice_array(arr_temp,cmpr_after,0,0,PL);  /**< Copy values after the detected peak (PL values) to the new array to be used for another detection*/
            j=PL-1;  /**< Re-initialize counter for the temporary array*/
         }


        /**< Missed peak*/

         if ( (j==high_range+PL-1) && (bool_detected==0) ){
            missed_peak=i; /**< Missed peak index is at location cpm from last peak*/
            last_peak_idx = missed_peak;
            inst_cpm=round(60 / ( (last_peak_idx - last_peak_idx_cpy) * (1 / fs)));
            printf("miss peak %i \n",missed_peak);

            if (bool_60s==0){
                inst_cpm_arr_60s[cntr_cpm_60s]=inst_cpm;
                cntr_cpm_60s+=1;
            }

            else{
                slice_array(cpm_arr_win,cpm_arr_win,0,1,cpm_win_len-1);
                cpm_arr_win[cpm_win_len-1]=inst_cpm;
                average_cpm = round(average_2(cpm_arr_win,cpm_win_len));
                if (average_cpm<min_cpm){
                        average_cpm=min_cpm;
                }
                define_range(&cpm_range,&low_range,&high_range,fs,average_cpm,std);
            }
            slice_array(arr_temp,cmpr_after,0,0,PL);  /**< Re-initialize arr_temp with last iterations up to the detection range
                            so that in the next iteration there we will be within the detection range (when we miss a peak, we look for another one directly after it)*/

            j = PL - 1;
            missed_pks++;
         }

        }
    last_peak_idx_cpy= last_peak_idx;
    next_iteration:
    j++;
    bool_detected =0;
}

/**< --------------------------------------------End of peak detection -----------------------------------------------*/
/**< ------------------------------------------------Print Statements------------------------------------------------*/

    printf("Number of detected peaks is %i \n",pks_counter);
    printf("Number of missed peaks is %i \n",missed_pks);
    printf("Number of Early peaks is %i \n",early_pks);
    return 0;

}



/**< -----------------------------------------------------------------------------------------------------------------------------------------/
/**< ----------------------------------------------------------------Functions---------------------------------------------------------------------/
/**< -----------------------------------------------------------------------------------------------------------------------------------------/*/

/**< ---------------------------------------------------------------Function #1-----------------------------------------------------------------/

/**< Function used to open a saved txt file with the slow wave data and returns starting pointer to the data array
and the size of the array in the memory (maloc) */


float *getarray(int* size_arr_main,char path[])
{

    char line[60];
    int lines= 0;
    int i=0;

    FILE* file = fopen(path, "r");

    if(!file){
        printf("\n Unable to open : %s ", path);
        exit(0);
        return -1;
    }

    while (fgets(line, sizeof(line), file)) {
        lines++;
    }

    int size_arr= lines*sizeof(float);
    * size_arr_main = size_arr; /**< dereference pointer that is passed in main to equal array size*/
    float *p = malloc(size_arr); /**< dynamic allocation of memory since array size can vary*/
    rewind(file); /**< return file reader pointer to its initial location to loop from the beginning of the file*/

    while (fgets(line, sizeof(line), file)) {
            p[i]= atof(line);  /**< convert string from txt to double and store it in the allocated memory*/
            i++;
    }

    fclose(file);
    return p;  /**<function returns a pointer to the array*/

}

/**< ---------------------------------------------------------------Function #2-----------------------------------------------------------------/

/**< Function to slice through array by choosing the source and destination array in additional to the starting location and number of
elements to slice through --> used to define the range to be averaged in the smoothing filter and also the range of comparison */

void slice_array (float dest_arr[],float src_arr[],int starting_location_dest,int starting_location_src,int nr_elements){

    memcpy(&dest_arr[starting_location_dest], &src_arr[starting_location_src], nr_elements*sizeof(*src_arr));

}

/**< ---------------------------------------------------------------Function #3-----------------------------------------------------------------/

/**< Function that returns a boolean (0 or 1) to compare if current datapoint is bigger than numbers defined in a certain range (positive lag
and optimal scale comparison)*/

int is_bigger(float cmp_value,float cmp_arr[],int len_arr) {
    bool is_bigger = true;
    for (int i =0;i<len_arr;i++){
            if(cmp_value<cmp_arr[i]){
                is_bigger = false;
                break;
            }
    }
return is_bigger;

}

/**< ---------------------------------------------------------------Function #4----------------------------------------------------------------/

/**< Function that returns the average of a given array used for the smoothing filter*/

float average (float a[],int size)
{
    int i;
    float avg, sum=0;
    for(i=0;i<size;++i)
    {
        sum+= a[i];
    }
    avg = sum/size;
    return avg;
}
/**< ---------------------------------------------------------------Function #5----------------------------------------------------------------/

/**< Function that returns the average of a given array used for average cpm*/

int average_2 (int a[],int size)
{
    int i;
    int avg, sum=0;
    for(i=0;i<size;++i)
    {
        sum+= a[i];
    }
    float size_2 = 1.0 * size;
    avg = round(sum/size_2);
    return avg;
}
/**< ----------------------------------------------------------------Function #6----------------------------------------------------------------*/

void write_peaks(char path_w[],int idx_peaks[],int n_peaks){

            char file_name_w[DEST_SIZE];
            strcat(strcat(file_name_w,recording_file),"-peaks_idx.txt");
            if( access( path_w, F_OK ) != -1 ) {
            printf("Warning: Peak data file already exists for this dataset ! Data file will be deleted and a new one will be \ncreated for this run \n\n");
            remove(path_w);
            }
            else{
                printf("No peak data file exists for this dataset. A new one will be created. \n");
            }

            FILE *fptr;
            fptr = fopen(path_w,"w");

            for(int i = 0; i < n_peaks; ++i){
                     fprintf(fptr,"%i \n",idx_peaks[i]);
            }
            fclose(fptr);
            printf("Peaks stored in text file: %s \n\n",strcat(recording_file,"_peaks_idx.txt"));

}

/**< ----------------------------------------------------------------Function #7----------------------------------------------------------------*/

void define_range(int *cpm_range,int *low_range,int *high_range,int fs,int cpm,float std){


    *cpm_range = round(((60.0/cpm)*fs)+1);  /**< Range of length cpm --> when no peak is detected, the missed peak will have a length of cpm from the last peak */
    *low_range = round(*cpm_range*(1-std)); /**< Lower range of the detection range*/
    *high_range =floor(*cpm_range*(1+std)); /**< Higher range of the detection range*/

}
/**< ----------------------------------------------------------------------(end)---------------------------------------------------------------*/
