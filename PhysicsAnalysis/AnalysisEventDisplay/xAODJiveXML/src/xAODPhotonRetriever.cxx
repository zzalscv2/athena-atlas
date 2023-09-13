/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODJiveXML/xAODPhotonRetriever.h"

#include "xAODEgamma/PhotonContainer.h" 

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
   *  xAOD::Photon, xAOD::Vertex, xAOD::Photon, xAOD::CaloCluster, xAOD::Jet
   *  xAOD::TrackParticle, xAOD::TauJet, xAOD::Muon
   *
   **/
  xAODPhotonRetriever::xAODPhotonRetriever(const std::string& type,const std::string& name,const IInterface* parent):
    AthAlgTool(type,name,parent), m_typeName("Photon"),
    m_sgKey("Photons") // is xAOD name

  {
    //Only declare the interface
    declareInterface<IDataRetriever>(this);

    declareProperty("StoreGateKey", m_sgKey, 
        "Collection to be first in output, shown in Atlantis without switching");
    declareProperty("OtherCollections" ,m_otherKeys,
        "Other collections to be retrieved. If list left empty, all available retrieved");
    declareProperty("DoWriteHLT"              , m_doWriteHLT = false, "Wether to write HLTAutoKey objects");
  }
  
  /**
   * For each jet collections retrieve basic parameters.
   * @param FormatTool the tool that will create formated output from the DataMap
   */
  StatusCode xAODPhotonRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {
    
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)  << "in retrieveAll()" << endmsg;
    
    SG::ConstIterator<xAOD::PhotonContainer> iterator, end;
    const xAOD::PhotonContainer* photons;
    
    //obtain the default collection first
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)  << "Trying to retrieve " << dataTypeName() << " (" << m_sgKey << ")" << endmsg;
    StatusCode sc = evtStore()->retrieve(photons, m_sgKey);
    if (sc.isFailure() ) {
      if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "Collection " << m_sgKey << " not found in SG " << endmsg; 
    }else{
      DataMap data = getData(photons);
      if ( FormatTool->AddToEvent(dataTypeName(), m_sgKey+"_xAOD", &data).isFailure()){ //suffix can be removed later
	if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "Collection " << m_sgKey << " not found in SG " << endmsg;
      }else{
         if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)  << dataTypeName() << " (" << m_sgKey << ") Photon retrieved" << endmsg;
      }
    }
 
    if ( m_otherKeys.empty() ) {
      //obtain all other collections from StoreGate
      if (( evtStore()->retrieve(iterator, end)).isFailure()){
         if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << 
	 "Unable to retrieve iterator for xAOD Muon collection" << endmsg;
//        return false;
      }
      
      for (; iterator!=end; ++iterator) {
	  if (iterator.key()!=m_sgKey) {
       	     if ((iterator.key().find("HLT",0) != std::string::npos) && (!m_doWriteHLT)){
	          if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "Ignoring HLT-AutoKey collection " << iterator.key() << endmsg;
	         continue;  }
             if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)  << "Trying to retrieve all. Current collection: " << dataTypeName() << " (" << iterator.key() << ")" << endmsg;
             DataMap data = getData(&(*iterator));
             if ( FormatTool->AddToEvent(dataTypeName(), iterator.key()+"_xAOD", &data).isFailure()){
	       if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "Collection " << iterator.key() << " not found in SG " << endmsg;
	    }else{
	      if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << dataTypeName() << " (" << iterator.key() << ") xAOD Photon retrieved" << endmsg;
	    }
          }
      }
    }else {
      //obtain all collections with the given keys
      std::vector<std::string>::const_iterator keyIter;
      for ( keyIter=m_otherKeys.begin(); keyIter!=m_otherKeys.end(); ++keyIter ){
	StatusCode sc = evtStore()->retrieve( photons, (*keyIter) );
	if (!sc.isFailure()) {
          if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG)  << "Trying to retrieve selected " << dataTypeName() << " (" << (*keyIter) << ")" << endmsg;
          DataMap data = getData(photons);
          if ( FormatTool->AddToEvent(dataTypeName(), (*keyIter), &data).isFailure()){
	    if (msgLvl(MSG::WARNING)) msg(MSG::WARNING) << "Collection " << (*keyIter) << " not found in SG " << endmsg;
	  }else{
	     if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << dataTypeName() << " (" << (*keyIter) << ") retrieved" << endmsg;
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
  const DataMap xAODPhotonRetriever::getData(const xAOD::PhotonContainer* phCont) {
    
    if (msgLvl(MSG::DEBUG)) msg(MSG::DEBUG) << "in getData()" << endmsg;

    DataMap DataMap;

    DataVect pt; pt.reserve(phCont->size());
    DataVect phi; phi.reserve(phCont->size());
    DataVect eta; eta.reserve(phCont->size());
    DataVect mass; mass.reserve(phCont->size());
    DataVect energy; energy.reserve(phCont->size());

    DataVect isEMString; isEMString.reserve(phCont->size());
    DataVect author; author.reserve(phCont->size());
    DataVect label; label.reserve(phCont->size());

    xAOD::PhotonContainer::const_iterator phItr  = phCont->begin();
    xAOD::PhotonContainer::const_iterator phItrE = phCont->end();

    int counter = 0;

    for (; phItr != phItrE; ++phItr) {

    if (msgLvl(MSG::DEBUG)) {
      msg(MSG::DEBUG) << "  Photon #" << counter++ << " : eta = "  << (*phItr)->eta() << ", phi = " 
          << (*phItr)->phi() << endmsg;
    }

    std::string photonAuthor = "";
    std::string photonIsEMString = "none";
    std::string photonLabel = "";

      phi.push_back(DataType((*phItr)->phi()));
      eta.push_back(DataType((*phItr)->eta()));
      pt.push_back(DataType((*phItr)->pt()/GeV));

      bool passesTight(false);
      bool passesMedium(false);
      bool passesLoose(false);
      const bool tightSelectionExists = (*phItr)->passSelection(passesTight, "Tight");
       msg(MSG::VERBOSE) << "tight exists " << tightSelectionExists 
	 << " and passes? " << passesTight << endmsg;
      const bool mediumSelectionExists = (*phItr)->passSelection(passesMedium, "Medium");
       msg(MSG::VERBOSE) << "medium exists " << mediumSelectionExists 
	 << " and passes? " << passesMedium << endmsg;
      const bool looseSelectionExists = (*phItr)->passSelection(passesLoose, "Loose");
       msg(MSG::VERBOSE) << "loose exists " << looseSelectionExists 
	<< " and passes? " << passesLoose << endmsg;

      photonAuthor = "author"+DataType( (*phItr)->author() ).toString(); // for odd ones eg FWD
      photonLabel = photonAuthor;
      if (( (*phItr)->author()) == 0){ photonAuthor = "unknown"; photonLabel += "_unknown"; }
      if (( (*phItr)->author()) == 8){ photonAuthor = "forward"; photonLabel += "_forward"; }
      if (( (*phItr)->author()) == 2){ photonAuthor = "softe"; photonLabel += "_softe"; }
      if (( (*phItr)->author()) == 1){ photonAuthor = "egamma"; photonLabel += "_egamma"; }

      if ( passesLoose ){  
            photonLabel += "_Loose"; 
            photonIsEMString = "Loose"; // assume that hierarchy is obeyed !
      } 
      if ( passesMedium ){ 
            photonLabel += "_Medium"; 
            photonIsEMString = "Medium"; // assume that hierarchy is obeyed !
      }   
      if ( passesTight ){ 
            photonLabel += "_Tight"; 
            photonIsEMString = "Tight"; // assume that hierarchy is obeyed !
      }     
      author.push_back( DataType( photonAuthor ) );
      label.push_back( DataType( photonLabel ) );
      isEMString.push_back( DataType( photonIsEMString ) );

      mass.push_back(DataType((*phItr)->m()/GeV));
      energy.push_back( DataType((*phItr)->e()/GeV ) );
    } // end PhotonIterator 

    // four-vectors
    DataMap["phi"] = phi;
    DataMap["eta"] = eta;
    DataMap["pt"] = pt;
    DataMap["energy"] = energy;
    DataMap["mass"] = mass;
    DataMap["isEMString"] = isEMString;
    DataMap["label"] = label;
    DataMap["author"] = author;

    if (msgLvl(MSG::DEBUG)) {
      msg(MSG::DEBUG) << dataTypeName() << " retrieved with " << phi.size() << " entries"<< endmsg;
    }

    //All collections retrieved okay
    return DataMap;

  } // retrieve

  //--------------------------------------------------------------------------
  
} // JiveXML namespace
