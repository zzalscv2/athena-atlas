################################################################################
# Package: MuonSimEventTPCnv
################################################################################

# Declare the package name:
atlas_subdir( MuonSimEventTPCnv )

# External dependencies:
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO pthread )

# Component(s) in the package:
atlas_add_library( MuonSimEventTPCnv
                   src/*.cxx
                   PUBLIC_HEADERS MuonSimEventTPCnv
                   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                   LINK_LIBRARIES GaudiKernel GeneratorObjectsTPCnv MuonSimEvent AthenaPoolCnvSvcLib StoreGateLib
                   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} TestTools GeoPrimitives )

atlas_add_dictionary( MuonSimEventTPCnvDict
                      MuonSimEventTPCnv/MuonSimEventTPCnvDict.h
                      MuonSimEventTPCnv/selection.xml
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv )

atlas_add_dictionary( OLD_MuonSimEventTPCnvDict
                      MuonSimEventTPCnv/OLD_MuonSimEventTPCnvDict.h
                      MuonSimEventTPCnv/OLD_selection.xml
                      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                      LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv )

atlas_add_test( CSCSimHitCollectionCnv_p1_test
                SOURCES
                test/CSCSimHitCollectionCnv_p1_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( CSCSimHitCollectionCnv_p2_test
                SOURCES
                test/CSCSimHitCollectionCnv_p2_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( CSCSimHitCollectionCnv_p3_test
                SOURCES
                test/CSCSimHitCollectionCnv_p3_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( GenericMuonSimHitCollectionCnv_p1_test
                SOURCES
                test/GenericMuonSimHitCollectionCnv_p1_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( GenericMuonSimHitCollectionCnv_p2_test
                SOURCES
                test/GenericMuonSimHitCollectionCnv_p2_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( MDTSimHitCollectionCnv_p1_test
                SOURCES
                test/MDTSimHitCollectionCnv_p1_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} 
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( MDTSimHitCollectionCnv_p2_test
                SOURCES
                test/MDTSimHitCollectionCnv_p2_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( MDTSimHitCollectionCnv_p3_test
                SOURCES
                test/MDTSimHitCollectionCnv_p3_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( RPCSimHitCollectionCnv_p1_test
                SOURCES
                test/RPCSimHitCollectionCnv_p1_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( RPCSimHitCollectionCnv_p2_test
                SOURCES
                test/RPCSimHitCollectionCnv_p2_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( RPCSimHitCollectionCnv_p3_test
                SOURCES
                test/RPCSimHitCollectionCnv_p3_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( TGCSimHitCollectionCnv_p1_test
                SOURCES
                test/TGCSimHitCollectionCnv_p1_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( TGCSimHitCollectionCnv_p2_test
                SOURCES
                test/TGCSimHitCollectionCnv_p2_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( TGCSimHitCollectionCnv_p3_test
                SOURCES
                test/TGCSimHitCollectionCnv_p3_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( TGCSimHitCollectionCnv_p4_test
                SOURCES
                test/TGCSimHitCollectionCnv_p4_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( sTGCSimHitCollectionCnv_p1_test
                SOURCES
                test/sTGCSimHitCollectionCnv_p1_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( sTGCSimHitCollectionCnv_p2_test
                SOURCES
                test/sTGCSimHitCollectionCnv_p2_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( sTGCSimHitCollectionCnv_p3_test
                SOURCES
                test/sTGCSimHitCollectionCnv_p3_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( MMSimHitCollectionCnv_p1_test
                SOURCES
                test/MMSimHitCollectionCnv_p1_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )

atlas_add_test( MMSimHitCollectionCnv_p2_test
                SOURCES
                test/MMSimHitCollectionCnv_p2_test.cxx
                INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
                LINK_LIBRARIES ${ROOT_LIBRARIES} AthenaPoolCnvSvcLib GaudiKernel GeneratorObjectsTPCnv MuonSimEvent TestTools StoreGateLib GeoPrimitives MuonSimEventTPCnv CxxUtils )
