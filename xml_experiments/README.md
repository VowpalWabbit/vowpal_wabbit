# Results of PLTs on datasets from [The Extreme Classification Repository](http://manikvarma.org/downloads/XC/XMLRepository.html)

| Dataset | P@1 | P@2 | P@3 | P@4 | P@5 |
| :-- | :-- | :-- | :-- | :-- | :-- |
| Bibtex | 0.622664 | 0.465408 | 0.377469 | 0.318489 | 0.276262 |
| Mediamill | 0.835605 | 0.772263 | 0.662537 | 0.575596 | 0.509587 |
| Delicious | 0.662480 | 0.634851 | 0.605965 | 0.583438 | 0.560063 |
| RCV1 2K | 0.905041 | 0.775150 | 0.725098 | 0.597501 | 0.516137 |
| EURLex 4K | 0.736414 | 0.663823 | 0.602695 | 0.547125 | 0.498714 |
| AmazonCat 13K | 0.914640 | 0.838762 | 0.760044 | 0.686985 | 0.614001 |
| AmazonCat 14K | 0.837557 | 0.712590 | 0.644535 | 0.575310 | 0.504690 |
| Wiki10 31K | 0.841445 | 0.784160 | 0.728839 | 0.678809 | 0.633464 |
| Delicious 200K | 0.457465 | 0.420665 | 0.393446 | 0.375079 | 0.360551 |
| WikiLSHTC 325K | 0.416196 | 0.322053 | 0.267770 | 0.230858 | 0.203816 |
| Amazon 670K | 0.368639 | 0.344385 | 0.324777 | 0.307536 | 0.291542 |

Use a `run_<dataset name>.sh` script to reproduce the results for a given dataset (run the script from this directory).
The scripts will download and process the datasets using scripts from the [datasets4vw](https://github.com/mwydmuch/datasets4vw) repository.
