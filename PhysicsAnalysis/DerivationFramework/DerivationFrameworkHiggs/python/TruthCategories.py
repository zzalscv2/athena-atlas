# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from DerivationFrameworkCore.DerivationFrameworkMaster import DerivationFrameworkJob
from AthenaCommon.GlobalFlags import globalflags

if globalflags.DataSource()=='geant4':     
    from DerivationFrameworkHiggs.DerivationFrameworkHiggsConf import DerivationFramework__TruthCategoriesDecorator
    DFHTXSdecorator = DerivationFramework__TruthCategoriesDecorator(name = "DFHTXSdecorator")
    DerivationFrameworkJob += DFHTXSdecorator