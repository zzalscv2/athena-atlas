#ifndef FPGATrackSim_READLOGICALHITS_H
#define FPGATrackSim_READLOGICALHITS_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"

#include "FPGATrackSimInput/IFPGATrackSimEventOutputHeaderTool.h"
#include "FPGATrackSimInput/FPGATrackSimRawToLogicalHitsTool.h"


class FPGATrackSimReadLogicalHitsAlg : public AthAlgorithm {
public:
  FPGATrackSimReadLogicalHitsAlg (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~FPGATrackSimReadLogicalHitsAlg () = default;
  
  virtual StatusCode initialize() override;
  virtual StatusCode execute()    override;
  

private:

  ToolHandle<IFPGATrackSimEventOutputHeaderTool> m_readOutputTool {this, "InputTool", "FPGATrackSimOutputHeaderTool/FPGATrackSimOutputHeaderTool"};
  
  unsigned int m_event = 0U;
  
};

#endif // FPGATrackSimREADLOGICALHITS_h
