# Flatbuffer RunTests Data Generation

Changes to the FB schema - particularly breaking ones, can easily lead to broken tests with silent, difficult-to-debug failures, because the stored input files use the old version of the schema. After this change becomes part of mainline VW, schema evolution will need to be carefully controlled, but if this PR gets put on hold for a significant time, regenerating the data may prove difficult without a record.

## General Approach

Given a command-line in VW, add `--fb_out <target_file>` and run via `"<build>/utl/flatbuffer/to_flatbuff"`

## Existing data files

| Test ID | --fb_out | Generation Args |
|---------|--------------|------------------------|
| 239 | train-sets/0001.fb | `-d train-sets/0001.fb` |
| 240 | train-sets/rcv1_raw_cb_small.df | `--cb_force_legacy --cb 2 --examples 500` |
| 241 | train-sets/multilabel.fb | `-d multilabel --multilabel_oaa 10` |
| 242 | train-sets/multiclass.fb | `-d multiclass -k --ect 10` |
| 243 | train-sets/cs.fb | `-d cs_test.ldf --invariant --csoaa_ldf multiline` |
| 244 | train-sets/rcv1_cb_eval.fb | `-d rcv1_cb_eval  --cb 2 --eval --examples 500` |
| 245 | train-sets/wiki256_no_label.fb  | `-d wiki256.dat --lda 100 --lda_alpha 0.01 --lda_rho 0.01 --lda_D 1000 -l 1 -b 13 --minibatch 128 -k` |
| 246 | train-sets/ccb.fb | `-d ccb_test.dat --ccb_explore_adf` |

