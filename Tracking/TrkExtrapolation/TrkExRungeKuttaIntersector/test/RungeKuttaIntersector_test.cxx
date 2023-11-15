/*
 * Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */
/**
 * @file TrkExRungeKuttaIntersector/test/RungeKuttaIntersector_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2019
 * @brief Regression tests for RungeKuttaIntersector
 */

#undef NDEBUG
#include "TrkExInterfaces/IIntersector.h"
#include "TestTools/initGaudi.h"
#include "TestTools/FLOATassert.h"
#include "AthenaKernel/Units.h"
#include "CxxUtils/ubsan_suppress.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/IIncidentListener.h"
#include "TInterpreter.h"
#include <iostream>
#include <cassert>
#include <cmath>

// for the field map
#include "PathResolver/PathResolver.h"
#include "TFile.h"
#include "TTree.h"

// for populating conditions store
#include "SGTools/TestStore.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "StoreGate/WriteCondHandle.h"

// for the conditions data
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"

using Athena::Units::meter;
using Athena::Units::GeV;


void assertVec3D (const Amg::Vector3D& a, const Amg::Vector3D& b)
{
  assert( Athena_test::isEqual (a.x(), b.x()) );
  assert( Athena_test::isEqual (a.y(), b.y()) );
  assert( Athena_test::isEqual (a.z(), b.z()) );
}


Amg::Vector3D unit (double x, double y, double z)
{
  return Amg::Vector3D (x, y, z).unit();
}


std::unique_ptr<Amg::Transform3D> transf (const Amg::Vector3D& pos,
                                          const Amg::Vector3D& norm)
{
  Trk::CurvilinearUVT c (norm);
  Amg::RotationMatrix3D curvilinearRotation;
  curvilinearRotation.col(0) = c.curvU();
  curvilinearRotation.col(1) = c.curvV();
  curvilinearRotation.col(2) = c.curvT();
  auto transf = std::make_unique<Amg::Transform3D>();
  *transf = curvilinearRotation;
  transf->pretranslate(pos);
  return transf;
}



void test_plane (Trk::IIntersector& tool)
{
  std::cout << "test_plane\n";
  Amg::Vector3D pos1 { 0, 0, 5*meter };
  Amg::Vector3D norm1 { 0, 1, 1 };
  Trk::PlaneSurface plane1 (*transf (pos1, norm1));
  Amg::Vector3D pos2 { 0, 0, 10*meter };
  Trk::PlaneSurface plane2 (*transf (pos2, norm1));
  
  Trk::TrackSurfaceIntersection isect0
    (Amg::Vector3D{0,0,0}, unit(1,1,1), 0);

  std::optional<Trk::TrackSurfaceIntersection> isect1
    (tool.intersectSurface (plane1, isect0, 1/(10*GeV)));
  assertVec3D (isect1->position(), {2773.12, 2402.05, 2597.95});
  assertVec3D (isect1->direction(), {0.620947, 0.527586, 0.579722});
  assert( Athena_test::isEqual (isect1->pathlength(), 4496.37) );

  std::optional<Trk::TrackSurfaceIntersection> isect1a
    (tool.intersectSurface (plane2, *isect1, 1/(10*GeV)));
  assertVec3D (isect1a->position(), {5497.63, 4749.85, 5250.15});
  assertVec3D (isect1a->direction(), {0.596962, 0.513091, 0.616745});
  assert( Athena_test::isEqual (isect1a->pathlength(), 8965.55) );

  Trk::TrackSurfaceIntersection isect2
    (Amg::Vector3D{0,0,0}, unit(0,0,-1), 0);
  std::optional<Trk::TrackSurfaceIntersection> isect3
    (tool.intersectSurface (plane1, isect2, 1/(10*GeV)));
  assertVec3D (isect3->position(), {-0.1490986, -0.287689, 5000.29});
  assertVec3D (isect3->direction(), {2.288634e-05, 0.000105063, -1});
  assert( Athena_test::isEqual (isect3->pathlength(), -5000.29) );

  std::optional<Trk::TrackSurfaceIntersection> isect3a
    (tool.intersectSurface (plane2, *isect3, 1/(10*GeV)));
  assertVec3D (isect3a->position(), {-0.35816, -0.796257, 10000.8});
  assertVec3D (isect3a->direction(), {5.16553e-05, 0.0001023313, -1});
  assert( Athena_test::isEqual (isect3a->pathlength(), -10000.8) );

  // Loopers.
  Trk::TrackSurfaceIntersection isect4
    (Amg::Vector3D{0,0,0}, unit(1,0,-1), 0);
  std::optional<Trk::TrackSurfaceIntersection> isect4a
    (tool.intersectSurface (plane1, isect4, 1/(0.01*GeV)));
  assert (!isect4a);

  Trk::TrackSurfaceIntersection isect5
    (Amg::Vector3D{0,0,0}, unit(1,0,-1), 0);
  std::optional<Trk::TrackSurfaceIntersection> isect6
    (tool.intersectSurface (plane2, isect5, 1/(0.03*GeV)));
  assert (!isect6);
}


void test_line (Trk::IIntersector& tool)
{
  std::cout << "test_line\n";
  Amg::Vector3D pos1 { 0, 0, 5*meter };
  Amg::Vector3D norm1 { 0, 1, 0 };
  Trk::StraightLineSurface line1 (*transf (pos1, norm1));
  Amg::Vector3D pos2 { 0, 0, 10*meter };
  Trk::StraightLineSurface line2 (*transf (pos2, norm1));

  Trk::TrackSurfaceIntersection isect0
    (Amg::Vector3D{0,0,0}, unit(1,0,1), 0);

  std::optional<const Trk::TrackSurfaceIntersection> isect1
    (tool.intersectSurface (line1, isect0, 1/(10*GeV)));
  assertVec3D (isect1->position(), {2502.7, -185.58, 2511.46});
  assertVec3D (isect1->direction(), { 0.703713, -0.0626336, 0.707718});
  assert( Athena_test::isEqual (isect1->pathlength(), 3551.23) );

  std::optional<Trk::TrackSurfaceIntersection> isect2
    (tool.intersectSurface (line2, *isect1, 1/(10*GeV)));
  assertVec3D (isect2->position(), {5059.02, -413.306, 5102.03});
  assertVec3D (isect2->direction(), { 0.693974, -0.0678801, 0.716793});
  assert( Athena_test::isEqual (isect2->pathlength(), 7197.76) );

  // Loopers.
  Trk::TrackSurfaceIntersection isect3
    (Amg::Vector3D{0,0,0}, unit(1,0,1), 0);
  std::optional<Trk::TrackSurfaceIntersection> isect4
    (tool.intersectSurface (line2, isect3, 1/(0.01*GeV)));
  assert (!isect4);

  Trk::TrackSurfaceIntersection isect5
    (Amg::Vector3D{0,0,0}, unit(1,0,1), 0);
  std::optional<Trk::TrackSurfaceIntersection> isect6
    (tool.intersectSurface (line2, isect5, 1/(0.03*GeV)));
  assert (!isect6);
}


void test_cylinder (Trk::IIntersector& tool)
{
  std::cout << "test_cylinder\n";

  Amg::Vector3D pos1 { 0, 0, 0 };
  Amg::Vector3D norm1 { 0, 0, 1 };
  Trk::CylinderSurface cyl1 (*transf (pos1, norm1).release(), 2*meter, 10*meter);
  Trk::CylinderSurface cyl2 (*transf (pos1, norm1).release(), 5*meter, 10*meter);

  Trk::TrackSurfaceIntersection isect0
    (Amg::Vector3D{0,0,0}, unit(1,0,1), 0);

  std::optional<Trk::TrackSurfaceIntersection> isect1
    (tool.intersectSurface (cyl1, isect0, 1/(10*GeV)));
  assertVec3D (isect1->position(), {1995.16, -139.11, 2001.34});
  assertVec3D (isect1->direction(), {0.703789, -0.0663505, 0.707304});
  assert( Athena_test::isEqual (isect1->pathlength(), 2830.13) );

  std::optional<Trk::TrackSurfaceIntersection> isect2
    (tool.intersectSurface (cyl2, *isect1, 1/(10*GeV)));
  assertVec3D (isect2->position(), {4983.49, -405.934, 5024.11});
  assertVec3D (isect2->direction(), {0.694755, -0.0676002, 0.716063});
  assert( Athena_test::isEqual (isect2->pathlength(), 7089.10) );

  // Loopers.
  Trk::TrackSurfaceIntersection isect3
    (Amg::Vector3D{0,0,0}, unit(1,0,1), 0);
  std::optional<Trk::TrackSurfaceIntersection> isect4
    (tool.intersectSurface (cyl1, isect3, 1/(0.01*GeV)));
  assert (!isect4);

  Trk::TrackSurfaceIntersection isect5
    (Amg::Vector3D{0,0,0}, unit(1,0,1), 0);
  std::optional<Trk::TrackSurfaceIntersection> isect6
    (tool.intersectSurface (cyl1, isect5, 1/(0.03*GeV)));
  assert (!isect6);
}


void test_disc (Trk::IIntersector& tool)
{
  std::cout << "test_disc\n";

  Amg::Vector3D pos1 { 0, 0, 3*meter };
  Amg::Vector3D norm1 { 0, 0, 1 };
  Trk::DiscSurface disc1 (*transf (pos1, norm1));
  Amg::Vector3D pos2 { 0, 0, 6*meter };
  Trk::DiscSurface disc2 (*transf (pos2, norm1));

  Trk::TrackSurfaceIntersection isect0
    (Amg::Vector3D{0,0,0}, unit(1,0,1), 0);

  std::optional<Trk::TrackSurfaceIntersection> isect1
    (tool.intersectSurface (disc1, isect0, 1/(10*GeV)));
  assertVec3D (isect1->position(), {2988.08, -227.624, 3000});
  assertVec3D (isect1->direction(), { 0.703426, -0.0598764, 0.708242});
  assert( Athena_test::isEqual (isect1->pathlength(), 4241.17) );

  std::optional<Trk::TrackSurfaceIntersection> isect2
    (tool.intersectSurface (disc2, *isect1, 1/(10*GeV)));
  assertVec3D (isect2->position(), {5915.1, -499.253, 6000});
  assertVec3D (isect2->direction(), {0.682016, -0.0699434, 0.727985});
  assert( Athena_test::isEqual (isect2->pathlength(), 8441.51) );

  // Loopers.
  Trk::TrackSurfaceIntersection isect3
    (Amg::Vector3D{0,0,0}, unit(1,0,1), 0);
  std::optional<Trk::TrackSurfaceIntersection> isect4
    (tool.intersectSurface (disc1, isect3, 1/(0.01*GeV)));
  assert (!isect4);

  Trk::TrackSurfaceIntersection isect5
    (Amg::Vector3D{0,0,0}, unit(100,0,1), 0);
  std::optional<Trk::TrackSurfaceIntersection> isect6
    (tool.intersectSurface (disc2, isect5, 1/(0.03*GeV)));
  assert (!isect6);
}


void test_perigee (Trk::IIntersector& tool)
{
  std::cout << "test_perigee\n";

  Amg::Vector3D pos1 { 0, 0, 4*meter };
  Amg::Vector3D norm1 { 0, 1, 0 };
  Trk::PerigeeSurface perigee1 (*transf (pos1, norm1));
  Amg::Vector3D pos2 { 0, 0, 8*meter };
  Trk::PerigeeSurface perigee2 (*transf (pos2, norm1));

  Trk::TrackSurfaceIntersection isect0
    (Amg::Vector3D{0,0,0}, unit(1,0,1), 0);

  std::optional<Trk::TrackSurfaceIntersection> isect1
    (tool.intersectSurface (perigee1, isect0, 1/(10*GeV)));
  assertVec3D (isect1->position(), {2001.86, -139.7423, 2008.08});
  assertVec3D (isect1->direction(), { 0.703793, -0.0662967, 0.707305});
  assert( Athena_test::isEqual (isect1->pathlength(), 2839.65) );

  std::optional<Trk::TrackSurfaceIntersection> isect2
    (tool.intersectSurface (perigee2, *isect1, 1/(10*GeV)));
  assertVec3D (isect2->position(), {4011.21, -314.772, 4031.51});
  assertVec3D (isect2->direction(), {0.70198, -0.0614824, 0.709537});
  assert( Athena_test::isEqual (isect2->pathlength(), 5696.65) );

  // Loopers.
  Trk::TrackSurfaceIntersection isect3
    (Amg::Vector3D{0,0,0}, unit(1,0,1), 0);
  std::optional<Trk::TrackSurfaceIntersection> isect4
    (tool.intersectSurface (perigee1, isect3, 1/(0.01*GeV)));
  assert (!isect4);

  Trk::TrackSurfaceIntersection isect5
    (Amg::Vector3D{0,0,0}, unit(1,0,1), 0);
  std::optional<Trk::TrackSurfaceIntersection> isect6
    (tool.intersectSurface (perigee1, isect5, 1/(0.03*GeV)));
  assert (!isect6);
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

void createAtlasFieldCacheCondObj(SGTest::TestStore &store) {
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
   std::unique_ptr<MagField::AtlasFieldMap> fieldMap=getFieldMap("MagneticFieldMaps/bfieldmap_7730_20400_14m.root",7730,20400);
   auto fieldCondObj = std::make_unique<AtlasFieldCacheCondObj>();
   fieldCondObj->initialize(1. /*solenoid current scale factor*/, 1. /*toroid current scale factor*/, fieldMap.release());
   assert( fieldHandle.record(r1_1, std::move(fieldCondObj)).isSuccess());
}

int main()
{
  std::cout << "RungeKuttaIntersector_test\n";
  CxxUtils::ubsan_suppress ([]() { TInterpreter::Instance(); });
  ISvcLocator* svcloc = nullptr;
  Athena_test::initGaudi ("RungeKuttaIntersector_test.txt", svcloc);

  StoreGateSvc *cs=nullptr;
  assert (svcloc->service("StoreGateSvc/ConditionStore",cs).isSuccess());

  SGTest::TestStore dumstore;
  createAtlasFieldCacheCondObj(dumstore);

  ToolHandle<Trk::IIntersector> tool ("Trk::RungeKuttaIntersector");
  assert( tool.retrieve().isSuccess() );

  test_plane (*tool);
  test_line (*tool);
  test_cylinder (*tool);
  test_disc (*tool);
  test_perigee (*tool);
  return 0;
}
