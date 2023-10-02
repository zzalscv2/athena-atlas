# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# All derivation framework formats must be listed here

# Example formats
# Skimming example
from DerivationFrameworkExamples.TEST1 import TEST1Cfg
# Skimming with strings example
from DerivationFrameworkExamples.TEST2 import TEST2Cfg
# Thinning example
from DerivationFrameworkExamples.TEST3 import TEST3Cfg
# Slimming example
from DerivationFrameworkExamples.TEST4 import TEST4Cfg
# Decoration example
from DerivationFrameworkExamples.TEST5 import TEST5Cfg
# Pre-selection example
from DerivationFrameworkExamples.TEST6 import TEST6Cfg

# Truth (EVNT->xAOD) formats
# TRUTH0 - complete copy of HepMC to xAOD truth
from DerivationFrameworkMCTruth.TRUTH0 import TRUTH0Cfg
# TRUTH1 - extended common ATLAS truth for analysis
from DerivationFrameworkMCTruth.TRUTH1 import TRUTH1Cfg
# TRUTH3 - standard common ATLAS truth for analysis
from DerivationFrameworkMCTruth.TRUTH3 import TRUTH3Cfg

# Common unskimmed formats for Run 3 physics analysis
# PHYS - uncalibrated, full slimming list
from DerivationFrameworkPhys.PHYS import PHYSCfg
# PHYSLITE - calibrated physics analysis objects, reduced slimming list
from DerivationFrameworkPhys.PHYSLITE import PHYSLITECfg

# Physics validation for run 3
# PHYSVAL - large bulk of the variables from AOD plus PHYS augmentations
from DerivationFrameworkPhysicsValidation.PHYSVAL import PHYSVALCfg

# Higgs derivations
# HIGG1D1 Higgs->gammagamma derivation
from DerivationFrameworkHiggs.HIGG1D1 import HIGG1D1Cfg

# LLP derivations
from DerivationFrameworkLLP.LLP1 import LLP1Cfg

# InDet derivations
from DerivationFrameworkInDet.IDTR2 import IDTR2Cfg

# BLS derivations
from DerivationFrameworkBPhys.BPHY1 import BPHY1Cfg
from DerivationFrameworkBPhys.BPHY2 import BPHY2Cfg
from DerivationFrameworkBPhys.BPHY3 import BPHY3Cfg
from DerivationFrameworkBPhys.BPHY4 import BPHY4Cfg
from DerivationFrameworkBPhys.BPHY5 import BPHY5Cfg
from DerivationFrameworkBPhys.BPHY6 import BPHY6Cfg
from DerivationFrameworkBPhys.BPHY10 import BPHY10Cfg
from DerivationFrameworkBPhys.BPHY12 import BPHY12Cfg
from DerivationFrameworkBPhys.BPHY13 import BPHY13Cfg
from DerivationFrameworkBPhys.BPHY15 import BPHY15Cfg
from DerivationFrameworkBPhys.BPHY16 import BPHY16Cfg
from DerivationFrameworkBPhys.BPHY18 import BPHY18Cfg
from DerivationFrameworkBPhys.BPHY21 import BPHY21Cfg
from DerivationFrameworkBPhys.BPHY22 import BPHY22Cfg
from DerivationFrameworkBPhys.BPHY23 import BPHY23Cfg
from DerivationFrameworkBPhys.BPHY24 import BPHY24Cfg


# STDM derivations
from DerivationFrameworkSM.STDM7 import STDM7Cfg

# TileCal derivations
from DerivationFrameworkTileCal.TCAL1 import TCAL1Cfg
from DerivationFrameworkTileCal.TCAL2 import TCAL2Cfg

# EGamma derivations
from DerivationFrameworkEGamma.EGAM1 import EGAM1Cfg
from DerivationFrameworkEGamma.EGAM2 import EGAM2Cfg
from DerivationFrameworkEGamma.EGAM3 import EGAM3Cfg
from DerivationFrameworkEGamma.EGAM4 import EGAM4Cfg
from DerivationFrameworkEGamma.EGAM5 import EGAM5Cfg
from DerivationFrameworkEGamma.EGAM7 import EGAM7Cfg
from DerivationFrameworkEGamma.EGAM8 import EGAM8Cfg
from DerivationFrameworkEGamma.EGAM9 import EGAM9Cfg
from DerivationFrameworkEGamma.EGAM10 import EGAM10Cfg
from DerivationFrameworkEGamma.EGAM11 import EGAM11Cfg
from DerivationFrameworkEGamma.EGAM12 import EGAM12Cfg

# FTAG derivations
from DerivationFrameworkFlavourTag.FTAG1 import FTAG1Cfg
from DerivationFrameworkFlavourTag.FTAG2 import FTAG2Cfg
from DerivationFrameworkFlavourTag.FTAG3 import FTAG3Cfg

# Jet/Etmiss derivations
# JETM1: dijet for MC calibrations, JER, MJB, eta-intercalibration
from DerivationFrameworkJetEtMiss.JETM1 import JETM1Cfg
# JETM2: MC only - tagger and JetDef developments
from DerivationFrameworkJetEtMiss.JETM2 import JETM2Cfg
# JETM3: Z(ll) + jets
from DerivationFrameworkJetEtMiss.JETM3 import JETM3Cfg
# JETM4: gamma+jets
from DerivationFrameworkJetEtMiss.JETM4 import JETM4Cfg
# JETM5: zero bias data - random cones
from DerivationFrameworkJetEtMiss.JETM5 import JETM5Cfg
# JETM6: tagging SFs
from DerivationFrameworkJetEtMiss.JETM6 import JETM6Cfg
# JETM7: by-vertex jet reconstruction
from DerivationFrameworkJetEtMiss.JETM7 import JETM7Cfg
# JETM10: MET trigger
from DerivationFrameworkJetEtMiss.JETM10 import JETM10Cfg
# JETM11: MET trigger (e + mu skimming)
from DerivationFrameworkJetEtMiss.JETM11 import JETM11Cfg
# JETM12: E/p studies in W to tau + v events
from DerivationFrameworkJetEtMiss.JETM12 import JETM12Cfg
# JETM14: MET trigger (single mu selection)
from DerivationFrameworkJetEtMiss.JETM14 import JETM14Cfg

# Trigger derivations
# TRIG8: ID trigger performance (extra trigger info eg online tracks and RoIs [idperf chain skimming])
from DerivationFrameworkTrigger.TRIG8 import TRIG8Cfg

# L1CALO1 derivation - runs primarily on RAWD
from DerivationFrameworkL1Calo.L1CALO1 import L1CALO1Cfg

# Muon combined performance DAODs
# MUON1: baseline format for T&P 
from DerivationFrameworkMuons.MUON1 import MUON1Cfg

# Avoids compilation warnings from Flake8
__all__ = ['TEST1Cfg','TEST2Cfg','TEST3Cfg','TEST4Cfg','TEST5Cfg','TEST6Cfg',
           'TRUTH0Cfg','TRUTH1Cfg','TRUTH3Cfg',
           'PHYSCfg','PHYSLITECfg',
           'PHYSVALCfg',
           'FTAG1Cfg', 'FTAG2Cfg', 'FTAG3Cfg',
           'HIGG1D1Cfg',
           'LLP1Cfg',
           'IDTR2Cfg',
           'BPHY1Cfg','BPHY2Cfg', 'BPHY3Cfg', 'BPHY4Cfg', 'BPHY5Cfg',
           'BPHY6Cfg',
           'BPHY10Cfg', 'BPHY12Cfg', 'BPHY13Cfg', 'BPHY15Cfg',
           'BPHY16Cfg', 'BPHY18Cfg',
           'BPHY21Cfg', 'BPHY22Cfg', 'BPHY23Cfg', 'BPHY24Cfg',
           'STDM7Cfg',
           'TCAL1Cfg', 'TCAL2Cfg',
           'EGAM1Cfg', 'EGAM2Cfg', 'EGAM3Cfg', 'EGAM4Cfg', 'EGAM5Cfg',
           'EGAM7Cfg', 'EGAM8Cfg', 'EGAM9Cfg', 'EGAM10Cfg', 'EGAM11Cfg', 'EGAM12Cfg',
           'JETM1Cfg','JETM2Cfg','JETM3Cfg','JETM4Cfg','JETM5Cfg','JETM6Cfg','JETM7Cfg',
           'JETM10Cfg','JETM11Cfg','JETM12Cfg','JETM14Cfg',
           'TRIG8Cfg','L1CALO1Cfg',
           'MUON1Cfg' 
           ]
