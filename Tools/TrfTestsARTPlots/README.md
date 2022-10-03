TrfTestsARTPlots
================

Create references:

* Produce `DAOD_PHYSLITE.art.pool.root`:
```
asetup Athena,master,latest
test_trf_q445_phys_physlite_mt_mp.sh > out.txt 2>&1 &
```
* Produce reference histograms `hist_physlite_2305.root`:
```
xAODHist.py --analysis --outputHISTFile hist_physlite_2305.root DAOD_PHYSLITE.art.pool.root
```
* Produce XML file `dcube_config_hist_physlite_2305.xml`:
```
lsetup dcube
$ATLAS_LOCAL_ROOT/dcube/current/DCubeClient/python/dcube.py -g -c dcube_config_hist_physlite_2305.xml -r hist_physlite_2305.root
```
* Copy references to EOS at `/eos/atlas/atlascerngroupdisk/data-art/grid-input/TrfTestsART/dcube`:
```
mkdir -p /eos/atlas/atlascerngroupdisk/data-art/grid-input/TrfTestsART/dcube/q445/v0
cp hist_physlite_2305.root /eos/atlas/atlascerngroupdisk/data-art/grid-input/TrfTestsART/dcube/q445/v0/
cp dcube_config_hist_physlite_2305.xml /eos/atlas/atlascerngroupdisk/data-art/grid-input/TrfTestsART/dcube/q445/v0/
```

Update in `test_trf_q445_phys_physlite_mt_mp.sh` the variables `dcubeRef` and `dcubeXML`.

When the EOS references files have been copied from EOS to CVMFS at `/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrfTestsART/dcube/` by the automatic cron job rerun `test_trf_q445_phys_physlite_mt_mp.sh`
