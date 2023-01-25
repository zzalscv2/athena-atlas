# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import mm

#########################################################################################
#--- Ancillary volumes TB 2000-2003  ------------------------------------------------
#########################################################################################

def TileTB_Beampipe1Cfg(flags, name="TileTB_BeamPipe1", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("DetectorName", "BEAMPIPE1")
    kwargs.setdefault("InnerRadius", 0.*mm)
    kwargs.setdefault("OuterRadius", 100.*mm)
    dz=268500.*mm
    kwargs.setdefault("dZ", dz)
    kwargs.setdefault("Material", 'Vacuum')
    kwargs.setdefault("OffsetX",-12080.*mm-dz)
    import math
    kwargs.setdefault("RotateY", math.radians(-90.))
    result.setPrivateTools(CompFactory.CylindricalEnvelope(name, **kwargs))
    return result


def TileTB_Beampipe2Cfg(flags, name="TileTB_BeamPipe2", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("DetectorName", "BEAMPIPE2")
    kwargs.setdefault("InnerRadius", 0.*mm)
    kwargs.setdefault("OuterRadius", 100.*mm)
    dz=6419.8*mm
    kwargs.setdefault("dZ", dz)
    kwargs.setdefault("Material", 'Vacuum')
    kwargs.setdefault("OffsetX",1945.*mm-dz)
    import math
    kwargs.setdefault("RotateY", math.radians(-90.))
    result.setPrivateTools(CompFactory.CylindricalEnvelope(name, **kwargs))
    return result


def TileTB_MYLAREQUIVCfg(flags, name="TileTB_MYLAREQUIV", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("DetectorName", "MYLAREQUIV")
    kwargs.setdefault("InnerRadius", 0.*mm)
    kwargs.setdefault("OuterRadius", 100.*mm)
    dz=0.00168*mm
    kwargs.setdefault("dZ", dz)
    kwargs.setdefault("Material", 'Mylar')
    kwargs.setdefault("OffsetX",-12080*mm+dz)
    import math
    kwargs.setdefault("RotateY", math.radians(-90.))
    result.setPrivateTools(CompFactory.CylindricalEnvelope(name, **kwargs))
    return result


def TileTB_S1Cfg(flags, name="TileTB_S1", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("DetectorName", "S1")
    kwargs.setdefault("dX", 5.*mm)
    kwargs.setdefault("dY", 52.5*mm)
    kwargs.setdefault("dZ", 50.*mm)
    kwargs.setdefault("Material", 'Scintillator')
    kwargs.setdefault("OffsetX",-12074.6*mm)
    result.setPrivateTools(CompFactory.BoxEnvelope(name, **kwargs))
    return result


def TileTB_S2Cfg(flags, name="TileTB_S2", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("DetectorName", "S2")
    kwargs.setdefault("dX", 10.*mm)
    kwargs.setdefault("dY", 40.*mm)
    kwargs.setdefault("dZ", 27.5*mm)
    kwargs.setdefault("Material", 'Scintillator')
    kwargs.setdefault("OffsetX",-11294.6*mm)
    result.setPrivateTools(CompFactory.BoxEnvelope(name, **kwargs))
    return result


def TileTB_S3Cfg(flags, name="TileTB_S3", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("DetectorName", "S3")
    kwargs.setdefault("dX", 10.*mm)
    kwargs.setdefault("dY", 40.*mm)
    kwargs.setdefault("dZ", 27.5*mm)
    kwargs.setdefault("Material", 'Scintillator')
    kwargs.setdefault("OffsetX",-10994.6*mm)
    result.setPrivateTools(CompFactory.BoxEnvelope(name, **kwargs))
    return result

#########################################################################################
#--- Tile TB 2000-2003  ------------------------------------------------
#########################################################################################

def TileTB_CALOEnvelopeCfg(flags, name="TileTB_CALO", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("DetectorName", "CALO")
    import math
    kwargs.setdefault("StartPhi", math.radians(-27))
    kwargs.setdefault("DeltaPhi", math.radians(60))
    kwargs.setdefault("NSurfaces", 3)
    kwargs.setdefault("InnerRadii", [2269.*mm,950.*mm,950.*mm])
    kwargs.setdefault("OuterRadii", [5145.*mm,5145.*mm,5145.*mm])
    kwargs.setdefault("ZSurfaces",  [-3400.*mm,-1050.*mm,6600.*mm])
    # Check the consistency of the flags
    if flags.TestBeam.Eta is not None and (flags.TestBeam.Theta is not None or flags.TestBeam.Z is not None):
        raise ValueError('THE ETA PARAMETER CAN NOT BE SET TOGETHER WITH THETA AND Z')
    elif (flags.TestBeam.Theta is None or flags.TestBeam.Z is None) and flags.TestBeam.Eta is None:
        raise ValueError('THETA AND Z ARE NOT SET')
    import math
    from AthenaCommon import PhysicalConstants
    DeltaY=0.0
    if flags.TestBeam.Y is not None :
        DeltaY=-flags.TestBeam.Y
    PhiZ=0.0
    if flags.TestBeam.Phi is not None:
        PhiZ=-math.radians(flags.TestBeam.Phi)
    if flags.TestBeam.Eta is not None:
        # Mode 1 -> User enters only eta
        eta=flags.TestBeam.Eta
        ThetaY=-(PhysicalConstants.pi*0.5)+2*math.atan(math.exp(-eta))
        DeltaX=float(2298-6208)/math.cosh(eta)+6208
        DeltaF=0.0
        DeltaZ=0.0
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
            DeltaX=2298.0+3910.0*(1.0-math.cos(ThetaY))
            DeltaF=(2290.0*math.tan(-ThetaY)-z)
            DeltaZ=DeltaF*math.cos(ThetaY)
        elif abs(abs(theta)-90.0) < 0.01:
            # Mode 3 -> User enters theta=(+/-)90 and Z
            # Z is the distance from ATLAS center to corresponding
            # tilerow
            # e.g center of first tile row is at 2300 + 100/2 = 2350
            sign=int(theta>0)*2-1
            ThetaY=sign*math.radians(90.0)
            DeltaX=2298.0+2290.0+5640.0/2
            DeltaF=0.0
            DeltaZ=-sign*math.fabs(z)
    else:
        print ('Tile table rotation: ERROR unknown rotation mode')
        raise ValueError('UNKNOWN MODE - NEITHER ETA NOR THETA AND Z ARE SET')
    kwargs.setdefault("RotateZ",PhiZ)
    kwargs.setdefault("RotateY",ThetaY)
    kwargs.setdefault("OffsetX",DeltaX)
    kwargs.setdefault("OffsetY",DeltaY)
    kwargs.setdefault("OffsetZ",DeltaZ)
    SubDetectorList=[]
    if flags.Detector.GeometryCalo:
        from G4AtlasTools.G4GeometryToolConfig import TileGeoDetectorToolCfg
        toolTile = result.popToolsAndMerge(TileGeoDetectorToolCfg(flags))
        SubDetectorList += [ toolTile ]
    from MuonWall.WuonWallConfig import MuonWallTileTBCfg
    muonWallTool = result.popToolsAndMerge(MuonWallTileTBCfg(flags))
    SubDetectorList += [muonWallTool]
    kwargs.setdefault("SubDetectors", SubDetectorList)
    result.setPrivateTools(CompFactory.PolyconicalEnvelope(name, **kwargs))
    return result


def TileTB_WorldEnvelopeCfg(flags, name="TileTB_World", **kwargs):
    result = ComponentAccumulator()
    kwargs.setdefault("DetectorName", "CTB")
    kwargs.setdefault("dX", 600000.*mm)
    kwargs.setdefault("dY",  5000.*mm)
    kwargs.setdefault("dZ", 10000.*mm)
    SubDetectorList=[]
    if flags.Detector.GeometryCalo:
        tbCaloTool = result.popToolsAndMerge(TileTB_CALOEnvelopeCfg(flags))
        SubDetectorList += [tbCaloTool]
    bp1Tool = result.popToolsAndMerge(TileTB_Beampipe1Cfg(flags))
    SubDetectorList += [bp1Tool]
    bp2Tool = result.popToolsAndMerge(TileTB_Beampipe2Cfg(flags))
    SubDetectorList += [bp2Tool]
    s1Tool = result.popToolsAndMerge(TileTB_S1Cfg(flags))
    SubDetectorList += [s1Tool]
    s2Tool = result.popToolsAndMerge(TileTB_S2Cfg(flags))
    SubDetectorList += [s2Tool]
    s3Tool = result.popToolsAndMerge(TileTB_S3Cfg(flags))
    SubDetectorList += [s3Tool]
    # Not used in currently supported Tile Test Beam setups, but left as a reminder
    #larEqvTool = result.popToolsAndMerge(TileTB_MYLAREQUIVCfg(flags))
    #SubDetectorList += [larEqvTool]
    kwargs.setdefault("SubDetectors", SubDetectorList)
    result.setPrivateTools(CompFactory.BoxEnvelope(name, **kwargs))
    return result
