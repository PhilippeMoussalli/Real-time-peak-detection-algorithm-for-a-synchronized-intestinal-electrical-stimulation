# Real-time-peak-detection-algorithm-for-a-synchronized-intestinal-electrical-stimulation  
  ## Overview
Porject in collaboration with the Univerity of Michigan and Transtimulation Research Inc.  
   ## Background
The epidemic of obesity has seen a rise in numbers throughout the last decades due to the increase of economic growth, indutrialization and adoption of a sedentary lifestyle. It affects around a third of the current world population and is considered a risk factor for a wide variety of diseases such as cardiovascular, immunological and neurological problems as well as an increase risk of developing type 2 diabetes (T2D).  
**Intestinal electrical stimulation (IES)** has been proposed as a method for treating obesity and diabetes due to the various physiological mechanisms associated with it that alleviate and treats diabetes and obesity:  
* Reduction in food intake due to delayed gastric emptying  
* Reduction in intestinal absorption due to the enhanced contratility of the smooth muscles of the intestines that accelarates the movements of the chyme (mixed food)  
* Increase in GLP-1 release which in turn stimulates the production and secretion of insuline  
* Decrease in Ghrelin (Hunger hormone) that causes more insulin to be secreted in the blood 


**Synchronized intestinal electical stimulation** has been proposed as a variant of IES where the electrical stimulation is delivered in synchrony with the peaks of the intestinal slow waves of the intestines to accelerate and enhance the effect of IES (in IES, the electrical stimulation is delivered at a random phase).  
  ## Project Goals  
  The main goal of thos project is to develop a real-time algorithm that is able to detect the peaks of the intestinal slow waves in real-time and deliver a stimulus in synchrony with peaks. The requirement of the algorithm are as follows:  
  * Minimal delay between the actual onset of the peaks and the time of detection in the algorithm
  * Robust against noise
  * Implemented in C (algorithm will be integrated on an IPG chip and tested on clinical experimental trials conducted on rats)  
  * Memory conservation and minimal calculations to ensure a maximal lifetime of the IPG battery
  * High accuracy: reflected by a high sensitivity to the algorithm and low number of missed peaks
  
  ## Methodolgy  
  I developd and tested the prototype of the algorithm that I developd in Python due to ease of implementation and visualization of the result, and then proceeded to adapt the final prototype to C code for implementation on the IPG chip. I was provided with intestinal slow wave recordings of rats and dogs (~12 recordings for each animal). The following steps were taken to develop the final algorithm:  
  
  ### 1) Determining the optimal scale for peak detection  
  For a given (Quasi)-periodic signal, a peak should have a higher amplitude than a certain number of datapoint before and afer it. This number is termed the **optimal scale** and it was derived using the **Automatic Multi-scale peak detection algorithm (AMPD)**. For more details on the working mechanism of the algorithm please refere to [1]. Insert figure  
  
  The maximal scale (\Lambda)  $\lambda$ $\Lambda$
  \hat{\imath} $\rightarrow$ $\hat{\imath}$, \hat{\jmath}
  
  
