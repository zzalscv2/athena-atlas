//        Copyright Iowa State University 2016.
//                  Author: Nils Krumnack
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// Please feel free to contact me (nils.erik.krumnack@cern.ch) for bug
// reports, feature suggestions, praise and complaints.


//
// includes
//

#include <EventLoopTest/UnitTestTool.h>

#include <gtest/gtest.h>
#include <map>

//
// method implementations
//

namespace EL
{
  UnitTestTool ::
  UnitTestTool (const std::string& val_name)
    : AsgTool (val_name)
  {
    declareProperty ("propertyInt", m_propertyInt, "the integer property");
    declareProperty ("subtool", m_subtool, "our subtool");
  }



  UnitTestTool ::
  ~UnitTestTool ()
  {
  }



  StatusCode UnitTestTool ::
  initialize ()
  {
    return StatusCode::SUCCESS;
  }



  int UnitTestTool ::
  getPropertyInt () const
  {
    return m_propertyInt;
  }



  const IUnitTestTool *UnitTestTool ::
  getSubtool () const
  {
    if (!m_subtool.empty())
      return &*m_subtool;
    else
      return nullptr;
  }
}
