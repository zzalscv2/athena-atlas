#ifndef FPGATrackSim_LOGICALHITSWRAPPERALG_H
#define FPGATrackSim_LOGICALHITSWRAPPERALG_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthContainers/DataVector.h"

#include "FPGATrackSimInput/IFPGATrackSimEventInputHeaderTool.h"
#include "FPGATrackSimInput/IFPGATrackSimEventOutputHeaderTool.h"
#include "FPGATrackSimMaps/FPGATrackSimClusteringToolI.h"
#include "FPGATrackSimInput/FPGATrackSimRawToLogicalHitsTool.h"

class FPGATrackSimEventInputHeader;
class FPGATrackSimLogicalEventInputHeader;

class FPGATrackSimLogicalHitsWrapperAlg : public AthAlgorithm {
public:
  FPGATrackSimLogicalHitsWrapperAlg (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~FPGATrackSimLogicalHitsWrapperAlg () = default;
  virtual StatusCode initialize() override;
  virtual StatusCode execute()    override;
  StatusCode BookHistograms();


private:
 
  ToolHandle<IFPGATrackSimEventInputHeaderTool>     m_hitInputTool    { this, "InputTool",  "FPGATrackSimInputHeaderTool/FPGATrackSimInputHeaderTool", "Input Tool" };
  ToolHandle<IFPGATrackSimEventOutputHeaderTool>    m_writeOutputTool { this, "OutputTool", "FPGATrackSimOutputHeaderTool/FPGATrackSimOutputHeaderTool", "Output Tool" };
  ToolHandle<FPGATrackSimClusteringToolI>           m_clusteringTool  { this, "ClusteringTool", "FPGATrackSimClusteringTool/FPGATrackSimClusteringTool", "FPGATrackSim Clustering Tool" };
  ToolHandle<FPGATrackSimRawToLogicalHitsTool>      m_hitMapTool      { this, "RawToLogicalHitsTool", "FPGATrackSimRawToLogicalHitsTool/FPGATrackSimRawToLogicalHitsTool", "Map Raw to Logical hit Tool" };

  Gaudi::Property<bool>  m_Clustering   {this, "Clustering", false, "flag to enable the clustering"};

};

#endif // FPGATrackSimSGRORAWHITSWRAPPERALG_h
