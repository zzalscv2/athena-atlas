/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef SYSTEMATICS_HANDLES__SELECTION_NAME_SVC_H
#define SYSTEMATICS_HANDLES__SELECTION_NAME_SVC_H

#include <AsgServices/AsgService.h>
#include <AsgServices/ServiceHandle.h>
#include <CxxUtils/checker_macros.h>
#include <PATCore/AcceptInfo.h>
#include <PATInterfaces/SystematicSet.h>
#include <SelectionHelpers/ISelectionNameSvc.h>
#include <SystematicsHandles/ISystematicsSvc.h>
#include <mutex>
#include <unordered_map>

namespace CP
{
  /// \brief the canonical implementation of \ref ISelectionNameSvc

  class SelectionNameSvc final : public asg::AsgService,
                                 virtual public ISelectionNameSvc
  {
    //
    // public interface
    //

    /// \brief standard constructor
    /// \par Guarantee
    ///   strong
    /// \par Failures
    ///   out of memory II
  public:
    SelectionNameSvc (const std::string& name,
                      ISvcLocator* pSvcLocator);



    //
    // inherited interface
    //

  public:

    virtual StatusCode initialize () override;
    virtual StatusCode addAcceptInfo (const std::string& objectName, const std::string& decorName,
          const asg::AcceptInfo& acceptInfo) override;
    virtual const asg::AcceptInfo* getAcceptInfo (const std::string& objectName,
          const std::string& decorName) const override;



    //
    // private interface
    //

  private:

    /// @brief the `ISystematicsSvc` we use
    ///
    /// This is needed to look up the source objects of object copies
    ServiceHandle<ISystematicsSvc> m_sysSvc {"SystematicsSvc", "SelectionNameSvc"};

    /// @brief the map of `AcceptInfo` objects
    std::unordered_map<std::string, std::unordered_map<std::string, asg::AcceptInfo>> m_acceptInfoMap;
  };
}

#endif
