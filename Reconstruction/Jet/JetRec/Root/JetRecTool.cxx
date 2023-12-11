/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// JetRecTool.cxx

#include "JetRec/JetRecTool.h"
#include <iomanip>
#include "xAODJet/JetAuxContainer.h"
#ifndef GENERATIONBASE
#include "xAODJet/JetTrigAuxContainer.h"
#endif //GENERATIONBASE
#include <fstream>

#include "xAODBase/IParticleHelpers.h"
#include "JetEDM/FastJetLink.h"
#include "xAODJet/Jet_PseudoJet.icc"

#include "JetRec/JetPseudojetRetriever.h"
#include "JetRec/PseudoJetContainer.h"
#include <algorithm>

#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/WriteHandle.h"

#if !defined (GENERATIONBASE) && !defined (XAOD_ANALYSIS)
  #include "AthenaMonitoringKernel/Monitored.h"
#endif

using ModifierArray = ToolHandleArray<IJetModifier>;
using ConsumerArray = ToolHandleArray<IJetConsumer>;

using std::string;
using std::setw;
using std::fixed;
using std::setprecision;
using xAOD::JetContainer;
using xAOD::Jet;

//**********************************************************************

JetRecTool::JetRecTool(const std::string& myname)
: AsgTool(myname),
  m_intool("",this),
#ifdef XAOD_ANALYSIS
  m_hpjr("",this),
#else
  m_hpjr("JetPseudojetRetriever/jpjr",this),
#endif
  m_finder("",this),
  m_groomer("",this),
  m_trigger(false),
  m_initCount(0),
  m_find(false), m_groom(false), m_copy(false),
  m_inputtype(xAOD::JetInput::Uncategorized),
  m_ppjr(nullptr) {
  declareProperty("InputTool", m_intool);
  declareProperty("JetPseudojetRetriever", m_hpjr);
  declareProperty("JetFinder", m_finder);
  declareProperty("JetGroomer", m_groomer);
  declareProperty("Trigger", m_trigger);
}

//**********************************************************************

StatusCode JetRecTool::initialize() {
  ATH_MSG_INFO("Initializing JetRecTool " << name() << ".");
  ++m_initCount;
  // Fetch the reconstruction mode.
  bool needinp = false;
  bool needout = false;
  string mode = "pseudojets";
  if ( !m_outcoll.key().empty() ) {
    m_outcolls.push_back(m_outcoll.key());
    if ( ! m_finder.empty() ) {
      mode = "find";
      m_find = true;
      needout = true;
      if ( m_psjsin.empty() ) {
        ATH_MSG_ERROR("Jet finding requested with no inputs.");
        return StatusCode::FAILURE;
      } else {
	ATH_CHECK( m_psjsin.initialize() );
      }
    } else if ( ! m_groomer.empty() ) {
      mode = "groom";
      m_groom = true;
      needinp = true;
      needout = true;
      ATH_CHECK(m_groomer.retrieve());
    } else {
      mode = "copy";
      m_copy = true;
      needinp = true;
      needout = true;
    }
  }
  else {
    m_finder.disable();
    m_groomer.disable();
  }

  ATH_CHECK( m_psjsin.initialize() );
  // Retrieve or create pseudojet retrieval tool.
  if ( !m_hpjr.empty() ) {
    if ( m_hpjr.retrieve().isSuccess() ) {
      m_ppjr = &*m_hpjr;
    } else {
      ATH_MSG_ERROR("Unable to retrieve requested pseudojet retriever: " << m_hpjr.name());
    }
  } else {
#ifdef XAOD_STANDALONE
      m_ppjr = new JetPseudojetRetriever(name()+"_retriever");
#else
      m_ppjr = nullptr;
#endif
  }
  ATH_MSG_INFO("Jet reconstruction mode: " << mode);
  // Check/set the input jet collection name.
  if ( needinp ) {
    if ( m_incoll.key().empty() ) {
      if ( ! m_intool.retrieve() ) {
        ATH_MSG_ERROR("Input collection must be specified.");
        return StatusCode::FAILURE;
      } else {
        const AsgTool* pasgtool = dynamic_cast<const asg::AsgTool*>(&*m_intool);
        if ( pasgtool != nullptr ) {
          const string* pval = pasgtool->getProperty<std::string>("OutputContainer");
          if ( pval != nullptr ) {
            m_incoll = *pval;
          }
        }
        if ( m_incoll.key().empty() ) {
          ATH_MSG_ERROR("Input tool does not have output collection name.");
          return StatusCode::FAILURE;
        }
      }
    } else {
      // Input DataHandles
      ATH_CHECK( m_incoll.initialize() );
    }
    m_incolls.push_back(m_incoll.key());
  }
  // Check/set the output jet collection name.
  if ( needout ) {
    if ( m_outcoll.key().empty() ) {
      ATH_MSG_ERROR("Output collection must be specified.");
      return StatusCode::FAILURE;
    } else {
      // Output DataHandle
      ATH_CHECK( m_outcoll.initialize() );
    }
  }
  // Other checks.
  if ( m_find ) {
    if ( m_psjsin.empty() ) {
      ATH_MSG_ERROR("Jet finding requested with no inputs.");
      return StatusCode::FAILURE;
    }
  } else if ( m_groom ) {
    m_groomer->setPseudojetRetriever(m_ppjr);
  } else if ( m_copy ) {
  } else {
    if ( m_psjsin.empty() ) {
      ATH_MSG_ERROR("No action requested.");
      return StatusCode::FAILURE;
    }
  }
  // Parse the pseudojet inputs
  StatusCode rstat = StatusCode::SUCCESS;
  string prefix = "--- ";
  ATH_MSG_INFO(prefix << "JetRecTool " << name() << " has " << m_psjsin.size()
               << " pseudojet inputs.");
  for ( size_t ilab(0); ilab<m_psjsin.size(); ++ilab ) {
    const std::string& pjcontname = m_psjsin[ilab].key();
    if(pjcontname.size()<9) {
      ATH_MSG_ERROR("Invalid pseudojet container name " << pjcontname);
      ATH_MSG_ERROR("This must be of the form \"PseudoJet\"+label");
      return StatusCode::FAILURE;
    }
    std::string label = pjcontname.substr(9);
    ATH_MSG_INFO(prefix << " " << label << " --> " << pjcontname);
    // Extract the input type from the first getter.
    if ( ilab == 0 ) {
      ATH_MSG_INFO(prefix << "Extracting input type from primary label.");
      ATH_MSG_INFO(prefix << "Input label: " << label);
      m_inputtype = xAOD::JetInput::inputType(label);
    } else {
      m_ghostlabs.push_back(label);
    }
  }
  ATH_MSG_INFO(prefix << "Input type: " << m_inputtype);
  // Fetch the jet modifiers.
  ATH_MSG_INFO(prefix << "JetRecTool " << name() << " has " << m_modifiers.size()
               << " jet modifiers.");
  ATH_CHECK(m_modifiers.retrieve());

  // Fetch the jet consumers.
  ATH_MSG_INFO(prefix << "JetRecTool " << name() << " has " << m_consumers.size()
               << " jet consumers.");
  ATH_CHECK(m_consumers.retrieve());

  ATH_MSG_INFO(prefix << "Input collection names:");
  for (const auto& name : m_incolls) ATH_MSG_INFO(prefix << "  " << name);
  ATH_MSG_INFO(prefix << "Output collection names:");
  for (const auto& name : m_outcolls) ATH_MSG_INFO(prefix << "  " << name);

#if !defined (GENERATIONBASE) && !defined (XAOD_ANALYSIS)
  if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());
#endif

  return rstat;
}

//**********************************************************************

const JetContainer* JetRecTool::build() const {
  if ( m_initCount == 0 ) {
    ATH_MSG_WARNING("Build requested before initialization.");
    return nullptr;
  }
  ATH_MSG_DEBUG("Building jets with " << name() << ".");

  std::unique_ptr<xAOD::JetContainer> pjets = fillOutputContainer();

  SG::WriteHandle<xAOD::JetContainer> jetsHandle(m_outcoll);

  // Record the jet collection.
  if(m_trigger){
    #ifndef GENERATIONBASE
    std::unique_ptr<xAOD::JetTrigAuxContainer> pjetsaux(dynamic_cast<xAOD::JetTrigAuxContainer*>( pjets->getStore() ));
    ATH_MSG_DEBUG("Check Aux store: " << pjets.get() << " ... " << &pjets->auxbase() << " ... " << pjetsaux.get() );
    if ( pjetsaux.get() == nullptr ) {
      ATH_MSG_ERROR("Unable to retrieve Aux container");
      return nullptr;
    }
    ATH_MSG_VERBOSE("Recording new Jet and Aux container.");
    if(jetsHandle.record(std::move(pjets), std::move(pjetsaux)).isFailure()){
      // TODO - put this back how it was
      ATH_MSG_ERROR("Unable to write new Jet collection and aux store to event store: " << m_outcoll.key());
      return nullptr;
    }
    #endif
  }
  else{
    std::unique_ptr<xAOD::JetAuxContainer> pjetsaux(dynamic_cast<xAOD::JetAuxContainer*>( pjets->getStore() ));
    ATH_MSG_DEBUG("Check Aux store: " << pjets.get() << " ... " << &pjets->auxbase() << " ... " << pjetsaux.get() );
    if ( pjetsaux.get() == nullptr ) {
      ATH_MSG_ERROR("Unable to retrieve Aux container");
      return nullptr;
    }
    ATH_MSG_VERBOSE("Recording new Jet and Aux container.");
    if(jetsHandle.record(std::move(pjets), std::move(pjetsaux)).isFailure()){
      ATH_MSG_ERROR("Unable to write new Jet collection and aux store to event store: " << m_outcoll.key());
      return nullptr;
    }
  }
  ATH_MSG_DEBUG("Created new Jet collection in event store: " << m_outcoll.key());

  // Modify jets.
  unsigned int nmod = m_modifiers.size();
  if ( nmod ) {
    if ( !jetsHandle.isValid() ) {
      ATH_MSG_WARNING("There is no jet collection to modify.");
    } else {
      ATH_MSG_DEBUG("Executing " << nmod << " jet modifiers.");
      for ( ModifierArray::const_iterator imod=m_modifiers.begin();
           imod!=m_modifiers.end(); ++imod ) {
        ATH_MSG_DEBUG("  Executing modifier " << imod->name());
        ATH_MSG_VERBOSE("    @ " << *imod);
        if((*imod)->modify(*jetsHandle).isFailure())
          ATH_MSG_DEBUG("    Modifier returned FAILURE!");
      }
    }
  }

  // Consume jets.
  unsigned int ncon = m_consumers.size();
  if ( ncon ) {
    if ( !jetsHandle.isValid() ) {
      ATH_MSG_WARNING("There is no jet collection to consume");
    } else {
      ATH_MSG_DEBUG("Executing " << ncon << " jet consumers.");
      for ( ConsumerArray::const_iterator icon=m_consumers.begin();
           icon!=m_consumers.end(); ++icon ) {
        ATH_MSG_DEBUG("  Executing consumer " << icon->name());
        ATH_MSG_VERBOSE("    @ " << *icon);
        (*icon)->process(*jetsHandle) ;
      }
    }
  }


#if !defined (GENERATIONBASE) && !defined (XAOD_ANALYSIS)
  // monitor jet multiplicity and basic jet kinematics
  auto njets = Monitored::Scalar<int>("JET_n");
  auto pt    = Monitored::Collection("JET_pt",  *jetsHandle, [c=m_mevtogev]( const xAOD::Jet* jet ) { return jet->pt()*c; });
  auto et    = Monitored::Collection("JET_et",  *jetsHandle, [c=m_mevtogev]( const xAOD::Jet* jet ) { return jet->p4().Et()*c; });
  auto mass  = Monitored::Collection("JET_m",   *jetsHandle, [c=m_mevtogev]( const xAOD::Jet* jet ) { return jet->m()*c; });
  auto eta   = Monitored::Collection("JET_eta", *jetsHandle, []( const xAOD::Jet* jet ) { return jet->eta(); });
  auto phi   = Monitored::Collection("JET_phi", *jetsHandle, []( const xAOD::Jet* jet ) { return jet->phi(); });
  auto mon   = Monitored::Group(m_monTool,njets,pt,et,mass,eta,phi);
  njets      = jetsHandle->size();
#endif

  return jetsHandle.isValid() ? &(*jetsHandle) : nullptr;
}

//**********************************************************************

int JetRecTool::execute() const {
  if ( m_initCount == 0 ) {
    ATH_MSG_WARNING("Execute requested before initialization.");
    return 1;
  }

  if ( build() == nullptr ) {
    ATH_MSG_ERROR("Unable to retrieve container");
    return 1;
  }
  return 0;
}

//**********************************************************************

void JetRecTool::print() const {
  ATH_MSG_INFO("Properties for JetRecTool " << name());

  ATH_MSG_INFO("  OutputContainer: " << m_outcoll.key());
  if ( m_incoll.key().empty() ) ATH_MSG_INFO("  InputContainer is not defined");
  else ATH_MSG_INFO("  InputContainer: " << m_incoll.key());
  if ( m_intool.empty() ) {
    ATH_MSG_INFO("  InputTool is not defined");
  } else {
    ATH_MSG_INFO("  InputTool: " << m_intool->name());
  }
  if ( !m_psjsin.empty() ) {
    ATH_MSG_INFO("  InputPseudoJet container count is " << m_psjsin.size());
    for ( const auto& pjcontkey : m_psjsin ) {
      ATH_MSG_INFO("    " << pjcontkey.key());
    }
  }
  if ( m_finder.empty() ) {
    ATH_MSG_INFO("  Jet finder is not defined");
  } else {
    ATH_MSG_INFO("  Jet finder: " << m_finder->name());
    m_finder->print();
    ATH_MSG_INFO("    Input type: " << m_inputtype);
    ATH_MSG_INFO("    There are " << m_ghostlabs.size() << " ghost labels:");
    for ( const string& lab : m_ghostlabs ) ATH_MSG_INFO("      " << lab);
  }
  if ( m_groomer.empty() ) {
    ATH_MSG_INFO("  Jet groomer is not defined");
  } else {
    ATH_MSG_INFO("  Jet groomer: " << m_groomer->name());
    m_groomer->print();
  }
  if ( !m_modifiers.empty() ) {
    ATH_MSG_INFO("  Modifier count is " << m_modifiers.size());
    for ( ModifierArray::const_iterator imod=m_modifiers.begin();
           imod!=m_modifiers.end(); ++imod ) {
      ATH_MSG_INFO("    Modifier " << imod->name());
      if ( msgLvl(MSG::DEBUG) ) {
        ToolHandle<IJetModifier> hmod = *imod;
        hmod->print();
      }
    }
  }
}

//**********************************************************************

int JetRecTool::inputContainerNames(std::vector<std::string>& connames) {
  if ( m_initCount == 0 ) {
    ATH_MSG_WARNING("Input container list requested before initialization.");
    return 1;
  }
  connames.insert(connames.end(), m_incolls.begin(), m_incolls.end());
  return 0;
}

//**********************************************************************

int JetRecTool::outputContainerNames(std::vector<std::string>& connames) {
  if ( m_initCount == 0 ) {
    ATH_MSG_WARNING("Output container list requested before initialization.");
    return 1;
  }
  connames.insert(connames.end(), m_outcolls.begin(), m_outcolls.end());
  return 0;
}

//**********************************************************************

 void JetRecTool::setInputJetContainer(const xAOD::JetContainer* cont) {
   m_trigInputJetsForGrooming = cont;
 }

//**********************************************************************

std::unique_ptr<PseudoJetContainer> JetRecTool::collectPseudoJets() const{
  // PseudoJetContainer used for jet finding

  auto allPseudoJets = std::make_unique<PseudoJetContainer>();

  ATH_MSG_DEBUG("Fetching pseudojet inputs.");

  for (const auto& pjcontkey : m_psjsin) {
    SG::ReadHandle<PseudoJetContainer> h_newpsjs( pjcontkey );
    ATH_MSG_DEBUG("Adding PseudoJetContainers for: " << h_newpsjs.key());
    if(! h_newpsjs.isValid()) {
      ATH_MSG_ERROR("Retrieval of PseudoJetContainer "
                    << h_newpsjs.key() << " failed");
      return nullptr;
    }
    allPseudoJets->append(h_newpsjs.get());
  }

  return allPseudoJets;
}

//**********************************************************************

std::unique_ptr<xAOD::JetContainer> JetRecTool::fillOutputContainer() const{

  if (!m_finder.empty()) {return findJets();}
  if (!m_groomer.empty()) {return groomJets();}
  return copyJets();
}

//**********************************************************************

const xAOD::JetContainer* JetRecTool::getOldJets() const{

  const xAOD::JetContainer* pjetsin{nullptr};
  auto handle_in = SG::makeHandle (m_incoll);
  if ( !m_incoll.key().empty() && handle_in.isValid()) {
    pjetsin = handle_in.cptr();
  }
  if ( pjetsin == nullptr && !m_intool.empty() ) {
    ATH_MSG_DEBUG("Executing input tool.");
    if ( m_intool->execute() ) {
      ATH_MSG_WARNING("Input tool execution failed.");
    }
  }

  if ( pjetsin == nullptr ) {
    ATH_MSG_ERROR("Unable to retrieve input jet container: " << m_incoll.key());
  } else {
    ATH_MSG_DEBUG("Input collection " << m_incoll.key()
                  << " jet multiplicity is "<< pjetsin->size());
  }
    return pjetsin;
}

//**********************************************************************

std::unique_ptr<xAOD::JetContainer> JetRecTool::makeOutputContainer() const{
  ATH_MSG_DEBUG("Creating output container.");

  auto pjets = std::make_unique<xAOD::JetContainer>();

  if ( !m_outcoll.key().empty() ) {
    if(m_trigger) {
      ATH_MSG_DEBUG("Attaching online Aux container.");
#ifndef GENERATIONBASE
      pjets->setStore(new xAOD::JetTrigAuxContainer);
#endif
    } else {
      ATH_MSG_DEBUG("Attaching offline Aux container.");
      pjets->setStore(new xAOD::JetAuxContainer);
    }
  }
  return pjets;
}

//**********************************************************************

std::unique_ptr<xAOD::JetContainer> JetRecTool::findJets() const {

  ATH_MSG_DEBUG("Finding jets.");

  // The new jet collection.
  auto jets = makeOutputContainer();

  // PseudoJetContainer used for jet finding
  auto pseudoJets = collectPseudoJets();

  m_finder->find(*pseudoJets, *jets, m_inputtype);

  return jets;
}

//**********************************************************************

std::unique_ptr<xAOD::JetContainer> JetRecTool::groomJets() const{

  // The new jet collection.
  auto jets = makeOutputContainer();

  // Retrieve the old jet collection.
  const auto *jetsIn = getOldJets();

  if(jetsIn == nullptr){
    ATH_MSG_WARNING("Grooming: but input jets not found ");
    return jets;
  }

  ATH_MSG_DEBUG("Grooming " << jetsIn->size() << " jets.");

  // PseudoJetContainer used for jet finding
  auto pseudoJets = collectPseudoJets();

  for (const auto *const ijet : *jetsIn){ m_groomer->groom(*ijet,
                                                    *pseudoJets,
                                                    *jets);}

  return jets;
}

//**********************************************************************

std::unique_ptr<xAOD::JetContainer> JetRecTool::copyJets() const{

  // The new jet collection.
  auto jets = makeOutputContainer();

  // Retrieve the old jet collection.
  const auto *jetsIn = getOldJets();

  if(jetsIn == nullptr){
    ATH_MSG_WARNING("Copying: but input jets not found ");
    return jets;
  }


  ATH_MSG_DEBUG("Copying " << jetsIn->size() << " jets.");

  for (const Jet* poldjet : *jetsIn) {
    Jet* pnewjet = new Jet;
    jets->push_back(pnewjet);
    *pnewjet = *poldjet;
  }

  return jets;
}
