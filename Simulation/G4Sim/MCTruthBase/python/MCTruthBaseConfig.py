# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def getEnvelopeMap(flags):
    #from G4AtlasApps.SimFlags import simFlags

    # Map of volume name to output collection name
    envelopeMap = dict()

    #todo - migrate ctb and cosmic config too
    """if ( hasattr(simFlags, "LArFarUpstreamMaterial") and
                     simFlags.LArFarUpstreamMaterial.statusOn and
                     simFlags.LArFarUpstreamMaterial.get_Value() ):
                    envelopeMap.update({'LARFARUPSTREAMMATERIAL::LARFARUPSTREAMMATERIAL':
                                        'LArFarUpstreamMaterialExitLayer'})
                if flags.Beam.Type is BeamType.Cosmics:
                    ## First filter volume
                    if simFlags.CosmicFilterVolumeName == "TRT_Barrel":
                        envelopeMap['TRT::BarrelOuterSupport'] = 'TRTBarrelEntryLayer'
                    elif simFlags.CosmicFilterVolumeName == "TRT_EC":
                        envelopeMap['TRT::WheelA'] = 'TRTECAEntryLayer'
                        envelopeMap['TRT::WheelB'] = 'TRTECBEntryLayer'
                    elif simFlags.CosmicFilterVolumeName == "SCT_Barrel":
                        envelopeMap['SCT::ThShieldOuterCly'] = 'SCTBarrelEntryLayer'# could be ThShieldInnerCly or Cyl..
                    elif simFlags.CosmicFilterVolumeName == "Pixel":
                        envelopeMap['Pixel::Pixel'] = 'PixelEntryLayer'
                    ## If second volume requested
                    if simFlags.CosmicFilterVolumeName2.statusOn:
                        if simFlags.CosmicFilterVolumeName2 == "TRT_Barrel":
                            envelopeMap['TRT::BarrelOuterSupport'] = 'TRTBarrelEntryLayer'
                        elif simFlags.CosmicFilterVolumeName2 == "TRT_EC":
                            envelopeMap['TRT::WheelA'] = 'TRTECAEntryLayer'
                            envelopeMap['TRT::WheelB'] = 'TRTECBEntryLayer'
                        elif simFlags.CosmicFilterVolumeName2 == "SCT_Barrel":
                            envelopeMap['SCT::ThShieldOuterCly'] = 'SCTBarrelEntryLayer'# could be ThShieldInnerCly or Cyl..
                        elif simFlags.CosmicFilterVolumeName2 == "Pixel":
                            envelopeMap['Pixel::Pixel'] = 'PixelEntryLayer'"""
    if not flags.Sim.ISFRun:
        if flags.Detector.GeometryID:
            envelopeMap['IDET::IDET'] = 'CaloEntryLayer'
        if flags.Detector.GeometryITk:
            envelopeMap['ITK::ITK'] = 'CaloEntryLayer'
        if flags.Detector.GeometryCalo:
            envelopeMap['CALO::CALO'] = 'MuonEntryLayer'
        if flags.Detector.GeometryMuon: #was geometry in old style, should it be?
            envelopeMap['MUONQ02::MUONQ02'] = 'MuonExitLayer'
    return envelopeMap


def MCTruthSteppingActionToolCfg(flags, name='G4UA::MCTruthSteppingActionTool', **kwargs):
    """Retrieve the MCTruthSteppingActionTool"""
    result = ComponentAccumulator()
    kwargs.setdefault("VolumeCollectionMap", getEnvelopeMap(flags))

    result.setPrivateTools( CompFactory.G4UA.MCTruthSteppingActionTool(name, **kwargs) )
    return result
