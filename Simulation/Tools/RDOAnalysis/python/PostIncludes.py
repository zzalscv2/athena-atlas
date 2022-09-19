# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

#Adding RDOAnalysis for whichever parts of ITk are running
def ITkRDOAnalysis(configFlags):
    
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

    result = ComponentAccumulator()

    if configFlags.Detector.EnableITkStrip:
        from RDOAnalysis.RDOAnalysisConfig import ITkStripRDOAnalysisCfg
        result.merge(ITkStripRDOAnalysisCfg(configFlags))
    
    if configFlags.Detector.EnableITkPixel:
        from RDOAnalysis.RDOAnalysisConfig import ITkPixelRDOAnalysisCfg
        result.merge(ITkPixelRDOAnalysisCfg(configFlags))

    if configFlags.Detector.EnablePLR:
        from RDOAnalysis.RDOAnalysisConfig import PLR_RDOAnalysisCfg
        result.merge(PLR_RDOAnalysisCfg(configFlags))

    return result
