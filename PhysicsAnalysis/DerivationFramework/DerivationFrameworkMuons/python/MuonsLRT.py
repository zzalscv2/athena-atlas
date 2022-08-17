# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#********************************************************************
# MuonsLRT.py
# Schedules additional tools needed for LRT muon object selection and writes
# results into SG. These may then be accessed along the train
#********************************************************************
def makeLRTMuonsDF():
    from DerivationFrameworkMuons import MuonsCommon
    MuonsCommon.makeMuonsDFCommon(postfix="LRT")
