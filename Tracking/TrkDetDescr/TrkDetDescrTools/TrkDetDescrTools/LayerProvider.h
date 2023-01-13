/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// LayerProvider.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRTOOLS_LAYERPROVIDER_H
#define TRKDETDESCRTOOLS_LAYERPROVIDER_H

// Trk
#include "TrkDetDescrInterfaces/ILayerProvider.h"
// Gaudi & Athena
#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"

namespace Trk {

    class Layer;
    class ILayerBuilder;


    /** @class LayerProvider

      Wrapper around an ILayerBuilder to feed into the StagedGeometryBuilder

      @author Andreas.Salzburger@cern.ch
     */
    class ATLAS_NOT_THREAD_SAFE LayerProvider
      : public AthAlgTool
      , virtual public ILayerProvider
    {

      public:
        /** Constructor */
        LayerProvider(const std::string&,const std::string&,const IInterface*);

        /** Destructor */
        virtual ~LayerProvider();
        
        /** initialize */
        StatusCode initialize();
        
        /** finalize */
        StatusCode finalize();

        /** LayerBuilder interface method - returning the endcap layer */
        std::pair<const std::vector<Layer*>, const std::vector<Layer*> >
          endcapLayer() const;

        /** LayerBuilder interface method - returning the central layers */
        const std::vector<Layer*> centralLayers() const; 

        /** Name identification */
        const std::string& identification() const;

      private:
        ToolHandle<ILayerBuilder>               m_layerBuilder;
    };


} // end of namespace

#endif // TRKDETDESCRTOOLS_LAYERPROVIDER_H

