/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @author Shaun Roe
 * @date Nov 2022
 * @brief Some tests for SurfaceConsistencyCheck
 */
 
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_TRKTRACK_UTILITIES

#include <boost/test/unit_test.hpp>
#include "TrkEventPrimitives/SurfaceConsistencyCheck.h"

#include "CxxUtils/checker_macros.h"
#include <memory>
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

typedef int Surface;

//convenient way to generate different types with the same methods and holding a pSurf
template<int N>
struct Thing{
  Surface  surf{};
  const Surface & associatedSurface() const{
   return surf;
  }
  int n(){
    return N;
  }
};



BOOST_AUTO_TEST_SUITE(ConsistentSurfacesTest)
  BOOST_AUTO_TEST_CASE(AllTheSame){
    Surface surf(1);
    {
      auto pThing1 = std::make_unique<Thing<1>>();
      auto pThing2 = std::make_unique<Thing<2>>();
      auto pThing3 = std::make_unique<Thing<3>>();

      pThing1->surf = surf;
      pThing2->surf = surf;
      pThing3->surf = surf;
      BOOST_CHECK(Trk::consistentSurfaces(pThing1.get(), pThing2.get(), pThing3.get()));
    }
  }
  
  BOOST_AUTO_TEST_CASE(OneDifferent){
    Surface surf1(1);
    Surface surf2(2);
    {
      auto pThing1 = std::make_unique<Thing<1>>();
      auto pThing2 = std::make_unique<Thing<2>>();
      auto pThing3 = std::make_unique<Thing<3>>();

      pThing1->surf = surf1;
      pThing2->surf = surf1;
      pThing3->surf = surf2;
      BOOST_CHECK(Trk::consistentSurfaces(pThing1.get(), pThing2.get(), pThing3.get()) == false);
      //check order doesnt matter
      BOOST_CHECK(Trk::consistentSurfaces(pThing3.get(), pThing2.get(), pThing1.get()) == false);
    }
  }
  BOOST_AUTO_TEST_CASE(TheSameAndOneNullPtr){
    Surface surf1(1);
    {
      std::unique_ptr<Thing<1>> pThing1{}; //nullptr
      auto pThing2 = std::make_unique<Thing<2>>();
      auto pThing3 = std::make_unique<Thing<3>>();
      //
      pThing2->surf = surf1;
      pThing3->surf = surf1;
      BOOST_CHECK(Trk::consistentSurfaces(pThing1.get(), pThing2.get(), pThing3.get()));
    }
  }
  BOOST_AUTO_TEST_CASE(OneDifferentAndOneNullPtr){
    Surface surf1(1);
    Surface surf2(2);
    {
      std::unique_ptr<Thing<1>> pThing1{}; //nullptr
      auto pThing2 = std::make_unique<Thing<2>>();
      auto pThing3 = std::make_unique<Thing<3>>();

      pThing2->surf = surf1;
      pThing3->surf = surf2;
      BOOST_CHECK(Trk::consistentSurfaces(pThing1.get(), pThing2.get(), pThing3.get()) == false);
      //check order doesnt matter
      BOOST_CHECK(Trk::consistentSurfaces(pThing3.get(), pThing2.get(), pThing1.get()) == false);
    }
  }
  BOOST_AUTO_TEST_CASE(AllNullPtr){
    {
      std::unique_ptr<Thing<1>> pThing1{}; //nullptr
      std::unique_ptr<Thing<2>> pThing2{};
      std::unique_ptr<Thing<3>> pThing3{};

      BOOST_CHECK(Trk::consistentSurfaces(pThing1.get(), pThing2.get(), pThing3.get()));
    }
  }
  BOOST_AUTO_TEST_CASE(OnlyOneNonNullPtr){
    Surface surf1(1);
    {
      std::unique_ptr<Thing<2>> pThing2{};
      auto pThing3 = std::make_unique<Thing<3>>();
      pThing3->surf = surf1;
      //..and just two arguments this time
      BOOST_CHECK(Trk::consistentSurfaces(pThing2.get(), pThing3.get()));
    }
  }
  

BOOST_AUTO_TEST_SUITE_END()

