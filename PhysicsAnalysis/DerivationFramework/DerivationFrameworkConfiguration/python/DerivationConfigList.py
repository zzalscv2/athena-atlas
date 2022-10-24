# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
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

# FTAG derivations
from DerivationFrameworkFlavourTag.FTAG1 import FTAG1Cfg
from DerivationFrameworkFlavourTag.FTAG2 import FTAG2Cfg

# Jet/Etmiss derivations
from DerivationFrameworkJetEtMiss.JETM1 import JETM1Cfg
from DerivationFrameworkJetEtMiss.JETM3 import JETM3Cfg
from DerivationFrameworkJetEtMiss.JETM4 import JETM4Cfg
from DerivationFrameworkJetEtMiss.JETM5 import JETM5Cfg
from DerivationFrameworkJetEtMiss.JETM6 import JETM6Cfg
from DerivationFrameworkJetEtMiss.JETM8 import JETM8Cfg
from DerivationFrameworkJetEtMiss.JETM10 import JETM10Cfg
from DerivationFrameworkJetEtMiss.JETM11 import JETM11Cfg
from DerivationFrameworkJetEtMiss.JETM12 import JETM12Cfg
from DerivationFrameworkJetEtMiss.JETM13 import JETM13Cfg
from DerivationFrameworkJetEtMiss.JETM14 import JETM14Cfg

# Avoids compilation warnings from Flake8
__all__ = ['TEST1Cfg','TEST2Cfg','TEST3Cfg','TEST4Cfg','TEST5Cfg','TEST6Cfg',
           'TRUTH0Cfg','TRUTH1Cfg','TRUTH3Cfg',
           'PHYSCfg','PHYSLITECfg',
           'PHYSVALCfg',
           'FTAG1Cfg', 'FTAG2Cfg',
           'HIGG1D1Cfg',
           'LLP1Cfg',
           'BPHY1Cfg','BPHY2Cfg', 'BPHY3Cfg', 'BPHY4Cfg', 'BPHY5Cfg',
           'BPHY6Cfg',
           'BPHY10Cfg', 'BPHY12Cfg', 'BPHY13Cfg', 'BPHY15Cfg',
           'BPHY16Cfg', 'BPHY18Cfg',
           'BPHY21Cfg', 'BPHY22Cfg',
           'TCAL1Cfg', 'TCAL2Cfg',
           'EGAM1Cfg', 'EGAM2Cfg', 'EGAM3Cfg', 'EGAM4Cfg', 'EGAM5Cfg',
           'EGAM7Cfg', 'EGAM8Cfg', 'EGAM9Cfg', 'EGAM10Cfg',
           'JETM1Cfg','JETM3Cfg','JETM4Cfg','JETM5Cfg','JETM6Cfg','JETM8Cfg',
           'JETM10Cfg','JETM11Cfg','JETM12Cfg','JETM13Cfg','JETM14Cfg',
           ]
