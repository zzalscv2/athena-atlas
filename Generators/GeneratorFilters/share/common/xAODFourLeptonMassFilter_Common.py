# common fragment for xAODDiLeptonMass filter
# conversion to XAOD, 
# creation of slimmed container containing electrons
# connecting the filter

include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthLightLeptons",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

from GeneratorFilters.GeneratorFiltersConf import xAODFourLeptonMassFilter
xAODFourLeptonMassFilter = xAODFourLeptonMassFilter("xAODFourLeptonMassFilter")
filtSeq += xAODFourLeptonMassFilter

# to modify cuts put into JOs e.g.:
# filtSeq.xAODFourLeptonMassFilter.MinPt = 5000.0
# filtSeq.xAODFourLeptonMassFilter.MaxEta = 2.7
# filtSeq.xAODFourLeptonMassFilter.MinMass1 = 60000
# filtSeq.xAODFourLeptonMassFilter.MaxMass1 = 14000000
# filtSeq.xAODFourLeptonMassFilter.MinMass1 = 12000
# filtSeq.xAODFourLeptonMassFilter.MaxMass1 = 14000000
# filtSeq.xAODFourLeptonMassFilter.AllowElecMu =  True
# filtSeq.xAODFourLeptonMassFilter.AllowSameCharge = True




