/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_SICLUSTERRETRIEVER_H
#define JIVEXML_SICLUSTERRETRIEVER_H

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "IInDetGeoModelTool.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "InDetPrepRawData/SiClusterContainer.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"
#include "StoreGate/ReadCondHandleKey.h"

namespace JiveXML
{

  
  /**
   * @class SiClusterRetriever
   * @brief Retrieves all @c InDet::SiCluster objects
   *
   *  - @b Properties
   *    - <em>SCTClusters</em><tt> = 'SCT_Clusters'</tt>: @copydoc m_SiClusterCollName
   *    - <em>SCT_TruthMap</em><tt> = 'PRD_MultiTruthSCT'</tt>: @copydoc m_SiTruthMapName
   *    .
   *
   *  - @b Retrieved @b Data
   *    - <em>x0,y0,z0</em> : local start position of the cluster
   *    - <em>x1,y1,z1</em> : local end position of the cluster
   *    - <em>width</em> : cluster width
   *    - <em>id</em> : id of of the SCT module
   *    - <em>phiModule,etaModule</em> @f$\eta@f$ and @f$\phi@f$ of module in detector coordinates
   *    - <em>side</em> : module side?
   *    - <em>numBarcodes</em> : number of associated truth tracks
   *    - <em>barcodes</em> : barcodes of the associated truth tracks
   *    .
   */
  class SiClusterRetriever : virtual public IDataRetriever,
                                     public AthAlgTool {
    
    public:
      
      /// Standard Constructor
      SiClusterRetriever(const std::string& type,const std::string& name,const IInterface* parent);
      
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool); 

      /// Return the name of the data type
      virtual std::string dataTypeName() const { return "STC"; }

      /// initialize
      virtual StatusCode initialize();

    private:
      
      /// A tool handle to the geo model tool
      const ToolHandle<IInDetGeoModelTool> m_geo
         {this,"GeoModelTool", "JiveXML::InDetGeoModelTool/InDetGeoModelTool",""};

      /// Condition object key of SiDetectorElementCollection for SCT
      SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{this, "SCTDetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection for SCT"};
      
      /// StoreGate key for the SiClusterContainer for SCT
      SG::ReadHandleKey<InDet::SiClusterContainer> m_SiClusterCollName{this, "SiClusterContKey", "SCT_Clusters", "Key of the SiClusterContainer for SCT"};

      /// StoreGate key for the PRD_MultiTruthCollection for SCT
      bool m_useSiTruthMap = false;
      SG::ReadHandleKey<PRD_MultiTruthCollection> m_SiTruthMapName{this, "SiTruthMapKey", "PRD_MultiTruthSCT", "Key of the PRD_MultiTruthCollection for SCT"};
    };
}
#endif
