# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaConfiguration.AutoConfigFlags import GetFileMD, DetDescrInfo
from AthenaConfiguration.Enums import LHCPeriod, ProductionStep, Project

def createGeoModelConfigFlags(analysis=False):
    gcf=AthConfigFlags()

    def __getTrigTag(flags):
        from TriggerJobOpts.TriggerConfigFlags import trigGeoTag
        return trigGeoTag(flags)

    gcf.addFlag("GeoModel.AtlasVersion", lambda flags :
                (__getTrigTag(flags) if flags.Trigger.doLVL1 or flags.Trigger.doHLT else None) or
                GetFileMD(flags.Input.Files).get("GeoAtlas", None) or
                "ATLAS-R2-2016-01-00-01")

    # Special handling of analysis releases where we only want AtlasVersion and Run
    if analysis:
        def _deduct_LHCPeriod(prevFlags):
            import logging
            log = logging.getLogger("GeoModelConfigFlags")
            log.info('Deducing LHC Run period from the geometry tag name "%s" as database access is not available in analysis releases', prevFlags.GeoModel.AtlasVersion)

            if prevFlags.GeoModel.AtlasVersion.startswith("ATLAS-R1"):
                period = LHCPeriod.Run1
            elif prevFlags.GeoModel.AtlasVersion.startswith("ATLAS-R2"):
                period = LHCPeriod.Run2
            elif prevFlags.GeoModel.AtlasVersion.startswith("ATLAS-R3"):
                period = LHCPeriod.Run3
            else:
                raise ValueError(f'Can not deduct LHC Run period from "{prevFlags.GeoModel.AtlasVersion}", please set "flags.GeoModel.Run" manually.')

            log.info('Using LHC Run period "%s"', period.value)
            return period

        gcf.addFlag("GeoModel.Run",  # Run deducted from other metadata
                    _deduct_LHCPeriod, type=LHCPeriod)
        return gcf

    gcf.addFlag("GeoModel.Run",  # Run from the geometry database
                lambda prevFlags : LHCPeriod(DetDescrInfo(prevFlags.GeoModel.AtlasVersion)['Common']['Run']),
                type=LHCPeriod)

    gcf.addFlag('GeoModel.Layout', 'atlas') # replaces global.GeoLayout

    gcf.addFlag("GeoModel.Align.Dynamic",
                lambda prevFlags : prevFlags.GeoModel.Run >= LHCPeriod.Run2 and not prevFlags.Input.isMC)
                # TODO: dynamic alignment is for now enabled by default for data overlay
                # to disable, add 'and prevFlags.Common.ProductionStep not in [ProductionStep.Simulation, ProductionStep.Overlay]'

    gcf.addFlag("GeoModel.Align.LegacyConditionsAccess",
                lambda prevFlags : prevFlags.Common.Project is Project.AthSimulation or prevFlags.Common.ProductionStep is ProductionStep.Simulation)
                # Mainly for G4 which still loads alignment on initialize

    gcf.addFlag("GeoModel.Type",
                lambda prevFlags : DetDescrInfo(prevFlags.GeoModel.AtlasVersion)['Common']['GeoType'])
                # Geometry type in {ITKLoI, ITkLoI-VF, etc...}

    gcf.addFlag("GeoModel.IBLLayout",
                lambda prevFlags : DetDescrInfo(prevFlags.GeoModel.AtlasVersion)['Pixel']['IBLlayout'])
                # IBL layer layout  in {"planar", "3D", "noIBL"}

    gcf.addFlag('GeoModel.SQLiteDB','')
                # Path to persistent GeoModel description file in SQLite format

    gcf.addFlag('GeoModel.IgnoreTagDifference',False)

    return gcf
