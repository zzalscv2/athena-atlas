/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file TrkVertexSeedFinderUtils/test/Trk2DDistanceFinder_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jun, 2019
 * @brief Unit test for Trk2DDistanceFinder.
 */


#include "TrkEventPrimitives/TrkObjectCounter.h"
#undef NDEBUG
#include "CxxUtils/ubsan_suppress.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ToolHandle.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "TInterpreter.h"
#include "TestTools/FLOATassert.h"
#include "TestTools/expect_exception.h"
#include "TestTools/initGaudi.h"
#include "TrkVertexSeedFinderUtils/ITrkDistanceFinder.h"
#include <cassert>
#include <cmath>
#include <iostream>

// for the field map
#include "PathResolver/PathResolver.h"
#include "TFile.h"
#include "TTree.h"

// for populating conditions store
#include "SGTools/TestStore.h"
#include "StoreGate/WriteCondHandle.h"
#include "StoreGate/WriteCondHandleKey.h"

// for the conditions data
#include "MagFieldElements/AtlasFieldCache.h"

std::ostream& printVec3D (const Amg::Vector3D& a)
{
  std::cout << a.x() << " " << a.y() << " "  << a.z();
  return std::cout;
}


void assertVec3D (const Amg::Vector3D& a, const Amg::Vector3D& b)
{
  assert( Athena_test::isEqual (a.x(), b.x(), 1e-5) );
  assert( Athena_test::isEqual (a.y(), b.y(), 1e-5) );
  assert( Athena_test::isEqual (a.z(), b.z(), 1e-5) );
}


// Very basic test.
void test1 (Trk::ITrkDistanceFinder& tool)
{
  std::cout << "test1\n";

  Amg::Vector3D pos1a { 2, 1, 0 };
  Amg::Vector3D mom1a { 400, 600, 200 };
  Amg::Vector3D pos1b { 1, 2, 0 };
  Amg::Vector3D mom1b { 600, 400, -200 };

  Trk::Perigee p1a (pos1a, mom1a,  1, pos1a);
  Trk::Perigee p1b (pos1b, mom1b, -1, pos1b);

  std::optional<Trk::ITrkDistanceFinder::TwoPoints> op =
    tool.CalculateMinimumDistance (p1a, p1b);
  assert( op );
  std::pair<Amg::Vector3D,Amg::Vector3D> pp = op.value();

  assertVec3D (pp.first,  { 4.01976, 4.01976, 0 });
  assertVec3D (pp.second, { 4.01976, 4.01976, 0 });

  //assert( ! tool.CalculateMinimumDistance (p1b, p1b) );

  Amg::Vector3D pos2a { 2, 1, 0 };
  Amg::Vector3D mom2a { 500, 500, 0 };
  Amg::Vector3D pos2b { 1, 2, 0 };
  Amg::Vector3D mom2b { 500, 500, 0 };

  Trk::Perigee p2a (pos2a, mom2a, -1, pos2a);
  Trk::Perigee p2b (pos2b, mom2b,  1, pos2b);

  op = tool.CalculateMinimumDistance (p2a, p2b);
  assert( op );
  pp = op.value();
  assertVec3D (pp.first,  {30.3934, 30.3934, 0 });
  assertVec3D(pp.second, { 30.3934, 30.3934, 0 });

  Amg::Vector3D pos3a { 10, 2, 2 };
  Amg::Vector3D mom3a { 10000, 30000, 50000 };
  Amg::Vector3D pos3b { 5, 5, -3 };
  Amg::Vector3D mom3b { 50000, 30000, -80000 };

  Trk::Perigee p3a (pos3a, mom3a,  1, pos3a);
  Trk::Perigee p3b (pos3b, mom3b, -1, pos3b);
  op = tool.CalculateMinimumDistance (p3a, p3b);
  assert( op );
  pp = op.value();
  assertVec3D (pp.first,  { 12.501, 9.50104, 0 });
  assertVec3D (pp.second, { 12.501, 9.50104, 0 });
}

std::unique_ptr<MagField::AtlasFieldMap> getFieldMap(const std::string& mapFile, double sol_current, double tor_current) {
       // find the path to the map file
    std::string resolvedMapFile = PathResolver::find_file( mapFile, "DATAPATH" );
    assert ( !resolvedMapFile.empty() );
    // Do checks and extract root file to initialize the map
    assert ( resolvedMapFile.find(".root") != std::string::npos );

    std::unique_ptr<TFile> rootfile( std::make_unique<TFile>(resolvedMapFile.c_str(), "OLD") );
    assert ( rootfile );
    assert ( rootfile->cd() );
    // open the tree
    TTree* tree = (TTree*)rootfile->Get("BFieldMap");
    assert(tree);

    // create map
    std::unique_ptr<MagField::AtlasFieldMap> field_map=std::make_unique<MagField::AtlasFieldMap>();

    // initialize map
    assert (field_map->initializeMap( rootfile.get(), sol_current, tor_current ));
    return field_map;

}

void createNewtonTrkDistanceFinderCondData(SGTest::TestStore &store) {
   SG::WriteCondHandleKey<AtlasFieldCacheCondObj> fieldKey {"fieldCondObj"};
   assert( fieldKey.initialize().isSuccess());

   // from StoreGate/test/WriteCondHandle_test.cxx
   EventIDBase now(0, EventIDBase::UNDEFEVT, 1);
   EventContext ctx(1, 1);
   ctx.setEventID( now );
   ctx.setExtension( Atlas::ExtendedEventContext(&store) );
   Gaudi::Hive::setCurrentContext(ctx);

   EventIDBase s1_1(0, EventIDBase::UNDEFEVT, 0);
   EventIDBase e1_1(0, EventIDBase::UNDEFEVT, 3);
   EventIDRange r1_1 (s1_1,e1_1);


   SG::WriteCondHandle<AtlasFieldCacheCondObj> fieldHandle {fieldKey};
   {
      std::unique_ptr<MagField::AtlasFieldMap> fieldMap=getFieldMap("MagneticFieldMaps/bfieldmap_7730_20400_14m.root",7730,20400);
      auto fieldCondObj = std::make_unique<AtlasFieldCacheCondObj>();
      fieldCondObj->initialize(1. /*solenoid current scale factor*/, 1. /*toroid current scale factor*/, fieldMap.release());
      assert( fieldHandle.record(r1_1, std::move(fieldCondObj)).isSuccess());
   }
}

int main()
{
  std::cout << "TrkVertexSeedFinderUtils/Trk2DDistanceFinder_test\n";
  CxxUtils::ubsan_suppress ([]() { TInterpreter::Instance(); });
  ISvcLocator* svcloc = nullptr;
  Athena_test::initGaudi ("TrkVertexSeedFinderUtils/TrkVertexSeedFinderUtils_tests.txt", svcloc);

  StoreGateSvc *cs=nullptr;
  assert (svcloc->service("StoreGateSvc/ConditionStore",cs).isSuccess());

  SGTest::TestStore dumstore;
  createNewtonTrkDistanceFinderCondData(dumstore);

  ToolHandle<Trk::ITrkDistanceFinder> tool ("Trk::Trk2DDistanceFinder");
  assert( tool.retrieve().isSuccess() );

  test1 (*tool);

  return 0;
}
