# common fragment for xAODTau filter
# conversion to XAOD, 
# creation of slimmed container containing taus
# connecting the filter

include ("GeneratorFilters/CreatexAODSlimContainers.py")
createxAODSlimmedContainer("TruthTaus",prefiltSeq)
prefiltSeq.xAODCnv.AODContainerName = 'GEN_EVENT'

from GeneratorFilters.GeneratorFiltersConf import xAODTauFilter
xAODTauFilter = xAODTauFilter("xAODTauFilter")  
filtSeq += xAODTauFilter

# to modify tau selection pT and eta cuts (on the Slimmer level) put into JOs eg:
#prefiltSeq.xAODTruthParticleSlimmerTau.tau_pt_selection = 1000.0
#prefiltSeq.xAODTruthParticleSlimmerTau.abseta_selection = 4.5

# to modify filter cuts put into JOs sth. like:
#filtSeq.xAODTauFilter.Ntaus = 2
#filtSeq.xAODTauFilter.EtaMaxe = 2.7
#filtSeq.xAODTauFilter.EtaMaxmu = 2.7
#filtSeq.xAODTauFilter.EtaMaxhad = 2.7 # no hadronic tau decays
#filtSeq.xAODTauFilter.Ptcute = 12000.0
#filtSeq.xAODTauFilter.Ptcutmu = 12000.0


