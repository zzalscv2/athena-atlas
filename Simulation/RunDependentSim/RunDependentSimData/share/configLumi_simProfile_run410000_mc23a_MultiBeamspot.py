# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

include.block('RunDependentSimData/configCommon.py')
RunDMC_testing_configuration = True  # to make sure JobMaker is retsined
include('RunDependentSimData/configLumi_run410000_mc23a_MultiBeamspot.py')

include('RunDependentSimData/configCommonSim.py')
del JobMaker
del JobMakerSim
