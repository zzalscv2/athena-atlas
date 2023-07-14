/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthMetaDataWriter.cxx
// Author: James Catmore (James.Catmore@cern.ch)

// Header for this class
#include "TruthMetaDataWriter.h"

// EDM Objects that we need
#include "xAODTruth/TruthMetaData.h"
#include "xAODTruth/TruthMetaDataAuxContainer.h"
#include "xAODEventInfo/EventInfo.h"

// For accessing the tagInfo
#include "AthenaPoolUtilities/CondAttrListCollection.h"

// Service for the weights
#include "GenInterfaces/IHepMCWeightSvc.h"

// Constructor
DerivationFramework::TruthMetaDataWriter::TruthMetaDataWriter(const std::string& t,
                                                              const std::string& n,
                                                              const IInterface* p)
  : AthAlgTool(t,n,p)
  , m_metaStore( "MetaDataStore", n )
  , m_weightSvc( "HepMCWeightSvc/HepMCWeightSvc" , n )
  , m_tagInfoMgr("TagInfoMgr", n)
{
    declareInterface<DerivationFramework::IAugmentationTool>(this);
    declareProperty( "MetaObjectName", m_metaName = "TruthMetaData" );
    declareProperty( "MetaDataStore", m_metaStore );
}

// Destructor
DerivationFramework::TruthMetaDataWriter::~TruthMetaDataWriter() {
}

// Athena initialize and finalize
StatusCode DerivationFramework::TruthMetaDataWriter::initialize()
{
    ATH_MSG_VERBOSE("initialize() ...");
    // Initialize the service handles
    CHECK( m_metaStore.retrieve() );
    CHECK( m_weightSvc.retrieve() );
    CHECK( m_tagInfoMgr.retrieve() );

    // Create an empty truth meta data container:
    xAOD::TruthMetaDataAuxContainer* aux = new xAOD::TruthMetaDataAuxContainer();
    m_tmd = new xAOD::TruthMetaDataContainer();
    m_tmd->setStore( aux );
    // Record it in the metadata store
    CHECK( m_metaStore->record( aux, m_metaName + "Aux." ) );
    CHECK( m_metaStore->record( m_tmd, m_metaName ) );

    return StatusCode::SUCCESS;
}

// Selection and collection creation
StatusCode DerivationFramework::TruthMetaDataWriter::addBranches() const
{

    //The mcChannelNumber is used as a unique identifier for which truth meta data belongs to
    uint32_t mcChannelNumber = 0;
    // If this fails, we are running on a datatype with no EventInfo.  Such data types should
    //  definitely not be mixing MC samples, so this should be safe (will fall back to 0 above)
    if (evtStore()->contains<xAOD::EventInfo>("EventInfo")){
      const xAOD::EventInfo* eventInfo = nullptr;
      CHECK( evtStore()->retrieve(eventInfo, "EventInfo") );
      mcChannelNumber = eventInfo->mcChannelNumber();
    }

    //Inserting in a (unordered_)set returns an <iterator, boolean> pair, where the boolean
    //is used to check if the key already exists (returns false in the case it exists)
    if( m_existingMetaDataChan.insert(mcChannelNumber).second ) {
        xAOD::TruthMetaData* md = new xAOD::TruthMetaData();
        m_tmd->push_back( md );

        // Get the list of weights from the metadata
        std::map<std::string,std::size_t> weight_name_map = m_weightSvc->weightNames();

        std::vector<std::string> orderedWeightNameVec;
        orderedWeightNameVec.reserve( weight_name_map.size() );
        for (auto& entry: weight_name_map) {
            orderedWeightNameVec.push_back(entry.first);
        }

        //The map from the HepMC record pairs the weight names with a corresponding index,
        //it is not guaranteed that the indices are ascending when iterating over the map
        std::sort(orderedWeightNameVec.begin(), orderedWeightNameVec.end(),
                  [&](const std::string& i, const std::string& j){return weight_name_map.at(i) < weight_name_map.at(j);});

        md->setMcChannelNumber(mcChannelNumber);
        md->setWeightNames( orderedWeightNameVec );

        md->setLhefGenerator(m_tagInfoMgr->findTag("lhefGenerator"));
        md->setGenerators(m_tagInfoMgr->findTag("generators"));
        md->setEvgenProcess(m_tagInfoMgr->findTag("evgenProcess"));
        md->setEvgenTune(m_tagInfoMgr->findTag("evgenTune"));
        md->setHardPDF(m_tagInfoMgr->findTag("hardPDF"));
        md->setSoftPDF(m_tagInfoMgr->findTag("softPDF"));

        // Done getting things from the TagInfo

    } // Done making the new truth metadata object
    return StatusCode::SUCCESS;
}
