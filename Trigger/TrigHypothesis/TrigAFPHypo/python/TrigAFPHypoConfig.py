# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory

def trigAFPDijetComboHypoToolCfg(flags, name="TrigAFPDijetComboHypoTool"):
    hypo = CompFactory.TrigAFPDijetComboHypoTool(
        name = name,
        protonTransportParamFileName1="2017_beta0p4_xAngle170_beam1.txt",
        protonTransportParamFileName2="2017_beta0p4_xAngle170_beam2.txt",
        alignmentCorrection_nearA=2.361,
        alignmentCorrection_nearC=2.172,
        TransportBeamA = CompFactory.AFPProtonTransportTool("AFPProtonTransportToolSideA_"+name),
        TransportBeamC = CompFactory.AFPProtonTransportTool("AFPProtonTransportToolSideC_"+name) )

    return hypo
