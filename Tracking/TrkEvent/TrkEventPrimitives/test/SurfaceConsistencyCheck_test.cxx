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

//dummy surface to add
struct Surface{
};

//convenient way to generate different types with the same methods and holding a pSurf
template<int N>
struct Thing{
  Surface  * pSurf{};
  Surface & associatedSurface() const{
   return *pSurf;
  }
  int n(){
    return N;
  }
};



BOOST_AUTO_TEST_SUITE(ConsistentSurfacesTest)
  BOOST_AUTO_TEST_CASE(AllTheSame){
    auto pSurf = new Surface;
    {
      auto pThing1 = std::make_unique<Thing<1>>();
      auto pThing2 = std::make_unique<Thing<2>>();
      auto pThing3 = std::make_unique<Thing<3>>();

      pThing1->pSurf = pSurf;
      pThing2->pSurf = pSurf;
      pThing3->pSurf = pSurf;
      BOOST_CHECK(Trk::consistentSurfaces(pThing1.get(), pThing2.get(), pThing3.get()));
    }
    delete pSurf;
  }
  
  BOOST_AUTO_TEST_CASE(OneDifferent){
    auto pSurf1 = new Surface;
    auto pSurf2 = new Surface;
    {
      auto pThing1 = std::make_unique<Thing<1>>();
      auto pThing2 = std::make_unique<Thing<2>>();
      auto pThing3 = std::make_unique<Thing<3>>();

      pThing1->pSurf = pSurf1;
      pThing2->pSurf = pSurf1;
      pThing3->pSurf = pSurf2;
      BOOST_CHECK(Trk::consistentSurfaces(pThing1.get(), pThing2.get(), pThing3.get()) == false);
      //check order doesnt matter
      BOOST_CHECK(Trk::consistentSurfaces(pThing3.get(), pThing2.get(), pThing1.get()) == false);
    }
    delete pSurf1;
    delete pSurf2;
  }
  BOOST_AUTO_TEST_CASE(TheSameAndOneNullPtr){
    auto pSurf1 = new Surface;
    {
      std::unique_ptr<Thing<1>> pThing1{}; //nullptr
      auto pThing2 = std::make_unique<Thing<2>>();
      auto pThing3 = std::make_unique<Thing<3>>();
      //
      pThing2->pSurf = pSurf1;
      pThing3->pSurf = pSurf1;
      BOOST_CHECK(Trk::consistentSurfaces(pThing1.get(), pThing2.get(), pThing3.get()));
    }
    delete pSurf1;
  }
  BOOST_AUTO_TEST_CASE(OneDifferentAndOneNullPtr){
    auto pSurf1 = new Surface;
    auto pSurf2 = new Surface;
    {
      std::unique_ptr<Thing<1>> pThing1{}; //nullptr
      auto pThing2 = std::make_unique<Thing<2>>();
      auto pThing3 = std::make_unique<Thing<3>>();

      pThing2->pSurf = pSurf1;
      pThing3->pSurf = pSurf2;
      BOOST_CHECK(Trk::consistentSurfaces(pThing1.get(), pThing2.get(), pThing3.get()) == false);
      //check order doesnt matter
      BOOST_CHECK(Trk::consistentSurfaces(pThing3.get(), pThing2.get(), pThing1.get()) == false);
    }
    delete pSurf1;
    delete pSurf2;
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
    auto pSurf1 = new Surface;
    {
      std::unique_ptr<Thing<2>> pThing2{};
      auto pThing3 = std::make_unique<Thing<3>>();
      pThing3->pSurf = pSurf1;
      //..and just two arguments this time
      BOOST_CHECK(Trk::consistentSurfaces(pThing2.get(), pThing3.get()));
    }
  }
  

BOOST_AUTO_TEST_SUITE_END()

