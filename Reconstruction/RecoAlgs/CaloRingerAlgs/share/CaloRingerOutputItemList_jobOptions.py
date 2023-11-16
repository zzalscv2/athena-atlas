# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration


from CaloRingerAlgs.CaloRingerKeys import (
    CaloRingerKeysDict,
)

from RecExConfig.RecFlags import rec
__doc__ = "Add containers to ESD/AOD ItemList using the definitions from CaloRingerKeys"

from AthenaCommon.Logging import logging

mlog = logging.getLogger('CaloRingerOutputItemList_jobOptions.py')
mlog.info('Entering')


class IncludeError (ImportError):
    pass

# Avoid duplication
caloRingerAODList = []

# Add itens into lists
from AthenaConfiguration.AllConfigFlags import ConfigFlags
from AthenaCommon.Configurable import ConfigurableCABehavior
from CaloRingerAlgs.CaloRingerAlgsConfig import CaloRingerOutputCfg

caloRingerFlags = ConfigFlags.clone()
caloRingerFlags.Output.doWriteESD = True
caloRingerFlags.Output.doWriteAOD = True
caloRingerFlags.lock()
with ConfigurableCABehavior():
    toOutput = CaloRingerOutputCfg(caloRingerFlags)

caloRingerESDList = list(toOutput.getEventAlgo('OutputStreamESD').ItemList)
caloRingerAODList = list(toOutput.getEventAlgo('OutputStreamAOD').ItemList)

toOutput.wasMerged()
