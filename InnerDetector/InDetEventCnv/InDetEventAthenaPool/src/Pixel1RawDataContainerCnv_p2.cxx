/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetRawData/Pixel1RawData.h"
#include "InDetEventAthenaPool/InDetRawData_p2.h"
#include "InDetEventAthenaPool/InDetRawDataCollection_p1.h"
#include "InDetRawData/PixelRDO_Container.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetRawData/PixelRDO_Collection.h"
#include "Pixel1RawDataCnv_p2.h"
#include "Pixel1RawDataContainerCnv_p2.h"
#include "MsgUtil.h"

#include "AthAllocators/DataPool.h"


void Pixel1RawDataContainerCnv_p2::transToPers(const PixelRDO_Container* transCont, InDetRawDataContainer_p2* persCont, MsgStream &log)
{

    // The transient model has a container holding collections and the
    // collections hold channels.
    //
    // The persistent model flattens this so that the persistent
    // container has two vectors:
    //   1) all collections, and
    //   2) all RDO
    //
    // The persistent collections, then only maintain indexes into the
    // container's vector of all channels.
    //
    // So here we loop over all collection and add their channels
    // to the container's vector, saving the indexes in the
    // collection.

    using TRANS = PixelRDO_Container;

    Pixel1RawDataCnv_p2  chanCnv;
    TRANS::const_iterator it_Coll     = transCont->begin();
    TRANS::const_iterator it_CollEnd  = transCont->end();
    unsigned int chanBegin = 0;
    unsigned int chanEnd = 0;
    unsigned int numColl = transCont->numberOfCollections();

    persCont->m_collections.resize(numColl);
    MSG_DEBUG(log, " Preparing " << persCont->m_collections.size() << " Collections" );

    for (unsigned int collIndex = 0; it_Coll != it_CollEnd; ++collIndex, ++it_Coll)  {
        // Add in new collection
        const PixelRDO_Collection& collection = (**it_Coll);
        chanBegin  = chanEnd;
        chanEnd   += collection.size();
        if(collIndex >= numColl) log << MSG::ERROR << "Accessing collIndex " << collIndex << "/" << numColl << endmsg;
        InDetRawDataCollection_p1& pcollection = persCont->m_collections[collIndex];

        pcollection.m_id    = collection.identify().get_identifier32().get_compact();
        pcollection.m_hashId = (unsigned int) collection.identifyHash();
        pcollection.m_begin = chanBegin;
        pcollection.m_end   = chanEnd;
        // Add in channels
        persCont->m_rawdata.resize(chanEnd);
        for (unsigned int i = 0; i < collection.size(); ++i) {
            InDetRawData_p2* pchan = &(persCont->m_rawdata[i + chanBegin]);
            const Pixel1RawData* chan = dynamic_cast<const Pixel1RawData*>(collection[i]);
            if (nullptr == chan) {
              throw std::runtime_error(
                  "Pixel1RawDataContainerCnv_p2::transToPers: ***  UNABLE TO "
                  "DYNAMIC CAST TO Pixel1RawData");
            }
            chanCnv.transToPers(chan, pchan, log);
        }
    }
    MSG_DEBUG(log," ***  Writing PixelRDO_Container (Pixel1RawData concrete type)");
}

void  Pixel1RawDataContainerCnv_p2::persToTrans(const InDetRawDataContainer_p2* persCont, PixelRDO_Container* transCont, MsgStream &log)
{

    // The transient model has a container holding collections and the
    // collections hold channels.
    //
    // The persistent model flattens this so that the persistent
    // container has two vectors:
    //   1) all collections, and
    //   2) all channels
    //
    // The persistent collections, then only maintain indexes into the
    // container's vector of all channels.
    //
    // So here we loop over all collection and extract their channels
    // from the vector.
    Pixel1RawDataCnv_p2  chanCnv;
    //
    const size_t numCollections = persCont->m_collections.size();
    std::vector<size_t> chans_per_collection{};
    chans_per_collection.reserve(numCollections);
    size_t totalChannels = 0;
    //
    MSG_DEBUG(log, " Reading " << numCollections << "Collections");
    for (unsigned int icoll = 0; icoll < numCollections; ++icoll) {
        const InDetRawDataCollection_p1& pcoll = persCont->m_collections[icoll];
        unsigned int nchans = pcoll.m_end - pcoll.m_begin;
        chans_per_collection.push_back(nchans);
        totalChannels += nchans;
    }
    DataPool<Pixel1RawData> dataItems;
    dataItems.prepareToAdd(totalChannels);

    for (unsigned int icoll = 0; icoll < numCollections; ++icoll) {
        const InDetRawDataCollection_p1& pcoll = persCont->m_collections[icoll];
        Identifier collID(pcoll.m_id);
        IdentifierHash collIDHash(IdentifierHash(pcoll.m_hashId));
        //
        PixelRDO_Collection* coll = new PixelRDO_Collection(collIDHash);
        coll->setIdentifier(collID);
        coll->clear(SG::VIEW_ELEMENTS);
        unsigned int nchans = chans_per_collection[icoll];
        coll->resize(nchans);
        // Fill with channels
        for (unsigned int ichan = 0; ichan < nchans; ++ichan) {
            const InDetRawData_p2* pchan =
                &(persCont->m_rawdata[ichan + pcoll.m_begin]);
            Pixel1RawData* chan = dataItems.nextElementPtr();
            chanCnv.persToTrans(pchan, chan, log);
            (*coll)[ichan] = chan;
        }

        StatusCode sc = transCont->addCollection(coll, collIDHash);
        if (sc.isFailure()) {
            throw std::runtime_error(
                "Failed to add collection to ID Container");
        }
    }

    MSG_DEBUG(log,
              " ***  Reading PixelRDO_Container (Pixel1RawData concrete type)");
}

//================================================================
PixelRDO_Container* Pixel1RawDataContainerCnv_p2::createTransient(const InDetRawDataContainer_p2* persObj, MsgStream& log) {
    std::unique_ptr<PixelRDO_Container> trans(std::make_unique<PixelRDO_Container>(m_pixId->wafer_hash_max()));
    persToTrans(persObj, trans.get(), log);
    return(trans.release());
}

