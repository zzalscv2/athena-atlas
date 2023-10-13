#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging
log = logging.getLogger('decodeBS_TLA_AOD.py')


# Configure AOD output
def outputCfg(flags):
    """ Configure AOD output """
    acc = ComponentAccumulator()

    from TrigEDMConfig.TriggerEDM import getTriggerEDMList
    edmList = getTriggerEDMList(flags.Trigger.ESDEDMSet, flags.Trigger.EDMVersion)

    ItemList = []
    for edmType, edmKeys in edmList.items():
        for key in edmKeys:
            ItemList.append(edmType+'#'+key)
    ItemList += [ "xAOD::EventInfo#EventInfo", "xAOD::EventAuxInfo#EventInfoAux." ]
    ItemList += [ 'xAOD::TrigCompositeContainer#*' ]
    ItemList += [ 'xAOD::TrigCompositeAuxContainer#*' ]
    ItemList += [ 'xAOD::TrigDecision#*' ]
    ItemList += [ 'xAOD::TrigDecisionAuxInfo#*']

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc.merge(OutputStreamCfg(flags, "AOD", ItemList=ItemList))
    

    return acc
