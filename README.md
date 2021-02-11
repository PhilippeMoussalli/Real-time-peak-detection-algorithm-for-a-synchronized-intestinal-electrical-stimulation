# Real-time-peak-detection-algorithm-for-a-synchronized-intestinal-electrical-stimulation  
  ## Overview
Project in collaboration with the Univerity of Michigan and Transtimulation Research Inc.  
   ## Background
The epidemic of obesity has seen a rise in numbers throughout the last decades due to the increase of economic growth, indutrialization and adoption of a sedentary lifestyle. It affects around a third of the current world population and is considered a risk factor for a wide variety of diseases such as cardiovascular, immunological and neurological problems as well as an increase risk of developing type 2 diabetes (T2D).  
**Intestinal electrical stimulation (IES)** has been proposed as a method for treating obesity and diabetes due to the various physiological mechanisms associated with it that alleviates and treats diabetes and obesity:  
* Reduction in food intake due to delayed gastric emptying  
* Reduction in intestinal absorption due to the enhanced contratility of the smooth muscles of the intestines that accelarates the movements of the chyme (mixed food)  
* Increase in GLP-1 release which in turn stimulates the production and secretion of insuline  
* Decrease in Ghrelin (Hunger hormone) that causes more insulin to be secreted in the blood  
<img src="/figures/IES.PNG" width="40%">

**Synchronized intestinal electical stimulation** has been proposed as a variant of IES where the electrical stimulation is delivered in synchrony with the peaks of the intestinal slow waves of the intestines to accelerate and enhance the effect of IES (in IES, the electrical stimulation is delivered at a random phase).  

<img src="/figures/slow_wave_rat.PNG" width="40%">  

  ## Project Goals  
  The main goal of thos project is to develop a real-time algorithm that is able to detect the peaks of the intestinal slow waves in real-time and deliver a stimulus in synchrony with peaks. The requirement of the algorithm are as follows:  
  * Minimal delay between the actual onset of the peaks and the time of detection in the algorithm    
  * Robust against noise  
  * Implemented in C (algorithm will be integrated on an IPG chip and tested on clinical experimental trials conducted on rats)  
  * Memory conservation and minimal calculations to ensure a maximal lifetime of the IPG battery  
  * High accuracy: reflected by a high sensitivity to the algorithm and low number of missed peaks  
  
  ## Methodolgy and implementation  
Please refer to the attached [document](SIES.pdf) for a full description relating to the implementation of the non-adaptive and adaptive version of the algorithm.  

## SIES Results 

### 1) Algorithm results
#### Visual illustration of the real-time peak detection algorithm 
* Click [here](/figures/rat_illustration.mp4) for a visual illustration of the non-adaptive algorithm for rat intestinal slow waves.
* Click [here](/figures/Dog_illustration.mp4) for a visual illustration of the non-adaptive algorithm for dog intestnial slow waves.  
  
  <img src="/figures/results_peak.png" width="60%"> 
#### Accuracy results for dog and rats for dogs and rats for different noise levels
An overall accuracy of >90% can be acheived with an overall delay of less than 10% of the slow wave cycle.  

##### Results Rat  

<img src="/figures/lag_rat.PNG" width="60%">  
<img src="/figures/table1_rat.PNG" width="60%">  

##### Results dog  

<img src="/figures/lag_dog.PNG" width="60%">  
<img src="/figures/table1_dog.PNG" width="60%">  
  
### 2) Physiological results  
Results from submitted paper following the experimental trial of the algorithm on rats: "CLOSED-LOOP INTESTINAL ELECTRICAL STIMULATION SYNCHRONIZED WITH INTRINSIC INTESTINAL SLOW WAVE AMELIORATES GLUCAGON-INDUCED HYPERGLYCEMIA IN RATS VIA ENHANCED RELEASE OF GLP-1" Paper in processdings for DDW 2021.

The results demonstrated that:
* Compared to sham (no stimulation), IES and SIES significantly reduced postprandial blood glucose at 30 min by 17% and 20%, respectively. SIES showed a further inhibitory effect at 60 min (147 vs. 171 mg/dl, P=0.001, vs. sham).  
* Compared to sham (137 pg/ml), GLP-1 at 30 min was increased in both IES (157 pg/ml) and SIES (169 pg/ml). GLP-1 level was still high at 60 min in rats with SIES while those in sham and IES groups returned to the basal level.  
* At 30 min, the plasma insulin level was 18.8 µIU/ml with SIES, which was significantly higher than that with sham (7.1 µIU/ml, p<0.001) and that with IES (13.2 µIU/ml, P=0.046).  
  
 **To conclude:**  
   
  Compared to IES, synchronized IES is more effective in reducing postprandial blood glucose in rats with glucagon-induced acute hyperglycemia by enhancing the release of GLP-1 and insulin.


  
