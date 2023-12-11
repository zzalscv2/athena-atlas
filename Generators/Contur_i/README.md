**Contacts: [Peng Wang](peng.w@cern.ch) and [Jonathan Butterworth](j.butterworth@cern.ch)**

Contur webpage can be found [here](https://hepcedar.gitlab.io/contur-webpage/introduction.html)

Main Contur gitlab page can be found [here](https://gitlab.com/hepcedar/contur/-/tree/main)

# Contur

Contur is a package designed to put constraints on theories Beyond the Standard Model using existing RIVET measurements.

# Setup Contur
The ```find_module``` for contur is not there until Athena 24.0.14. Generally, it should work with the most recent version.
```bash
setupATLAS
asetup main,latest,Athena
source setupContur
```
Running ```setupContur``` will also setup the environent for Rivet.

For the first time running contur, it is necessary to create your own analysis database in a ```$CONTUR_USER_DIR```. 

```bash
cd $CONTUR_DATA_PATH
make
```
This will create a ```contur_user``` directory inside your home directory. To check if the right files are generated inside the user directory:
```bash
cd ~/contur_user
ls
```
It should contain a list of ```.ana``` files, one for each beam energy and one for each analysis pool, together with ```models.db```, ```Rivet-ConturOverload.so``` and a directory called ```SM```.

_Note that there is an issue with the LCG installation of ```Rivet 3.1.8```, where the ```$RIVET_ANALYSIS_PATH``` is not correctly set. Therefore with athena versions directed to ```Rivet 3.1.8```, an additional step is needed before running ```make```:_
```bash
export RIVET_ANALYSIS_PATH=$RIVET_PATH/lib/Rivet:$RIVET_PATH/share/Rivet
```

_Also note that this ```make``` step only needs to be run for the very first time using Contur (unless a different version of Contur is used)._

_Once the ```contur_user``` directory is created, steps up to ```source setupContur``` would be enough if the same version of Contur is used._ 

_If a different version of Contur is used then the analysis database will need to be updated by re-running the ```make``` steps._

# Running Contur
## Locally run Contur on a Yoda file
There are two steps needed to run Contur in this condition.
1. After Contur environment has been successfully setup, the process can be initiated via a simple command
    ```bash
    contur your_yoda_file.yoda
    ```
    The output for this will be a directory with a default name ```ANALYSIS```. The directory contains a series of ```.dat``` files and a summary file (```Summary.txt```), which will tell you the overall combined exclusion for the model using all the Rivet analyses in the current setup.

    For more options in contur, run:
    ```bash
    contur --help
    ```

2. Then is to generate histograms based on the ```.dat``` files.
    
    For Contur version 2.4.4 or older, this can be done by running
    ```bash
    contur-mkhtml
    ```
    This command has to be called in the same directory where the ```contur``` command is compiled.

## Running Contur on Rivet-on-the-fly GRID outputs

Rivet jobs on GRID usually don't result in a single ```.yoda``` file as output. This is true for large sample sizes (typically larger than 10k events) going through the central production system, or sending Rivet jobs to GRID based on an EVNT container via [PMGSystematicsTools](https://gitlab.cern.ch/atlas/athena/-/tree/main/Generators/PMGSystematicsTools).

Therefore it is necessary to unpack the tarballs, and safely merge all the ```.yoda``` bits together before performing contur on the result.

There is a script called ```contur_unpack.py``` written for this purpose. Which will do all the unpacking + merging + contur together.

To run this, first download your output directory from GRID and run:
```bash
cd <downloaded_tarballs_location>
contur_unpack
```
or 
```bash
contur_unpack -d <path_to_output_dir>
```

Sometimes merged yoda file will need to be scaled by their cross-section, generator efficiency and K-factor (if available). If the original merged file has cross-section 1pb, warning messages for rescaling will be printed unless ```--dsid``` or ```--xsec``` options are called.

```--dsid``` will take in the Job id and search through the database file to grab the relevant quantities and rescale the original histograms by this factor. ```--xsec``` option does similar jobs but in this way the rescaling factor is entered directly by user.

Once the above steps are finished, repeat step 2 in running contur on a single yoda file section to get the exclusions.

## Analysis using 13.6TeV Samples

If using 13TeV rivet measurements to analyse 13.6TeV samples, the option ```--ignore-beams``` will need to be called via Rivet, or if JO script is used, include ```rivet.IgnoreBeamCheck = True``` in the script.

This option will be soon added to the on-the-fly generation and this page will be updated once this is available. 
