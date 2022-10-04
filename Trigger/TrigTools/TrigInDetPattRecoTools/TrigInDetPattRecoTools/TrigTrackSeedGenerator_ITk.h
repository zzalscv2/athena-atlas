/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
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

  void loadSpacePoints(const std::vector<TrigSiSpacePointBase>&);
  void createSeeds(const IRoiDescriptor*);
  void createSeedsZv();
  void getSeeds(std::vector<TrigInDetTriplet>&);

private:

  void storeTriplets(INTERNAL_TRIPLET_BUFFER&);

  TrigFTF_GNN_DataStorage* m_storage;

  const TrigCombinatorialSettings& m_settings;
  float m_phiSliceWidth;
  float m_minDeltaRadius, m_maxDeltaRadius, m_zTol;

  float m_CovMS, m_minR_squ, m_dtPreCut;

  INTERNAL_TRIPLET_BUFFER m_triplets;

} TRIG_TRACK_SEED_GENERATOR_ITK;



#endif 
