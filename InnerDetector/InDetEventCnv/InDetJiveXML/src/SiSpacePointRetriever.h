/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_SISPACEPOINTRETRIEVER_H
#define JIVEXML_SISPACEPOINTRETRIEVER_H

#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "IInDetGeoModelTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "TrkSpacePoint/SpacePointContainer.h"
#include "TrkTruthData/PRD_MultiTruthCollection.h"

namespace JiveXML{

  /**
   * @class SiSpacePointRetriever
   * @brief Retrieves all @c InDet::SpacePoint data for Pixel and SCT
   *
   *  - @b Properties
   *    - <em>PixelSpacePoints</em><tt> = 'PixelSpacePoints'</tt>: @copydoc m_PixelSPContainerName;
   *    - <em>SCT_SpacePoints</em><tt> = 'SCTSpacePoints'</tt>: @copydoc m_PixelSPContainerName;
   *    - <em>PRD_TruthPixel</em><tt> = 'PRD_MultiTruthPixel'</tt>: m_PixelPRDTruthName;
   *    - <em>PRD_TruthSCT</em><tt> = 'PRD_MultiTruthSCT'</tt>:  @copydoc m_SCTPRDTruthName;
   *
   *  - @b Retrieved @b Data
   *    - <em>x,y,z</em>: coordinates of the spacePoint
   *    - <em>clusters</em>: identifier of first (and second for SCT) associated cluster
   *    - <em>phiModule,etaModule</em>: @f$\eta@f$ and @f$\phi@f$ of module in detector coordinates
   *    - <em>numBarcodes</em>: number of truth particles associated with this SpacePoint
   *    - <em>barcodes</em>: barcodes of associated truth particles
   *    .
   */
  class SiSpacePointRetriever : virtual public IDataRetriever,
                                        public AthAlgTool {
    
    public:

      /// Standard Constructor
      SiSpacePointRetriever(const std::string& type,const std::string& name,const IInterface* parent);
        
      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool); 

      /// Return the name of the data type
      virtual std::string dataTypeName() const { return "S3D"; };

      /// Only retrieve geo tool in initialize
      virtual StatusCode initialize();
    
    private:
      
      /// A tool handle to the geo model tool
      const ToolHandle<IInDetGeoModelTool> m_geo
         {this,"GeoModelTool","JiveXML::InDetGeoModelTool/InDetGeoModelTool",""};

      /** StoreGate key for Pixel space points*/
      SG::ReadHandleKey<SpacePointContainer> m_PixelSPContainerName;

      /** StoreGate key for SCT space points*/
      SG::ReadHandleKey<SpacePointContainer> m_SCTSPContainerName;

      /** StoreGate key for pixel PRD_MultiTruth*/
      bool m_usePixelTruthMap = false;
      SG::ReadHandleKey<PRD_MultiTruthCollection> m_PixelPRDTruthName;

      /** StoreGate key for SCT PRD_MultiTruth*/
      bool m_useSCTTruthMap = false;
      SG::ReadHandleKey<PRD_MultiTruthCollection> m_SCTPRDTruthName;
  };

}
#endif



