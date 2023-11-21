/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file DataModelTestDataCommon/src/setConverterLibrary.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Dec, 2014
 * @brief Helper to explicitly set library for a converter.
 */


#include "DataModelTestDataCommon/setConverterLibrary.h"
#include "Gaudi/PluginService.h"
#include "TClass.h"


using namespace Gaudi::PluginService;


namespace DMTest {


void setPluginLibrary ATLAS_NOT_THREAD_SAFE (const std::string& name, const std::string& lib)
{
  const Details::Registry& reg = Details::Registry::instance();
  Details::Registry::FactoryMap& map =
    const_cast<Details::Registry::FactoryMap&>(reg.factories());
  Details::Registry::FactoryMap::iterator i = map.find (name);
  if (i == map.end())
    map.insert (std::make_pair (name, Details::Registry::FactoryInfo{lib}));
  else
    i->second = Details::Registry::FactoryInfo{lib};
}


void setConverterLibrary ATLAS_NOT_THREAD_SAFE (CLID clid, const std::string& lib)
{
  std::string name = "CNV_256_" + std::to_string (clid);
  setPluginLibrary (name, lib);
}


void setConverterLibrary ATLAS_NOT_THREAD_SAFE (const std::string& lib)
{
  setConverterLibrary (9631, lib); // DMTest::BDer
  setConverterLibrary (9632, lib); // DMTest::DDer
  setConverterLibrary (9633, lib); // DataVector<DMTest::B>
  setConverterLibrary (9634, lib); // DataVector<DMTest::D>
  setConverterLibrary (9639, lib); // DMTest::ELVec
  setConverterLibrary (9721, lib); // DMTest::CVec_v1
  setConverterLibrary (9722, lib); // DMTest::CAuxContainer_v1
  setConverterLibrary (9723, lib); // DMTest::CInfoAuxContainer_v1
  setConverterLibrary (9724, lib); // DMTest::C_v1
  setConverterLibrary (9725, lib); // DMTest::CTrigAuxContainer_v1
  setConverterLibrary (9741, lib); // DMTest::GVec_v1
  setConverterLibrary (9742, lib); // DMTest::GAuxContainer_v1
  setConverterLibrary (9744, lib); // DMTest::G_v1
  setConverterLibrary (9779, lib); // DMTest::CView_v1
  setConverterLibrary (9841, lib); // DMTest::AllocTestContainer
  setConverterLibrary (9842, lib); // DMTest::AllocTestAuxContainer
  setConverterLibrary (9844, lib); // DMTest::AllocTest
  setConverterLibrary (243020043, lib); // DMTest::S1
  setConverterLibrary (243020042, lib); // DMTest::S2
  setPluginLibrary ("_PERS_DMTest::HVec_v1", lib);
  setPluginLibrary ("_PERS_DataVector<DMTest::H_v1>", lib);
}


void setTrigConverterLibrary ATLAS_NOT_THREAD_SAFE (CLID clid, const std::string& lib)
{
  std::string name = "CNV_81_" + std::to_string (clid);
  setPluginLibrary (name, lib);
}


void setTrigConverterLibrary ATLAS_NOT_THREAD_SAFE (const std::string& lib)
{
  setTrigConverterLibrary (9781, lib); // DMTest::HVec
  setTrigConverterLibrary (9782, lib); // DMTest::HAuxContainer
  setTrigConverterLibrary (9787, lib); // DMTest::HView
}


} // namespace DMTest


TClass* xclass (const char* s)
{
  return TClass::GetClass(s);
}



