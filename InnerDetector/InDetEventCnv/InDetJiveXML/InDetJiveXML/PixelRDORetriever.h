/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JIVEXML_PIXELRDORETRIEVER_H
#define JIVEXML_PIXELRDORETRIEVER_H

#include "InDetCondTools/ISiLorentzAngleTool.h"
#include "JiveXML/IDataRetriever.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetJiveXML/IInDetGeoModelTool.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"

namespace JiveXML {
  
  /**
   * @class PixelRDORetriever
   * @brief Retrieve all @c PixelRDORawData information (pixel raw hits)
   *
   *  - @b Properties
   *    - <em>PixelRDOContainer</em><tt>= 'PixelRDOs'</tt> @copydoc m_PixelRDOContainerName
   *
   *  - @b Retrieved @b Data
   *    - <em>id</em>: the identifier of the hit
   *    - <em>x,y,z</em>: coordinates of the spacePoint
   *    - <em>phiModule,etaModule</em>: @f$\eta@f$ and @f$\phi@f$ of module in detector coordinates
   *    .
   */
  class PixelRDORetriever : virtual public IDataRetriever,
                                    public AthAlgTool {
    
  public:
    
    /// Standard constructor
    PixelRDORetriever(const std::string& type,const std::string& name,const IInterface* parent);

      /// Retrieve all the data
      virtual StatusCode retrieve(ToolHandle<IFormatTool> &FormatTool); 

      /// Return the name of the data type
      virtual std::string dataTypeName() const { return m_typeName; };

      /// Only retrieve geo tool in initialize
      virtual StatusCode initialize();
    
    private:
      
      ///The data type that is generated by this retriever
      const std::string m_typeName;

      /// A tool handle to the geo model tool
      const ToolHandle<IInDetGeoModelTool> m_geo
         {this,"GeoModelTool", "JiveXML::InDetGeoModelTool/InDetGeoModelTool","" };

      /// A tool handle to the SiLorentzAngleTool
      ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool{this, "LorentzAngleTool", "SiLorentzAngleTool/SCTLorentzAngleTool", "Tool to retreive Lorentz angle"};

      /// The StoreGate key for the PixelRDO container
      std::string m_PixelRDOContainerName;

      SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey{this, "PixelDetEleCollKey", "PixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};

  };
}

#endif
