if not hasattr(filtSeq, "xAODParticleFilter"):
    from GeneratorFilters.GeneratorFiltersConf import xAODParticleFilter
    filtSeq += xAODParticleFilter("xAODParticleFilter")

# Example usage of this filter
#xAODParticleFilter = filtSeq.xAODParticleFilter
#xAODParticleFilter.Ptcut = 0.0
#xAODParticleFilter.Etacut = 10.0
#xAODParticleFilter.PDG = 1000022
#xAODParticleFilter.MinParts = 2
#xAODParticleFilter.StatusReq = 11
