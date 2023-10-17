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
from CaloRingerAlgs.CaloRingerAlgsConfig import caloRingerOutputList

toOuput = caloRingerOutputList(ConfigFlags)

caloRingerESDList = toOuput
caloRingerAODList = toOuput
