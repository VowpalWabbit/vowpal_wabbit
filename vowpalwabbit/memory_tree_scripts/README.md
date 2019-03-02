Contextual Memory Tree (CMT)
-------------------------------

This demo exercises CMT for logarithmic time 
multiclass classification in both online and offline manner.
For Online, we report progressive error while for offline we report 
standard prediction error on a holdout test set.

The dataset used is [ALOI](http://aloi.science.uva.nl/) and WikiPara. ALOI
has 1000 classes, and each class has in average 100 training examples. WikiPara
contains 10000 classes. We consider two versions of WikiPara here: 1-shot version which 
contains 1 training example per class, and 2-shot version which contains 2 training examples per class. 

We refer readers to the manuscript (https://arxiv.org/pdf/1807.06473.pdf) for detailed datastrutures and algorithms in CMT

## Training Online Contextual Memory Tree on ALOI and WikiPara:
```bash
python aloi_script_progerror.py
python wikipara10000_script_progerror.py
```

## Training Offline Contextual Memory Tree on ALOI:
```bash
python aloi_script.py
python wikipara10000_script.py
```

