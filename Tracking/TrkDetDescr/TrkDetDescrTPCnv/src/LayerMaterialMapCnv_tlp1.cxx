/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// LayerMaterialMapCnv_tlp1.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkDetDescrTPCnv/LayerMaterialMapCnv_tlp1.h"

LayerMaterialMapCnv_tlp1::LayerMaterialMapCnv_tlp1( )
{

   addMainTPConverter();

   addTPConverter( &m_materialPropertiesCnv );
   
   addTPConverter( &m_blMaterialCnv );    
   addTPConverter( &m_clMaterialCnv ); 
   addTPConverter( &m_hlMaterialCnv );      

}

LayerMaterialMapCnv_tlp1::~LayerMaterialMapCnv_tlp1()
{}

void LayerMaterialMapCnv_tlp1::setPStorage( Trk::LayerMaterialMap_tlp1 *storage )
{
   // for implicit TrackCollection_p1 from the base class:
     setMainCnvPStorage( &storage->layerMaterialMaps );

     // for all converters defined in this top level converter
     m_materialPropertiesCnv.setPStorage( &storage->materialStore );    
    
     m_blMaterialCnv.setPStorage( &storage->binnedLayerMaterial );              
     m_clMaterialCnv.setPStorage( &storage->compressedLayerMaterial );          
     m_hlMaterialCnv.setPStorage( &storage->homogeneousLayerMaterial );         

}