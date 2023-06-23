# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

CSV_DictFormats = {
                    #All obtained AuxElement content using the HitsToxAODCopier tool
                    "PixelHits": { "col": "int", "row": "int", "tot": "int", "eta_module": "int", "phi_module": "int", "layer_disk": "int", "barrel_ec": "int", "detid": "unsigned long"},
                    "StripHits": { "strip": "int", "side": "int", "eta_module": "int", "phi_module": "int", "layer_disk": "int", "barrel_ec": "int", "detid": "unsigned long"},
                    
                    #https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODInDetMeasurement/xAODInDetMeasurement/versions/PixelClusterAuxContainer_v1.h
                    "ITkPixelClusters": {"globalPosition":"std::array<float,3>", "channelsInPhi":"int", "channelsInEta":"int", "widthInEta":"float", "omegaX":"float", "omegaY":"float", "totalToT":"int", "totalCharge":"float",   "energyLoss":"float", "splitProbability1":"float", "splitProbability2":"float", "lvl1a":"int"},
                    # ITkPixelClusters.isSplit is char with all ''? Keeping out for the moment

                    #https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODInDetMeasurement/xAODInDetMeasurement/versions/StripClusterAuxContainer_v1.h
                    "ITkStripClusters": {"globalPosition":"std::array<float,3>", "channelsInPhi":"int"},

                    #https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODInDetMeasurement/xAODInDetMeasurement/versions/SpacePoint_v1.h
                    "ITkStripSpacePoints": {"globalPosition":"std::array<float,3>", "radius":"float", "varianceR":"float", "varianceZ":"float", "topHalfStripLength":"float", "bottomHalfStripLength":"float", "topStripDirection":"xAOD::ArrayFloat3", "bottomStripDirection":"xAOD::ArrayFloat3", "stripCenterDistance":"xAOD::ArrayFloat3", "topStripCenter":"xAOD::ArrayFloat3"},

                    #https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODInDetMeasurement/xAODInDetMeasurement/versions/SpacePoint_v1.h
                    "ITkPixelSpacePoints": {"globalPosition":"std::array<float,3>", "radius":"float", "varianceR":"float", "varianceZ":"float"},

                    #https://gitlab.cern.ch/atlas/athena/-/blob/master/Event/xAOD/xAODTracking/xAODTracking/versions/TrackParticle_v1.h
                    "InDetTrackParticles": {"pt":"float", "eta":"float", "phi":"float", "charge":"float", "d0":"float", "z0":"float"},

                }