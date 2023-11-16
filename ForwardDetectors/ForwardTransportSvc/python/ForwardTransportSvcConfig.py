# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from ForwardRegionProperties.ForwardRegionPropertiesConfig import resolveTwissBeamFilePath, buildTwissFilePath

def ForwardTransportBeta055mPreInclude(flags): # TODO think of a better name for this preInclude
    flags.Sim.TwissFileBeam1="3.5TeV/0000.55m/nominal/v01/beam1.tfs"
    flags.Sim.TwissFileBeam2="3.5TeV/0000.55m/nominal/v01/beam2.tfs"
    from AthenaCommon.SystemOfUnits import TeV,m
    flags.Beam.Energy=3.5*TeV
    flags.Sim.TwissFileBeta=0.55*m
    flags.Sim.TwissFileNomReal='nominal'
    flags.Sim.TwissFileVersion="v01"


def ForwardTransportBeta90mPreInclude(flags): # TODO think of a better name for this preInclude
    flags.Sim.TwissFileBeam1="3.5TeV/0090.00m/nominal/v02/beam1.tfs"
    flags.Sim.TwissFileBeam2="3.5TeV/0090.00m/nominal/v02/beam2.tfs"
    from AthenaCommon.SystemOfUnits import TeV,m
    flags.Beam.Energy=3.5*TeV
    flags.Sim.TwissFileBeta=90.*m
    flags.Sim.TwissFileNomReal='nominal'
    flags.Sim.TwissFileVersion="v02"


def ForwardTransportSvcCfg(flags, name="ForwardTransportSvc", **kwargs):
    from AthenaCommon.Logging import logging
    msg = logging.getLogger("ForwardTransportSvcCfg")
    # Settings of optics to be used
    twiss_beam1 = resolveTwissBeamFilePath(flags.Sim.TwissFileBeam1, msg)
    twiss_beam2 = resolveTwissBeamFilePath(flags.Sim.TwissFileBeam2, msg)
    if twiss_beam1 is None or twiss_beam2 is None:
        msg.info("Attempting to build TwissFileBeam paths manually")
        # Getting paths to the twiss files, momentum calculation; you can switch to local files
        twiss_beam1 = buildTwissFilePath(flags, msg, 'beam1.tfs')
        twiss_beam2 = buildTwissFilePath(flags, msg, 'beam2.tfs')

    # properties of the field set according to the optics settings above
    kwargs.setdefault("TwissFile1", twiss_beam1)
    kwargs.setdefault("TwissFile2", twiss_beam2)
    kwargs.setdefault("PositionC1", 149)
    kwargs.setdefault("PositionC2", 184)
    kwargs.setdefault("ApertureC1", 999)
    kwargs.setdefault("ApertureC2", 999)
    if flags.Detector.GeometryALFA or flags.Detector.GeometryAFP:
        return ALFAForwardTransportSvcCfg(name, **kwargs)
    if flags.Detector.GeometryZDC:
        return ZDCForwardTransportSvcCfg (name, **kwargs)
    msg.warning("ALFA and ZDC are deactivated.")
    return ComponentAccumulator()


def ALFAForwardTransportSvcCfg(name="ForwardTransportSvc", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("EndMarker",     236.888)
    kwargs.setdefault("TransportFlag", 1)
    kwargs.setdefault("EtaCut",        7.5)
    kwargs.setdefault("XiCut",         0.8)
    result.addService(CompFactory.ForwardTransportSvc(name,**kwargs), create=True, primary=True)
    return result


def ZDCForwardTransportSvcCfg(name="ForwardTransportSvc", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("EndMarker",     141.580)
    kwargs.setdefault("TransportFlag", 0)
    kwargs.setdefault("EtaCut",        7.5)
    kwargs.setdefault("XiCut",         0)
    result.addService(CompFactory.ForwardTransportSvc(name,**kwargs), create=True, primary=True)
    return result
