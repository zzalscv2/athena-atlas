# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

include.block('RunDependentSimData/configCommon.py')
RunDMC_testing_configuration = True  # to make sure JobMaker is retsined
include('RunDependentSimData/configLumi_run450000_mc23d_MultiBeamspot.py')

include('RunDependentSimData/configCommonSim.py')
del JobMaker
del JobMakerSim
