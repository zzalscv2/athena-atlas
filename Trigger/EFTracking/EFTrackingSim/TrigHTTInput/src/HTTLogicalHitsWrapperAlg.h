#ifndef HTT_LOGICALHITSWRAPPERALG_H
#define HTT_LOGICALHITSWRAPPERALG_H


#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthContainers/DataVector.h"

#include "TrigHTTInput/IHTTEventInputHeaderTool.h"
#include "TrigHTTInput/IHTTEventOutputHeaderTool.h"
#include "TrigHTTMaps/HTTClusteringToolI.h"
#include "TrigHTTInput/HTTRawToLogicalHitsTool.h"

class HTTEventInputHeader;
class HTTLogicalEventInputHeader;

class HTTLogicalHitsWrapperAlg : public AthAlgorithm {
public:
  HTTLogicalHitsWrapperAlg (const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~HTTLogicalHitsWrapperAlg () = default;
  virtual StatusCode initialize() override;
  virtual StatusCode execute()    override;
  StatusCode BookHistograms();


private:
 
  ToolHandle<IHTTEventInputHeaderTool>     m_hitInputTool    { this, "InputTool",  "HTTInputHeaderTool/HTTInputHeaderTool", "Input Tool" };
  ToolHandle<IHTTEventOutputHeaderTool>    m_writeOutputTool { this, "OutputTool", "HTTOutputHeaderTool/HTTOutputHeaderTool", "Output Tool" };
  ToolHandle<HTTClusteringToolI>           m_clusteringTool  { this, "ClusteringTool", "HTTClusteringTool/HTTClusteringTool", "HTT Clustering Tool" };
  ToolHandle<HTTRawToLogicalHitsTool>      m_hitMapTool      { this, "RawToLogicalHitsTool", "HTTRawToLogicalHitsTool/HTTRawToLogicalHitsTool", "Map Raw to Logical hit Tool" };

  Gaudi::Property<bool>  m_Clustering   {this, "Clustering", false, "flag to enable the clustering"};

};

#endif // HTTSGRORAWHITSWRAPPERALG_h
