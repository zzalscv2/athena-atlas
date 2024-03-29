/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// NnPixelClusterSplitter.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef INDETRECTOOLS_NNPIXELCLUSTERSPLITTER_H
#define INDETRECTOOLS_NNPIXELCLUSTERSPLITTER_H


#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "InDetRecToolInterfaces/IPixelClusterSplitter.h"
#include "InDetPrepRawData/PixelClusterParts.h"
#include "InDetPrepRawData/PixelClusterSplitProb.h"
#include "BeamSpotConditionsData/BeamSpotData.h"

namespace InDet
{
    
  class NnClusterizationFactory;
  class PixelCluster;
    
    /** @class NnPixelClusterSplitter
        @author Andreas.Salzburger@cern.ch
    */
    class NnPixelClusterSplitter final: public extends<AthAlgTool, IPixelClusterSplitter> {
    public :
      /** Constructor*/
      NnPixelClusterSplitter(const std::string &type,
                             const std::string &name,
                             const IInterface *parent);
      
      /** Destructor*/
      ~NnPixelClusterSplitter() = default;
      
      /** AthAlgTool interface methods */
      virtual StatusCode initialize() override;            
      virtual StatusCode finalize() override;            
      
      /** take one, give zero or many */
      virtual std::vector<InDet::PixelClusterParts> splitCluster(
          const InDet::PixelCluster& origCluster) const override;

      /** take one, give zero or many - with split probability object */
      virtual std::vector<InDet::PixelClusterParts> splitCluster(
          const InDet::PixelCluster& origCluster,
          const InDet::PixelClusterSplitProb& spo) const override;

     private:
      ToolHandle<NnClusterizationFactory> m_NnClusterizationFactory { this, "NnClusterizationFactory", "InDet::NnClusterizationFactory/NnClusterizationFactory" };
      SG::ReadCondHandleKey<InDet::BeamSpotData> m_beamSpotKey { this, "BeamSpotKey", "BeamSpotData", "SG key for beam spot" };
      DoubleProperty m_thresholdSplittingIntoTwoClusters { this, "ThresholdSplittingIntoTwoClusters", 0.95 };
      DoubleProperty m_thresholdSplittingIntoThreeClusters { this, "ThresholdSplittingIntoThreeClusters", 0.90 };
      BooleanProperty m_splitOnlyOnBLayer { this, "SplitOnlyOnBLayer", true };
      BooleanProperty m_useBeamSpotInfo { this, "useBeamSpotInfo", true };

    };
}
#endif
