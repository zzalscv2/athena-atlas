// dear emacs, this is -*- C++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAKERNEL_ICLASSIDSVC_H
# define ATHENAKERNEL_ICLASSIDSVC_H

#include <string>

#include "GaudiKernel/IService.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/StatusCode.h"

namespace Athena {
  class PackageInfo;
}

/** @class IClassIDSvc
 * @brief  interface to the CLID database
 * @author Paolo Calafiura <pcalafiura@lbl.gov> - ATLAS Collaboration
 *$Id: IClassIDSvc.h,v 1.7 2009-01-15 19:05:54 binet Exp $	
 */

class IClassIDSvc : virtual public IService {
public:
  /// get next available CLID 
  /// @throws std::runtime_error if no CLID can be allocated
  virtual CLID nextAvailableID() const = 0;
  /// check if id is used
  virtual bool isIDInUse(const CLID& id) const = 0;
  /// check if name is used
  virtual bool isNameInUse(const std::string& name) const = 0;
  /// get user assigned type name associated with clID
  virtual StatusCode getTypeNameOfID(const CLID& id, std::string& typeName) const = 0;
  /// get user assigned type-info name associated with clID
  virtual StatusCode getTypeInfoNameOfID(const CLID& id, std::string& typeInfoName) const = 0;
  /// get id associated with type name (if any)
  virtual StatusCode getIDOfTypeName(const std::string& typeName, CLID& id) const = 0;
  /// get id associated with type-info name (if any)
  virtual StatusCode getIDOfTypeInfoName(const std::string& typeInfoName, CLID& id) const = 0;
  /// get type name associated with clID (if any)
  virtual StatusCode getPackageInfoForID(const CLID& id, Athena::PackageInfo& info) const = 0;
  /// associate type name, package info and type-info name with clID
  virtual 
  StatusCode setTypePackageForID(const CLID&, 
				 const std::string& typeName,
				 const Athena::PackageInfo&,
				 const std::string& typeInfoName = "") = 0;
  /// Gaudi boilerplate
  static const InterfaceID& interfaceID();
  
  /// destructor
  virtual ~IClassIDSvc();
};

inline
const InterfaceID& 
IClassIDSvc::interfaceID() {
  static const InterfaceID IID("IClassIDSvc", 1, 0);
  return IID;
}

#endif // ATHENAKERNEL_ICLASSIDSVC_H
