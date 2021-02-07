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
<img src="/figures/IES.PNG" width="30%">

**Synchronized intestinal electical stimulation** has been proposed as a variant of IES where the electrical stimulation is delivered in synchrony with the peaks of the intestinal slow waves of the intestines to accelerate and enhance the effect of IES (in IES, the electrical stimulation is delivered at a random phase).  

<img src="/figures/slow_wave_rat.PNG" width="50%">  

  ## Project Goals  
  The main goal of thos project is to develop a real-time algorithm that is able to detect the peaks of the intestinal slow waves in real-time and deliver a stimulus in synchrony with peaks. The requirement of the algorithm are as follows:  
  * Minimal delay between the actual onset of the peaks and the time of detection in the algorithm    
  * Robust against noise  
  * Implemented in C (algorithm will be integrated on an IPG chip and tested on clinical experimental trials conducted on rats)  
  * Memory conservation and minimal calculations to ensure a maximal lifetime of the IPG battery  
  * High accuracy: reflected by a high sensitivity to the algorithm and low number of missed peaks  
  
  ## Methodolgy and implementation  
Please refer to the attached [document](SIES.pdf) for a full description relating to the implementation of the non-adaptive and adaptive version of the algorithm.  

## Results 

### Visual illustration of the real-time peak detection algorithm 
* Click [here](/figures/rat_illustration.mp4) for a visual illustration of the non-adaptive algorithm for rat intestinal slow waves.
* Click [here](/figures/Dog_illustration.mp4) for a visual illustration of the non-adaptive algorithm for dog intestnial slow waves.
### Accuracy results for dog and rats for dogs and rats for different noise levels
An overall accuracy of 90% can be acheived with an overall delay of less than 10% of the slow wave cycle.  
<img src="/figures/dog_results.PNG" width="60%">  
  
<img src="/figures/rat_results.PNG" width="60%">

  
