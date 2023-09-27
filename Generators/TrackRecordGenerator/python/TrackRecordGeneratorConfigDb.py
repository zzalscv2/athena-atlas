# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CfgGetter import addAlgorithm

addAlgorithm("TrackRecordGenerator.TrackRecordGeneratorConfigLegacy.getInput_TrackRecordGenerator", "TrackRecordGenerator")
addAlgorithm("TrackRecordGenerator.TrackRecordGeneratorConfigLegacy.getTrackRecordCosmicGenerator", "TrackRecordCosmicGenerator")
