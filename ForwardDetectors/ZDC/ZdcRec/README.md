This package contains ZDC reco algorithms and tools to be used in ATHENA.


# Standalone transform
The ZdcRecConfig can be used as as standalone script/transform like this:
```
python3 -m ZdcRec.ZdcRecConfig --evtMax=100 --filesInput raw_data_file
```
It generates AOD file. 
Specific functionalities for calibration purposes can be enabled with flags e.g.:
```
python3 -m ZdcRec.ZdcRecConfig --evtMax=100 --filesInput raw_data_file   DoCalib=True
```
All other flags can be altered to modify the job further e.g. output AOD file name:
```
python3 -m ZdcRec.ZdcRecConfig --evtMax=100 --filesInput raw_data_file   DoCalib=True Output.AODFileName=test.AOD.pool.root
```
