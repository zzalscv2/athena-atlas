//        Copyright Iowa State University 2017.
//                  Author: Nils Krumnack
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// Please feel free to contact me (nils.erik.krumnack@cern.ch) for bug
// reports, feature suggestions, praise and complaints.


//
// includes
//

#include <SystematicsHandles/SysListHandle.h>

#include <AsgTools/MessageCheck.h>
#include <AsgTools/StatusCode.h>
#include <PATInterfaces/SystematicSet.h>
#include <RootCoreUtils/Assert.h>
#include <SystematicsHandles/ISysHandleBase.h>
#include <regex>

#include <stdexcept>

//
// method implementations
//

namespace EL
{
  void SysListHandle ::
  addHandle (ISysHandleBase& handle)
  {
    assert (!isInitialized());
    m_sysHandles.push_back (&handle);
  }



  void SysListHandle ::
  addAffectingSystematics (const CP::SystematicSet& /*affectingSystematics*/)
  {
    assert (!isInitialized());
    // no-op
  }



  ::StatusCode SysListHandle ::
  initialize ()
  {
    m_evtStore = m_evtStoreGetter();
    m_isInitialized = true;
    return StatusCode::SUCCESS;
  }



  std::unordered_set<CP::SystematicSet> SysListHandle ::
  systematicsVector ()
  {
    assert (isInitialized());

    if (m_fullAffecting.empty())
    {
      std::string affecting = m_affectingRegex;
      for (ISysHandleBase *handle : m_sysHandles)
      {
        std::string subAffecting = handle->getInputAffecting ();
        if (!subAffecting.empty())
        {
          if (!affecting.empty())
            affecting += "|";
          affecting += subAffecting;
        }
      }
      if (affecting.empty())
        affecting = "^$";
      m_fullAffecting = std::move (affecting);
    }

    const SysListType *systematicsList = nullptr;
    ANA_CHECK_THROW (m_evtStore->retrieve (systematicsList, m_systematicsListName));

    std::unordered_set<CP::SystematicSet> mysysList;
    for (const auto& sys : *systematicsList)
    {
      auto iter = m_affectingCache.find (sys);
      if (iter != m_affectingCache.end())
      {
        mysysList.insert (iter->second);
      } else
      {
        CP::SystematicSet mysys;
        std::regex affecting (m_fullAffecting);

        for (const CP::SystematicVariation& subsys : sys)
        {
          if (regex_match (subsys.basename(),affecting))
            mysys.insert (subsys);
        }
        m_affectingCache.insert (std::make_pair (sys, mysys));
        mysysList.insert (mysys);
      }
    }
    return mysysList;
  }



  StatusCode SysListHandle ::
  foreach (const std::function<StatusCode(const CP::SystematicSet)>& func)
  {
    assert (isInitialized());

    for (const CP::SystematicSet& sys : systematicsVector())
    {
      if (func (sys).isFailure())
      {
        ANA_MSG_ERROR ("failed to evaluate algorithm for systematic " << sys.name());
        return StatusCode::FAILURE;
      }
    }
    return StatusCode::SUCCESS;
  }
}
