/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_SICLUSTERRETRIEVER_H
#define JIVEXML_SICLUSTERRETRIEVER_H

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetJiveXML/IInDetGeoModelTool.h"

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
      virtual std::string dataTypeName() const { return typeName; }

      /// initialize only geo model tool
      virtual StatusCode initialize() { return geo.retrieve(); }

    private:
      
      /// The data type that is generated by this retriever
      const std::string typeName;

      /// A tool handle to the geo model tool
      const ToolHandle<IInDetGeoModelTool> geo;
   
      /// The StoreGate key for the SiClusterCollection to retrieve
      std::string m_SiClusterCollName;
      /// The StoreGate key for the PRD MultiTruthMap with the track associations
      std::string m_SiTruthMapName;
     

    };
}
#endif
