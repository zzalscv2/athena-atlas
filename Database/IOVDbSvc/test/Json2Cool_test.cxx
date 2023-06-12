/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file IOVDbSvc/test/Json2Cool_test.cxx
 * @author Shaun Roe
 * @date May, 2019
 * @brief Some tests for Json2Cool class in the Boost framework
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IOVDBSVC


#include <boost/test/unit_test.hpp>

#include <boost/test/tools/output_test_stream.hpp>

#include "../src/Json2Cool.h"
#include "../src/BasicFolder.h"

#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeListSpecification.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoolKernel/StorageType.h"
//
#include "CoolKernel/RecordSpecification.h"
#include "CoolKernel/Record.h"

#include "CxxUtils/checker_macros.h"

#include <istream>
#include <string>
#include <sstream>

using namespace std::string_literals;
using namespace IOVDbNamespace;
using namespace cool;
const auto larJson=R"foo({"data":{"0":["[DB=8C26DCEB-1065-E011-8322-00145EDD7651][CNT=CollectionTree(CaloRec::CaloCellPositionShift/LArCellPositionShift)][CLID=3B3CCC72-7238-468E-B25E-6F85BA5C9D64][TECH=00000202][OID=00000003-00000000]"]}})foo";
const std::string spec="PoolRef:String4k";

BOOST_AUTO_TEST_SUITE(Json2CoolTest)
  BOOST_AUTO_TEST_CASE(Constructor){
    BasicFolder b;
    std::istringstream initializerStream(larJson);
    BOOST_CHECK_NO_THROW(Json2Cool j(initializerStream,b, spec));
    BOOST_CHECK(b.empty() == false);
  }
  BOOST_AUTO_TEST_CASE(convertedProperties){
    auto *pSpec=new coral::AttributeListSpecification;
    pSpec->extend<std::string>("PoolRef");
    coral::AttributeList attrList(*pSpec, true);
    //MUST use string literal suffix 's' to set the value to a string
    attrList[0].setValue("[DB=8C26DCEB-1065-E011-8322-00145EDD7651][CNT=CollectionTree(CaloRec::CaloCellPositionShift/LArCellPositionShift)][CLID=3B3CCC72-7238-468E-B25E-6F85BA5C9D64][TECH=00000202][OID=00000003-00000000]"s);
    BasicFolder b;
    std::istringstream initializerStream(larJson);
    BOOST_CHECK_NO_THROW(Json2Cool j(initializerStream,b, spec));
    BOOST_CHECK(b.getPayload(0) == attrList);
    const std::pair<cool::ValidityKey, cool::ValidityKey> refIov(0, 9223372036854775807);
    BOOST_CHECK(b.iov() == refIov);
  }
  
  BOOST_AUTO_TEST_CASE(parsePayloadSpec){
    const std::string testSpecString="crate: UChar, ROB: Int32, BCIDOffset: Int16, AName: String255";
    auto *referenceSpec = new cool::RecordSpecification();
    referenceSpec->extend("crate", StorageType::UChar);
    referenceSpec->extend("ROB", StorageType::Int32);
    referenceSpec->extend("BCIDOffset", StorageType::Int16);
    referenceSpec->extend("AName", StorageType::String255);
    auto *returnedSpec = Json2Cool::parsePayloadSpec(testSpecString);
    BOOST_CHECK(*(returnedSpec) == *static_cast<const cool::IRecordSpecification*>(referenceSpec));
  }
  BOOST_AUTO_TEST_CASE(createAttributeList){
    auto *referenceSpec = new cool::RecordSpecification();
    referenceSpec->extend("crate", StorageType::UChar);
    referenceSpec->extend("ROB", StorageType::Int32);
    referenceSpec->extend("BCIDOffset", StorageType::Int16);
    referenceSpec->extend("AName", StorageType::String255);
    const std::string jsonValues="[\"1\",\"2\",\"3\",\"purple\"]";
    std::istringstream initializerStream(jsonValues);
    nlohmann::json j;
    initializerStream >>j;
    auto record=Json2Cool::createAttributeList(referenceSpec, j);
    cool::Record reference(*referenceSpec);
    BOOST_CHECK(record.size() == reference.size());
    //
    auto & att0 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[0]);
    unsigned char set0(1);
    att0.setValue<unsigned char>(set0);
    //
    auto & att1 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[1]);
    att1.setValue<int>(2);
    //
    auto & att2 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[2]);
    short set2(3);
    att2.setValue<short>(set2);
    //
    auto & att3 ATLAS_THREAD_SAFE = const_cast<coral::Attribute&>(reference.attributeList()[3]);
    att3.setValue<std::string>("purple");
    //
    BOOST_CHECK(reference == record);
  }

BOOST_AUTO_TEST_SUITE_END()

