/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETPATTRECOTOOLS_TRIG_TRACK_SEED_GENERATOR_ITK_H
#define TRIGINDETPATTRECOTOOLS_TRIG_TRACK_SEED_GENERATOR_ITK_H

#include<vector>
#include<algorithm>

#include "TrigInDetPattRecoTools/TrigCombinatorialSettings.h"

class TrigSiSpacePointBase;
class TrigInDetTriplet;
class TrigFTF_GNN_DataStorage;
class IRoiDescriptor;

typedef std::vector<std::pair<float, TrigInDetTriplet*> > INTERNAL_TRIPLET_BUFFER;

typedef class TrigTrackSeedGeneratorITk {

 public:

  TrigTrackSeedGeneratorITk(const TrigCombinatorialSettings&);
  ~TrigTrackSeedGeneratorITk(); 

  TrigTrackSeedGeneratorITk(const TrigTrackSeedGeneratorITk&) = delete;
  TrigTrackSeedGeneratorITk& operator=(const TrigTrackSeedGeneratorITk&) = delete;

  void loadSpacePoints(const std::vector<TrigSiSpacePointBase>&);
  void createSeeds(const IRoiDescriptor*);
  void createSeedsZv();
  void getSeeds(std::vector<TrigInDetTriplet>&);

private:

  void storeTriplets(INTERNAL_TRIPLET_BUFFER&);

  TrigFTF_GNN_DataStorage* m_storage;

  const TrigCombinatorialSettings& m_settings;
  float m_phiSliceWidth;
  float m_minDeltaRadius, m_maxDeltaRadius;

  float m_minR_squ, m_maxCurv;

  INTERNAL_TRIPLET_BUFFER m_triplets;

} TRIG_TRACK_SEED_GENERATOR_ITK;



#endif 
