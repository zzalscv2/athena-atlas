#ifndef MUONCALIBSTREAMCNVSVC_MUONCALIBSTREAMCNVSVC_H
#define MUONCALIBSTREAMCNVSVC_MUONCALIBSTREAMCNVSVC_H

#include "CoralBase/Attribute.h"
#include "AthenaBaseComps/AthCnvSvc.h"

// Forward declarations
template <class TYPE> class SvcFactory;

class MuonCalibStreamCnvSvc : public AthCnvSvc {
    /// Allow the factory class access to the constructor
    friend class SvcFactory<MuonCalibStreamCnvSvc>;

public:
    MuonCalibStreamCnvSvc(const std::string &name, ISvcLocator *svc);
    virtual ~MuonCalibStreamCnvSvc();
    virtual StatusCode initialize();
    // query interface method
    virtual StatusCode queryInterface(const InterfaceID &riid, void **ppvInterface);
    /// Update state of the service
    virtual StatusCode updateServiceState(IOpaqueAddress *pAddress);

protected:
    // initialize the converters for these data classes.
    std::vector<std::string> m_initCnvs;
};
#endif
