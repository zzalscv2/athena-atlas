
# conversion to XAOD, 
# connecting the filter

include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthLightLeptons",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'


from GeneratorFilters.GeneratorFiltersConf import xAODJetFilter
xAODJetFilter = xAODJetFilter("xAODJetFilter")
filtSeq += xAODJetFilter