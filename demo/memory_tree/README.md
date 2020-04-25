Contextual Memory Tree (CMT)
===============================

This demo exercises CMT for applications of logarithmic time 
multiclass classification (online and offline), and logarithmic time multilabel classification.


The datasets for multiclass classification used are [ALOI](http://aloi.science.uva.nl/) and WikiPara. ALOI
has 1000 classes, and each class has in average 100 training examples. WikiPara
contains 10000 classes. We consider two versions of WikiPara here: 1-shot version which 
contains 1 training example per class, and 2-shot version which contains 2 training examples per class. 

The datasets for multilabel classification used are RCV1-2K, AmazonCat-13K, and Wiki10-31K from the XML [repository](http://manikvarma.org/downloads/XC/XMLRepository.html).

We refer users to the [manuscript](https://arxiv.org/pdf/1807.06473.pdf) for detailed datastrutures and algorithms in CMT

## Dependency:
python 3

## Training Online Contextual Memory Tree on ALOI and WikiPara:
```bash
python aloi_script_progerror.py
python wikipara10000_script_progerror.py
```

## Training Offline Contextual Memory Tree on ALOI, WikiPara, RCV1-2K, AmazonCat-13K and Wiki10-31K:
```bash
python aloi_script.py
python wikipara10000_script.py
python xml_rcv1x.script.py
python xml_amazoncat_13K_script.py
python xml_wiki10.script.py
```

