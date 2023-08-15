# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from InDetTrigRecExample.InDetTrigCommonTools import CAtoLegacyPrivateToolWrapper

# ------load association tool from Inner Detector to handle pixel ganged ambiguities
from InDetConfig.InDetAssociationToolsConfig import TrigPrdAssociationToolCfg
InDetTrigPrdAssociationTool = CAtoLegacyPrivateToolWrapper(TrigPrdAssociationToolCfg)

