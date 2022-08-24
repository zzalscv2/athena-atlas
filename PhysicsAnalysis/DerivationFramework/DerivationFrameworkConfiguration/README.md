# DerivationFrameworkConfiguration

This package contains the python scripts and methods for configuration of the derivation framework using the Component Accumulator (CA). It consists of the following files:

* `scripts/Derivation_tf.py`: definition of the CA-compatible derivation framework job transformation. This script contains very little code - most of the "action" takes place either in the files listed below, or in the job transforms machinery
* `python/DerivationTransformHelpers.py`: sets up the command line options needed by the DAOD job transform above, and defines the relevant substep and executors for the job transforms mechanism. Note that the executors used are defined elsewhere in the ATLAS software - not in this or other derivation framework packages. Note also that a different executor is used depending on whether the output is to be DAOD (the usual use-case) or physics validation n-tuples.
* `python/DerivationSkeleton.py`: this is the main script controlling the workflow of the derivation framework. It establishes the input and output formats and then merges all of the various components needed for DAOD production into the component accumulator, which it then executes. Specifically, it merges (in this order) the components for reading and writing POOL files, all components from the individual DAOD definitions, and performance monitoring. It also provides the means to disable skimming ("pass through mode") and to apply pre/post exec/includes.
* `python/PhysicsValidationSkeleton.py`: as above, but instead defines the workflow for the production of physics validation n-tuples
* `python/DerivationConfigList.py`: this maps the format argument provided by the user when running the job transform (`--formats`) to the config scripts, which in turn return a component accumulator to the `DerivationSkeleton`. Consequently all DAOD formats must be listed here.

For more information please refer to the [derivation framework manual](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/DerivationFramework)
