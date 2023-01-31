# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def StandardFieldSvcCfg(flags, name="StandardField", **kwargs):
    result = ComponentAccumulator()
    result.addService(CompFactory.StandardFieldSvc(name, **kwargs), primary=True)
    return result


def ForwardFieldSvcCfg(flags, name="ForwardField", **kwargs):
    result = ComponentAccumulator()
    # FIXME Once it exists this version should use the new MagField Service defined in ForwardRegionMgField
    # kwargs.setdefault("MagneticFieldSvc", result.getService("AtlasFieldSvc"))
    # kwargs.setdefault("FieldOn", True)
    kwargs.setdefault("UseMagFieldSvc", True)
    result.addService(CompFactory.StandardFieldSvc(name, **kwargs), primary=True)
    return result


def Q1FwdG4FieldSvcCfg(flags, name='Q1FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q1",
                                                                   # FIXME find a better way to do this.
                                                                   Magnet=0,
                                                                   MQXA_DataFile="MQXA_NOMINAL.dat"),
                        primary=True)

    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q2FwdG4FieldSvcCfg(flags, name='Q2FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q2",
                                                                   # FIXME find a better way to do this.
                                                                   Magnet=1,
                                                                   MQXA_DataFile="MQXA_NOMINAL.dat"),
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q3FwdG4FieldSvcCfg(flags, name='Q3FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q3",
                                                                   # FIXME find a better way to do this.
                                                                   Magnet=2,
                                                                   MQXA_DataFile="MQXA_NOMINAL.dat"),
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def D1FwdG4FieldSvcCfg(flags, name='D1FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("D1", Magnet=3),  # FIXME find a better way to do this.
                        primary=True)

    kwargs.setdefault("MagneticFieldSvc",result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def D2FwdG4FieldSvcCfg(flags, name='D2FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("D2", Magnet=4),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q4FwdG4FieldSvcCfg(flags, name='Q4FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q4", Magnet=5),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q5FwdG4FieldSvcCfg(flags, name='Q5FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q5", Magnet=6),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q6FwdG4FieldSvcCfg(flags, name='Q6FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q6", Magnet=7),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q7FwdG4FieldSvcCfg(flags, name='Q7FwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q7", Magnet=8),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q1HKickFwdG4FieldSvcCfg(flags, name='Q1HKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q1HKick", Magnet=9),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


# note is lower case "v" in ForwardRegionMgFieldConfig.py
def Q1VKickFwdG4FieldSvcCfg(flags, name='Q1VKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q1VKick", Magnet=10),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q2HKickFwdG4FieldSvcCfg(flags, name='Q2HKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q2HKick", Magnet=11),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q2VKickFwdG4FieldSvcCfg(flags, name='Q2VKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q2VKick", Magnet=12),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q3HKickFwdG4FieldSvcCfg(flags, name='Q3HKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q3HKick", Magnet=13),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q3VKickFwdG4FieldSvcCfg(flags, name='Q3VKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q3VKick", Magnet=14),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q4VKickAFwdG4FieldSvcCfg(flags, name='Q4VKickAFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q4VKickA", Magnet=15),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q4HKickFwdG4FieldSvcCfg(flags, name='Q4HKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q4HKick", Magnet=16),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q4VKickBFwdG4FieldSvcCfg(flags, name='Q4VKickBFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q4VKickB", Magnet=17),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q5HKickFwdG4FieldSvcCfg(flags, name='Q5HKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q5HKick", Magnet=18),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result


def Q6VKickFwdG4FieldSvcCfg(flags, name='Q6VKickFwdG4FieldSvc', **kwargs):
    result = ComponentAccumulator()
    fieldAcc = ComponentAccumulator()
    fieldAcc.addService(CompFactory.MagField.ForwardRegionFieldSvc("Q6VKick", Magnet=19),  # FIXME find a better way to do this.
                        primary=True)
    kwargs.setdefault("MagneticFieldSvc", result.getPrimaryAndMerge(fieldAcc).name)
    fieldSvc = result.getPrimaryAndMerge(ForwardFieldSvcCfg(flags, name, **kwargs))
    result.addService(fieldSvc, primary=True)
    return result
