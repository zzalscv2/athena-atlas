"""ComponentAccumulator BarcodeServices configurations

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def BarcodeSvcCfg(flags, **kwargs):
    """Return the MCxBarcodeSvcCfg config flagged by Sim.TruthStrategy"""
    from SimulationConfig.SimEnums import TruthStrategy
    stratmap = {
        TruthStrategy.MC12: MC12BarcodeSvcCfg,
        TruthStrategy.MC12LLP: MC12LLPBarcodeSvcCfg,
        TruthStrategy.MC12Plus: MC12PlusBarcodeSvcCfg,
        TruthStrategy.MC15: MC15BarcodeSvcCfg,
        TruthStrategy.MC15a: MC15aBarcodeSvcCfg,
        TruthStrategy.MC15aPlus: MC15aPlusBarcodeSvcCfg,
        TruthStrategy.MC15aPlusLLP: MC15aPlusLLPBarcodeSvcCfg,
        TruthStrategy.MC16: MC16BarcodeSvcCfg,
        TruthStrategy.MC16LLP: MC16LLPBarcodeSvcCfg,
        TruthStrategy.MC18: MC18BarcodeSvcCfg,
        TruthStrategy.MC18LLP: MC18LLPBarcodeSvcCfg,
        TruthStrategy.PhysicsProcess: PhysicsProcessBarcodeSvcCfg,
        TruthStrategy.Global: GlobalBarcodeSvcCfg,
        TruthStrategy.Validation: ValidationBarcodeSvcCfg,
        # TruthStrategy.Cosmic: CosmicBarcodeSvcCfg,
    }
    MCxCfg = stratmap[flags.Sim.TruthStrategy]
    return MCxCfg(flags, **kwargs)


def MC15BarcodeSvcCfg(flags, name="Barcode_MC15BarcodeSvc", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FirstSecondaryVertexBarcode"   ,  -1000001 )
    kwargs.setdefault("VertexIncrement"               ,        -1 )
    kwargs.setdefault("FirstSecondaryBarcode"         ,   1000001 )
    kwargs.setdefault("SecondaryIncrement"            ,         1 )
    kwargs.setdefault("ParticleRegenerationIncrement" ,  10000000 )
    kwargs.setdefault("DoUnderAndOverflowChecks"      ,      True )
    kwargs.setdefault("EncodePhysicsProcessInVertexBC",     False )
    svc = CompFactory.Barcode.GenericBarcodeSvc(name, **kwargs)
    result.addService(svc, primary=True)
    return result


def MC12BarcodeSvcCfg(flags, name="Barcode_MC12BarcodeSvc", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FirstSecondaryVertexBarcode" , -200001)
    kwargs.setdefault("VertexIncrement"             , -1)
    kwargs.setdefault("FirstSecondaryBarcode"       , 200001)
    kwargs.setdefault("SecondaryIncrement"          , 1)
    kwargs.setdefault("ParticleGenerationIncrement" , 1000000)
    svc = CompFactory.Barcode.LegacyBarcodeSvc(name, **kwargs)
    result.addService(svc, primary=True)
    return result


def MC12LLPBarcodeSvcCfg(flags, name="Barcode_MC12LLPBarcodeSvc", **kwargs):
    return MC12BarcodeSvcCfg(flags, name, **kwargs)


def MC12PlusBarcodeSvcCfg(flags, name="Barcode_MC12PlusBarcodeSvc", **kwargs):
    return MC12BarcodeSvcCfg(flags, name, **kwargs)


def MC15aPlusBarcodeSvcCfg(flags, name="Barcode_MC15aPlusBarcodeSvc", **kwargs):
    return MC12BarcodeSvcCfg(flags, name, **kwargs)


def MC15aPlusLLPBarcodeSvcCfg(flags, name="Barcode_MC15aPlusLLPBarcodeSvc", **kwargs):
    return MC12BarcodeSvcCfg(flags, name, **kwargs)


def MC15aBarcodeSvcCfg(flags, name="Barcode_MC15aBarcodeSvc", **kwargs):
    return MC12BarcodeSvcCfg(flags, name, **kwargs)


def MC16BarcodeSvcCfg(flags, name="Barcode_MC16BarcodeSvc", **kwargs):
    return MC12BarcodeSvcCfg(flags, name, **kwargs)


def MC16LLPBarcodeSvcCfg(flags, name="Barcode_MC16LLPBarcodeSvc", **kwargs):
    return MC12BarcodeSvcCfg(flags, name, **kwargs)


def MC18BarcodeSvcCfg(flags, name="Barcode_MC18BarcodeSvc", **kwargs):
    kwargs.setdefault("FirstSecondaryVertexBarcode" ,  -1000001 )
    kwargs.setdefault("FirstSecondaryBarcode"       ,   1000001 )
    kwargs.setdefault("ParticleGenerationIncrement" ,  10000000 )
    return MC12BarcodeSvcCfg(flags, name, **kwargs)


def MC18LLPBarcodeSvcCfg(flags, name="Barcode_MC18LLPBarcodeSvc", **kwargs):
    return MC18BarcodeSvcCfg(flags, name, **kwargs)


def PhysicsProcessBarcodeSvcCfg(flags, name="Barcode_PhysicsProcessBarcodeSvc", **kwargs):
    kwargs.setdefault("EncodePhysicsProcessInVertexBC",  False  )
    kwargs.setdefault("FirstSecondaryVertexBarcode"   , -200000 )
    kwargs.setdefault("VertexIncrement"               , -1000000)
    kwargs.setdefault("FirstSecondaryBarcode"         ,  200001 )
    kwargs.setdefault("SecondaryIncrement"            ,  1      )
    kwargs.setdefault("EncodePhysicsProcessInVertexBC",  True   )
    return MC15BarcodeSvcCfg(flags, name, **kwargs)


def GlobalBarcodeSvcCfg(flags, name="Barcode_GlobalBarcodeSvc", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FirstSecondaryVertexBarcode"   ,  -200000  )
    kwargs.setdefault("VertexIncrement"               ,  -1000000 )
    kwargs.setdefault("FirstSecondaryBarcode"         ,   200001  )
    kwargs.setdefault("SecondaryIncrement"            ,   1       )
    kwargs.setdefault("DoUnderAndOverflowChecks"      ,   True    )
    kwargs.setdefault("EncodePhysicsProcessInVertexBC",   True    )
    svc = CompFactory.Barcode.GlobalBarcodeSvc(name, **kwargs)
    result.addService(svc, primary=True)
    return result


def ValidationBarcodeSvcCfg(flags, name="Barcode_ValidationBarcodeSvc", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("FirstSecondaryVertexBarcode" , -200001)
    kwargs.setdefault("VertexIncrement"             , -1)
    kwargs.setdefault("FirstSecondaryBarcode"       , 200001)
    kwargs.setdefault("SecondaryIncrement"          , 1)
    kwargs.setdefault("ParticleGenerationIncrement" , 1000000)
    kwargs.setdefault("DoUnderAndOverflowChecks"    , True)
    svc = CompFactory.Barcode.ValidationBarcodeSvc(name, **kwargs)
    result.addService(svc, primary=True)
    return result
