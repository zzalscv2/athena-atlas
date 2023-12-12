/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GLOBALTOPOHISTSVC_H
#define GLOBALTOPOHISTSVC_H
/* Adaption of StandaloneL1TopoHistSvc
 *
 * WARNING: This code is a necessary place holder provided to to satisfy
 * the needs of L!TopoSim Algorithms. These Algorithms are being used
 * temporaily for GlobalSim development. This "Service" is created every event
 * in a constant ReentrantAlgorithm execute() method
 * and so WILL NOT STORE DATA ACCUMULATED OVER >1 EVENT.
 *
 * StandaloneL1TopoHistSvc presented a problem in its destructor
 * - more accurately in the destructor of StandaloneL1TopoHistSvcImpl
 *
 *   The destructor of StandaloneL1TopoHistSvcImpl attempts to delete
 *   bare pointer to its TH1 and TH2 objects. However, L1Topo Algorithm
 *   objects are contained in the (global) singleton AlgFactory. When,
 *   at the end of a job, this AlgFactory instance is destroyed, the pointers
 *   to the histogrm objects are no longer valid and there is a crash.
 *
 *   Note the crash happens just before the program exits, and thus is
 *   "harmless".
 *
 *  I conclude that the histogram objects (produced by "new")  have been
 *  destructed by the system (they were necessarily constructed after
 *  StandaloneL1TopoHistSvcImpl.
 *
 *  Given that the the Algorithms go to live in AlgFactory I kludge
 *  a solution by not deleting the histograms, and relying on the system to
 *  clean up onn exit.
 *
 *  A real Svc would handle the change of state not allowed within the execute()
 *  method of an AthReentrantAlgorithm.
 *  This would involve modifying a certain amount of L1TopoSimulation code.
 *  Using an Athena service would introduce Athena dependencies the Global community
 *  would like to avoid.
 *
 */

#include <memory>

#include "L1TopoInterfaces/IL1TopoHistSvc.h"
#include <map>


namespace GlobalSim {
  class GlobalTopoHistSvc : public IL1TopoHistSvc
  {
  public:
    GlobalTopoHistSvc() = default;
    virtual ~GlobalTopoHistSvc() = default;

    virtual void registerHist(TH1 * h) override;

    virtual void registerHist(TH2 * h) override;
    
    virtual void fillHist1D(const std::string & histName, double x) override;
    
    virtual void fillHist2D(const std::string & histName,
			    double x,
			    double y) override;
    
    virtual void setBaseDir(const std::string & baseDir) override;
    
    virtual void save() override;
    
  private:

    
    std::map<std::string,TH1*>   m_hists1D;
    std::map<std::string,TH2*>   m_hists2D;
    std::string             m_baseDir {""};
    
    // std::unique_ptr<GlobalTopoHistSvcImpl> m_impl;
    
    virtual TH1 * findHist(const std::string & histName) override;
    
  };
}
#endif
