Contextual Memory Tree (CMT)
-------------------------------

This demo exercises the memory tree for logarithmic time 
multiclass classification in both online and offline manner.
For Online, we report progressive error while for offline we report 
standard prediction error on a holdout test set.

The dataset used is [ALOI](http://aloi.science.uva.nl/), which
has 1000 classes in it. 

We refer readers to the manuscript (https://arxiv.org/pdf/1807.06473.pdf) for detailed datastrutures and algorithms in CMT

## Training Online Contextual Memory Tree on ALOI:
```bash
python aloi_script_progerror.py
```

## Training Offline Contextual Memory Tree on ALOI:
```bash
python aloi_script.py
```

