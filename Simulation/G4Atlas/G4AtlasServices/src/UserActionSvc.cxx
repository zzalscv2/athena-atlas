/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#include <mutex>

// Framework includes

// Geant4 includes
#include "G4RunManager.hh"

// Local includes
#include "UserActionSvc.h"

namespace G4UA
{

  //---------------------------------------------------------------------------
  // Constructor
  // I was using private tool handle arrays, but the OutputLevels weren't
  // getting set correctly (in mig1), so I will just make them all public.
  //---------------------------------------------------------------------------
  UserActionSvc::UserActionSvc(const std::string& name,
                               ISvcLocator* pSvcLocator)
    : base_class(name, pSvcLocator)
    , m_userActionTools(this)
  {
    declareProperty("UserActionTools", m_userActionTools);
  }

  //---------------------------------------------------------------------------
  // Initialize the service
  //---------------------------------------------------------------------------
  StatusCode UserActionSvc::initialize()
  {
    ATH_MSG_INFO("Initializing " << m_userActionTools.size() << " user action tools.");

    for(const auto& action : m_userActionTools) {
      ATH_MSG_INFO( "      -> " << action.name() );
    }

    ATH_CHECK( m_userActionTools.retrieve() );

    return StatusCode::SUCCESS;
  }

  //---------------------------------------------------------------------------
  // Initialize the user actions for the current thread.
  // In this code, "action" refers to the G4 action classes, and "plugin"
  // refers to the custom action objects that get assigned to the G4 action.
  // TODO: this prototype could be cleaner
  //---------------------------------------------------------------------------
  StatusCode UserActionSvc::initializeActions()
  {
    // This method is called concurrently in AthenaMT during initialization.
    // We conservatively just lock the whole thing to protect MsgStream and
    // downstream code.
    static std::mutex userActionMutex;
    std::lock_guard<std::mutex> userActionLock(userActionMutex);

    ATH_MSG_DEBUG("initializeActions");

    // sanity check: this is to make sure there are no other instances of this
    // svc that have been already initialized, or that nobody else in some other
    // code has already been registering actions to the run manager, which would
    // be a major error in the configuration checking the run action is probably
    // enough fo multiple instances of this service, since all roles are usually
    // set at the same time. but other code may as well have registered just one
    // role, so it is safer to do all checks here.

    if( G4RunManager::GetRunManager()->GetUserRunAction() ||
        G4RunManager::GetRunManager()->GetUserEventAction() ||
        G4RunManager::GetRunManager()->GetUserStackingAction() ||
        G4RunManager::GetRunManager()->GetUserTrackingAction() ||
        G4RunManager::GetRunManager()->GetUserSteppingAction() )
    {
      ATH_MSG_FATAL("UserActionSvc has found that actions were already " <<
                    "registered to the G4RunManager. Check your code/configuration");
      return StatusCode::FAILURE;
    }

    // Retrieve the new user actions
    G4AtlasUserActions actions;
    for(auto& tool : m_userActionTools) {
      ATH_CHECK( tool->fillUserAction(actions) );
    }

    // Initialize the ATLAS run action.
    if(m_runActions.get()) {
      ATH_MSG_ERROR("Run action already exists for current thread!");
      return StatusCode::FAILURE;
    }
    auto runAction = std::make_unique<G4AtlasRunAction>();
    // Assign run plugins
    for(auto* action : actions.runActions)
      runAction->addRunAction(action);
    G4RunManager::GetRunManager()->SetUserAction( runAction.get() );
    m_runActions.set( std::move(runAction) );

    // Initialize the ATLAS event action.
    if(m_eventActions.get()) {
      ATH_MSG_ERROR("Event action already exists for current thread!");
      return StatusCode::FAILURE;
    }
    auto eventAction = std::make_unique<G4AtlasEventAction>();
    // Assign event plugins
    for(auto* action : actions.eventActions) {
      eventAction->addEventAction(action);
      // set the event manager
      action->SetEventManager( G4EventManager::GetEventManager() );
    }
    G4RunManager::GetRunManager()->SetUserAction( eventAction.get() );
    m_eventActions.set( std::move(eventAction) );

    // Initialize the ATLAS stacking action.
    if(m_stackingActions.get()) {
      ATH_MSG_ERROR("Stacking action already exists for current thread!");
      return StatusCode::FAILURE;
    }
    auto stackAction = std::make_unique<G4AtlasStackingAction>();
    // Assign stacking plugins
    for(auto* action : actions.stackingActions) {
      stackAction->addAction(action);
      // set the stack manager
      action->SetStackManager( G4EventManager::GetEventManager()->GetStackManager() );
    }
    G4RunManager::GetRunManager()->SetUserAction( stackAction.get() );
    m_stackingActions.set( std::move(stackAction) );

    // Initialize the ATLAS tracking action.
    if(m_trackingActions.get()) {
      ATH_MSG_ERROR("Tracking action already exists for current thread!");
      return StatusCode::FAILURE;
    }
    auto trackAction = std::make_unique<G4AtlasTrackingAction>();
    // Assign tracking plugins
    for(auto* action : actions.trackingActions) {
      trackAction->addTrackAction(action);
      // set the tracking manager
      action->SetTrackingManagerPointer ( G4EventManager::GetEventManager()->GetTrackingManager() );
    }
    G4RunManager::GetRunManager()->SetUserAction( trackAction.get() );
    m_trackingActions.set( std::move(trackAction) );

    // Initialize the ATLAS stepping action.
    if(m_steppingActions.get()) {
      ATH_MSG_ERROR("Stepping action already exists for current thread!");
      return StatusCode::FAILURE;
    }
    auto stepAction = std::make_unique<G4AtlasSteppingAction>();
    // Assign stepping plugins
    for(auto* action : actions.steppingActions) {
      stepAction->addAction(action);
      // set the stepping manager
      action->SetSteppingManagerPointer( G4EventManager::GetEventManager()->GetTrackingManager()->GetSteppingManager() );
    }
    G4RunManager::GetRunManager()->SetUserAction( stepAction.get() );
    m_steppingActions.set( std::move(stepAction) );

    return StatusCode::SUCCESS;
  }

  // For ISF, get UserActions that could have stored secondary particles
  StatusCode UserActionSvc::getSecondaryActions( std::vector< G4UserSteppingAction* >& actions ) {

    // Only stepping actions can return secondaries? Maybe turn this into a templated method
    actions = m_steppingActions.get()->getActions();

    return StatusCode::SUCCESS;
  }
} // namespace G4UA
