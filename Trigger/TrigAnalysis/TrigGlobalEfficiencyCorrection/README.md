## Usage
See the instructions of the [TrigGlobalEfficiencyCorrectionTool](https://twiki.cern.ch/twiki/bin/viewauth/Atlas/TrigGlobalEfficiencyCorrectionTool) twiki page (ATLAS internal). 

Note that the tool interface header `ITrigGlobalEfficiencyCorrectionTool.h` is hosted in another package, `PhysicsAnalysis/Interfaces/TriggerAnalysisInterfaces`, so usually in order to use this tool you only need to link to the `TriggerAnalysisInterfaces` library in your CMakeLists.txt file. 


## Formulas used to calculate the combined efficiencies
They are documented in this package in [doc/formulas.pdf](doc/formulas.pdf). 


## Examples of configuration of the tool

Several examples are provided, corresponding to the source files `util/TrigGlobEffCorrExample*.cxx` (dual-use (Ath)AnalysisBase executables). To run them:
```
TrigGlobEffCorrExample0 [--debug] <input DxAOD file>.root
```

Illustrated features:
- Example 0: minimal configuration
- Example 1: singe+dilepton triggers combination
- Example 2: [removed]
- Examples 3a-3e: usage of lepton selection tags
- Example 4: usage of the static helper method suggestElectronMapKeys()
- Example 5a: photon triggers, simplest example (symmetric diphoton trigger)
- Example 5b: photon triggers, more complex (asymmetric diphoton trigger)
- Example 06: trigger matching
More details can be found in the comments of each source file. 
