from AMDP_functions import *
import pandas as pd
import numpy as np
""" 
1) Start by initalizing the paths
"""

path = "C:\\Users\\Philippe\\Desktop\\USA Internship\\AMDP\\Recording data\\Rats (35 cpm)\\Initial recording"
file_list = ["R78 fasing", "R6 postprandial", "R6 fasting", "rat 78 postprandial", "R80 postprandial", "R80 fasting",
          "R2 fasting","R1 fasting","R1 postprandial","R2 postprandial"]

file_list = [x+"-20_hZ" for x in file_list]
file_extension=".txt"

"""
2) Initalize the sampling frequency and the average cpm for a certain animal (15 cpm for dog and 35 cpm for rats), how
may second to split the recording for AMPD estimation before running the online peak detection
 """

min_cpm = 35
fs = 20
cpm = 40
positive_lag = [1,2,3,4];  # Positive lag
Window_size = [1,2,3,4]; # window size
l_scale = 15; # optimal scale as defined by AMPD (how many values to look at before the current sample)
std = 0.25
""" 
3) Define the different ranges that will be used during detection 
"""
# In seconds
cpm_range_s = (1 / (cpm / 60))
low_range_s = cpm_range_s * (1-std)
high_range_s = cpm_range_s * (1+std)

# In terms of datapoints

low_range_n = int(np.floor(low_range_s / (1 / fs))) + 1
high_range_n = int(np.floor(high_range_s / (1 / fs))) + 1
cpm_range_n = int(np.floor(cpm_range_s / (1 / fs))) + 1

start = 0.5
end = -0.55
preceding_lag = (np.arange(start, end, -1 / fs))
detection_stats = [preceding_lag]
missed_peak_stats = []
""" 

4) First step is to calculate the AMPD to extract the max_scale(l_scale), the average cpm and the last peak 

    3.1) We will calculate it on the full signal to extract the Ground truth peak (GT) and the time and array. This is 
    only for reporting the performance
"""
#SNR_db = [40,35,30,25,20]

#for snr in SNR_db:

for name in file_list:
    print(name)
    recording_file = name
    [pks_GT,_,_,t,x,x_nd] = AMPD(path,recording_file,file_extension,fs,min_cpm,seconds="all",show_plot=False)
    # x_nd = add_noise(x_nd,snr)

    # plt.plot(t, x, 'b-')
    # plt.plot(t, x_nd ,'g-')
    # plt.legend(["Signal", "Real Peaks", "Online peaks", "Missed peaks","Last peak AMPD"])
    # plt.xlabel('time/s')
    # plt.ylabel('Amplitude(mV)')
    # plt.show()

    """ 
    4) Run the online peak detection algorithm to detect the signals in real time (Optional: smooth the signal before online
    peak detection)
     """


    for PL in positive_lag:

        for WS in Window_size:

            start_range = low_range_n
            end_range = high_range_n
            arr_len = high_range_n+PL
            temp_arr=[0] *arr_len
            N = len(x)
            t = np.linspace(0, N / fs, N)
            smooth_array = []
            pknew = []
            pkmissed = []
            last_peak_idx = 0
            bool_detected = False
            j=0

            for i in range(N):
                temp_arr[j]=x_nd[i]

                if(i>=(WS-1)):
                    smooth_array = temp_arr[j-(WS-1):j+1]
                    temp_arr[j]= np.average(smooth_array)

                if( j>=low_range_n-PL and j<high_range_n+PL):

                    cmpr_before = temp_arr[j-l_scale-PL:j-PL]
                    cmpr_after = temp_arr[j-PL+1:j+1]
                    cmpr_value = temp_arr [j-PL]

             ## Detected peak

                    if all(cmpr_value > n for n in cmpr_before) and all(cmpr_value > v for v in cmpr_after):
                        last_peak_idx = i
                        pknew.append(last_peak_idx)
                        bool_detected = True
                        temp_arr[0:PL] = cmpr_after
                        j=PL-1

             ## Missed peak

                    if ((j == high_range_n+ PL - 1) and (bool_detected == False)):
                        missed_peak = last_peak_idx+cpm_range_n
                        pkmissed.append(missed_peak)
                        last_peak_idx= missed_peak
                        start_index_cpy = j+1-(low_range_n-PL)  #start index to copy last elements
                        end_index_temp = low_range_n-PL #end index for pasting (last element that is pasted is in beginning of detection range)
                        temp_arr[0:end_index_temp]= temp_arr[start_index_cpy:j+1]
                        j = low_range_n - PL - 1;

                j+=1;
                bool_detected = False
            """ 
            5) End of peak detection algorithm --> estimation of accuracy
             """
            print("reached")

            left_distance = int((end) / (-1 / fs))
            right_distance = len(preceding_lag) - left_distance
            index_list = np.arange(left_distance - 1, -right_distance - 1, -1)

            pks_GT_compare = pks_GT # Compare only peaks that were detected online


            total_peaks_original = len(pks_GT_compare)
            total_peaks_online = len(pknew)
            missed_peaks = ((total_peaks_original-total_peaks_online)/total_peaks_original)

            total = 0
            accuracy_lag =[]
            for k in index_list:
                match= countPairs(pks_GT_compare,pknew,k)/total_peaks_original  #percentage of peaks that were correctly detected for a certain lag
                accuracy_lag.append(match)

            detection_stats.append(accuracy_lag)
            missed_peak_stats.append(missed_peaks)

df1 = pd.DataFrame(detection_stats)
df2 = pd.DataFrame(missed_peak_stats)

output_name = str(recording_file)+"PL"+str(PL)+"WS"+str(WS)+"_results_C.xlsx"

    #Excel file output with two tabs for accuracy for different lags and missing peaks

with pd.ExcelWriter(output_name) as writer:
    df1.to_excel(writer, sheet_name='Detection_stats')
    df2.to_excel(writer, sheet_name='Missed_peaks')
    #
        #
        # offset = 0.01 * max(abs(x))
        # plt.plot(t, x, '-')
        # plt.plot(t[pks_GT], x[pks_GT], 'bo')
        # plt.plot(t[pknew], x[pknew] + offset, 'ro')
        # plt.plot(t[pkmissed], x[pkmissed], 'go')
        # plt.legend(["Signal", "Real Peaks", "Online peaks", "Missed peaks","Last peak AMPD"])
        # plt.xlabel('time/s')
        # plt.ylabel('Amplitude(mV)')
        # plt.show()