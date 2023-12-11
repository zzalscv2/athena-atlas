///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

// JetCnv_p1.cxx 
// Implementation file for class JetCnv_p2
// Author: R.Seuster<seuster@cern.ch>
///////////////////////////////////////////////////////////////////

// STL includes

// JetEvent includes
#include "JetEvent/Jet.h"
#include "JetEvent/JetTagInfoBase.h"
#include "JetEvent/JetAssociationBase.h"

// DataModelAthenaPool includes
#include "DataModelAthenaPool/NavigableCnv_p1.h"

// EventCommonTPCnv includes
#include "EventCommonTPCnv/P4ImplPxPyPzECnv_p1.h"

// JetEventTPCnv includes
#include "JetEventTPCnv/JetCnv_p2.h"

using NavigableCnv_t = NavigableCnv_p1<Navigable<INavigable4MomentumCollection, double>>;

// pre-allocate converters
static const P4ImplPxPyPzECnv_p1   momCnv;
static const NavigableCnv_t        navCnv;


void JetCnv_p2::persToTrans( const Jet_p2* pers,
                             Jet* trans, 
                             MsgStream& msg ) const
{
  msg << MSG::DEBUG << "Loading Jet from persistent state..."
      << endmsg;

   navCnv.persToTrans( &pers->m_nav,      
		       &trans->navigableBase(), msg );
   momCnv.persToTrans( &pers->m_momentum, &trans->momentumBase(),  msg );
   
   trans->m_jetAuthor = pers->m_author;

   //trans->m_combinedLikelihood = std::vector<double>( pers->m_combinedLikelihood );
   trans->setCombinedLikelihood(pers->m_combinedLikelihood);
   trans->m_jetId = Jet::s_defaultJetId;
   
   // use swap() to avoid copying vector contents.
   //const_cast<Jet_p2*>(pers)->m_shapeStore.swap ( *trans->m_shapeStore);
   
   // create the store only if non-zero size (waste of space for constituents)

   // Use swap to avoid copying the vector contents.

   JetKeyDescriptorInstance * keydesc = JetKeyDescriptorInstance::instance();
   std::vector<std::string> momentNames = keydesc->getKeys(JetKeyConstants::ShapeCat);
   if( momentNames.size() != (pers)->m_shapeStore.size() ) msg << MSG::ERROR << " JEtCnv_p2 can't convert moments ! expected moment n= "<< momentNames.size() << " persistatn has "<< (pers)->m_shapeStore.size() <<endmsg;
   else for(size_t i=0;i<momentNames.size();i++){
       trans->setMoment(momentNames[i], (pers)->m_shapeStore[i], true);
     }


   if (trans->m_tagInfoStore)
     const_cast<Jet_p2*>(pers)->m_tagJetInfo.swap (*trans->m_tagInfoStore);
   else if ( !(pers)->m_tagJetInfo.empty() ) {
     trans->m_tagInfoStore = new Jet::tagstore_t;
     const_cast<Jet_p2*>(pers)->m_tagJetInfo.swap (*trans->m_tagInfoStore);
   }

   if (trans->m_assocStore)
     const_cast<Jet_p2*>(pers)->m_associations.swap (*trans->m_assocStore);
   else if ( !(pers)->m_associations.empty() ) {
     trans->m_assocStore   = new Jet::assostore_t;
     const_cast<Jet_p2*>(pers)->m_associations.swap (*trans->m_assocStore);
   }

   // Avoid memory leaks.
   for (size_t i = 0; i < pers->m_tagJetInfo.size(); i++)
     delete pers->m_tagJetInfo[i];
   for (size_t i = 0; i < pers->m_associations.size(); i++)
     delete pers->m_associations[i];
   
   const_cast<Jet_p2*>(pers)->m_shapeStore.clear();
   const_cast<Jet_p2*>(pers)->m_tagJetInfo.clear();
   const_cast<Jet_p2*>(pers)->m_associations.clear();

   // default signal state
   trans->setSignalState(P4SignalState::CALIBRATED);

  // Jet comes from a DataPool.
  // So we need to be sure to re-initialize everything in the Jet.
  trans->setRawE( trans->e() ) ;
  trans->setRawPx( trans->px() ) ;
  trans->setRawPy( trans->py() ) ;
  trans->setRawPz( trans->pz() ) ;

  static const Jet jtmp;
  trans->particleBase() = jtmp.particleBase();

   msg << MSG::DEBUG << "Loaded Jet from persistent state [OK]"
       << endmsg;
}

void JetCnv_p2::transToPers( const Jet* /*trans*/, 
                             Jet_p2* /*pers*/, 
                             MsgStream& msg ) const
{
  msg << MSG::ERROR << " Don't write Jet_p2 anymore"
      << endmsg;

  // pers->m_ownPointers = false;

  // navCnv.transToPers( &trans->navigableBase(), &pers->m_nav,      msg );
  // momCnv.transToPers( &trans->momentumBase(),  &pers->m_momentum, msg );

  
  // pers->m_author = trans->m_jetAuthor;
  
  // // no more needed : we don't save nomore to p2
  // //pers->m_combinedLikelihood = std::vector<double>( trans->m_combinedLikelihood );
  

  // if ( bool(trans->m_tagInfoStore) )
  //   pers->m_tagJetInfo   = *(trans->m_tagInfoStore);
  // else
  //   pers->m_tagJetInfo.clear();
  // if ( bool(trans->m_assocStore) )
  //   pers->m_associations = *(trans->m_assocStore);
  // else
  //   pers->m_associations.clear();
  
  // msg << MSG::DEBUG << "Created persistent state of Jet [OK]"
  //     << endmsg;
  // return;
}
