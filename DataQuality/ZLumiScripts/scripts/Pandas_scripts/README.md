# Introduction
This framework runs over the merged outputs produced by running the DQTGlobalWZFinder algorithm over primary AODs. Information on how to run the main code can be found here [https://twiki.cern.ch/twiki/bin/viewauth/Atlas/ZCountingLumi](https://twiki.cern.ch/twiki/bin/viewauth/Atlas/ZCountingLumi).
Once the merging step has been performed (an example of which is below), the dqt_zlumi_pandas.py script can be ran over the merged output to produce a single csv file containing all information. 
Each row of the csv file corresponds to a single luminosity block, and contains all information for both channels; such as the number of reconstructed Zs per channel, trigger efficiency, reconstruction efficiency, luminosity, as well as the arithmeric mean of the Zee and Zmumu luminosities and all axuilliary official information (livetime, pileup, luminosity, GRL).
```
ls <grid_output>/* > tomerge.txt
DQHistogramMerge.py tomerge.txt tree_<run_number>.root
```
i.e. *tree_430580.root is used in the following example*

#Running the code
Using a single 2022 run as an illistrative example:
```
source setup.sh
infile="tree_430580.root"
grl="/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/GoodRunLists/data22_13p6TeV/20220902/data22_13p6TeV.periodF_DetStatus-v108-pro28_MERGED_PHYS_StandardGRL_All_Good_25ns.xml"
outdir="<your_output_directory>"

python -u dqt_zlumi_pandas.py --dblivetime --useofficial --campaign mc21 --infile $infile --grl $grl --outdir $outdir
```

#Making single run plots
*Note*: The output directory (outdir) will need to be changed inside plotting/efficiency.py and plotting/luminiosity.py. Both of these scripts calculate an average over successive bunches of 20 luminosity blocks to increase statistical precision. All other plots use the single-LB luminosity, and not the 20 LB merged value, when calculating the integrated luminosity of an LHC fill/pileup bin.
```
# Time dependent efficiency and luminosity plots
python plotting/efficiency.py --infile <input_csv_file>
python plotting/luminosity.py --infile <input_csv_file>

# Pileup dependent efficiency and luminosity plots
python plotting/efficiency.py --usemu --infile <input_csv_file>
python plotting/luminosity.py --usemu --infile <input_csv_file>

# Kinematic plots - this plotting script takes the histograms in the merged root file rather than the produced csv
python plotting/plot_kinematics.py --infile $infile
```

# Running over the entire dataset
_Note_: The input (indir) and output (out_dir) directories will need to be changed at the top of the script run_code.sh.
```
submit=local # set to local to run all jobs locally in a loop
for year in 22
do
    ./run_code.sh $year $submit
done
```

# Making plots for the entire Run 3 dataset
_Note_: The input (indir) and output (outdir) directories will need to be changed in both scripts.

```
for year in 22
do
    # Yearwise L(ee) / L(mumu) comparison vs. time and pileup
    python -u plotting/yearwise_luminosity.py --year $year --comp
    python -u plotting/yearwise_luminosity_vs_mu.py --year $year --comp
    for channel in Zee Zmumu Zll
    do
        # Yearwise L_Z / L_ATLAS comparison vs. time and pileup
        python -u plotting/yearwise_luminosity.py --channel $channel --year $year
        python -u plotting/yearwise_luminosity_vs_mu.py --channel $channel --year $year
    done
done

```
This can also be done by running:
```
./plot_yearwise.sh
```
_Note_: The input (indir) and output (outdir) directories will need to be changed in this script.

# Making separate sets of plots for all runs in one directory at once
__Note_: The input (indir) and output (outdir) directories will need to be changed in this script.

```
./plot_all.sh
```
Loops over all .csv files in a given directory and plots efficiency and luminosity against LB and pileup.