/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack


#ifndef SYSTEMATICS_HANDLES__I_SELECTION_NAME_SVC_H
#define SYSTEMATICS_HANDLES__I_SELECTION_NAME_SVC_H

#include <AsgServices/IAsgService.h>

namespace asg
{
  class AcceptInfo;
}

namespace CP
{
  /// \brief the interface for the central systematics service
  ///
  /// This service is used as part of the common CP algorithms to allow
  /// communicating the name of individual selection bits from the selection
  /// algorithm to the cut flow algorithm.

  class ISelectionNameSvc : virtual public asg::IAsgService
  {
  public:

    DeclareInterfaceID (CP::ISelectionNameSvc, 1, 0);

    virtual StatusCode addAcceptInfo (const std::string& objectName, const std::string& decorName,
          const asg::AcceptInfo& acceptInfo) = 0;

    virtual const asg::AcceptInfo* getAcceptInfo (const std::string& objectName,
          const std::string& decorName) const = 0;
  };
}

#endif
