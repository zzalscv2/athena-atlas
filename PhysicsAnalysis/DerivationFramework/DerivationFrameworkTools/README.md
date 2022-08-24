# DerivationFrameworkTools

This package contains a number of tools used performing common operations whilst building DAODs. For more information please refer to the [derivation framework manual](https://twiki.cern.ch/twiki/bin/view/AtlasProtected/DerivationFramework)

* `AsgSelectionToolWrapper`: IAugmentationTool that wraps ASG selection tools, allowing them to be called from DerivationKernels and their decisions to be recorded as object decorations
* `xAODStringSkimmingTool`: ISkimmingTool for performing skimming on the basis of ExpressionEvaluation strings. Works for all xAOD objects and triggers except certain special triggers with unusual names (see next item)
* `TriggerSkimmingTool`: special tool for handling skimming by triggers whose names can't be handled by the ExpressionEvaluation (dots, dashes etc)
* `FilterCombinationAND/OR`: ISkimmingTool allowing decisions of other ISkimmingTools to be combined. The FilterCombination tool is then passed to the kernel instead of the individual skimming tools.
* `PrescaleTool`: ISkimmingTool that rejects every event except the n-th one
* `DeltaRTool`: IAugmentationTool that calculates DeltaR between pairs of xAOD objects and records the results for use downstream
* `GenericObjectThinning`: IThinningTool for applying thinning decisions to given xAOD objects, with the thinning criteria being defined by ExpressionEvaluation strings
* `InvariantMassTool`: IAugmentationTool that calculates the invariant masses of pairs groups of xAOD objects and records the results for use downstream
* `NTUPStringSkimmingTool`: allows ExpressionEvaluation skimming where the input is NTUP rather than xAOD. Generally obsolete but might be useful one day...

