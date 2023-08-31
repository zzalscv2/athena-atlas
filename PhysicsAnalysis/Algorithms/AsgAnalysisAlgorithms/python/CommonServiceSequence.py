# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnaAlgorithm.DualUseConfig import createService

def makeCommonServiceSequence (algSeq, runSystematics=True, ca=None) :
    """make the common services for the CP algorithms"""

    # Set up the systematics loader/handler service:
    sysService = createService( 'CP::SystematicsSvc', 'SystematicsSvc', sequence = algSeq )
    if runSystematics :
        sysService.sigmaRecommended = 1
    selectionSvc = createService( 'CP::SelectionNameSvc', 'SelectionNameSvc', sequence = algSeq )
    if ca is not None :
        ca.addService( sysService )
        ca.addService( selectionSvc )
