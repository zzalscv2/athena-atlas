/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_DICT_H
#define ACTSTRKEVENT_DICT_H 1

#include "AthLinks/DataLink.h"
#include "AthLinks/ElementLink.h"

#define INSTANTIATE_TYPES( NS, TYPE ) \
  NS::TYPE dummy_##NS##_##TYPE##_1;                                     \
  DataLink< NS::TYPE > dummy_##NS##_##TYPE##_2;                         \
  std::vector< DataLink< NS::TYPE > > dummy_##NS##_##TYPE##_3;		

#define INSTANTIATE_CONTAINER_TYPES( NS, TYPE )				\
  NS::TYPE dummy_##NS##_##TYPE##_1;					\
  DataLink< NS::TYPE > dummy_##NS##_##TYPE##_2;				\
  ElementLink< NS::TYPE > dummy_##NS##_##TYPE##_3;			\
  std::vector< DataLink< NS::TYPE > > dummy_##NS##_##TYPE##_4;		\
  std::vector< ElementLink< NS::TYPE > > dummy_##NS##_##TYPE##_5;	\
  std::vector< std::vector< ElementLink< NS::TYPE > > >			\
  dummy_##NS##_##TYPE##_6;  

#include "ActsTrkEvent/Seed.h"
#include "ActsTrkEvent/TrackParameters.h"

// Instantiate all necessary types for the dictionary.
namespace {
  struct GCCXML_DUMMY_INSTANTIATION_ACTSEDM {
    INSTANTIATE_CONTAINER_TYPES( ActsTrk, SeedContainer )
    INSTANTIATE_CONTAINER_TYPES( ActsTrk, BoundTrackParametersContainer )
  };
}


#endif
