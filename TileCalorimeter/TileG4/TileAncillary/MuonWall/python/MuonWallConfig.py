# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def MuonWallSDCfg(flags, name="MuonWallSD", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("LogicalVolumeNames", ['MuonWall::MuScintillatorLayer','S1::S1','S2::S2','S3::S3'])
    kwargs.setdefault("OutputCollectionNames", ["TileTBHits"])
    result.setPrivateTools(CompFactory.MuonWallSDTool(name, **kwargs))
    return result


def MuonWallToolCfg(flags, name="MuonWall", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("DetectorName", "MuonWall")
    kwargs.setdefault("ZLength", 200.)
    kwargs.setdefault("YLength", 400.)
    kwargs.setdefault("XLength", 20.)
    eta=0
    theta=3.1415926
    delta=0.0
    zmax=2380.22441662
    r=5115.0
    z=0.0
    import math
    if (math.fabs(theta)<1.57):
        z=r*math.tan(-theta)-delta
    elif (eta!=0):
        z=r/math.tan(2*math.atan(math.exp(-eta)))
    if (z>zmax):
        z=zmax
    elif (z<-zmax):
        z=-zmax
    kwargs.setdefault("OffsetX", r)
    kwargs.setdefault("OffsetY", 0.0)
    kwargs.setdefault("OffsetZ", z)
    result.setPrivateTools(CompFactory.MuonWallTool(name, **kwargs))
    return result


def MuonWallTileTBCfg(flags, name="MuonWallTileTB", **kwargs):
    # Check the consistency of the flags
    if flags.TestBeam.Eta is not None and (flags.TestBeam.Theta is not None or flags.TestBeam.Z is not None):
        raise ValueError('THE ETA PARAMETER CAN NOT BE SET TOGETHER WITH THETA AND Z')
    elif (flags.TestBeam.Theta is None or flags.TestBeam.Z is None) and flags.TestBeam.Eta is None:
        raise ValueError('THETA AND Z ARE NOT SET')
    import math
    from AthenaCommon import PhysicalConstants
    if flags.TestBeam.Eta is not None:
        # Mode 1 -> User enters only eta
        eta=flags.TestBeam.Eta
        ThetaY=-(PhysicalConstants.pi*0.5)+2*math.atan(math.exp(-eta))
        DeltaF=0.0
    elif not (flags.TestBeam.Theta is None or flags.TestBeam.Z is None):
        theta=flags.TestBeam.Theta
        z=flags.TestBeam.Z
        eta=0.0
        if abs(theta) < 70.01:
            # Mode 2 -> User enters theta!=+/-90 and Z
            # Z is the distance from center of the module to the desired
            # entrace point calculated at R=2290 (start of tilecal
            # module)
            ThetaY=math.radians(theta)
            DeltaF=(2290.0*math.tan(-ThetaY)-z)
        elif abs(abs(theta)-90.0) < 0.01:
            # Mode 3 -> User enters theta=(+/-)90 and Z
            # Z is the distance from ATLAS center to corresponding
            # tilerow
            # e.g center of first tile row is at 2300 + 100/2 = 2350
            sign=int(theta>0)*2-1
            ThetaY=sign*math.radians(90.0)
            DeltaF=0.0
    else:
        print ('Tile table rotation: ERROR unknown rotation mode')
        raise ValueError('UNKNOWN MODE - NEITHER ETA NOR THETA AND Z ARE SET')
    zmax=2380.22441662
    r=5115.0
    z=0.0
    if (math.fabs(ThetaY)<1.57):
        z=r*math.tan(-ThetaY)-DeltaF
    elif (eta!=0):
        z=r/math.tan(2*math.atan(math.exp(-eta)))
    if (z>zmax):
        z=zmax
    elif (z<-zmax):
        z=-zmax
    kwargs.setdefault("OffsetX", r)
    kwargs.setdefault("OffsetY", 0.0)
    kwargs.setdefault("OffsetZ", z)
    return MuonWallToolCfg(name, **kwargs)

