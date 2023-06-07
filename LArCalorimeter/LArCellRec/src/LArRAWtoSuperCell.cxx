/*
 *   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 *
 * Name :       LArRAWtoSuperCell.cxx
 * PACKAGE :    LArCalorimeter/LArCell/LArRAWtoSuperCell
 *
 * AUTHOR :     Denis Oliveira Damazio
 *
 * PURPOSE :    prepares SuperCellContainer in CaloCellContainer formar from LArRawSCContainer
 *
 * **/

#include "LArRAWtoSuperCell.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "xAODEventInfo/EventInfo.h"
#include "LArRecConditions/LArBadChannel.h"

LArRAWtoSuperCell::LArRAWtoSuperCell( const std::string& name, ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm( name, pSvcLocator)
{
}

StatusCode
LArRAWtoSuperCell::initialize()
{

        ATH_CHECK( m_sCellContainerInKey.initialize() );
        ATH_CHECK( m_sCellContainerOutKey.initialize() );
        ATH_CHECK( m_cablingKey.initialize() );
	ATH_CHECK(detStore()->retrieve(m_laronline_id,"LArOnline_SuperCellID"));
	ATH_CHECK( m_caloMgrKey.initialize());
	ATH_CHECK( m_bcContKey.initialize(SG::AllowEmpty) );
        return StatusCode::SUCCESS;
}

StatusCode
LArRAWtoSuperCell::execute(const EventContext& context) const
{

	SG::ReadCondHandle<CaloSuperCellDetDescrManager> caloMgrHandle{m_caloMgrKey, context};
        const CaloSuperCellDetDescrManager* sem_mgr = *caloMgrHandle;;

        SG::WriteHandle<CaloCellContainer> scellContainerHandle( m_sCellContainerOutKey, context);
        auto new_scell_cont = std::make_unique<CaloCellContainer> ();
	
        auto cellsHandle = SG::makeHandle( m_sCellContainerInKey, context );
        if ( not cellsHandle.isValid() ) {
          ATH_MSG_ERROR("Did not get CaloCellContainer input");
	  // to avoid crash
	  ATH_CHECK( scellContainerHandle.record( std::move(new_scell_cont) ) );
          return StatusCode::FAILURE;
        }
	const LArOnOffIdMapping* cabling;
	SG::ReadCondHandle<LArOnOffIdMapping> cablingHdl{m_cablingKey,context};
	cabling=*cablingHdl;

	const LArBadChannelCont* badchannel(nullptr);
        if ( !m_bcContKey.empty() ){
        SG::ReadCondHandle<LArBadChannelCont> larBadChan{ m_bcContKey, context };
	badchannel = *larBadChan;
        }
        
        const LArRawSCContainer* scells_from_sg = cellsHandle.cptr();
        ATH_MSG_DEBUG("Got a CaloCellContainer input with size : "<<scells_from_sg->size());
	if ( scells_from_sg->size() == 0 ) {
	  ATH_MSG_WARNING("Got an empty input collection, maybe the key is wrong : "
		<< m_sCellContainerInKey );
	  // to avoid crash
	  ATH_CHECK( scellContainerHandle.record( std::move(new_scell_cont) ) );
	  return StatusCode::SUCCESS;
	}

	const EventIDBase& EIHandle = context.eventID();
        const unsigned int bcid = EIHandle.bunch_crossing_id();
        
        new_scell_cont->reserve(scells_from_sg->size());

        for(auto sc : *scells_from_sg){
                if ( !sc ) continue;
                Identifier off_id = cabling->cnvToIdentifier(sc->hardwareID()); 
                const CaloDetDescrElement* dde = sem_mgr ->get_element(off_id);
		CaloCell* cell = new CaloCell();
		cell->setCaloDDE(dde);
		const std::vector< unsigned short >& bcids = sc->bcids();
		const std::vector< int >& energies = sc->energies();
		const std::vector< bool>& satur = sc->satur();
		float energy(0.);
		bool saturation(false);
		for(unsigned int i=0;i<bcids.size();++i){ 
                   if ( bcids[i]==bcid+m_bcidOffset ) {
                      energy=energies[i]; 
                      saturation = satur[i];
                      break;
                   }
                }
		// convert ET (coming from LATOMEs) into Energy and
		// apply magic 12.5 factor
                cell->setEnergy( 12.5*energy*cosh(cell->eta()) );

		// set some provenance to indicate bad channel
		if(badchannel) {
		   LArBadChannel bc = badchannel->offlineStatus(off_id);
		   if ( !bc.good() && bc.statusBad(LArBadChannel::LArBadChannelSCEnum::maskedOSUMBit) ){
		     cell->setProvenance(cell->provenance()|0x80);
		   }
		   
		}
		// we probably should soon associate some quality information to the saturation, maybe the bcid to provenance
		cell->setQuality((unsigned short)saturation);
                new_scell_cont->push_back( std::move(cell) );
        }
	ATH_CHECK( scellContainerHandle.record( std::move(new_scell_cont) ) );

        return StatusCode::SUCCESS;
}
