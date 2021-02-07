from AMDP_functions import *
import pandas as pd
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
positive_lag = [3];  # Positive lag
Window_size = [1]; # window size
l_scale = 15; # optimal scale as defined by AMPD (how many values to look at before the current sample)
cpm_win_len =5; # Length of moving window for estimation of insant cpm
std = 0.1 # Standard deviation away from the average peak where detected peak might be too soon or too late
""" 
3) Define the different ranges that will be used during detection 
"""

[low_range_n,high_range_n,cpm_range_n] = define_range (cpm,fs,std)
""" 

4) First step is to calculate the AMPD to extract the max_scale(l_scale), the average cpm and the last peak 

    4.1) We will calculate it on the full signal to extract the Ground truth peak (GT) and the time and array. This is 
    only for reporting the performance
"""

for name in file_list:
    print(name)
    recording_file = name
    [pks_GT,_,_,t,x,x_nd] = AMPD(path,recording_file,file_extension,fs,min_cpm,seconds="all",show_plot=False)
    #x_nd is the signal before detrending and is what will use in the online peak detection algorithm (cannot detredn the signal
    #online)
    """ 
    4.2) Run the online peak detection algorithm to detect the signals in real time (Optional: smooth the signal before online
    peak detection)
     """
    start = 0.5
    end = -0.55
    preceding_lag = (np.arange(start, end, -1 / fs))
    detection_stats = [preceding_lag]
    missed_peak_stats = []

    for PL in positive_lag:

        for WS in Window_size:

            ## Parameter Intialization

            start_range = low_range_n
            end_range = high_range_n
            arr_len = high_range_n+PL
            temp_arr=[0] * arr_len*2  #Temporary array to store (multiplied by 3 to stay within safety range as the detection range will change depending on the avg_cpm)
            N = len(x)
            t = np.linspace(0, N / fs, N)
            smooth_array = []
            pknew = []  #Store peaks within detection range
            pkmissed = []  #Store missed peaks (when no peak is detected, those peaks are at the upper limit of the detection range)
            pkearly=[]  #Store early peaks (detected before the lower limit of the detection range)
            pkearly_sol=[] #Store peaks within distance cpm of last peak when peak are detected early
            pkall =[] # pknew,pkmissed and pkearlysol are stored here
            cpm_arr=[]
            last_peak_idx = 0
            bool_detected = False
            counter_pass = 0 #Counter activated when peak is detected early
            j=0 # index of temporary array
            inst_cpm= cpm
            inst_cpm_arr_60s=[] #Used to store cpm values up to 60 seconds
            avg_cpm = []
            average_cpm = 0
            ## Online peak detection algorithm

            for i in range(N):

                # Populate temporary array
                temp_arr[j]=x_nd[i]

                # Store average cpm values in moving window once 60 seconds have been reached
                if (i==60*fs):
                    cpm_arr_win = inst_cpm_arr_60s[-cpm_win_len:] #Populate array with last elements
                    average_cpm = round(np.average(cpm_arr_win))
                    del(inst_cpm_arr_60s) #delete array that was used to store elements up to 60 seconds
                    [low_range_n, high_range_n, cpm_range_n] = define_range(average_cpm, fs, std) #Redefine ranges

                # smoothing the signal (Optional--> if WS=1 no smoothing occurs)
                if(i>=(WS-1)):
                    smooth_array = temp_arr[j-(WS-1):j+1]
                    temp_arr[j]= np.average(smooth_array)

                #This counter is activated when there is a peak that is detected too early w.r.t avg_peak(CONDITION 1). Its purpose it to continue looping
                #through the dataset until the datapoint that is at a distance avg_cpm from last peak is detected

                if(counter_pass>=1):

                    counter_pass-=1
                    j+=1

                    # This is the condition when the datapoint is reached --> new peak (or stimulation point)--> reinitialize temporary array
                    if(counter_pass==1):

                        temp_arr[0:PL] = temp_arr[j:j+PL]
                        j = PL
                        last_peak_idx= i
                        inst_cpm = int(round(60 / ((last_peak_idx - last_peak_idx_cpy) * (1 / fs))))

                        try: #this is activated when we are below 60 seconds
                            inst_cpm_arr_60s.append(inst_cpm)
                        except Exception: #this is activated after 60 seconds since inst_cpm_arr_60s is deleted
                            cpm_arr_win[0:cpm_win_len-1] = cpm_arr_win[-cpm_win_len+1::] #Updating cpm window array (shift of one element to the left)
                            cpm_arr_win[-1] = inst_cpm #last element of window is the instant cpm
                            average_cpm = round(np.average(cpm_arr_win)) #recaluclate average cpm
                            if average_cpm<min_cpm: #Keep cpm within acceptable range
                                average_cpm=min_cpm
                            avg_cpm.append(average_cpm)
                            [low_range_n, high_range_n, cpm_range_n] = define_range(average_cpm, fs, std) #redefine ranges


                        last_peak_idx_cpy = last_peak_idx
                        pkearly_sol.append(last_peak_idx)
                        pkall.append(last_peak_idx)


                    continue

                # Estimate peak when we are within the detection range

                if( j>=low_range_n-PL and j<high_range_n+PL):

                    cmpr_before = temp_arr[j-l_scale-PL:j-PL]
                    cmpr_after = temp_arr[j-PL+1:j+1]
                    cmpr_value = temp_arr [j-PL]

                ## If this condition is activated then we have found peak --> either optimal peak or too early
                    if all(cmpr_value > n for n in cmpr_before) and all(cmpr_value > v for v in cmpr_after):

                        #Estimate the instant cpm
                        last_peak_idx = i
                        inst_cpm = int(round(60 / ((last_peak_idx - last_peak_idx_cpy) * (1 / fs))))


                        if(inst_cpm>cpm+cpm*std):
                        #CONDITION (1): EARLY PEAK--> if so stimulate at distance cpm from last peak
                        #by initalizing a counter to keep looping until the next datapoint is reached
                            next_peak= last_peak_idx_cpy+cpm_range_n
                            counter_pass = next_peak-i+1
                            pkearly.append(i)
                            j+=1
                            print("Early at",t[i],"next peak at",t[next_peak])
                            continue

                        #CONDITION (2): peak within acceptable range

                        try: #this is activated when we are below 60 seconds
                            inst_cpm_arr_60s.append(inst_cpm)
                        except Exception: #this is activated after 60 seconds since inst_cpm_arr_60s is deleted
                            cpm_arr_win[0:cpm_win_len-1] = cpm_arr_win[-cpm_win_len+1::] #Updating cpm window array (shift of one element to the left)
                            cpm_arr_win[-1] = inst_cpm #last element of window is the instant cpm
                            average_cpm = round(np.average(cpm_arr_win)) #recaluclate average cpm
                            if average_cpm<min_cpm:
                                average_cpm=min_cpm
                            avg_cpm.append(average_cpm)
                            [low_range_n, high_range_n, cpm_range_n] = define_range(average_cpm, fs, std) #redefine ranges

                        pknew.append(last_peak_idx)
                        pkall.append(last_peak_idx)
                        bool_detected = True
                        temp_arr[0:PL] = cmpr_after
                        j=PL-1

                    #CONDITION (3): Late detection (missed peak)

                    if ((j == high_range_n+ PL - 1) and (bool_detected == False)):

                        missed_peak = i
                        if (missed_peak==1035):
                            print("Debug")
                        pkmissed.append(missed_peak)
                        last_peak_idx = missed_peak
                        inst_cpm = int(round(60 / ((last_peak_idx - last_peak_idx_cpy) * (1 / fs))))
                        try: #this is activated when we are below 60 seconds
                            inst_cpm_arr_60s.append(inst_cpm)
                        except Exception: #this is activated after 60 seconds since inst_cpm_arr_60s is deleted
                            cpm_arr_win[0:cpm_win_len-1] = cpm_arr_win[-cpm_win_len+1::] #Updating cpm window array (shift of one element to the left)
                            cpm_arr_win[-1] = inst_cpm #last element of window is the instant cpm
                            average_cpm = round(np.average(cpm_arr_win)) #recaluclate average cpm
                            if average_cpm<min_cpm:
                                average_cpm=min_cpm
                            avg_cpm.append(average_cpm)
                            [low_range_n, high_range_n, cpm_range_n] = define_range(average_cpm, fs, std) #redefine ranges

                        pkall.append(last_peak_idx)

                        temp_arr[0:PL] = cmpr_after
                        j = PL - 1

                last_peak_idx_cpy = last_peak_idx
                inst_cpm_cpy = inst_cpm
                j+=1;
                print("i is ",i,"low range",low_range_n,"high_range",high_range_n)
                bool_detected = False

    offset = 0.01 * max(abs(x))
    plt.plot(t, x, '-')
    plt.plot(t[pks_GT], x[pks_GT], 'bo')
    plt.plot(t[pkearly], x[pkearly], 'ko')
    plt.plot(t[pkearly_sol], x[pkearly_sol], 'yo')
    plt.plot(t[pknew], x[pknew] + offset, 'ro')
    plt.plot(t[pkmissed], x[pkmissed], 'go')

    plt.legend(["Signal", "Real Peaks","(1.1) Early peak","(1.2) Early detection solution(stimulate at distant cpm_avg from lastpk)","(2) Detection within acceptable range", "(3) Late peaks (missed) distance cpm_avg+std from lstpk"])
    plt.xlabel('time/s')
    plt.ylabel('Amplitude(mV)')
    plt.show()