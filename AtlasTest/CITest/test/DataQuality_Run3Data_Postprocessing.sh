#!/bin/bash

echo ../RecoRun3Data/run_q449/myHIST.root >> tomerge.txt && \
DQ_POSTPROCESS_ERROR_ON_FAILURE=1 DQHistogramMerge tomerge.txt merged.root 1