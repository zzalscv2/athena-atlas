# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

"""
BarcodeServices configurations
Elmar Ritsch, 23/10/2014
"""

from AthenaCommon import CfgMgr


def getMC15BarcodeSvc(name="Barcode_MC15BarcodeSvc", **kwargs):
    return CfgMgr.Barcode__GenericBarcodeSvc(name, **kwargs)


def getMC12BarcodeSvc(name="Barcode_MC12BarcodeSvc", **kwargs):
    return CfgMgr.Barcode__LegacyBarcodeSvc(name, **kwargs)


def getMC12LLPBarcodeSvc(name="Barcode_MC12LLPBarcodeSvc", **kwargs):
    return getMC12BarcodeSvc(name, **kwargs)


def getMC12PlusBarcodeSvc(name="Barcode_MC12PlusBarcodeSvc", **kwargs):
    return getMC12BarcodeSvc(name, **kwargs)


def getMC15aPlusBarcodeSvc(name="Barcode_MC15aPlusBarcodeSvc", **kwargs):
    return getMC12BarcodeSvc(name, **kwargs)


def getMC15aPlusLLPBarcodeSvc(name="Barcode_MC15aPlusLLPBarcodeSvc", **kwargs):
    return getMC12BarcodeSvc(name, **kwargs)


def getMC15aBarcodeSvc(name="Barcode_MC15aBarcodeSvc", **kwargs):
    return getMC12BarcodeSvc(name, **kwargs)


def getValidationBarcodeSvc(name="Barcode_ValidationBarcodeSvc", **kwargs):
    return CfgMgr.Barcode__ValidationBarcodeSvc(name, **kwargs)


def getBarcodeSvc(name="BarcodeSvc", **kwargs):
    stratmap = {
        'MC12': getMC12BarcodeSvc,
        'MC12LLP': getMC12LLPBarcodeSvc,
        'MC12Plus': getMC12PlusBarcodeSvc,
        'MC15a': getMC15aBarcodeSvc,
        'MC15aPlus': getMC15aPlusBarcodeSvc,
        'MC15aPlusLLP': getMC15aPlusLLPBarcodeSvc,
        'MC15':  getMC15BarcodeSvc,
        'Validation': getValidationBarcodeSvc
    }
    from G4AtlasApps.SimFlags import simFlags
    barcodesvcfunc = stratmap[simFlags.TruthStrategy.get_Value()]
    return barcodesvcfunc(name, **kwargs)


def barcodeOffsetForTruthStrategy(strategyName):
    return 200000
