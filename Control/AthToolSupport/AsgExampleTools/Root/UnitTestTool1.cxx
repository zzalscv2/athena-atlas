/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include <AsgExampleTools/UnitTestTool1.h>

#include <gtest/gtest.h>
#include <map>

#include <CxxUtils/checker_macros.h>
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;  // unit test

//
// method implementations
//

namespace asg
{
  UnitTestTool1 ::
  UnitTestTool1 (const std::string& val_name)
    : AsgTool (val_name)
  {
    declareProperty ("propertyInt", m_propertyInt, "the integer property");
    declareProperty ("propertyString", m_propertyString, "the string property");
    declareProperty ("initializeFail", m_initializeFail, "whether initialize should fail");

    ++ instance_counts (name());

    ANA_MSG_DEBUG ("create UnitTestTool1 " << this);
  }



  UnitTestTool1 ::
  ~UnitTestTool1 ()
  {
    ANA_MSG_DEBUG ("destroy UnitTestTool1 " << this);

    -- instance_counts (name());
  }



  StatusCode UnitTestTool1 ::
  initialize ()
  {
    ANA_MSG_INFO ("initialize UnitTestTool1 " << this);
    ANA_MSG_INFO ("  propertyString: " << m_propertyString);
    ANA_MSG_INFO ("  propertyInt: " << m_propertyInt);

    if (m_initializeFail)
    {
      ATH_MSG_ERROR ("tool configured to fail initialize");
      return StatusCode::FAILURE;
    }
    if (m_isInitialized)
    {
      ATH_MSG_ERROR ("initialize called twice");
      return StatusCode::FAILURE;
    }
    m_isInitialized = true;
    return StatusCode::SUCCESS;
  }



  std::string UnitTestTool1 ::
  getPropertyString () const
  {
    return m_propertyString;
  }



  int UnitTestTool1 ::
  getPropertyInt () const
  {
    return m_propertyInt;
  }



  void UnitTestTool1 ::
  setPropertyInt (int val_property)
  {
    m_propertyInt = val_property;
  }



  bool UnitTestTool1 ::
  isInitialized () const
  {
    return m_isInitialized;
  }



  int& UnitTestTool1 ::
  instance_counts (const std::string& name)
  {
    static std::map<std::string,int> counts;
    auto iter = counts.find (name);
    if (iter == counts.end())
      iter = counts.insert (std::make_pair (name, 0)).first;
    assert (iter != counts.end());
    return iter->second;
  }
}
