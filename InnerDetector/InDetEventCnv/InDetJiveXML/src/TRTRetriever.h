/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_TRTRETRIEVER_H
#define JIVEXML_TRTRETRIEVER_H

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "IInDetGeoModelTool.h"

#include "InDetPrepRawData/TRT_DriftCircleContainer.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"

#include "StoreGate/ReadHandleKey.h"

namespace JiveXML{

  
  /**
   * @class TRTRetriever
   * @brief Retrieves all @c InDet::TRT_DriftCircle objects
   *
   *  - @b Properties
   *    - TRTClusters: @copydoc m_TRTDriftCircleCollKey
   *    - TRT_TruthMap: @copydoc m_TRTTruthMapKey
   *    .
   *
   *  - @b Retrieved @b Data
   *    - <em>id</em> : unique identifier of the drift circle
   *    - <em>rhoz</em> : @f$\rho@f$ (for barrel) or @f$z@f$ (for endcap) coordinate of the hit
   *    - <em>phi</em> : @f$\phi@f$ coordinate of the hit
   *    - <em>driftR</em> : the drift radius
   *    - <em>threshold</em> : wether the hit passed the high threshold or not
   *    - <em>numBarcodes</em> : number of associated truth tracks
   *    - <em>barcodes</em> : barcodes of the associated truth tracks
   *    - <em>sub</em> : the barrel/endcap subdetector in which the drift circle is
   *    - <em>noise</em> : wether the hit is flagged as noise
   *    - <em>timeOverThreshold</em> : time over threshold of the hit
   *    - <em>bitPattern</em> : the status word of the drift circle
   *    .
   */
  class TRTRetriever : virtual public IDataRetriever,
                                   public AthAlgTool {
    
    public:
      
      /// Standard Constructor
      TRTRetriever(const std::string& type,const std::string& name,const IInterface* parent);
      
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool); 

      /// Return the name of the data type
      virtual std::string dataTypeName() const { return "TRT"; };

      /// initialize only geo model tool
      virtual StatusCode initialize();

    private:
      
      /// A tool handle to the geo model tool
      const ToolHandle<IInDetGeoModelTool> m_geo
         {this,"GeoModelTool","JiveXML::InDetGeoModelTool/InDetGeoModelTool",""};

      /// The StoreGate key for the TRT Cluster collection to retrieve
      SG::ReadHandleKey<InDet::TRT_DriftCircleContainer> m_TRTDriftCircleCollKey{ this, "TRTClusters", "TRT_DriftCircles", "Container name for TRT Drift Circles" }; 
      /// The StoreGate key for the TRT MultiTruthMap with the track associations
      bool m_useTRTTruthMap = false;
      SG::ReadHandleKey<PRD_MultiTruthCollection> m_TRTTruthMapKey{ this, "TRT_TruthMap", "PRD_MultiTruthTRT", "Container name for PRD Multi-truth TRT MAP" };
  };

}
#endif
