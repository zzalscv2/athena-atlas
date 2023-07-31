SUSYTools
====================================
**Current Main Developers**:   
sara.alderweireldt@cern.ch,  
claudia.merlassino@cern.ch,  
marco.rimoldi@cern.ch,
OR atlas-phys-susy-bgforum-conveners@cern.ch    
  
Note that the newest recommendations are always on the [background forum TWiki page](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/BackgroundStudies).  Bugs can be reported on [JIRA](https://its.cern.ch/jira/projects/ATLSUSYBGF).  In case you are starting out, here are some useful tutorials for [GIT](https://twiki.cern.ch/twiki/bin/view/AtlasComputing/GitTutorial) and [CMake](https://twiki.cern.ch/twiki/bin/view/AtlasComputing/CMakeTestProjectInstructions) as they are used in ATLAS.

------------------------------------
Recommended tags
------------------------------------

The recommended tags are on the [background forum TWiki page](https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/BackgroundStudies).

------------------------------------
AnalysisBase / AthAnalysisBase Setup
------------------------------------
Set up the latest recommended AnalysisBase release:
```bash
setupATLAS
lsetup git
asetup "AnalysisBase,24.2.2"
```

Or the latest AthAnalysis release::

```bash
setupATLAS
asetup "AthAnalysis,24.2.2"
```
  
For working with code, a sparse checkout is pretty straightforward.  
(In order for this to work, you need your own fork of the athena project, see the [ATLAS git tutorial](https://atlassoftwaredocs.web.cern.ch/gittutorial/gitlab-fork/))  

```bash
git atlas init-workdir https://:@gitlab.cern.ch:8443/atlas/athena.git
```

And then the version of SUSYTools in the release can be checked out via:  
```bash
cd athena
git atlas addpkg SUSYTools
git checkout -b mybranchname upstream/main
```

Then, to compile:  
```bash
cd ..
mkdir build
cd build
cmake ../athena/Projects/WorkDir
make
source x86_64-centos7-gcc11-opt/setup.sh
```

Additional packages needed on top of the recommended release are documented in `doc/packages.txt`; for now you can add those to your work area via git atlas addpkg.  Afterwards, be sure to recompile everything:  
```bash   
cmake ../athena/Projects/WorkDir
make -j
source x86_64-centos7-gcc11-opt/setup.sh
```

and you are ready to go!

------------------------------------
Testing
------------------------------------
Unit tests now use [CTest](https://cmake.org/Wiki/CMake/Testing_With_CTest).  To run unit tests, simply **go to your build area** and type:

```bash
gmake test
```

More complex testing is best done directly with the `ctest` command.  To see what tests are available:  
```bash
ctest -N
```

To run specific tests (or tests for specific packages), use the `-R` option to restrict running to tests with a string in the name (e.g. `-R SUSYTools`) or `-E` to exclude certain tests (e.g. `-E SUSYToolsTester`).  To get logs / verbose output, use `-V`.

To test locally in an AnalysisBase release, get your favourite benchmark sample (e.g. `mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.deriv.DAOD_PHYS.e6337_s3681_r13145_p5057/` or `data18_13TeV.00358031.physics_Main.deriv.DAOD_PHYS.r13286_p4910_p5057`), and run:

```bash
SUSYToolsTester <myAOD.pool.root> maxEvents=100 isData=0 isAtlfast=0 Debug=0 NoSyst=0 2>&1 | tee log
```

The standard `SUSYToolsTester` code is meant to test the configuration of all the relevant CP tools and apply a minimal analysis flow. It includes the recalibration and correction of all physics objects, the recalculation of the MissingET, the extraction of SFs and other specific functionalities that are enabled based on the DxAOD stream. All systematic variations available in the central registry are tested by default, and a cutflow is displayed for each of them. This can be disabled by setting `NoSyst=1`.

To test locally in an AthAnalysis release, run the test job options as::

```bash
cd ..
athena.py SUSYTools/minimalExampleJobOptions_mc.py
```

which is the athena-friendly equivalent of the `SUSYToolsTester` code above for running on MC.  You can also change "mc" to "data" or "atlfast" to run on data or fast simulation if you would prefer.

------------------------------------------------------------------------
For expert only: Update the SUSYTools reference files for the CI unit test.
------------------------------------------------------------------------

1) Contact atlas-phys-susy-bgforum-conveners@cern.ch
2) If they agree, proceed with:
```bash
cd src/athena
git atlas addpkg SUSYTools (or any other additional packages)
cd ../../build
cmake ../athena/Projects/WorkDir
make -j
source x86_64-centos7-gcc11-opt/setup.sh
ctest 
```
or
```bash
ctest -R SUSYTools_ut_SUSYToolsTester_data_ctest
ctest -R SUSYTools_ut_SUSYToolsTester_data_Run3_ctest
ctest -R SUSYTools_ut_SUSYToolsTester_mc_ctest
ctest -R SUSYTools_ut_SUSYToolsTester_mc_Run3_ctest
ctest -R SUSYTools_ut_SUSYToolsTester_mc23a_Run3_ctest

```
3) Before copying the files, check that all the changes are expected.
```bash
cd $WorkDir_DIR/../PhysicsAnalysis/SUSYPhys/SUSYTools/CMakeFiles/unitTestRun/
cp ut_SUSYToolsTester_data.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsTester_data.ref
cp ut_SUSYToolsTester_data_Run3.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsTester_data_Run3.ref
cp ut_SUSYToolsTester_mc.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsTester_mc.ref
cp ut_SUSYToolsTester_mc_Run3.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsTester_mc_Run3.ref
cp ut_SUSYToolsTester_mc23a_Run3.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsTester_mc23a_Run3.ref
```
Or if you are using AthAnalysis these are the current test:
```bash
ctest -R SUSYTools_ut_SUSYToolsAlg_data_ctest
ctest -R SUSYTools_ut_SUSYToolsAlg_data_Run3_ctest
ctest -R SUSYTools_ut_SUSYToolsAlg_mc_ctest
ctest -R SUSYTools_ut_SUSYToolsAlg_mc_Run3_ctest
```
3) Before copying the files, check that all the changes are expected.
```bash
cd $WorkDir_DIR/../PhysicsAnalysis/SUSYPhys/SUSYTools/CMakeFiles/unitTestRun/
cp ut_SUSYToolsAlg_data.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsAlg_data.ref
cp ut_SUSYToolsAlg_data_Run3.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsAlg_data_Run3.ref
cp ut_SUSYToolsAlg_mc.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsAlg_mc.ref
cp ut_SUSYToolsAlg_mc_Run3.log-todiff $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/ut_SUSYToolsAlg_mc_Run3.ref
```
4) Add the new reference file on git:
```bash
cd $WorkDir_DIR/../../src/athena/PhysicsAnalysis/SUSYPhys/SUSYTools/share/
git add ut_SUSYToolsTester*
```
Please tag the SUSY bgforum conveners in the athena MR.
