///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

// RootCnvSvc.h
// Header file for class Athena::RootCnvSvc
// Author: Peter van Gemmeren <gemmeren@anl.gov>
// Author: S.Binet<binet@cern.ch>
///////////////////////////////////////////////////////////////////
#ifndef ATHENAROOTCOMPS_ROOTCNVSVC_H
#define ATHENAROOTCOMPS_ROOTCNVSVC_H 1

/** @file AthenaRootCnvSvc.h
 *  @brief This file contains the class definition for the Athena::RootCnvSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

// AthenaRootKernel includes
#include "AthenaRootKernel/IAthenaRootCnvSvc.h"
#include "AthenaRootKernel/IIoSvc.h"

// fwk includes
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthCnvSvc.h"

// fwd declares
class IDictLoaderSvc;
class ITPCnvSvc;
namespace Athena { class RootSvc; }
class IRootSvc;

namespace Athena {
/** @class Athena::RootCnvSvc
 *  @brief This class provides the interface between Athena and RootSvc.
 **/
class RootCnvSvc:
    virtual public ::IAthenaRootCnvSvc,
            public ::AthCnvSvc
{
  friend class SvcFactory<Athena::RootCnvSvc>;

public:
  /// Standard Service Constructor
  RootCnvSvc(const std::string& name, ISvcLocator* pSvcLocator);

  /// Gaudi Service Interface method implementations:
  StatusCode initialize();
  StatusCode finalize();
  StatusCode queryInterface(const InterfaceID& riid, void** ppvInterface);

  /// Connect the output file to the service with open mode.
  StatusCode connectOutput(const std::string& file,
                           const std::string& mode);

  /// Connect the output file to the service.
  StatusCode connectOutput(const std::string& file);

  /// Connect the output file to the service with open mode.
  StatusCode connectOutput(IIoSvc::Fd fd,
                           IIoSvc::IoType mode);

  /// Commit pending output.
  StatusCode commitOutput(const std::string& output, bool do_commit);


  /// Connect the output file to the service.
  StatusCode connectOutput(IIoSvc::Fd ofd);

  /// Commit pending output.
  StatusCode commitOutput(IIoSvc::Fd output, bool do_commit);

  /// Create a Generic address using explicit arguments to
  /// identify a single object.
  StatusCode createAddress(long svc_type,
                           const CLID& clid,
                           const std::string* par,
                           const unsigned long* ip,
                           IOpaqueAddress*& refpAddress);

  /// Convert an address to string form
  StatusCode convertAddress(const IOpaqueAddress* pAddress,
                            std::string& refAddress);

  /// Convert an address in string form to object form
  StatusCode createAddress(long svc_type,
                           const CLID& clid,
                           const std::string& refAddress,
                           IOpaqueAddress*& refpAddress);

  /// Update state of the service
  StatusCode updateServiceState(IOpaqueAddress* pAddress);

protected:
  /// Destructor:
  virtual ~RootCnvSvc();

  ///@{ RootType-based API
  /// Load the class (dictionary) from Root.
  RootType getType(const CLID& clid) const;
  RootType getType(const std::type_info& type) const;

  /// Write object of a given class to Root, using optional T/P converter.
  unsigned long writeObject(const std::string& placement,
                            RootType typeDesc,
                            const void* pObj,
                            ITPCnvBase* tpConverter);
  ///@}

  /// Get T/P converter for a transient class
  ITPCnvBase* getTPConverter(const CLID& clid) const;

  /// Create new converter using factory
  IConverter* createConverter(long typ,
                              const CLID& clid,
                              const ICnvFactory* fac);

private:
  /// Default constructor:
  RootCnvSvc(); //< not implemented
  RootCnvSvc(const RootCnvSvc&); //< not implemented
  RootCnvSvc& operator=(const RootCnvSvc&); //< not implemented

private:
  /// ServiceHandle to the dictionary service
  ServiceHandle< ::IDictLoaderSvc> m_dictsvc;
  /// ServiceHandle to the T/P conversion service
  ServiceHandle< ::ITPCnvSvc> m_tpcnvsvc;
  /// ServiceHandle to the root service
  ServiceHandle< ::IRootSvc> m_rootsvc;
  /// handle to the I/O svc
  ServiceHandle< ::IIoSvc> m_iosvc;
};

} //> end namespace Athena

#endif /* !ATHENAROOTCOMPS_ROOTCNVSVC_H */
