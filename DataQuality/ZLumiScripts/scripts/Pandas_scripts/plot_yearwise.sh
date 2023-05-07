#!/bin/bash

indir="/eos/atlas/atlascerngroupdisk/perf-lumi/Zcounting/Run3/CSVOutputs/HighMu/"
outdir="/eos/user/s/salibocu/run3/Plots/FullRun3/"

for year in 22
do
    # Yearwise L(ee) / L(mumu) comparison vs. time and pileup
    python -u plotting/yearwise_luminosity.py --year $year --comp --indir $indir --outdir $outdir
    python -u plotting/yearwise_luminosity_vs_mu.py --year $year --comp --indir $indir --outdir $outdir
    for channel in Zee Zmumu Zll
    do
        # Yearwise L_Z / L_ATLAS comparison vs. time and pileup
        python -u plotting/yearwise_luminosity.py --channel $channel --year $year --indir $indir --outdir $outdir
        python -u plotting/yearwise_luminosity.py --channel $channel --year $year --absolute --indir $indir --outdir $outdir
        python -u plotting/yearwise_luminosity_vs_mu.py --channel $channel --year $year --indir $indir --outdir $outdir
    done
done

