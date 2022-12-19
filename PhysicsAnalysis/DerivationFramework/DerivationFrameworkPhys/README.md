# DerivationFrameworkPhys

This package contains the configuration files for the unskimmed `PHYS` and `PHYSLITE` data types which are to be used in the majority of ATLAS analyses for run 3. Additionally the common configuration used in these two data types may alse be used as the basis for other ("residual") data types. Since these files impact a large number of physicists they should not be adjusted without prior consultation in the analysis model group (AMG).

The package contains the following files:

* `PHYS.py` and `PHYSLITE.py`: the configuration of the `PHYS` and `PHYSLITE` data types themselves. As well as setting up the driving algorithm ("kernel") for the jobs, these files mainly configure directly the slimming (branch-wise content) but not the common physics content which is configured elsewhere (see below).    
* `PhysCommonConfig.py`: the configuration of the common physics content, shared by `PHYS`, `PHYSLITE` and any residual DAOD data types that need it. This file pulls in the config fragments from all of the reconstruction domains plus truth and trigger matching (in the case of upgrade samples). 
* `PhysCommonThinningConfig.py`: configures the thinning (object removal) tools settings for the `PHYS` and `PHYSLITE` formats. The two data types share the majority of the settings, but `PHYSLITE` has a few extra in addition. The lists of tools to be used are defined in the individual config files for the two formats. 
* `TriggerMatchingCommonConfig.py`: configures the trigger matching for DAOD content for the run 2 trigger EDM. In run 3 the analysis-level trigger navigation is small enough to be added so the matching is done by the physicists directly, and this file isn't required.

The other files pertain to the legacy configuration and can be ignored. They will be deleted during early 2023.

For more details please refer to [the manual](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/DerivationFramework)

