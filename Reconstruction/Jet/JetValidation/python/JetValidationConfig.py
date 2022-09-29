#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

'''@file JetValidationConfig.py
@author N.Pettersson
@date 2022-07-04
@brief Main CA-based python configuration for JetValidation
'''

def PhysValJetCfg(flags, **kwargs):
	kwargs.setdefault("EnableLumi", False)
	from AthenaCommon.Constants import WARNING
	kwargs.setdefault("OutputLevel", WARNING)
	kwargs.setdefault("DetailLevel", 10)

	from JetValidation.JetValidationToolsConfig import PhysValJetToolCfg
	return PhysValJetToolCfg(flags, **kwargs)
