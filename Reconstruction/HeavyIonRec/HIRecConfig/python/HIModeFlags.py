# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.Enums import HIMode


def HImode(flags):
    # set the mode and then any dedicated flags
    flags.Reco.HIMode = HIMode.HI


def HIPmode(flags):
    # set the mode and then any dedicated flags
    flags.Reco.HIMode = HIMode.HIP


def UPCmode(flags):
    flags.Reco.HIMode = HIMode.UPC
