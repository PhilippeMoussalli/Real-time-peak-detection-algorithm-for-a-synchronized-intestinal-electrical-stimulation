#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include <string.h>
#include<malloc.h>
#include <math.h>
#include <unistd.h>

#define DEST_SIZE 500

/**< -----------------------------------------------------------------------------------------------------------------------------------------/

/**< Functions declaration */

float *getarray(int* size_arr_main,char path[]);
void Detrend (float x[], float *p,int n);
int maximal_scale(int *ptr_LSM,int L,int N);
void write_peaks(char path_w[],int idx_peaks[],int n_peaks);

/**< -----------------------------------------------------------------------------------------------------------------------------------------/

/**< Global variables*/

/**< Those are the only variables that need to be changed

1) Specify the name of the path where the data is stored, the name of the recording file and the file extension
2) Specify the minimum cycles per minute (cpm) depending on the animal from which the data is derived
3) Specify the sampling frequency used to record the data. The usual range is (20-200 Hz) */


char path[] = "C:\\Users\\Philippe\\Desktop\\USA Internship\\AMDP\\Recording data_stats\\Rats (35 cpm)\\Initial recording\\Animation";
char recording_file[] = "ISW1-10hz";
char file_extension[] =".txt";
static float min_cpm = 39;  /**<Set up the minimum cycles per min for an animal (needed to estimate the maximal LSM scale
                                                                      Dog = 15 cpm
                                                                       Human = 9 cpm
                                                                       Rat = 35 cpm)
                                                                       */
static float fs=10;  /**<Initialize Sampling frequency*/

/**< -----------------------------------------------------------------------------------------------------------------------------------------/
/**< ----------------------------------------------------------------Main---------------------------------------------------------------------/
/**< ----------------------------------------------------------------------------------------------------------------------------------------*/

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

    /**< (1.3) Array length and constructing time array */

    int N=size_arr_main/sizeof(ptr[0]);  /**< Number of data points*/

        /**< Construct time array*/

        float time[N];
        time[0] = 0;
        for (int i=1;i<N;i++){
            time[i] = time[i-1]+(1/fs);
        }

    /**< --------------------------------------------------------(2)AMPD Algorithm----------------------------------------------------------/
    /**< (2) AMPD Algorithm  */

        /**< (2.1) Detrend the recorded data*/

        Detrend(time,ptr,N);

        /**< (2.2) Define a maximum scale in seconds (depends on minimum cpm of animal)*/

        int max_seconds = ceil( (1/(min_cpm/60)) );
        int L = ceil(max_seconds/(1/fs));
        int L1 = (N/fs)*max_seconds;
        /**< (2.3) Initialize LSM matrix with 1 values)*/
        int (*LSM)[N] = malloc(sizeof(int[L][N]));
        for(int i=0;i<L;i++){
            for(int j=0;j<N;j++){
                 LSM[i][j]=1;
            }
        }
        /**< (2.4) fill the LSM matrix)*/

            /**< (2.4.1) compare to Right neighbours)*/
            for(int k=1;k<L+1;k++){
                for(int n=0;n<N-k;n++){
                    LSM[k-1][n] = *(ptr+n)>*(ptr+k+n);
                }

                /**< (2.4.2) compare to Left neighbours)*/
                for(int n=0;n<N-k;n++){
                    if( (*(ptr+k+n)>*(ptr+n)) && (LSM[k-1][k+n]==1) ){ /**< (2.4.3)(Right and left hand side comparison have to be
                                                                            fulfilled for an index to count as a peak for a certain
                                                                            scale*/
                        LSM[k-1][k+n] = 1;

                    } else{

                        LSM[k-1][k+n] = 0;
                      }
                }
            }

         /**< (2.5) Find scale with most maxima)*/
////        for(int n=0;n<L;n++){
////                for(int i=0;i<N;i++){
////                        printf("[%i][%i] %i \n",n,i,LSM[n][i]);
////                }
////
////        }
        int max_scale = maximal_scale(LSM,L,N);
        printf("L is %i L1 is %i \n\n",L,L1);
        printf("the maximum scale is %d \n\n", max_scale) ;

        /**< (2.6) Find peaks by testing all scales for each data point in LSM matrix up to "max_scale")*/

        int binary_peak[N];   /**< values of (1) correspond to peak and values of (0) is not peak */
        printf("Finding peaks and storing their indices ... \n");
        printf("\n");
            /**< (2.6.1)find minimum value in LSM matrix up to "max_scale". If there is peak then the
                    matrix contains only 1 values up to "max_scale" and the minimum value will be 1. Otherwise if one of the scales
                    does not satisfy the condition, then the value will be (0)*/

            for (int i=0;i<N;i++){
                int min_value = LSM[0][i];
                for(int j=1;j<max_scale;j++){
                        if(LSM[j][i]<min_value){
                            min_value = LSM[j][i];
                        }
                    }
                binary_peak[i] = min_value;
                }

            free(LSM); /**< Free heap from LSM matrix since we will not need it anymore */

            /**< (2.6.2) Now that we have an array of the peaks, we need to extract the indexes of those peaks and store them in a matrix
            (useful for plotting purposes). Peaks will be stored in a text file in the same path where the data was read from*/

            int n_peaks=0;

            /**< Count how many peaks there are */
            for (int n=0;n<N;n++){
                if(binary_peak[n]==1){
                    n_peaks++;
                }
            }

            /**< Store peaks indexes */
            int i=0;
            int idx_peaks[n_peaks];
            for (int n=0;n<N;n++){
                if(binary_peak[n]==1){
                    idx_peaks[i]=n;
                    i++;
                }
            }
            printf("Indices have been stored in an array. Saving them in a text file... \n\n");
            float min_cpm_copy= min_cpm;
            float fs_copy= fs;  /**< Create a copy for printing purposes */

            write_peaks(path_w,idx_peaks,n_peaks);

            printf("Indices have been saved. \n\n");

        /**< (2.7) Print statementS to give status of the outcome of the AMPD (check if everything is ok)*/

            float recorded_cpm = (n_peaks*60)/time[N-1];
            float recorded_cpm_seconds =  (1/(recorded_cpm/60));
            float seconds_max_scale = (max_scale/fs_copy);
            printf("The Algorithm was completed successfully! \n\nStatus:\n");
            printf("1) Sampling frequency is %.2f and number of data points is %i \n",fs_copy, N);
            printf("2) The number of detected peaks is %i peaks recorded for a period of %.2f seconds \n",n_peaks, time[N-1]);
            printf("3) The maximal setup scale (%i) that corresponds to the minimum of %i cpm is %i seconds \n",L,(int)min_cpm_copy,max_seconds);
            printf("4) The scale with the most maxima based on LSM matrix is (%i) which corresponds to %.2f seconds \n",max_scale,seconds_max_scale);
            printf("5) The estimated cpm from AMPD is: %.2f cpm which corresponds to %.2f seconds\n\n", recorded_cpm, recorded_cpm_seconds);
            printf("To visualize the data with the overlaid peaks, use the python script 'Plot_AMPD.py'\n\n ");

return 0;

}



/**< -----------------------------------------------------------------------------------------------------------------------------------------/
/**< ----------------------------------------------------------------Functions---------------------------------------------------------------------/
/**< -----------------------------------------------------------------------------------------------------------------------------------------/


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

/**< --------------------------------------------------------------Function #2--------------------------------------------------------------------/

/**< Function that estimates the linear fit of the slow wave array and returns a detrended version of the array
 (linear detrending) */

void Detrend (float x[], float *p,int n){

    float x_s[n];
    float xy[n];
    float sum_x=0;
    float sum_y=0;
    float sum_xs=0;
    float sum_xy=0;
    float m=0;
    float b=0;
    for(int i=0;i<n;i++){
        x_s[i] = x[i]* x[i];
        xy[i] = x[i]* *(p+i);
    }

    for(int i=0;i<n;i++){
        sum_x+= x[i];
        sum_y+= *(p+i);
        sum_xs+= x_s[i];
        sum_xy+= xy[i];
    }

    m = ((n*sum_xy) - sum_x*sum_y)/(n*sum_xs-(sum_x*sum_x));
    b = (sum_y-m*sum_x)/n;


    for(int j=0;j<n;j++){
        *(p+j) = *(p+j) - (m*x[j]+b);
    }
    printf("Data has been detrended \n\n");
}

/**< ----------------------------------------------------------------Function #3----------------------------------------------------------------*/

int maximal_scale(int *ptr_LSM,int L,int N){

        /**< Create matrix to store scales with most maximal values)*/
        int maxima[L];
        memset(maxima, 0, L*sizeof(int)); /**< Initialize with zero values)*/
        for (int i=0; i<L;i++){
            for (int j=0; j<N; j++){
                maxima[i]+= *(ptr_LSM+(i*N)+j);
            }
        }

        /**< Normalization to adjust for positive edges)*/
            /**< Create normalization scale array(in descending order since lower scales have the most positive edges that were set a priori)*/
            int norm_scale[L];
            for (int j=0; j<L;j++){
                norm_scale[j]= (N/2)-j;
            }
            /**< Multiply maxima array with normalization scale array to normalize data before finding the maxima)*/

            for (int j=0; j<L;j++){
                maxima[j] = maxima[j]*norm_scale[j];
            }

        /**< Find maximum value of the array (index)*/
            int max_idx;
            int max_value = maxima[0];
            for(int j=1;j<L;j++){
                if(maxima[j]>max_value){
                    max_value=maxima[j];
                    max_idx=j;
                }
            }

        return max_idx;
}

/**< ----------------------------------------------------------------Function #4----------------------------------------------------------------*/

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
/**< ----------------------------------------------------------------------(end)---------------------------------------------------------------*/
