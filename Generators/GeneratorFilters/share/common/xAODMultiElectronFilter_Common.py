# common fragment for xAODMultiElectron filter
# conversion to XAOD, 
# connecting the filter

include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthElectrons",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

from GeneratorFilters.GeneratorFiltersConf import xAODMultiElectronFilter
xAODMultiElectronFilter = xAODMultiElectronFilter("xAODMultiElectronFilter")  
filtSeq += xAODMultiElectronFilter

# to modify cuts put into JOs e.g.:
#filtSeq.xAODMultiElectronFilter.Ptcut = 12000.0
#filtSeq.xAODMultiElectronFilter.Etacut = 10.0
#filtSeq.xAODMultiElectronFilter.NElectrons = 2
