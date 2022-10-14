/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "ZdcConditions/ZdcLucrodMapRun3.h"
#include "PathResolver/PathResolver.h"

#include <fstream>

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

ZdcLucrodMapRun3* ZdcLucrodMapRun3::getInstance()
{
  static ZdcLucrodMapRun3 map_run3;
  return &map_run3;
}

void ZdcLucrodMapRun3::deleteInstance()
{
  // clear resources if needed
}

//constructor
ZdcLucrodMapRun3::ZdcLucrodMapRun3() : asg::AsgMessaging("ZdcLucrodMapRun3") 
{
  msg().setLevel(MSG::INFO);

  std::string filePath = PathResolver::find_file("ZDC_Run3_conditions.json","DATAPATH", PathResolver::RecursiveSearch);
  if (!filePath.empty())
    {
      ATH_MSG_DEBUG( "ZdcLucrodMapRun3::found ZDC JSON at " << filePath );
    }
  else
    {
      ATH_MSG_DEBUG(  "ZdcLucrodMapRun3::mapping NOT FOUND!" ) ;
      filePath = "./ZDC_Run3_mapping.json";
    }

  std::ifstream ifs(filePath);
 
  ifs >> m_mainJson;

  m_lucrodInfo.resize(6);

  for (auto& element: m_mainJson)
    {
      int source_ID = element["source_ID"] ; 
      m_lucrodInfo[source_ID] = element;
    }

}
