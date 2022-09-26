/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJiveXML/xAODMissingETRetriever.h"

#include "AthenaKernel/Units.h"
using Athena::Units::GeV;

namespace JiveXML {

  /**
   * This is the standard AthAlgTool constructor
   * @param type   AlgTool type name
   * @param name   AlgTool instance name
   * @param parent AlgTools parent owning this tool
   *
   * code reference for xAOD:     jpt6Feb14
   *  https://svnweb.cern.ch/trac/atlasgroups/browser/PAT/AODUpgrade/xAODReaderAlgs
   *
   * This is afirst 'skeleton' try for many xAOD retrievers to be done:
   *  xAOD::MissingET, xAOD::Vertex, xAOD::MissingET, xAOD::CaloCluster, xAOD::Jet
   *  xAOD::TrackParticle, xAOD::TauJet, xAOD::Muon
   *
   **/
  xAODMissingETRetriever::xAODMissingETRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent), m_typeName("ETMis"){

    //Only declare the interface
    declareInterface<IDataRetriever>(this);

    declareProperty("FavouriteMETCollection" ,m_sgKeyFavourite = "MET_RefFinal" ,
	"Collection to be first in output, shown in Atlantis without switching");
    declareProperty("OtherMETCollections" ,m_otherKeys,
	"Other collections to be retrieved. If list left empty, all available retrieved");
  }
  
  /**
   * For each jet collections retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODMissingETRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    
    ATH_MSG_DEBUG( "in retrieveAll()" );
    
    SG::ConstIterator<xAOD::MissingETContainer> iterator, end;
    const xAOD::MissingETContainer* MissingETs;

    //obtain the default collection first
    ATH_MSG_DEBUG( "Trying to retrieve " << dataTypeName() << " (" << m_sgKeyFavourite << ")" );
    StatusCode sc = evtStore()->retrieve(MissingETs, m_sgKeyFavourite);
    if (sc.isFailure() ) {
      ATH_MSG_WARNING( "Collection " << m_sgKeyFavourite << " not found in SG " );
    }else{
      DataMap data = getData(MissingETs);
      if ( FormatTool->AddToEvent(dataTypeName(), m_sgKeyFavourite+"_xAOD", &data).isFailure()){ //suffix can be removed later
	ATH_MSG_WARNING( "Collection " << m_sgKeyFavourite << " not found in SG " );
      }else{
         ATH_MSG_DEBUG( dataTypeName() << " (" << m_sgKeyFavourite << ") MissingET retrieved" );
      }
    }
 
    if ( m_otherKeys.empty() ) {
      //obtain all other collections from StoreGate
      if (( evtStore()->retrieve(iterator, end)).isFailure()){
         ATH_MSG_WARNING( "Unable to retrieve iterator for MET collection" );
//        return false;
      }
      
      for (; iterator!=end; iterator++) {
	  if (iterator.key()!=m_sgKeyFavourite) {
             ATH_MSG_DEBUG( "Trying to retrieve all " << dataTypeName() << " (" << iterator.key() << ")" );
             DataMap data = getData(&(*iterator));
             if ( FormatTool->AddToEvent(dataTypeName(), iterator.key()+"_xAOD", &data).isFailure()){
	       ATH_MSG_WARNING( "Collection " << iterator.key() << " not found in SG " );
	    }else{
	      ATH_MSG_DEBUG( dataTypeName() << " (" << iterator.key() << ") xAOD_MET retrieved" );
	    }
	}
      }
    }else {
      //obtain all collections with the given keys
      std::vector<std::string>::const_iterator keyIter;
      for ( keyIter=m_otherKeys.begin(); keyIter!=m_otherKeys.end(); ++keyIter ){
        if ( !evtStore()->contains<xAOD::MissingETContainer>( (*keyIter) ) ){ continue; } // skip if not in SG
	StatusCode sc = evtStore()->retrieve( MissingETs, (*keyIter) );
	if (!sc.isFailure()) {
          ATH_MSG_DEBUG( "Trying to retrieve selected " << dataTypeName() << " (" << (*keyIter) << ")" );
          DataMap data = getData(MissingETs);
          if ( FormatTool->AddToEvent(dataTypeName(), (*keyIter)+"_xAOD", &data).isFailure()){
	    ATH_MSG_WARNING( "Collection " << (*keyIter) << " not found in SG " );
	  }else{
	     ATH_MSG_DEBUG( dataTypeName() << " (" << (*keyIter) << ") retrieved" );
	  }
	}
      }
    } 
    //All collections retrieved okay
    return StatusCode::SUCCESS;
  }


  /**
   * Retrieve basic parameters, mainly four-vectors, for each collection.
   * Also association with clusters and tracks (ElementLink).
   */
  const DataMap xAODMissingETRetriever::getData(const xAOD::MissingETContainer* metCont) {
    
    ATH_MSG_DEBUG( "in getData()" );

    DataMap DataMap;

    DataVect etx; etx.reserve(metCont->size());
    DataVect ety; ety.reserve(metCont->size());
    DataVect et; et.reserve(metCont->size());

    float mpx = 0.;
    float mpy = 0.;
    float sumet = 0.;

    xAOD::MissingETContainer::const_iterator metItr  = metCont->begin();
    xAOD::MissingETContainer::const_iterator metItrE = metCont->end();

    // current understanding is that we only need the final value
    // out of the ~9 values within each MET container ('final')

    for (; metItr != metItrE; ++metItr) {
	sumet = (*metItr)->sumet()/GeV;
    	mpx = (*metItr)->mpx()/GeV;
       	mpy = (*metItr)->mpy()/GeV;
 
        ATH_MSG_DEBUG( "  Components: MissingET [GeV] mpx= "  << mpx
              << ", mpy= " << mpy
              << ", sumet= " << sumet );

    } // end MissingETIterator 

	   ATH_MSG_DEBUG( "  FINAL: MissingET [GeV] mpx= "  << mpx
		<< ", mpy= " << mpy << ", sumet= " << sumet );

    etx.push_back(DataType( mpx ));
    ety.push_back(DataType( mpy ));
    et.push_back(DataType( sumet ));

    // four-vectors
    DataMap["et"] = et;
    DataMap["etx"] = etx;
    DataMap["ety"] = ety;

    ATH_MSG_DEBUG( dataTypeName() << " retrieved with " << et.size() << " entries" );

    //All collections retrieved okay
    return DataMap;

  } // retrieve

  //--------------------------------------------------------------------------
  
} // JiveXML namespace
