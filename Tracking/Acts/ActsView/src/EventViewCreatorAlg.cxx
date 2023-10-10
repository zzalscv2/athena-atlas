/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "EventViewCreatorAlg.h"
#include "AthViews/ViewHelper.h"

EventViewCreatorAlg::EventViewCreatorAlg(const std::string& name, 
					 ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode EventViewCreatorAlg::initialize() 
{
  ATH_MSG_DEBUG("Initialising " << name() << " ...");

  ATH_CHECK(m_viewsKey.initialize());
  ATH_CHECK(m_inViewRoIs.initialize());

  ATH_CHECK(m_roiTool.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode EventViewCreatorAlg::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("Executing " << name() << " ...");

  SG::WriteHandle< ViewContainer > viewsHandle = SG::makeHandle( m_viewsKey, ctx ); 
  ATH_CHECK( viewsHandle.record( std::make_unique<ViewContainer>() ) );
  ViewContainer *viewVector = viewsHandle.ptr();

  std::vector< ElementLink< TrigRoiDescriptorCollection > > roiELs;
  ATH_CHECK( m_roiTool->defineRegionsOfInterest(ctx, roiELs) );

  // Make the Event Views
  for (const ElementLink< TrigRoiDescriptorCollection >& roiEL : roiELs) {
    SG::View* newView = ViewHelper::makeView( name()+"_view", viewVector->size(), true);
    viewVector->push_back( newView );

    ATH_CHECK( placeRoIInView( roiEL, newView, ctx ) );
  }

  return StatusCode::SUCCESS;
}

StatusCode EventViewCreatorAlg::placeRoIInView( const ElementLink<TrigRoiDescriptorCollection>& roiEL, SG::View* view, const EventContext& ctx ) const {
  // fill the RoI output collection
  auto oneRoIColl = std::make_unique< ConstDataVector< TrigRoiDescriptorCollection > >(SG::VIEW_ELEMENTS);
  oneRoIColl->push_back( *roiEL );
  view->setROI(roiEL);

  //store the RoI in the view
  auto handle = SG::makeHandle( m_inViewRoIs, ctx );
  ATH_CHECK( handle.setProxyDict( view ) );
  ATH_CHECK( handle.record( std::move( oneRoIColl ) ) );
  return StatusCode::SUCCESS;
}


