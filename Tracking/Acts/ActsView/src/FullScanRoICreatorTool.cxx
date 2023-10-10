/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "src/FullScanRoICreatorTool.h"


FullScanRoICreatorTool::FullScanRoICreatorTool(const std::string& type,
					       const std::string& name,
					       const IInterface* parent)
  : base_class(type, name, parent) 
{}

StatusCode FullScanRoICreatorTool::initialize()
{
  ATH_MSG_DEBUG("Inizializing " << name() << " ..." );

  ATH_CHECK(m_roiCollectionKey.initialize());

  return StatusCode::SUCCESS;
}

StatusCode FullScanRoICreatorTool::defineRegionsOfInterest(const EventContext& ctx,
							   std::vector< ElementLink< TrigRoiDescriptorCollection > >& ELs) const 
{
  // RoI collection gets stored in the SG
  SG::WriteHandle< TrigRoiDescriptorCollection > roiCollectionHandle = SG::makeHandle( m_roiCollectionKey, ctx );
  ATH_CHECK( roiCollectionHandle.record( std::make_unique< TrigRoiDescriptorCollection >() ) );
  TrigRoiDescriptorCollection *collectionRoI = roiCollectionHandle.ptr();

  // Add a Full Scan RoI
  collectionRoI->push_back( new TrigRoiDescriptor(true) );

  // Return element links to the created RoIs so that they can be used from the outside
  ELs.push_back( ElementLink< TrigRoiDescriptorCollection >( *collectionRoI, 0 ) );
  ATH_CHECK( ELs.back().isValid() );
  return StatusCode::SUCCESS;
}
