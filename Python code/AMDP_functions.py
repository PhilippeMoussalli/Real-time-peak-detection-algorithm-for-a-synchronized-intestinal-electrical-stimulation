import numpy as np
import os
import matplotlib.pyplot as plt
import matplotlib
from scipy.signal import detrend
from scipy import signal
import scipy
from numpy import sin, pi
from scipy.signal import butter,filtfilt
matplotlib.use("TkAgg")
matplotlib.rcParams["legend.loc"] = 'best'


def AMPD(path,recording_file,file_extension,fs,min_cpm,seconds="all",show_plot=False):

    # Open txt file and convert it to list
    data_path_1 = os.path.join(path, recording_file + file_extension)
    data = []
    data = txt_to_list(data_path_1, data, "float")

    # Initializing data array and
    x = np.array(data)
    x_nd = np.copy(x)
    x_nd = x_nd-np.average(x_nd)
    x = detrend(x)
    if seconds == "all":
        N = len(x)
        t = np.linspace(0, N / fs, N)
    else:
        x=x[0:seconds*fs]
        N = len(x)
        t = np.linspace(0, N / fs, N)

    # Define a maximum scale based on the minimum cpm of an animal (15 cpm for dog and 35 for rats)
    max_seconds = np.ceil((1 / (min_cpm / 60)))
    L = int((N / fs) * max_seconds)

    # create LSM matix
    LSM = np.ones((L, N), dtype=bool)
    for k in np.arange(1, L + 1):
        LSM[k - 1, 0:N - k] &= (x[0:N - k] > x[k:N])  # compare to right neighbours
        LSM[k - 1, k:N] &= (x[k:N] > x[0:N - k])  # compare to left neighbours

    # Find scale with most maxima
    G = LSM.sum(axis=1)
    G = G * np.arange(N // 2, N // 2 - L, -1)  # normalize to adjust for new edge regions
    l_scale = np.argmax(G)
    pks_logical = np.min(LSM[0:l_scale, :], axis=0)
    pks = np.flatnonzero(pks_logical)

    # Estimate cpm
    cpm = round((len(pks)* 60)/t[-1],2)

    if show_plot==True:

        plt.plot(t, x, '-')
        plt.plot(t[pks], x[pks], 'bo')
        plt.hlines(np.mean(x), 0, t[-1])
        plt.xlabel('time/s')
        plt.ylabel('Amplitude(mV)')
        plt.legend(['Signal','Peaks'])
        plt.show()

    return pks,l_scale,cpm,t,x,x_nd



def txt_to_list(data_path,list,type):

    """ Functions that reads recording from text file line by line and returns a list. output of list can be float (for
    slow wave recordings) or int (for datapeaks index)"""

    with open(data_path, "r") as data2:
        datalines = (line.rstrip('\t\n') for line in data2)
        for line in datalines:
            list.append(line)
        if (type =="float"):
            list = [float(x) for x in list]
        else:
            list = [int(x) for x in list]
        return list

def split_recordings(path,recording_file,file_extension,fs,seconds_split):

    """ Split long recordings into sub files depending on the seconds specified per each recording and the
    sampling frequency of the recorded file. Returns a folder containing the split recordings located in the path of
    the read file  """


    N = (fs * seconds_split) + 1  # number of data points per file for a chosen seconds_split
    data_path = os.path.join(path, recording_file + file_extension)
    write_path = os.path.join(path, recording_file + " split_folder")
    if not os.path.exists(write_path):
        os.mkdir(write_path)
    print("Number of data points per file is", N)

    data = []
    data = txt_to_list(data_path,data,"float")

    N_files = int(np.ceil(len(data) / N))

    start_range = 0
    end_range = N

    for i in range(N_files):

        file = open(os.path.join(write_path, recording_file + "_split_" + str(i) + file_extension), "w")

        for n in range(start_range, end_range):
            file.write(str(data[n]) + '\n')
        file.close()

        start_range = end_range
        if (i == (N_files - 2)):
            end_range = len(data)
        else:
            end_range = end_range + N + 1

    return write_path


def resample(path,recording_file,file_extension,fs_old,fs_new):

        sample_factor= fs_old//fs_new

        a = recording_file.find("hz")
        if (a > 0) and recording_file[a-3].isdigit():
            write_file= recording_file[0:(a - 3)] + str(fs_new) + "hZ" + recording_file[(a + 2):]
        elif (a > 0) and recording_file[a-2].isdigit():
            write_file = recording_file[0:(a - 2)] + str(fs_new) + "hZ" + recording_file[(a + 2):]
        else:
            write_file= recording_file + "-" + str(fs_new) + "_hZ"

        write_path = os.path.join(path, write_file + file_extension)
        data_path = os.path.join(path, recording_file + file_extension)
        data = []

        data = txt_to_list(data_path, data, "float")

        x = np.array(data)
        x = detrend(x)
        N = len(data)
        #resampled_data = scipy.signal.resample(x, N //sample_factor)
        resampled_data =scipy.signal.decimate(x,sample_factor)
        with open(write_path, "w") as txt_file:
            for line in resampled_data:
                txt_file.write(str(line) + "\n")

        print("Data has been resampled from",fs_old,"hz to",fs_new ,"hz with a a sample factor of",sample_factor)
        print("File name:",write_file)

        return write_file

def countPairs(AMPD_peaks,online_peaks,delay):

    """ Used to estimate accuracy of AMPD for different delays """

    online_peaks = [x -delay for x in online_peaks]
    counter =0
    for i in range(len(AMPD_peaks)):
        for j in range(len(online_peaks)):
            if AMPD_peaks[i]==online_peaks[j]:
                counter+=1

    return counter

def detect_peaks_online(x,fs,cpm,last_peak_index,GT_pks,max_scale,positive_lag,window_size,show_plot=False):

    """
    1) Option to smooth the signal before detection (useful when signal is noisy especially for rats)
    """

    x = np.array(moving_average(x, window_size))

    """ 
    2) Define the different ranges that will be used during detection 
    """
    # In seconds
    cpm_range_s = (1 / (cpm / 60))
    low_range_s = cpm_range_s * 0.75
    high_range_s = cpm_range_s * 1.25


    # In terms of datapoints

    low_range_n = int(np.floor(low_range_s / (1 / fs))) + 1
    high_range_n = int(np.floor(high_range_s / (1 / fs))) + 1
    cpm_range_n = int(np.floor(cpm_range_s / (1 / fs))) + 1

    """ 
    3) Intialize detection range, lists for storing missed peaks and new peaks
    and peak index for detecting the first peak online 
    """

    start_range = last_peak_index + low_range_n
    end_range = last_peak_index + high_range_n
    range_pks = x[start_range:end_range]   # range where peaks should be detected
    N = len(x)
    t = np.linspace(0, N / fs, N)

    pknew = []
    pkmissed = []
    peak_index = last_peak_index

    while (end_range < N): # Keep looping until the end of the signal

        for i in range(len(range_pks)): #Loop through every datapoint in the detection range (to evaluate whether it's a peak or not)

            compare_lst_before = x[start_range - max_scale + i : start_range + i]# list of values to be compared before the current peak
            compare_lst_after = x[i + start_range + 1 :i + start_range + 1 + positive_lag]
            detected = False  # Boolean is used to trigger the section for "missed peak" if no peak was actually detected

            # if condition applies ONLY when the current data point is bigger than all the previous datapoints (number of datapoints
            # is defined by max_scale) AND the current datapoint is bigger than the next datapoint

            if all(range_pks[i] > n for n in compare_lst_before ) and all(range_pks[i] > v for v in compare_lst_after):
                """Peak detected """
                peak_index = i + start_range  #this if condition will only apply when peak is detected, in that case intialize a new peak index and store it
                pknew.append(peak_index)
                detected = True
                start_range = peak_index + low_range_n  #Re-intialize the start-range, end_range and detection range for peaks
                end_range = peak_index + high_range_n
                range_pks = x[start_range:end_range]
                break

        if (detected == False):   # Is activated when no peaks are detected in the detection_range --> "missed peak")
            """Missed peak """
            peak_missed_index = peak_index + cpm_range_n  #The missed peak is initialized at a distance of cpm from the last peak (this will happen when the current
            #datapoint reaches the end-range of the detection range)
            start_range = end_range  # The starting point of detection will be the end range of the previous detection range where no peak was detected
            end_range = start_range + low_range_n
            pkmissed.append(peak_missed_index)
            range_pks = x[start_range:end_range]  #re-intialize the peak detection range

    if show_plot == True:

        offset = 0.025 * max(abs(x))
        plt.plot(t, x, '-')
        plt.plot(t[GT_pks], x[GT_pks], 'bo')
        plt.plot(t[pknew], x[pknew] + offset, 'ro')
        plt.plot(t[pkmissed], x[pkmissed], 'go')
        plt.vlines(t[last_peak_index],-max(abs(x))*0.75,max(abs(x))*0.75)
        plt.legend(["Signal", "Real Peaks", "Online peaks", "Missed peaks","Last peak AMPD"])
        plt.xlabel('time/s')
        plt.ylabel('Amplitude(mV)')
        plt.show()

    return pknew,pkmissed

def moving_average(signal,window_size):
    # Window_size defines the number of datapoints to be taken into account when averaging
    averaged = np.copy(signal)
    N = len(averaged)
    N_prior = window_size-1 # number of datapoints to be averaged
    for i in range(N_prior,N-N_prior):
        average= (sum(averaged[i - N_prior:i]) + averaged[i]) / window_size
        averaged[i] = average

    return averaged

def plot_PSD(path,recording_file,file_extension,fs):

    data_path_1 = os.path.join(path, recording_file + file_extension)
    data = []
    data = txt_to_list(data_path_1, data, "float")
    f, Pxx_den = signal.periodogram(data, fs)
    f_w, Pxx_den_w = signal.welch(data, fs, nperseg=1024)
    plt.figure(figsize=(5, 4))
    plt.semilogy(f, Pxx_den)
    plt.semilogy(f_w, Pxx_den_w)
    plt.ylim([1e-7, 1e2])
    plt.xlabel('frequency [Hz]')
    plt.ylabel('PSD [V**2/Hz]')

def butter_lowpass_filter(path, recording_file, file_extension, cutoff, fs, order):

    data_path = os.path.join(path, recording_file + file_extension)
    data = []
    data = txt_to_list(data_path, data, "float")
    nyq = 0.5*fs
    normal_cutoff = cutoff / nyq
    # Get the filter coefficients
    b, a = butter(order, normal_cutoff, btype='low', analog=False)
    y = filtfilt(b, a, data)
    write_file = recording_file+"_filtered"
    write_path = os.path.join(path, write_file + file_extension)

    with open(write_path, "w") as txt_file:
        for line in y:
            txt_file.write(str(line) + "\n")

    return y,data


def detect_peaks_online_2(x,fs,cpm,second_split,GT_pks,max_scale,positive_lag,window_size,show_plot=False):

    """
    1) Option to smooth the signal before detection (useful when signal is noisy especially for rats)
    """
    x2= np.copy(x)
    x = np.array(moving_average(x, window_size))

    """ 
    2) Define the different ranges that will be used during detection 
    """
    # In seconds
    cpm_range_s = (1 / (cpm / 60))
    low_range_s = cpm_range_s * 0.75
    high_range_s = cpm_range_s * 1.25


    # In terms of datapoints

    low_range_n = int(np.floor(low_range_s / (1 / fs))) + 1
    high_range_n = int(np.floor(high_range_s / (1 / fs))) + 1
    cpm_range_n = int(np.floor(cpm_range_s / (1 / fs))) + 1

    """ 
    3) Intialize detection range, lists for storing missed peaks and new peaks
    and peak index for detecting the first peak online 
    """
    last_peak_index = 0
    start_range = last_peak_index + low_range_n
    end_range = last_peak_index + high_range_n
    range_pks = x[start_range:end_range]   # range where peaks should be detected
    N = len(x)
    t = np.linspace(0, N / fs, N)

    pknew = []
    pkmissed = []
    peak_index = last_peak_index

    while (end_range < N): # Keep looping until the end of the signal

        for i in range(len(range_pks)): #Loop through every datapoint in the detection range (to evaluate whether it's a peak or not)
            compare_lst_before = x[start_range - max_scale + i : start_range + i]# list of values to be compared before the current peak
            compare_lst_after = x[i + start_range + 1 :i + start_range + 1 + positive_lag]
            detected = False  # Boolean is used to trigger the section for "missed peak" if no peak was actually detected
            # if condition applies ONLY when the current data point is bigger than all the previous datapoints (number of datapoints
            # is defined by max_scale) AND the current datapoint is bigger than the next datapoint

            if all(range_pks[i] > n for n in compare_lst_before ) and all(range_pks[i] > v for v in compare_lst_after):
                """Peak detected """
                peak_index = i + start_range  #this if condition will only apply when peak is detected, in that case intialize a new peak index and store it
                pknew.append(peak_index)
                detected = True
                start_range = peak_index + low_range_n  #Re-intialize the start-range, end_range and detection range for peaks
                end_range = peak_index + high_range_n
                range_pks = x[start_range:end_range]
                break

        if (detected == False):   # Is activated when no peaks are detected in the detection_range --> "missed peak")
            """Missed peak """
            peak_missed_index = peak_index + cpm_range_n

                #The missed peak is initialized at a distance of cpm from the last peak (this will happen when the current
            #datapoint reaches the end-range of the detection range)
            start_range = end_range  # The starting point of detection will be the end range of the previous detection range where no peak was detected
            end_range = start_range + low_range_n
            pkmissed.append(peak_missed_index)
            range_pks = x[start_range:end_range]  #re-intialize the peak detection range

    if show_plot == True:
        pks_plot= [x+positive_lag for x in pknew]
        offset = 0.025 * max(abs(x))
        plt.plot(t, x, '-')
        plt.plot(t[GT_pks], x[GT_pks], 'bo')
        plt.plot(t[pks_plot], x[pks_plot] + offset, 'ro')
        plt.plot(t[pkmissed], x[pkmissed], 'go')
        plt.vlines(t[last_peak_index],-max(abs(x))*0.75,max(abs(x))*0.75)
        plt.legend(["Signal", "Real Peaks", "Online peaks", "Missed peaks","Last peak AMPD"])
        plt.xlabel('time/s')
        plt.ylabel('Amplitude(mV)')
        plt.show()

    return pknew,pkmissed,x

def define_range (cpm,fs,std):

    # In terms of datapoints
    cpm_range_n = int(round( ((60/cpm)*fs)+1 ))
    low_range_n = int(round(cpm_range_n*(1-std)))
    high_range_n = int(np.floor(cpm_range_n*(1+std)))


    return low_range_n,high_range_n,cpm_range_n

def add_noise (signal,target_SNR):

    # Calculate signal power and convert to dB
    power = signal**2
    sig_avg_watts = np.mean(power)
    sig_avg_db = 10 * np.log10(sig_avg_watts)

    # Calculate noise according to [2] then convert to watts
    noise_avg_db = sig_avg_db - target_SNR
    noise_avg_watts = 10 ** (noise_avg_db / 10)

    # Generate n sample of white noise
    noise = np.random.normal(0, np.sqrt(noise_avg_watts), len(signal))
    signal_noise = signal+noise

    return signal_noise