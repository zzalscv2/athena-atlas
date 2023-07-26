/*
  General-purpose view creation algorithm <bwynne@cern.ch>
  
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include "EventViewCreatorAlgorithm.h"
#include "AthLinks/ElementLink.h"
#include "AthViews/ViewHelper.h"
#include "AthViews/View.h"
#include "TrigCompositeUtils/TrigCompositeUtils.h"

#include <sstream>

using namespace TrigCompositeUtils;

EventViewCreatorAlgorithm::EventViewCreatorAlgorithm( const std::string& name, ISvcLocator* pSvcLocator )
: InputMakerBase( name, pSvcLocator ) {}

EventViewCreatorAlgorithm::~EventViewCreatorAlgorithm(){}

StatusCode EventViewCreatorAlgorithm::initialize() {
  ATH_MSG_DEBUG("Will produce views=" << m_viewsKey << " roIs=" << m_inViewRoIs );
  ATH_CHECK( m_viewsKey.initialize() );
  ATH_CHECK( m_inViewRoIs.initialize() );
  ATH_CHECK( m_roiTool.retrieve() );
  ATH_CHECK( m_cachedViewsKey.initialize(SG::AllowEmpty) );
  if (not m_cachedViewsKey.empty()) {
    renounce(m_cachedViewsKey); // Reading in and using cached inputs is optional, not guarenteed to be produced in every event.
  }

  if (m_isEmptyStep) {
    ATH_MSG_ERROR("The EventViewCreatorAlgorithm class cannot be used as the InputMaker for an empty step.");
    return StatusCode::FAILURE;
  }

  // Muon slice code
  ATH_CHECK( m_inViewMuons.initialize(m_placeMuonInView) );
  ATH_CHECK( m_inViewMuonCandidates.initialize(m_placeMuonInView) );

  // Jet slice code
  ATH_CHECK( m_inViewJets.initialize(m_placeJetInView) );

  return StatusCode::SUCCESS;
}


StatusCode EventViewCreatorAlgorithm::execute( const EventContext& context ) const {

   // create the output decisions from the input collections
  ATH_MSG_DEBUG("Starting to merge " << decisionInputs().size() << " inputs to the " << decisionOutputs().key() << " output.");
  SG::WriteHandle<DecisionContainer> outputHandle = createAndStore( decisionOutputs(), context );
  ATH_CHECK(outputHandle.isValid());
  ATH_CHECK(decisionInputToOutput(context, outputHandle));
  ATH_MSG_DEBUG("Merging complete");

  // make the views
  auto viewsHandle = SG::makeHandle( m_viewsKey, context ); 
  ATH_CHECK( viewsHandle.record( std::make_unique<ViewContainer>() ) );
  auto viewVector = viewsHandle.ptr();

  // Check for an optional input handle to use as a source of cached, already-executed, views.
  const DecisionContainer* cachedViews = nullptr;
  MatchingCache matchingCache; // Used to remember temporarily which hash is associated with each DecisionObject when performing maching in a PROBE IM to TAG IM DecisionObjects
  if (!m_cachedViewsKey.empty()) {
    SG::ReadHandle<DecisionContainer> cachedRH = SG::makeHandle(m_cachedViewsKey, context);
    // Even if the handle is configured, this precursor EventViewCreatorAlg may not have executed in a given event
    if (cachedRH.isValid()) {
      cachedViews = cachedRH.ptr();
      ATH_CHECK(populateMatchingCacheWithCachedViews(cachedViews, matchingCache, context));
    }
  }

  // Keep track of the ROIs we spawn a View for, do not spawn duplicates.
  // For many cases, this will be covered by the Merging operation preceding this.
  std::vector<ElementLink<TrigRoiDescriptorCollection>> RoIsFromDecision;

  if( outputHandle->size() == 0) {
    ATH_MSG_DEBUG( "Have no decisions in output handle "<< outputHandle.key() << ". Handle is valid but container is empty. "
      << "This can happen if a ROI-based HLT chain leg was activated in a chain whose L1 item which does not explicitly require the ROI.");
  } else {
    ATH_MSG_DEBUG( "Have output " << outputHandle.key() << " with " << outputHandle->size() << " elements" );
  }

  // Find and link to the output Decision objects the ROIs to run over
  ATH_CHECK( m_roiTool->attachROILinks(*outputHandle, context) );

  for ( Decision* outputDecision : *outputHandle ) { 

    if (!outputDecision->hasObjectLink(roiString(), ClassID_traits<TrigRoiDescriptorCollection>::ID())) {
      ATH_MSG_ERROR("No '" << roiString() << "'link was attached by the ROITool. Decision object dump:" << *outputDecision);
      return StatusCode::FAILURE;
    }
    const ElementLink<TrigRoiDescriptorCollection> roiEL = outputDecision->objectLink<TrigRoiDescriptorCollection>(roiString());
    ATH_CHECK(roiEL.isValid());

    // We do one of three things here, either... 
    // a) We realise that an identically configured past EVCA has already run a View on an equivalent ROI. If so we can re-use this.
    // b) We encounter a new ROI and hence need to spawn a new view.
    // c) We encounter a ROI that we have already seen in looping over this outputHandle, we can re-use a view.
      
    // cachedIndex and useCached are to do with a)
    size_t cachedIndex = std::numeric_limits<std::size_t>::max();
    const bool useCached = checkCache(cachedViews, outputDecision, cachedIndex, matchingCache, context);

    // roiIt is to do with b) and c)
    auto roiIt = find(RoIsFromDecision.begin(), RoIsFromDecision.end(), roiEL);

    if (useCached) {

      // Re-use an already processed view from a previously executed EVCA instance
      const Decision* cached = cachedViews->at(cachedIndex);
      ElementLink<ViewContainer> cachedViewEL = cached->objectLink<ViewContainer>(viewString());
      ElementLink<TrigRoiDescriptorCollection> cachedROIEL = cached->objectLink<TrigRoiDescriptorCollection>(roiString());
      ATH_CHECK(cachedViewEL.isValid());
      ATH_CHECK(cachedROIEL.isValid());
      ATH_MSG_DEBUG("Re-using cached existing view from " << cachedViewEL.dataID() << ", index:" << cachedViewEL.index() << " on ROI " << **cachedROIEL);
      outputDecision->setObjectLink( viewString(), cachedViewEL );
      outputDecision->setObjectLink( roiString(), cachedROIEL );
      // Note: This overwrites the link created in the above tool with what should be a spatially identical ROI (check?)

    } else if ( roiIt == RoIsFromDecision.end() ) {

      // We have not yet spawned an ROI on this View. Do it now.
      RoIsFromDecision.push_back(roiEL);
      ATH_MSG_DEBUG("Found RoI:" << **roiEL << " FS=" << (*roiEL)->isFullscan() << ". Making View.");
      SG::View* newView = ViewHelper::makeView( name()+"_view", viewVector->size() /*view counter*/, m_viewFallThrough );
      viewVector->push_back( newView );
      // Use a fall-through filter if one is provided
      if ( m_viewFallFilter.size() ) {
        newView->setFilter( m_viewFallFilter );
      }
      // Set parent view, if required. Note: Must be called before we link the new view to outputDecision.
      ATH_CHECK(linkViewToParent(outputDecision, newView));
      // Add the single ROI into the view to seed it.
      ATH_CHECK(placeRoIInView(roiEL, viewVector->back(), context));
      // Special muon case - following from a FullScan view, seed each new View with its MuonCombined::MuonCandidate
      if (m_placeMuonInView) {
        std::vector<LinkInfo<xAOD::MuonContainer>> muonELInfo = findLinks<xAOD::MuonContainer>(outputDecision, featureString(), TrigDefs::lastFeatureOfType);
        ATH_CHECK( muonELInfo.size() == 1 );
        ATH_CHECK( muonELInfo.at(0).isValid() );
        ATH_CHECK( placeMuonInView( *(muonELInfo.at(0).link), viewVector->back(), context ) );
      }
      // Special jet case - following from a FullScan view, seed each new View with its xAOD::Jet
      if (m_placeJetInView) {
        std::vector<LinkInfo<xAOD::JetContainer>> jetELInfo = findLinks<xAOD::JetContainer>(outputDecision, featureString(), TrigDefs::lastFeatureOfType);
        ATH_CHECK( jetELInfo.size() == 1 );
        ATH_CHECK( jetELInfo.at(0).isValid() );
        ATH_CHECK( placeJetInView( *(jetELInfo.at(0).link), viewVector->back(), context ) );
      }
      // Link the view to the Decision object
      outputDecision->setObjectLink( viewString(), ElementLink<ViewContainer>(m_viewsKey.key(), viewVector->size()-1 ));
      ATH_MSG_DEBUG( "Made new View, storing view in viewVector " << m_viewsKey.key() << " index:" << viewVector->size()-1 );

    } else {

      // We have already spawned a ROI in this View. Link it here too.
      const size_t existingIndex = std::distance(RoIsFromDecision.begin(), roiIt);
      ATH_MSG_DEBUG("Found existing View, stored in view in viewVector " << m_viewsKey.key() << " index:" << existingIndex );
      outputDecision->setObjectLink( viewString(), ElementLink<ViewContainer>(m_viewsKey.key(), existingIndex )); //adding View link to Decision

    }
  } // loop over output decisions   

  // launch view execution
  ATH_MSG_DEBUG( "Launching execution in " << viewVector->size() << " unique views" );
  ATH_CHECK( ViewHelper::scheduleViews( viewVector, // Vector containing views
    m_viewNodeName,                                 // CF node to attach views to
    context,                                        // Source context
    getScheduler(),                                 // Scheduler to launch with
    m_reverseViews ) );                             // Debug option
  
  return StatusCode::SUCCESS;
}

bool endsWith(const std::string& value, const std::string& ending) {
  if (ending.size() > value.size()) {
    return false;
  }
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::vector<LinkInfo<ViewContainer>> EventViewCreatorAlgorithm::viewsToLink(const Decision* outputDecision) const { 
  return findLinks<ViewContainer>(outputDecision, viewString(), TrigDefs::lastFeatureOfType);
}

bool EventViewCreatorAlgorithm::checkCache(const DecisionContainer* cachedViews, const Decision* outputDecision, size_t& cachedIndex, MatchingCache& matchingCache, const EventContext& context) const {
  if (cachedViews == nullptr or m_cacheDisabled) {
    return false; // No cached input configured, which is fine.
  }

  // If we ever stop using cached views mid-processing of a chain, then it is by far safer to continue to not use cached views in all following steps. See for example towards the end of ATR-25996
  // We can tell this by querying for View instances in previous steps and looking for evidence of the instance being cached (no "_probe" postfix, from initiall "tag" pass)
  // or not cached (with "_probe" postfix, from second "probe" pass)
  std::vector<LinkInfo<ViewContainer>> previousStepViews = findLinks<ViewContainer>(outputDecision, viewString(), TrigDefs::allFeaturesOfType);
  // If this collection is empty then we're the 1st step, so OK to look for a cached EventView to re-use. Otherwise...
  if (previousStepViews.size()) { 
    // If there are one or more prior steps, we want to focus on the most recent which will be the first entry in the vector
    ElementLink<ViewContainer> previousView = previousStepViews.at(0).link;
    const bool previousStepDidNotUsedCachedView = endsWith(previousView.dataID(), "_probe");
    if (previousStepDidNotUsedCachedView) {
      // If we are not the 1st step, and the previous step did not use a cached view, then we are safer here to not use one either. Don't search for one. Just say no to caching here.
      ATH_MSG_DEBUG("Previous probe step used a probe EventView for this decision object. Do not attempt to use a cached EventView in this step.");
      return false; 
    }
  }

  bool usedROIMatchingFlag{false}; // Sanity check
  bool result = matchInCollection(cachedViews, outputDecision, cachedIndex, usedROIMatchingFlag, matchingCache, context);
  if (usedROIMatchingFlag and m_mergeUsingFeature) {
    ATH_MSG_ERROR("Called matchInCollection in an EVCA configured with mergeUsingFeature=True, however ROI matching was used instead?! Should not be possible.");
  }

  if (result) {
    // We have another check we have to make before we are confident that we can re-use this View
    // The view will have originally be launched linked to N proxies (N>=0), which are the previous Steps which the View
    // needs to link to (in which one might need to later search for physics collections needed in this or later Steps).
    // 
    // But the list of proxies in the probe pass might be different than what it was in the tag pass.
    // We cannot change the list of proxies in the existing tag EventView (it is now immutable)
    // 
    // So if we are missing required proxies then we cannot re-use this EventView and have to reject the cached EV.
    const SG::View* view = *(cachedViews->at(cachedIndex)->objectLink<ViewContainer>(viewString()));

    // What prior views would we have linked if we were spawning a new View here in probe?
    std::vector<LinkInfo<ViewContainer>> viewsToLinkVector = viewsToLink(outputDecision);
    for (const LinkInfo<ViewContainer>& toLinkLI : viewsToLinkVector) {
      const SG::View* toLink = *(toLinkLI.link);
      // Was toLink linked as a proxy back in the tag stage?
      bool foundIt = false;
      for (const SG::View* prevLinked : view->getParentLinks()) {
        if (prevLinked == toLink) {
          foundIt = true;
          break;
        }
      }
      if (!foundIt) {
        ATH_MSG_DEBUG("The cached view from the tag step is not linked to the required views from earlier steps which we need in the probe processing, we cannot re-use it.");
        result = false;
        break;
      }
    }
  }

  return result;
}

StatusCode EventViewCreatorAlgorithm::populateMatchingCacheWithCachedViews(const DecisionContainer* cachedViews, MatchingCache& matchingCache, const EventContext& context) const {
  const std::string linkNameToMatch = m_mergeUsingFeature ? featureString() : m_roisLink.value();
  for (const Decision* cachedView : *cachedViews) {
    const uint64_t matchingHash = getMatchingHashForDecision(cachedView, linkNameToMatch, context);
    if (matchingHash == std::numeric_limits<std::size_t>::max()) {
      return StatusCode::FAILURE;
    }
    matchingCache.setMatchingHash(cachedView, matchingHash);
    // There is no output-to-input redirection required when we're matching against the old TAG collection in the PROBE EVCA, so we can set key=value in this redirection map
    matchingCache.linkOutputToInput(cachedView, cachedView);
  }
  return StatusCode::SUCCESS;
}


StatusCode EventViewCreatorAlgorithm::linkViewToParent( const TrigCompositeUtils::Decision* outputDecision, SG::View* newView ) const {
  if (!m_requireParentView) {
    ATH_MSG_DEBUG("Parent view linking not required");
    return StatusCode::SUCCESS;
  }
  // We must call this BEFORE having added the new link, check
  if (outputDecision->hasObjectLink(viewString())) {
    ATH_MSG_ERROR("Called linkViewToParent on a Decision object which already has been given a '" 
      << viewString() << "' link. Call this fn BEFORE linking the new View.");
    return StatusCode::FAILURE;
  }
  std::vector<LinkInfo<ViewContainer>> viewsToLinkVector = viewsToLink(outputDecision);
  if (viewsToLinkVector.size() == 0) {
    ATH_MSG_ERROR("Could not find the parent View, but 'RequireParentView' is true.");
    return StatusCode::FAILURE;
  }
  // Note: Some Physics Objects will have diverging reco paths, but later re-combine.
  // Examples include an ROI processed as both Electron and Photon re-combining for PrecisionCalo. 
  // Or, a tau ROI processed with different algorithms for different chains in an earlier Step.
  // This will only cause a problem if downstream a collection is requested which was produced in more that one
  // of the linked parent Views (or their parents...) as it is then ambiguous which collection should be read. 
  ATH_MSG_DEBUG( "Will link " << viewsToLinkVector.size() << " parent view(s)" );
  for (const LinkInfo<ViewContainer>& toLinkLI : viewsToLinkVector) {
    ATH_CHECK(toLinkLI.isValid());
    newView->linkParent( *(toLinkLI.link) );
    ATH_MSG_DEBUG( "Parent view linked (" << toLinkLI.link.dataID() << ", index:" << toLinkLI.link.index() << ")" );
  }

  return StatusCode::SUCCESS;
}

StatusCode EventViewCreatorAlgorithm::placeRoIInView( const ElementLink<TrigRoiDescriptorCollection>& roiEL, SG::View* view, const EventContext& context ) const {
  // fill the RoI output collection
  auto oneRoIColl = std::make_unique< ConstDataVector<TrigRoiDescriptorCollection> >();
  oneRoIColl->clear( SG::VIEW_ELEMENTS ); //Don't delete the RoIs
  oneRoIColl->push_back( *roiEL );

  view->setROI(roiEL);

  //store the RoI in the view
  auto handle = SG::makeHandle( m_inViewRoIs, context );
  ATH_CHECK( handle.setProxyDict( view ) );
  ATH_CHECK( handle.record( std::move( oneRoIColl ) ) );
  return StatusCode::SUCCESS;
}


StatusCode EventViewCreatorAlgorithm::placeMuonInView( const xAOD::Muon* theObject, SG::View* view, const EventContext& context ) const {
  // fill the Muon output collection
  ATH_MSG_DEBUG( "Adding Muon To View : " << m_inViewMuons.key()<<" and "<<m_inViewMuonCandidates.key() );
  auto oneObjectCollection = std::make_unique< xAOD::MuonContainer >();
  auto oneObjectAuxCollection = std::make_unique< xAOD::MuonAuxContainer >();
  oneObjectCollection->setStore( oneObjectAuxCollection.get() );

  xAOD::Muon* copiedMuon = new xAOD::Muon();
  oneObjectCollection->push_back( copiedMuon );
  *copiedMuon = *theObject;

  auto muonCandidate = std::make_unique< ConstDataVector< MuonCandidateCollection > >();
  auto msLink = theObject->muonSpectrometerTrackParticleLink();
  auto extTrackLink = theObject->extrapolatedMuonSpectrometerTrackParticleLink();
  if(msLink.isValid() && extTrackLink.isValid()) muonCandidate->push_back( new MuonCombined::MuonCandidate(msLink, (*extTrackLink)->trackLink(), (*extTrackLink)->index()) );

  //store both in the view
  auto handleMuon = SG::makeHandle( m_inViewMuons,context );
  ATH_CHECK( handleMuon.setProxyDict( view ) );
  ATH_CHECK( handleMuon.record( std::move( oneObjectCollection ), std::move( oneObjectAuxCollection )) );

  auto handleCandidate = SG::makeHandle( m_inViewMuonCandidates,context );
  ATH_CHECK( handleCandidate.setProxyDict( view ) );
  ATH_CHECK( handleCandidate.record( std::move( muonCandidate ) ) );

  return StatusCode::SUCCESS;
}

// TODO - Template this?
StatusCode EventViewCreatorAlgorithm::placeJetInView( const xAOD::Jet* theObject, SG::View* view, const EventContext& context ) const {

  // fill the Jet output collection
  ATH_MSG_DEBUG( "Adding Jet To View : " << m_inViewJets.key() );

  auto oneObjectCollection = std::make_unique< xAOD::JetContainer >();
  auto oneObjectAuxCollection = std::make_unique< xAOD::JetAuxContainer >();
  oneObjectCollection->setStore( oneObjectAuxCollection.get() );

  xAOD::Jet* copiedJet = new xAOD::Jet();
  oneObjectCollection->push_back( copiedJet );
  *copiedJet = *theObject;

  auto handle = SG::makeHandle( m_inViewJets,context );  
  ATH_CHECK( handle.setProxyDict( view ) ); 
  ATH_CHECK( handle.record( std::move(oneObjectCollection),std::move(oneObjectAuxCollection) ) );

  return StatusCode::SUCCESS;
}
