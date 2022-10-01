# common fragment for xAODMultiLepton filter
# conversion to XAOD, 
# creation of slimmed container containing electrons and muons
# connecting the filter

include ("GeneratorFilters/CreatexAODSlimContainers.py")

from GeneratorFilters.GeneratorFiltersConf import xAODMultiElectronFilter
xAODMultiElectronFilter = xAODMultiElectronFilter("xAODMultiElectronFilter")
filtSeq += xAODMultiElectronFilter

# to modiify cuts put into JOs e.g.:
#filtSeq.xAODMultiElectronFilter.MinPt = 12000.0
#filtSeq.xAODMultiElectronFilter.MaxEta = 10.0
#filtSeq.xAODMultiElectronFilter.MinVisPtHadTau = 10000
#filtSeq.xAODMultiElectronFilter.NLeptons = 4
#filtSeq.xAODMultiElectronFilter.IncludeHadTaus = True
#filtSeq.xAODMultiElectronFilter.TwoSameSignLightLeptonsOneHadTau = False 

