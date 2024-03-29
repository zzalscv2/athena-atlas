/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "ParticleTruth/TrackParticleTruthCollection.h"
#include "TrackParticleTruthTPCnv/TrackParticleTruthCollectionCnv_p1.h"

#include "TrackParticleTruthTPCnv/TrackParticleTruthCollection_p1.h"

#include "GeneratorObjectsTPCnv/HepMcParticleLinkCnv_p1.h"
#include "DataModelAthenaPool/DataLinkCnv_p1.h"
#include "DataModelAthenaPool/ElementLinkCnv_p3.h"

#include "StoreGate/StoreGateSvc.h"

#include "TrackParticleTruthTPCnv/TrackParticleTruthCollectionAccessor.h"

namespace {
    const HepMcParticleLinkCnv_p1 particleLinkConverter;
    const DataLinkCnv_p1<DataLink<Rec::TrackParticleContainer> > dataLinkConverter;
}

typedef ElementLinkCnv_p3<ElementLink<Rec::TrackParticleContainer> > TrackLinkCnv_t;

void TrackParticleTruthCollectionCnv_p1::persToTrans( const Rec::TrackParticleTruthCollection_p1* pers,
                                                      TrackParticleTruthCollection* trans, 
                                                      MsgStream& msg ) const
{
    trans->clear();

    if (msg.level() <= MSG::DEBUG) msg << MSG::DEBUG << "TrackParticleTruthCollectionCnv_p1::persToTrans(): PERS size = " << pers->m_entries.size() << endmsg;

    dataLinkConverter.persToTrans(pers->m_trackCollectionLink, TrackParticleTruthCollectionAccessor::trackParticleContainerLink(trans), msg);

    for(Rec::TrackParticleTruthCollection_p1::CollectionType::const_iterator i=pers->m_entries.begin(); i!=pers->m_entries.end(); ++i) {
        HepMcParticleLink link;
        particleLinkConverter.persToTrans(&i->particle, &link, msg);
        // Reconstruct EL from the key of the DataLink and the saved index
        ElementLink<Rec::TrackParticleContainer>  el(pers->m_trackCollectionLink.m_SGKeyHash, i->index);
        trans->insert(trans->end(), std::make_pair(el, TrackParticleTruth(link, i->probability)) );
    }

    if (msg.level() <= MSG::DEBUG) msg << MSG::DEBUG << "TrackParticleTruthCollectionCnv_p1::persToTrans() DONE" << endmsg;
}

void TrackParticleTruthCollectionCnv_p1::transToPers( const TrackParticleTruthCollection* trans, 
                                                      Rec::TrackParticleTruthCollection_p1* pers, 
                                                      MsgStream& msg ) const
{
    if (msg.level() <= MSG::DEBUG) {
        msg << MSG::DEBUG << "TrackParticleTruthCollectionCnv_p1::transToPers()" << endmsg;
        msg << MSG::DEBUG << "TrackParticleTruthCollectionCnv_p1::transToPers(): input size = " << trans->size() << endmsg;
    }
  
    dataLinkConverter.transToPers(trans->trackParticleContainerLink(), pers->m_trackCollectionLink, msg);

    // For the map of EL keys and HepParticleLinks, we only write out
    // pairs which were not thinned away. RDS 02/2012
    pers->m_entries.reserve(trans->size());
    Rec::TrackParticleTruthCollection_p1::CollectionType::size_type ipers(0);
    TrackParticleTruthCollection::const_iterator itrans = trans->begin(); 
    for( ; itrans!=trans->end(); ++itrans, ++ipers) {
        Rec::TrackParticleTruthCollection_p1::Entry entry;
        const TrackParticleTruth& truth = itrans->second;
        // We must first convert the element link to check whether or
        // not it was thinned, then we write it out if it was not
        // thinned
        ElementLinkInt_p3 el_p;
        TrackLinkCnv_t    elCnv_p;
        elCnv_p.transToPers(itrans->first.link(), el_p, msg);
        if (0 != el_p.m_SGKeyHash) {
            entry.index = el_p.m_elementIndex; // save index from ElementLink
            entry.probability = truth.probability();
            particleLinkConverter.transToPers(&itrans->second.particleLink(), &entry.particle, msg);
            pers->m_entries.push_back(entry);
        }
    }

    if (msg.level() <= MSG::DEBUG) msg << MSG::DEBUG << "TrackParticleTruthCollectionCnv_p1::transToPers() DONE" << endmsg;
}
