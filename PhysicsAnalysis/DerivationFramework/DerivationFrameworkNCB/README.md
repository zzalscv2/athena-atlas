# DerivationFrameworkNCB

This package contains the configuration files for the Non-Collision Background (NCB) derivations used for cosmic rays and Beam Induced Background (BIB) studies.

The package contains the following files:

* `NCB1.py`: the configuration of the `NCB1` data type. As well as setting up the driving algorithm ("kernel") for the jobs, these files mainly configure directly the slimming (branch-wise content) but not the common physics content which is configured elsewhere (see below).    
* `NCBCommonConfig.py`: the configuration of the common physics content. This file pulls in the config fragments from all of the reconstruction domains plus truth and trigger matching (in the case of upgrade samples). 

For more details please refer to [the manual](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/DerivationFramework)

