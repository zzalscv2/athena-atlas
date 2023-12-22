///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// METMaker.cxx
// Implementation file for class METMaker
// Author: T.J.Khoo<khoo@cern.ch>
///////////////////////////////////////////////////////////////////

// METUtilities includes
#include "METUtilities/METMaker.h"
#include "METUtilities/METHelpers.h"

// MET EDM
#include "xAODMissingET/MissingETContainer.h"
#include "xAODMissingET/MissingETComposition.h"
#include "xAODMissingET/MissingETAuxContainer.h"
#include "xAODMissingET/MissingETAssociationMap.h"
#include "xAODMissingET/MissingETAssociationHelper.h"

// Jet EDM
#include "xAODJet/JetAttributes.h"

// Tracking EDM
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/VertexContainer.h"

// Shallow copy
#include "xAODCore/ShallowCopy.h"

// Muon EDM
#include "xAODMuon/MuonContainer.h"

// Electron EDM
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

// framework includes
#include "AsgDataHandles/ReadHandle.h"
#include <AsgTools/AsgToolConfig.h>
#include "xAODPFlow/PFOAuxContainer.h"
#include <xAODCore/AuxContainerBase.h>
#include <AthContainers/AuxElement.h>

#include <memory>

namespace met {

  using std::vector;

  using xAOD::MissingET;
  using xAOD::MissingETContainer;
  using xAOD::MissingETAssociation;
  using xAOD::MissingETAssociationMap;
  using xAOD::MissingETAuxContainer;
  using xAOD::MissingETComposition;
  //
  using xAOD::IParticle;
  using xAOD::IParticleContainer;
  //
  using xAOD::JetContainer;
  using xAOD::JetConstituentVector;
  //
  using xAOD::TrackParticle;
  // using xAOD::VertexContainer;
  // using xAOD::Vertex;

  using iplink_t = ElementLink<xAOD::IParticleContainer>;
  static const SG::AuxElement::ConstAccessor< iplink_t  > acc_originalObject("originalObjectLink");
  static const SG::AuxElement::ConstAccessor< iplink_t  > acc_nominalObject("nominalObjectLink");
  static const SG::AuxElement::ConstAccessor< std::vector<iplink_t > > acc_ghostMuons("GhostMuon");
  static const SG::AuxElement::ConstAccessor< std::vector<iplink_t > > acc_ghostElecs("GhostElec");

  static const SG::AuxElement::ConstAccessor< std::vector<int> > acc_trkN("NumTrkPt500");
  static const SG::AuxElement::ConstAccessor< std::vector<float> > acc_trksumpt("SumPtTrkPt500");
  static const SG::AuxElement::ConstAccessor< std::vector<float> > acc_sampleE("EnergyPerSampling");

  static const SG::AuxElement::ConstAccessor<float> acc_emf("EMFrac");
  static const SG::AuxElement::ConstAccessor<float> acc_psf("PSFrac");
  static const SG::AuxElement::ConstAccessor<float> acc_width("Width");
  static const SG::AuxElement::ConstAccessor<float> acc_Eloss("EnergyLoss");

  static const SG::AuxElement::Accessor< std::vector<iplink_t> > dec_constitObjLinks("ConstitObjectLinks");
  static const SG::AuxElement::Accessor< std::vector<float> > dec_constitObjWeights("ConstitObjectWeights");


  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////

  // Constructors
  ////////////////
  METMaker::METMaker(const std::string& name) :
    AsgTool(name),
    m_PVkey("PrimaryVertices"),
    m_acc_jetJvtMoment(nullptr),
    m_acc_jetRejectionDec(nullptr),
    m_JvtCutTight(-100.0),
    m_JvtTightPtMax(-100.0),
    m_JvtCutMedium(-100.0),
    m_JvtMediumPtMax(-100.0),
    m_trkseltool(""),
    m_JvtTool("", this)
  {
    //
    // Property declaration
    //

    // declareProperty("VxColl",             m_pvcoll             = "PrimaryVertices"   );

    declareProperty("JetJvtMomentName",   m_jetJvtMomentName   = "Jvt"               );
    declareProperty("JetRejectionDec",    m_jetRejectionDec    = ""                  );
    declareProperty("JetMinEFrac",        m_jetMinEfrac        = 0.0                 );
    declareProperty("JetMinWeightedPt",   m_jetMinWeightedPt   = 20.0e3              );
    //declareProperty("JetConstitScaleMom", m_jetConstitScaleMom = "JetLCScaleMomentum");
    declareProperty("JetConstitScaleMom", m_jetConstitScaleMom = "JetConstitScaleMomentum");
    declareProperty("CorrectJetPhi",      m_jetCorrectPhi      = false               );
    declareProperty("DoPFlow",            m_doPFlow            = false               );
    declareProperty("DoSoftTruth",        m_doSoftTruth        = false               );
    declareProperty("DoJetTruth",         m_doConstJet         = false               );

    declareProperty("JetSelection",       m_jetSelection       = "Tight"             );
    declareProperty("JetEtaMax",          m_JetEtaMax          = 4.5                 );
    declareProperty("JetEtaForw",         m_JetEtaForw         = 2.5                 );
    declareProperty("UseR21JvtFallback",  m_useR21JvtFallback  = false               );
    declareProperty("CustomCentralJetPt", m_customCenJetPtCut  = 20e3                );
    declareProperty("CustomForwardJetPt", m_customFwdJetPtCut  = 20e3                );
    declareProperty("CustomJetJvtCut",    m_customJvtCut       = 0.59                );
    declareProperty("CustomJetJvtPtMax",  m_customJvtPtMax     = 60e3                );

    declareProperty("DoMuonEloss",        m_muEloss            = false               );
    declareProperty("ORCaloTaggedMuons",  m_orCaloTaggedMuon   = true                );
    declareProperty("GreedyPhotons",      m_greedyPhotons      = false               );
    declareProperty("VeryGreedyPhotons",  m_veryGreedyPhotons  = false               );

    declareProperty("UseGhostMuons",      m_useGhostMuons      = false               );
    declareProperty("DoRemoveMuonJets",   m_doRemoveMuonJets   = true                );
    declareProperty("DoSetMuonJetEMScale", m_doSetMuonJetEMScale = true              );

    declareProperty("DoRemoveElecTrks",   m_doRemoveElecTrks   = true                );
    declareProperty("DoRemoveElecTrksEM", m_doRemoveElecTrksEM = false               );

    declareProperty("DoSimpleOR",         m_doSimpleOR         = false                );

    // muon overlap variables (expert use only)
    declareProperty("JetTrkNMuOlap",      m_jetTrkNMuOlap = 5                        );
    declareProperty("JetWidthMuOlap",     m_jetWidthMuOlap = 0.1                     );
    declareProperty("JetPsEMuOlap",       m_jetPsEMuOlap = 2.5e3                     );
    declareProperty("JetEmfMuOlap",       m_jetEmfMuOlap = 0.9                       );
    declareProperty("JetTrkPtMuPt",       m_jetTrkPtMuPt = 0.8                       );
    declareProperty("muIDPTJetPtRatioMuOlap", m_muIDPTJetPtRatioMuOlap = 2.0         );

    declareProperty("MissingObjWarnThreshold", m_missObjWarningPtThreshold = 7.0e3   );

    declareProperty("TrackSelectorTool",  m_trkseltool                               );
    declareProperty("JvtSelTool",         m_JvtTool                                  );
  }

  // Destructor
  ///////////////
  METMaker::~METMaker()
  = default;

  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode METMaker::initialize()
  {
    ATH_MSG_INFO ("Initializing " << name() << "...");

    //default jet selection i.e. pre-recommendation
    ATH_MSG_INFO("Use jet selection criterion: " << m_jetSelection << " PFlow: " << m_doPFlow);
    // note: default in R22 is to let the JvtTool apply the NNJvt cuts based on the chosen WP;
    // only if m_useR21JvtFallback is used the Jvt requirements based on m_JvtCut are applied manually
    if (m_jetSelection == "Loose")       { m_CenJetPtCut = 20e3; m_FwdJetPtCut = 20e3; m_JvtWP = "FixedEffPt"; m_JvtCut = m_doPFlow ? 0.5 : 0.59; m_JvtPtMax = 60e3; }
    else if (m_jetSelection == "Tight")  { m_CenJetPtCut = 20e3; m_FwdJetPtCut = 30e3; m_JvtWP = "FixedEffPt"; m_JvtCut = m_doPFlow ? 0.5 : 0.59; m_JvtPtMax = 60e3; }
    else if (m_jetSelection == "Tighter"){ m_CenJetPtCut = 20e3; m_FwdJetPtCut = 35e3; m_JvtWP = "FixedEffPt"; m_JvtCut = m_doPFlow ? 0.5 : 0.59; m_JvtPtMax = 60e3; }
    else if (m_jetSelection == "Tenacious")  {
      m_CenJetPtCut  = 20e3; m_FwdJetPtCut = 35e3;
      m_JvtWP = "TightFwd";
      m_JvtCutTight  = 0.91; m_JvtTightPtMax  = 40.0e3;
      m_JvtCutMedium = 0.59; m_JvtMediumPtMax = 60.0e3;
      m_JvtCut       = 0.11; m_JvtPtMax = 120e3;
    }
    else if (m_jetSelection == "Tier0")  { m_CenJetPtCut = 0;    m_FwdJetPtCut = 0;    m_JvtCut = -1;   m_JvtPtMax = 0; m_useR21JvtFallback = true;}
    else if (m_jetSelection == "Expert")  {
      ATH_MSG_INFO("Custom jet selection configured. *** FOR EXPERT USE ONLY ***");
      m_CenJetPtCut = m_customCenJetPtCut;
      m_FwdJetPtCut = m_customFwdJetPtCut;
      m_JvtCut = m_customJvtCut;
      m_JvtPtMax = m_customJvtPtMax;
      m_JvtWP = m_customJvtWP;
    }
    else if (m_jetSelection == "HRecoil")  {
      ATH_MSG_INFO("Jet selection for hadronic recoil calculation is configured.");
      m_CenJetPtCut = 9999e3;
      m_FwdJetPtCut = 9999e3;
      m_JetEtaMax   = 5;
      //m_JvtCut   = 0.;    // currently skip
      //m_JvtPtMax = 0.;  // currently skip
      // this WP also requires that we place the Jvt cuts manually
      m_useR21JvtFallback = true;
    }
    else {
      if (m_jetSelection == "Default") ATH_MSG_WARNING( "WARNING:  Default is now deprecated" );
      ATH_MSG_ERROR( "Error: No available jet selection found! Please update JetSelection in METMaker. Choose one: Loose, Tight (recommended), Tighter, Tenacious" );
      return StatusCode::FAILURE;
    }

    // if using the old R21 Jvt cuts also enforce the same forward threshold
    if (m_useR21JvtFallback) m_JetEtaForw = 2.4;

    if (!m_trkseltool.empty()) ATH_CHECK( m_trkseltool.retrieve() );

    // do not setup Jvt tool if we use the fallback option, i.e. apply Jvt requirements manually
    if (!m_useR21JvtFallback) {
      if (m_JvtTool.empty()) {
        asg::AsgToolConfig config_jvt ("CP::NNJvtSelectionTool/JvtSelTool");
        ATH_CHECK(config_jvt.setProperty("WorkingPoint", m_JvtWP));
        ATH_CHECK(config_jvt.setProperty("JvtMomentName", "NNJvt"));
        ATH_CHECK(config_jvt.setProperty("MaxPtForJvt", m_JvtPtMax));
        ATH_CHECK(config_jvt.makePrivateTool(m_JvtTool));
      }
      ATH_CHECK(m_JvtTool.retrieve());
    }

    // ReadHandleKey(s)
    ATH_CHECK( m_PVkey.initialize() );

    // configurable accessors
    m_acc_jetJvtMoment = std::make_unique<SG::AuxElement::ConstAccessor<float>>(m_jetJvtMomentName);
    if (!m_jetRejectionDec.empty()) {
      m_acc_jetRejectionDec = std::make_unique<SG::AuxElement::ConstAccessor<char>>(m_jetRejectionDec);
      ATH_MSG_INFO("Applying additional jet rejection criterium in MET calculation: " << m_jetRejectionDec);
    }

    ATH_MSG_INFO("Suppressing warnings of objects missing in METAssociationMap for objects with pT < " << m_missObjWarningPtThreshold/1e3 << " GeV.");

    // overlap removal simplification?
    if (m_doSimpleOR) {
      ATH_MSG_INFO("Requesting simplified overlap removal procedure in MET calculation");
    }
    
    return StatusCode::SUCCESS;
  }


  // **** Rebuild generic MET term ****

  StatusCode METMaker::rebuildMET(const std::string& metKey,
                                  xAOD::Type::ObjectType metType,
                                  xAOD::MissingETContainer* metCont,
                                  const xAOD::IParticleContainer* collection,
                                  xAOD::MissingETAssociationHelper& helper,
                                  MissingETBase::UsageHandler::Policy objScale)
  {
    MissingETBase::Types::bitmask_t metSource;
    switch(metType) {
    case xAOD::Type::Electron:
      metSource = MissingETBase::Source::electron();
      break;
    case xAOD::Type::Photon:
      metSource = MissingETBase::Source::photon();
      break;
    case xAOD::Type::Tau:
      metSource = MissingETBase::Source::tau();
      break;
    case xAOD::Type::Muon:
      metSource = MissingETBase::Source::muon();
      break;
    case xAOD::Type::Jet:
      ATH_MSG_WARNING("Incorrect use of rebuildMET -- use rebuildJetMET for RefJet term");
      return StatusCode::FAILURE;
    default:
      ATH_MSG_WARNING("Invalid object type provided: " << metType);
      return StatusCode::FAILURE;
    }

    MissingET* met = nullptr;
    if( fillMET(met,metCont, metKey , metSource) != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("failed to fill MET term \"" << metKey << "\"");
      return StatusCode::FAILURE;
    }

    // If muon eloss corrections are required, create a new term to hold these if it doesn't already exist
    if(metType==xAOD::Type::Muon && (m_muEloss || m_doSetMuonJetEMScale) && !(*metCont)["MuonEloss"]) {
      MissingET* met_muEloss = nullptr;
      if( fillMET(met_muEloss,metCont,"MuonEloss",
                  MissingETBase::Source::Type::Muon | MissingETBase::Source::Category::Calo) != StatusCode::SUCCESS) {
        ATH_MSG_ERROR("failed to create Muon Eloss MET term");
        return StatusCode::FAILURE;
      }
    }

    return rebuildMET(met,collection,helper,objScale);
  }

  StatusCode METMaker::rebuildMET(xAOD::MissingET* met,
                                  const xAOD::IParticleContainer* collection,
                                  xAOD::MissingETAssociationHelper& helper,
                                  MissingETBase::UsageHandler::Policy objScale)
  {
    MissingETBase::UsageHandler::Policy p = MissingETBase::UsageHandler::OnlyCluster;
    bool removeOverlap = true;
    if(!collection->empty()) {
      const IParticle* obj = collection->front();
      if(obj->type()==xAOD::Type::Muon) {
        p = MissingETBase::UsageHandler::OnlyTrack;
        removeOverlap = false;
      }
    }
    if (m_doSoftTruth) p = MissingETBase::UsageHandler::TruthParticle;
    if (m_doPFlow) p = MissingETBase::UsageHandler::ParticleFlow;
    return rebuildMET(met,collection,helper,p,removeOverlap,objScale);
  }

  StatusCode METMaker::rebuildMET(xAOD::MissingET* met,
                                  const xAOD::IParticleContainer* collection,
                                  xAOD::MissingETAssociationHelper& helper,
                                  MissingETBase::UsageHandler::Policy p,
                                  bool removeOverlap,
                                  MissingETBase::UsageHandler::Policy objScale) {
    if(!met || !collection) {
      ATH_MSG_WARNING("Invalid pointer supplied for "
                      << "MET (" << met << ") or "
                      << "collection (" << collection << ").");
      return StatusCode::SUCCESS;
    }
    const xAOD::MissingETAssociationMap* map = helper.map();
    if(!map){
      ATH_MSG_WARNING("MET Association Helper isn't associated with a MissingETAssociationMap!");
      return StatusCode::SUCCESS;
    }
    if(map->empty()) {
      ATH_MSG_WARNING("Incomplete association map received. Cannot rebuild MET.");
      ATH_MSG_WARNING("Note: METMaker should only be run on events containing at least one PV");
      return StatusCode::SUCCESS;
    }
    ATH_MSG_VERBOSE("Building MET term " << met->name());
    dec_constitObjLinks(*met) = vector<iplink_t>(0);
    dec_constitObjWeights(*met) = vector<float>(0);
    std::vector<iplink_t>& uniqueLinks = dec_constitObjLinks(*met);
    std::vector<float>& uniqueWeights = dec_constitObjWeights(*met);
    uniqueLinks.reserve(collection->size());
    uniqueWeights.reserve(collection->size());

    // Get the hashed key of this collection, if we can. Though his only works
    // if
    //   1. the container is an owning container, and not just a view;
    //   2. the container is in the event store already.
    // Since we will be creating ElementLink-s to these objects later on in the
    // code, and it should work in AnalysisBase, only the first one of these
    // is checked. Since the code can not work otherwise.
    SG::sgkey_t collectionSgKey = 0;
    if(collection->ownPolicy() == SG::OWN_ELEMENTS) {
      collectionSgKey = getKey(collection);
      if(collectionSgKey == 0) {
        ATH_MSG_ERROR("Could not find the collection with pointer: "
                      << collection);
        return StatusCode::FAILURE;
      }
    }

    if(!collection->empty()) {
      bool originalInputs = !acc_originalObject.isAvailable(*collection->front());
      bool isShallowCopy = dynamic_cast<const xAOD::ShallowAuxContainer*>(collection->front()->container()->getConstStore());
      ATH_MSG_VERBOSE("const store = " << collection->front()->container()->getConstStore());
      if(isShallowCopy && originalInputs) {
        ATH_MSG_WARNING("Shallow copy provided without \"originalObjectLinks\" decoration! "
                        << "Overlap removal cannot be done. "
                        << "Will not compute this term.");
        ATH_MSG_WARNING("Please apply xAOD::setOriginalObjectLinks() from xAODBase/IParticleHelpers.h");
        return StatusCode::SUCCESS;
      } else {
        ATH_MSG_VERBOSE("Original inputs? " << originalInputs);
      }
      for(const auto *const obj : *collection) {
        const IParticle* orig = obj;
        bool selected = false;
        if(!originalInputs) { orig = *acc_originalObject(*obj); }
        std::vector<const xAOD::MissingETAssociation*> assocs = xAOD::MissingETComposition::getAssociations(map,orig);
        if(assocs.empty()) {
          std::string message = "Object is not in association map. Did you make a deep copy but fail to set the \"originalObjectLinks\" decoration? "
                                "If not, Please apply xAOD::setOriginalObjectLinks() from xAODBase/IParticleHelpers.h";
          // Avoid warnings for leptons with pT below threshold for association map
          if (orig->pt()>m_missObjWarningPtThreshold) {
              ATH_MSG_WARNING(message);
          } else {
              ATH_MSG_DEBUG(message);
          }
          // if this is an uncalibrated electron below the threshold, then we put it into the soft term
          if(orig->type()==xAOD::Type::Electron){
            iplink_t objLink;
            if(collectionSgKey == 0) {
              const xAOD::IParticleContainer* ipc = static_cast<const xAOD::IParticleContainer*>(obj->container());
              objLink = iplink_t(*ipc, obj->index());
            } else {
              objLink = iplink_t(collectionSgKey, obj->index());
            }
            uniqueLinks.emplace_back( objLink );
            uniqueWeights.emplace_back( 0. );
            message = "Missing an electron from the MET map. Included as a track in the soft term. pT: " + std::to_string(obj->pt()/1e3) + " GeV";
            if (orig->pt()>m_missObjWarningPtThreshold) {
                ATH_MSG_WARNING(message);
            } else {
                ATH_MSG_DEBUG(message);
            }
            continue;
          } else {
            ATH_MSG_ERROR("Missing an object: " << orig->type() << " pT: " << obj->pt()/1e3 << " GeV, may be duplicated in the soft term.");
          }
        }

        // If the object has already been selected and processed, ignore it.
        if(MissingETComposition::objSelected(helper,orig)) continue;
        selected = MissingETComposition::selectIfNoOverlaps(helper,orig,p) || !removeOverlap;
        ATH_MSG_VERBOSE(obj->type() << " (" << orig <<") with pt " << obj->pt()
                        << " is " << ( selected ? "non-" : "") << "overlapping");

        // Greedy photon options: set selection flags
        if ((m_greedyPhotons || m_veryGreedyPhotons) && selected && obj->type() == xAOD::Type::Photon){
          for(const xAOD::MissingETAssociation* assoc : assocs){
            std::vector<size_t> indices = assoc->overlapIndices(orig);
            std::vector<const xAOD::IParticle*> allObjects = assoc->objects();
            for (size_t index : indices){
              const xAOD::IParticle* thisObj = allObjects[index];
              if(!thisObj) continue;
              if ((thisObj->type() == xAOD::Type::Jet && m_veryGreedyPhotons) ||
                   thisObj->type() == xAOD::Type::Electron)
                helper.setObjSelectionFlag(assoc, thisObj, true);
            }
          }
        }

        //Do special overlap removal for calo tagged muons
        if(m_orCaloTaggedMuon && !removeOverlap && orig->type()==xAOD::Type::Muon && static_cast<const xAOD::Muon*>(orig)->muonType()==xAOD::Muon::CaloTagged) {
          for (size_t i = 0; i < assocs.size(); i++) {
            std::vector<size_t> ind = assocs[i]->overlapIndices(orig);
            std::vector<const xAOD::IParticle*> allObjects = assocs[i]->objects();
            for (size_t indi = 0; indi < ind.size(); indi++) if (allObjects[ind[indi]]) {
                if (allObjects[ind[indi]]->type()==xAOD::Type::Electron
                    && helper.objSelected(assocs[i], ind[indi])) {
                  selected = false;
                  break;
                }
              }
          }
        }
        // Don't overlap remove muons, but flag the non-overlapping muons to take out their tracks from jets
        // Removed eloss from here -- clusters already flagged.
        // To be handled in rebuildJetMET
        if(selected) {
          if(objScale==MissingETBase::UsageHandler::PhysicsObject) {
            ATH_MSG_VERBOSE("Add object with pt " << obj->pt());
            *met += obj;
          } else {
            MissingETBase::Types::constvec_t constvec = MissingETComposition::getConstVec(map,obj,objScale);
            ATH_MSG_VERBOSE("Add truth object with pt " << constvec.cpt());
            met->add(constvec.cpx(),constvec.cpy(),constvec.cpt());
          }
        }
        if(selected) {
          iplink_t objLink;
          if(collectionSgKey == 0) {
            const xAOD::IParticleContainer* ipc =
              static_cast<const xAOD::IParticleContainer*>(obj->container());
            objLink = iplink_t(*ipc, obj->index());
          } else {
            objLink = iplink_t(collectionSgKey, obj->index());
          }
          uniqueLinks.push_back( objLink );
          uniqueWeights.push_back( 1. );
        }
      }
    }
    ATH_MSG_DEBUG("Built met term " << met->name() << ", with magnitude " << met->met());
    return StatusCode::SUCCESS;
  }

  StatusCode METMaker::rebuildJetMET(const std::string& metJetKey,
                                     const std::string& softKey,
                                     xAOD::MissingETContainer* metCont,
                                     const xAOD::JetContainer* jets,
                                     const xAOD::MissingETContainer* metCoreCont,
                                     xAOD::MissingETAssociationHelper& helper,
                                     bool doJetJVT)
  {
    ATH_MSG_VERBOSE("Rebuild jet term: " << metJetKey << " and soft term: " << softKey);

    MissingET* metJet = nullptr;
    if( fillMET(metJet,metCont, metJetKey, MissingETBase::Source::jet()) != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("failed to fill MET term \"" << metJetKey << "\"");
      return StatusCode::FAILURE;
    }

    const MissingET *coreSoftClus(nullptr), *coreSoftTrk(nullptr);
    MissingET *metSoftClus(nullptr), *metSoftTrk(nullptr);

    const MissingET* coreSoft = (*metCoreCont)[softKey+"Core"];
    if(!coreSoft) {
      ATH_MSG_WARNING("Invalid soft term key supplied: " << softKey);
      return StatusCode::FAILURE;
    }
    if(MissingETBase::Source::isTrackTerm(coreSoft->source())) {
      coreSoftTrk = coreSoft;

      metSoftTrk = nullptr;
      if( fillMET(metSoftTrk,metCont, softKey , coreSoftTrk->source() ) != StatusCode::SUCCESS) {
        ATH_MSG_ERROR("failed to fill MET term \"" << softKey << "\"");
        return StatusCode::FAILURE;
      }
    } else {
      coreSoftClus = coreSoft;

      metSoftClus = nullptr;
      if( fillMET(metSoftClus, metCont, softKey , coreSoftClus->source() ) != StatusCode::SUCCESS) {
        ATH_MSG_ERROR("failed to fill MET term \"" << softKey << "\"");
        return StatusCode::FAILURE;
      }
    }

    return rebuildJetMET(metJet, jets, helper,
                         metSoftClus, coreSoftClus,
                         metSoftTrk,  coreSoftTrk,
                         doJetJVT);
  }

  StatusCode METMaker::rebuildTrackMET(const std::string& metJetKey,
                                       const std::string& softKey,
                                       xAOD::MissingETContainer* metCont,
                                       const xAOD::JetContainer* jets,
                                       const xAOD::MissingETContainer* metCoreCont,
                                       xAOD::MissingETAssociationHelper& helper,
                                       bool doJetJVT)
  {
    ATH_MSG_VERBOSE("Rebuild jet term: " << metJetKey << " and soft term: " << softKey);

    MissingET* metJet = nullptr;
    if( fillMET(metJet , metCont,  metJetKey  , MissingETBase::Source::jet() | MissingETBase::Source::track() ) != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("failed to fill MET term \"" << metJetKey << "\"");
      return StatusCode::FAILURE;
    }

    const MissingET *coreSoftTrk(nullptr);
    MissingET *metSoftTrk(nullptr);

    const MissingET* coreSoft = (*metCoreCont)[softKey+"Core"];
    if(!coreSoft) {
      ATH_MSG_WARNING("Invalid soft term key supplied: " << softKey);
      return StatusCode::FAILURE;
    }
    coreSoftTrk = coreSoft;

    metSoftTrk = nullptr;
    if( fillMET(metSoftTrk , metCont, softKey  , coreSoftTrk->source()) != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("failed to fill MET term \"" << softKey << "\"");
      return StatusCode::FAILURE;
    }

    return rebuildTrackMET(metJet, jets, helper,
                           metSoftTrk,  coreSoftTrk,
                           doJetJVT);
  }

  StatusCode METMaker::rebuildJetMET(const std::string& metJetKey,
                                     const std::string& softClusKey,
                                     const std::string& softTrkKey,
                                     xAOD::MissingETContainer* metCont,
                                     const xAOD::JetContainer* jets,
                                     const xAOD::MissingETContainer* metCoreCont,
                                     xAOD::MissingETAssociationHelper& helper,
                                     bool doJetJVT)
  {

    ATH_MSG_VERBOSE("Create Jet MET " << metJetKey);
    MissingET* metJet = nullptr;
    if( fillMET(metJet , metCont ,metJetKey , MissingETBase::Source::jet()) != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("failed to fill MET term \"" << metJetKey << "\"");
      return StatusCode::FAILURE;
    }
    ATH_MSG_VERBOSE("Create SoftClus MET " << softClusKey);
    const MissingET* coreSoftClus = (*metCoreCont)[softClusKey+"Core"];
    ATH_MSG_VERBOSE("Create SoftTrk MET " << softTrkKey);
    const MissingET* coreSoftTrk = (*metCoreCont)[softTrkKey+"Core"];
    if(!coreSoftClus) {
      ATH_MSG_WARNING("Invalid cluster soft term key supplied: " << softClusKey);
      return StatusCode::FAILURE;
    }
    if(!coreSoftTrk) {
      ATH_MSG_WARNING("Invalid track soft term key supplied: " << softTrkKey);
      return StatusCode::FAILURE;
    }
    MissingET* metSoftClus = nullptr;
    if( fillMET(metSoftClus, metCont, softClusKey,  coreSoftClus->source()) != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("failed to fill MET term \"" << softClusKey << "\"");
      return StatusCode::FAILURE;
    }

    MissingET* metSoftTrk = nullptr;
    if( fillMET(metSoftTrk, metCont, softTrkKey,  coreSoftTrk->source()) != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("failed to fill MET term \"" << softTrkKey << "\"");
      return StatusCode::FAILURE;
    }

    return rebuildJetMET(metJet, jets, helper,
                         metSoftClus, coreSoftClus,
                         metSoftTrk, coreSoftTrk,
                         doJetJVT);
  }

  StatusCode METMaker::rebuildJetMET(xAOD::MissingET* metJet,
                                     const xAOD::JetContainer* jets,
                                     xAOD::MissingETAssociationHelper& helper,
                                     xAOD::MissingET* metSoftClus,
                                     const xAOD::MissingET* coreSoftClus,
                                     xAOD::MissingET* metSoftTrk,
                                     const xAOD::MissingET* coreSoftTrk,
                                     bool doJetJVT,
                                     bool tracksForHardJets,
                                     std::vector<const xAOD::IParticle*>* softConst) {
    if(!metJet || !jets) {
      ATH_MSG_WARNING("Invalid pointer supplied for "
                      << "MET (" << metJet << ") or "
                      << "jet collection (" << jets << ").");
      return StatusCode::SUCCESS;
    }
    const xAOD::MissingETAssociationMap* map = helper.map();
    if(!map){
      ATH_MSG_WARNING("MET Association Helper isn't associated with a MissingETAssociationMap!");
      return StatusCode::SUCCESS;
    }
    if(softConst && m_trkseltool.empty() && !m_doPFlow && !m_doSoftTruth) {
      ATH_MSG_WARNING( "Requested soft track element links, but no track selection tool supplied.");
    }
    const xAOD::Vertex *pv = softConst?getPV():nullptr;

    if(map->empty()) {
      ATH_MSG_WARNING("Incomplete association map received. Cannot rebuild MET.");
      ATH_MSG_WARNING("Note: METMaker should only be run on events containing at least one PV");
      return StatusCode::SUCCESS;
    }
    ATH_MSG_VERBOSE("Building MET jet term " << metJet->name());
    if(!metSoftClus && !metSoftTrk) {
      ATH_MSG_WARNING("Neither soft cluster nor soft track term has been supplied!");
      return StatusCode::SUCCESS;
    }
    static const SG::AuxElement::ConstAccessor<std::vector<ElementLink<IParticleContainer> > > acc_softConst("softConstituents");
    if(metSoftClus) {
      dec_constitObjLinks(*metSoftClus) = vector<iplink_t>(0);
      if(coreSoftClus) {
        ATH_MSG_VERBOSE("Building MET soft cluster term " << metSoftClus->name());
        ATH_MSG_VERBOSE("Core soft cluster mpx " << coreSoftClus->mpx()
                        << ", mpy " << coreSoftClus->mpy()
                        << " sumet " << coreSoftClus->sumet());
        *metSoftClus += *coreSoftClus;
      } else {
        ATH_MSG_WARNING("Soft cluster term provided without a core term!");
        return StatusCode::SUCCESS;
      }
      // Fill a vector with the soft constituents, if one was provided.
      // For now, only setting up to work with those corresponding to the jet constituents.
      // Can expand if needed.
      if(softConst && acc_softConst.isAvailable(*coreSoftClus)) {
        for(const auto& constit : acc_softConst(*coreSoftClus)) {
          softConst->push_back(*constit);
        }
        ATH_MSG_DEBUG(softConst->size() << " soft constituents from core term");
      }
    }
    if(metSoftTrk) {
      dec_constitObjLinks(*metSoftTrk) = vector<iplink_t>(0);
      if(coreSoftTrk) {
        ATH_MSG_VERBOSE("Building MET soft track term " << metSoftTrk->name());
        ATH_MSG_VERBOSE("Core soft track mpx " << coreSoftTrk->mpx()
                        << ", mpy " << coreSoftTrk->mpy()
                        << " sumet " << coreSoftTrk->sumet());
        *metSoftTrk += *coreSoftTrk;
      } else {
        ATH_MSG_WARNING("Soft track term provided without a core term!");
        return StatusCode::SUCCESS;
      }
      if(softConst && acc_softConst.isAvailable(*coreSoftTrk) && !m_doPFlow && !m_doSoftTruth) {
        for(const auto& constit : acc_softConst(*coreSoftTrk)) {
          softConst->push_back(*constit);
        }
        ATH_MSG_DEBUG(softConst->size() << " soft constituents from trk core term");
      }
    }

    dec_constitObjLinks(*metJet) = std::vector<iplink_t>(0);
    dec_constitObjWeights(*metJet) = std::vector<float>(0);
    std::vector<iplink_t>& uniqueLinks = dec_constitObjLinks(*metJet);
    std::vector<float>& uniqueWeights = dec_constitObjWeights(*metJet);
    uniqueLinks.reserve(jets->size());
    uniqueWeights.reserve(jets->size());
    std::vector<iplink_t> softJetLinks;
    std::vector<float> softJetWeights;
    bool originalInputs = jets->empty() ? false : !acc_originalObject.isAvailable(*jets->front());

    // Get the hashed key of this jet, if we can. Though his only works if
    //   1. the container is an owning container, and not just a view;
    //   2. the container is in the event store already.
    // Since we will be creating ElementLink-s to these jets later on in the
    // code, and it should work in AnalysisBase, only the first one of these
    // is checked. Since the code can not work otherwise.
    SG::sgkey_t jetsSgKey = 0;
    if(jets->ownPolicy() == SG::OWN_ELEMENTS) {
      jetsSgKey = getKey(jets);
      if(jetsSgKey == 0) {
        ATH_MSG_ERROR("Could not find the jets with pointer: " << jets);
        return StatusCode::FAILURE;
      }
    }

    for(const auto *const jet : *jets) {
      const MissingETAssociation* assoc = nullptr;
      if(originalInputs) {
        assoc = MissingETComposition::getAssociation(map,jet);
      } else {
        const IParticle* orig = *acc_originalObject(*jet);
        assoc = MissingETComposition::getAssociation(map,static_cast<const xAOD::Jet*>(orig));
      }
      if(assoc && !assoc->isMisc()) {

        // init nominal_jet and either actually asign nominal jet or fall back to systematic jet
        const xAOD::Jet * nominal_jet = nullptr;
        if(m_doSimpleOR) {
          // retrieve nominal calibrated jet
          if (acc_nominalObject.isAvailable(*jet))
            nominal_jet = static_cast<const xAOD::Jet*>(*acc_nominalObject(*jet));
          else {
            ATH_MSG_ERROR("No nominal calibrated jet available for jet " << jet->index() << ". Cannot simplify overlap removal!");
            nominal_jet = jet;
          }
        }
        else
          nominal_jet = jet;

        ATH_MSG_VERBOSE( "Jet (nom. calib) pt = " << nominal_jet->pt());
        ATH_MSG_VERBOSE( "Jet pt = " << jet->pt());

        // use nominal calibrated jets instead
        bool selected = (std::abs(nominal_jet->eta())<m_JetEtaForw && nominal_jet->pt()>m_CenJetPtCut) || (std::abs(nominal_jet->eta())>=m_JetEtaForw && nominal_jet->pt()>m_FwdJetPtCut );
        bool JVT_reject(false);
        bool isMuFSRJet(false);

        // Apply a cut on the maximum jet eta. This restricts jets to those with calibration. Excluding more forward jets was found to have a minimal impact on the MET in Zee events
        // use nominal calibrated jets instead
        if (m_JetEtaMax > 0.0 && std::abs(nominal_jet->eta()) > m_JetEtaMax)
          JVT_reject = true;

        if(doJetJVT) {
          if (!m_useR21JvtFallback) {
            // intrinsically checks that is within range to apply Jvt requirement
            // use nominal calibrated jets instead
            JVT_reject  = !bool(m_JvtTool->accept(nominal_jet));
          }
          else {
            if(jet->pt()<m_JvtPtMax && std::abs(jet->eta())<m_JetEtaForw) {
              float jvt;
              bool gotJVT = m_acc_jetJvtMoment->isAvailable(*jet);
              if(gotJVT) {
                jvt = (*m_acc_jetJvtMoment)(*jet);
                JVT_reject = jvt<m_JvtCut;
                if(m_JvtMediumPtMax>0.0 && jet->pt()<m_JvtMediumPtMax) JVT_reject = (jvt<m_JvtCutMedium);
                if(m_JvtTightPtMax>0.0  && jet->pt()<m_JvtTightPtMax)  JVT_reject = (jvt<m_JvtCutTight);
                ATH_MSG_VERBOSE("Jet " << (JVT_reject ? "fails" : "passes") <<" JVT selection");
              } else {
                JVT_reject = true;
                ATH_MSG_WARNING("Tried to retrieve JVT but this was not set. Failing this jet.");
              }
            }
          }
          ATH_MSG_VERBOSE("Jet " << (JVT_reject ? "fails" : "passes") <<" JVT selection");
        }

        // if defined apply additional jet criterium
        // use nominal calibrated jets instead
        if (m_acc_jetRejectionDec && (*m_acc_jetRejectionDec)(*nominal_jet)==0) JVT_reject = true;
        bool hardJet(false);
        MissingETBase::Types::constvec_t calvec = assoc->overlapCalVec(helper);
        bool caloverlap = false;
        caloverlap = calvec.ce()>0;
        ATH_MSG_DEBUG("Jet " << jet->index() << " is " << ( caloverlap ? "" : "non-") << "overlapping");
        if(caloverlap) {
          for(const auto& object : assoc->objects()) {
            if(helper.objSelected(assoc, object)) {
              ATH_MSG_VERBOSE("  Jet overlaps with " << object->type() << " " << object->index()
                           << " with pt " << object->pt() << ", phi " << object->phi() );
            }

            // Correctly handle this jet if we're using very greedy photons
            if (object && object->type() == xAOD::Type::Photon && m_veryGreedyPhotons) hardJet = true;

          }
        }

        xAOD::JetFourMom_t constjet;
        double constSF(1);
        if(m_jetConstitScaleMom.empty() && assoc->hasAlternateConstVec()){
          constjet = assoc->getAlternateConstVec();
        } else { // we use this case but I don't think I need to use nominal calibrated jets - has no effect on OR decision
          constjet = jet->jetP4(m_jetConstitScaleMom);//grab a constituent scale added by the JetMomentTool/JetConstitFourMomTool.cxx
          double denom = (assoc->hasAlternateConstVec() ? assoc->getAlternateConstVec() : jet->jetP4("JetConstitScaleMomentum")).E();
          constSF = denom>1e-9 ? constjet.E()/denom : 0.;
          ATH_MSG_VERBOSE("Scale const jet by factor " << constSF);
          calvec *= constSF;
        }
        double jpx = constjet.Px();
        double jpy = constjet.Py();
        double jpt = constjet.Pt();
        double opx = jpx - calvec.cpx();
        double opy = jpy - calvec.cpy();

        MissingET* met_muonEloss(nullptr);
        if(m_muEloss || m_doSetMuonJetEMScale) {
          // Get a term to hold the Eloss corrections
          MissingETContainer* metCont = static_cast<MissingETContainer*>(metJet->container());
          met_muonEloss = (*metCont)["MuonEloss"];
          if(!met_muonEloss) {
            ATH_MSG_WARNING("Attempted to apply muon Eloss correction, but corresponding MET term does not exist!");
            return StatusCode::FAILURE;
          }
        }

        float total_eloss(0);
        MissingETBase::Types::bitmask_t muons_selflags(0);
        std::vector<const xAOD::Muon*> muons_in_jet;
        std::vector<const xAOD::Electron*> electrons_in_jet;
        bool passJetForEl=false;
        if(m_useGhostMuons) { // for backwards-compatibility
          if(acc_ghostMuons.isAvailable(*jet)) {
            for(const auto& el : acc_ghostMuons(*jet)) {
              if(el.isValid()) {
                       muons_in_jet.push_back(static_cast<const xAOD::Muon*>(*el));
              } else {
                       ATH_MSG_WARNING("Invalid element link to ghost muon! Quitting.");
                       return StatusCode::FAILURE;
              }
            }
          } else {
            ATH_MSG_WARNING("Ghost muons requested but not found!");
            return StatusCode::FAILURE;
          }
        }
        for(const auto& obj : assoc->objects()) {
          if (!obj) { continue; }
          if(obj->type()==xAOD::Type::Muon && !m_useGhostMuons) {
            const xAOD::Muon* mu_test(static_cast<const xAOD::Muon*>(obj));
            ATH_MSG_VERBOSE("Muon " << mu_test->index() << " found in jet " << jet->index());
            if((m_doRemoveMuonJets || m_doSetMuonJetEMScale)) {
              if(acc_originalObject.isAvailable(*mu_test)) mu_test = static_cast<const xAOD::Muon*>(*acc_originalObject(*mu_test));
              if(MissingETComposition::objSelected(helper,mu_test)) { //
                muons_in_jet.push_back(mu_test);
                ATH_MSG_VERBOSE("Muon is selected by MET.");
              }
            }
          } else if(obj->type()==xAOD::Type::Electron && m_doRemoveElecTrks) {
            const xAOD::Electron* el_test(static_cast<const xAOD::Electron*>(obj));
            ATH_MSG_VERBOSE("Electron " << el_test->index() << " found in jet " << jet->index());
            if(acc_originalObject.isAvailable(*el_test)) el_test = static_cast<const xAOD::Electron*>(*acc_originalObject(*el_test));
            if(helper.objSelected(assoc,el_test)){
              if(el_test->pt()>90.0e3) { // only worry about high-pt electrons?
                electrons_in_jet.push_back(el_test);
                ATH_MSG_VERBOSE("High-pt electron is selected by MET.");
              }
            }
          }
        }
        if(m_doRemoveElecTrks) {
          MissingETBase::Types::constvec_t initialTrkMom = assoc->jetTrkVec();
          float jet_ORtrk_sumpt = assoc->overlapTrkVec(helper).sumpt();
          float jet_all_trk_pt =  initialTrkMom.sumpt();
          float jet_unique_trk_pt = jet_all_trk_pt - jet_ORtrk_sumpt;
          MissingETBase::Types::constvec_t el_calvec;
          MissingETBase::Types::constvec_t el_trkvec;
          for(const auto& elec : electrons_in_jet) {
              el_calvec += assoc->calVec(elec);
              el_trkvec += assoc->trkVec(elec);
          }
          float el_cal_pt = el_calvec.cpt();
          float el_trk_pt = el_trkvec.cpt();
          ATH_MSG_VERBOSE("Elec trk: " << el_trk_pt
                          << " jetalltrk: " << jet_all_trk_pt
                          << " jetORtrk: " << jet_ORtrk_sumpt
                          << " electrk-jetORtrk: " << (el_trk_pt-jet_ORtrk_sumpt)
                          << " elec cal: " << el_cal_pt
                          << " jetalltrk-electrk: " << (jet_all_trk_pt-el_trk_pt)
                          << " jetalltrk-jetORtrk: " << (jet_all_trk_pt-jet_ORtrk_sumpt) );
          // Want to use the jet calo measurement if we had at least one electron
          // and the jet has a lot of residual track pt
          // Is the cut appropriate?
          if(el_trk_pt>1e-9 && jet_unique_trk_pt>10.0e3) passJetForEl=true;
        } // end ele-track removal

        for(const xAOD::Muon* mu_in_jet : muons_in_jet) {
          if (not mu_in_jet) continue;
          float mu_Eloss = acc_Eloss(*mu_in_jet);

          if(!JVT_reject) {
            if (m_doRemoveMuonJets) {
              // need to investigate how this is affected by the recording of muon clusters in the map
              float mu_id_pt = mu_in_jet->trackParticle(xAOD::Muon::InnerDetectorTrackParticle) ? mu_in_jet->trackParticle(xAOD::Muon::InnerDetectorTrackParticle)->pt() : 0.;
              float jet_trk_sumpt = acc_trksumpt.isAvailable(*jet) && this->getPV() ? acc_trksumpt(*jet)[this->getPV()->index()] : 0.;

              // missed the muon, so we should add it back
              if(0.9999*mu_id_pt>jet_trk_sumpt)
                jet_trk_sumpt+=mu_id_pt;
              float jet_trk_N = acc_trkN.isAvailable(*jet) && this->getPV() ? acc_trkN(*jet)[this->getPV()->index()] : 0.;
              ATH_MSG_VERBOSE("Muon has ID pt " << mu_id_pt);
              ATH_MSG_VERBOSE("Jet has pt " << jet->pt() << ", trk sumpt " << jet_trk_sumpt << ", trk N " << jet_trk_N);
              // those corrections are negligible but use nominal calibrated jets instead
              bool jet_from_muon = mu_id_pt>1e-9 && jet_trk_sumpt>1e-9 && (nominal_jet->pt()/mu_id_pt < m_muIDPTJetPtRatioMuOlap && mu_id_pt/jet_trk_sumpt>m_jetTrkPtMuPt) && jet_trk_N<m_jetTrkNMuOlap;
              if(jet_from_muon) {
                ATH_MSG_VERBOSE("Jet is from muon -- remove.");
                JVT_reject = true;
              }
            }

            if (m_doSetMuonJetEMScale) {
              // need to investigate how this is affected by the recording of muon clusters in the map
              float mu_id_pt = mu_in_jet->trackParticle(xAOD::Muon::InnerDetectorTrackParticle) ? mu_in_jet->trackParticle(xAOD::Muon::InnerDetectorTrackParticle)->pt() : 0.;
              float jet_trk_sumpt = acc_trksumpt.isAvailable(*jet) && this->getPV() ? acc_trksumpt(*jet)[this->getPV()->index()] : 0.;
              // missed the muon, so we should add it back
              if(0.9999*mu_id_pt>jet_trk_sumpt)
                jet_trk_sumpt+=mu_id_pt;
              float jet_trk_N = acc_trkN.isAvailable(*jet) && this->getPV() ? acc_trkN(*jet)[this->getPV()->index()] : 0.;

              float jet_psE = 0.;
              if (acc_psf.isAvailable(*jet)){
                jet_psE = acc_psf(*jet);
              } else if (acc_sampleE.isAvailable(*jet)){
                jet_psE = acc_sampleE(*jet)[0] + acc_sampleE(*jet)[4];
              } else {
                ATH_MSG_ERROR("Jet PS fraction or sampling energy must be available to calculate MET with doSetMuonJetEMScale");
                return StatusCode::FAILURE;
              }

              bool jet_from_muon = jet_trk_sumpt>1e-9 && jet_trk_N<3 && mu_id_pt / jet_trk_sumpt > m_jetTrkPtMuPt && acc_emf(*jet)>m_jetEmfMuOlap && acc_width(*jet)<m_jetWidthMuOlap && jet_psE>m_jetPsEMuOlap;
              ATH_MSG_VERBOSE("Muon has ID pt " << mu_id_pt);
              ATH_MSG_VERBOSE("Jet has trk sumpt " << jet_trk_sumpt << ", trk N " << jet_trk_N << ", PS E " << jet_psE << ", width " << acc_width(*jet) << ", emfrac " << acc_emf(*jet));

              if(jet_from_muon) {
                ATH_MSG_VERBOSE("Jet is from muon -- set to EM scale and subtract Eloss.");
                // Using constjet now because we focus on AntiKt4EMTopo.
                // Probably not a massive difference to LC, but PF needs some consideration
                ATH_MSG_VERBOSE("Jet e: " << constjet.E() << ", mu Eloss: " << mu_Eloss);
                float elosscorr = mu_Eloss >= constjet.e() ? 0. : 1.-mu_Eloss/constjet.e();
                // Effectively, take the unique fraction of the jet times the eloss-corrected fraction
                // This might in some cases oversubtract, but should err on the side of undercounting the jet contribution
                opx *= elosscorr;
                opy *= elosscorr;
                ATH_MSG_VERBOSE(" Jet eloss factor " << elosscorr << ", final pt: " << sqrt(opx*opx+opy*opy));
                // Don't treat this jet normally. Instead, just add to the Eloss term
                isMuFSRJet = true;
              }
            }
          } // end muon-jet overlap-removal

          switch(mu_in_jet->energyLossType()) {
            case xAOD::Muon::Parametrized:
            case xAOD::Muon::MOP:
            case xAOD::Muon::Tail:
            case xAOD::Muon::FSRcandidate:
            case xAOD::Muon::NotIsolated:
              // For now don't differentiate the behaviour
              // Remove the Eloss assuming the parameterised value
              // The correction is limited to the selected clusters
              total_eloss += mu_Eloss;
              muons_selflags |= (1<<assoc->findIndex(mu_in_jet));
          }
        }
        ATH_MSG_VERBOSE("Muon selection flags: " << muons_selflags);
        ATH_MSG_VERBOSE("Muon total eloss: " << total_eloss);

        MissingETBase::Types::constvec_t mu_calovec;
        // borrowed from overlapCalVec
        for(size_t iKey = 0; iKey < assoc->sizeCal(); iKey++) {
          bool selector = (muons_selflags & assoc->calkey()[iKey]);
          if(selector) mu_calovec += assoc->calVec(iKey);
          ATH_MSG_VERBOSE("This key: " << assoc->calkey()[iKey] << ", selector: " << selector);
        }
        ATH_MSG_VERBOSE("Mu calovec pt, no Eloss:   " << mu_calovec.cpt());
        if(m_muEloss) mu_calovec *= std::max<float>(0.,1-(total_eloss/mu_calovec.ce()));
        ATH_MSG_VERBOSE("Mu calovec pt, with Eloss: " << mu_calovec.cpt());

        // re-add calo components of muons beyond Eloss correction
        ATH_MSG_VERBOSE("Jet " << jet->index() << " const pT before OR " << jpt);
        ATH_MSG_VERBOSE("Jet " << jet->index() << " const pT after OR " << sqrt(opx*opx+opy*opy));
        opx += mu_calovec.cpx();
        opy += mu_calovec.cpy();
        double opt = sqrt( opx*opx+opy*opy );
        ATH_MSG_VERBOSE("Jet " << jet->index() << " const pT diff after OR readding muon clusters " << opt-jpt);
        double uniquefrac = 1. - (calvec.ce() - mu_calovec.ce()) / constjet.E();
        ATH_MSG_VERBOSE( "Jet constscale px, py, pt, E = " << jpx << ", " << jpy << ", " << jpt << ", " << constjet.E() );
        ATH_MSG_VERBOSE( "Jet overlap E = " << calvec.ce() - mu_calovec.ce() );
        ATH_MSG_VERBOSE( "Jet OR px, py, pt, E = " << opx << ", " << opy << ", " << opt << ", " << constjet.E() - calvec.ce() );

        if(isMuFSRJet) {

          if(met_muonEloss) {
            met_muonEloss->add(opx,opy,opt);
          } else {
            ATH_MSG_WARNING("Attempted to apply muon Eloss correction, but corresponding MET term does not exist!");
            return StatusCode::FAILURE;
          }
        } else {
          if(selected && !JVT_reject) {
            if(!caloverlap) {
              // add jet full four-vector
              hardJet = true;
              if (!tracksForHardJets) {
                if(m_doConstJet) {
                  metJet->add(jpx,jpy,jpt);
                } else {*metJet += jet;}
              }
            } else {
              // check unique fraction
              if((uniquefrac>m_jetMinEfrac || passJetForEl) && opt>m_jetMinWeightedPt) {
                // add jet corrected for overlaps
                hardJet = true;
                if(!tracksForHardJets) {
                  if(m_jetCorrectPhi) {
                    if (m_doConstJet) metJet->add(opx,opy,opt);
                    else {
                      double jesF = jet->pt() / jpt;
                      metJet->add(opx*jesF,opy*jesF,opt*jesF);
                    }
                  } else {
                    if (m_doConstJet){
                      metJet->add(uniquefrac*jpx,uniquefrac*jpy,uniquefrac*jpt);
                    }
                    else{
                      if(passJetForEl){
                        if(m_doRemoveElecTrksEM) metJet->add(opx,opy,opt);
                        else metJet->add(uniquefrac*jet->px(),uniquefrac*jet->py(),uniquefrac*jet->pt());
                      }else{
                        metJet->add(uniquefrac*jet->px(),uniquefrac*jet->py(),uniquefrac*jet->pt());
                      }
                    }
                  }
                }
              }
            }
          }  // hard jet selection

          // Create the appropriate ElementLink for this jet just the once.
          iplink_t jetLink;
          if(jetsSgKey == 0) {
            const xAOD::IParticleContainer* ipc =
              static_cast<const xAOD::IParticleContainer*>(jet->container());
            jetLink = iplink_t(*ipc, jet->index());
          } else {
            jetLink = iplink_t(jetsSgKey, jet->index());
          }

          if(hardJet){
            ATH_MSG_VERBOSE("Jet added at full scale");
            uniqueLinks.push_back( jetLink );
            uniqueWeights.push_back( uniquefrac );
          } else {
            if(metSoftClus && !JVT_reject) {
              // add fractional contribution
              ATH_MSG_VERBOSE("Jet added at const scale");
              if (std::abs(jet->eta())<2.5 || !(coreSoftClus->source()&MissingETBase::Source::Region::Central)) {
                softJetLinks.push_back( jetLink );
                softJetWeights.push_back( uniquefrac );
                metSoftClus->add(opx,opy,opt);
              }

              // Fill a vector with the soft constituents, if one was provided.
              // For now, only setting up to work with those corresponding to the jet constituents.
              // Can expand if needed.
              // This ignores overlap removal.
              //
              if(softConst) {
                for(size_t iConst=0; iConst<jet->numConstituents(); ++iConst) {
                  const IParticle* constit = jet->rawConstituent(iConst);
                  softConst->push_back(constit);
                }
              }
            }
          } // hard jet or CST

          if(metSoftTrk && (!hardJet || tracksForHardJets)) {
            // use jet tracks
            // remove any tracks already used by other objects
            MissingETBase::Types::constvec_t trkvec = assoc->overlapTrkVec(helper);
            MissingETBase::Types::constvec_t jettrkvec = assoc->jetTrkVec();
            if(jettrkvec.ce()>1e-9) {
              jpx = jettrkvec.cpx();
              jpy = jettrkvec.cpy();
              jpt = jettrkvec.sumpt();
              jettrkvec -= trkvec;
              opx = jettrkvec.cpx();
              opy = jettrkvec.cpy();
              opt = jettrkvec.sumpt();
              ATH_MSG_VERBOSE( "Jet track px, py, sumpt = " << jpx << ", " << jpy << ", " << jpt );
              ATH_MSG_VERBOSE( "Jet OR px, py, sumpt = " << opx << ", " << opy << ", " << opt );
            } else {
              opx = opy = opt = 0;
              ATH_MSG_VERBOSE( "This jet has no associated tracks" );
            }
            if (hardJet) metJet->add(opx,opy,opt);
            // use nominal calibrated jets instead
            else if (std::abs(nominal_jet->eta())<2.5 || !(coreSoftTrk->source()&MissingETBase::Source::Region::Central)) {
              metSoftTrk->add(opx,opy,opt);
              // Don't need to add if already done for softclus.
              if(!metSoftClus) {
                softJetLinks.push_back( jetLink );
                softJetWeights.push_back( uniquefrac );
              }

              // Fill a vector with the soft constituents, if one was provided.
              // For now, only setting up to work with those corresponding to the jet constituents.
              // Can expand if needed.
              // This ignores overlap removal.
              //
              if(softConst && !m_doPFlow && !m_doSoftTruth) {
                std::vector<const IParticle*> jettracks;
                jet->getAssociatedObjects<IParticle>(xAOD::JetAttribute::GhostTrack,jettracks);
                for(size_t iConst=0; iConst<jettracks.size(); ++iConst) {
                  const TrackParticle* pTrk = static_cast<const TrackParticle*>(jettracks[iConst]);
                  if (acceptTrack(pTrk,pv)) softConst->push_back(pTrk);
                }
              }
            }
          } // soft track

        } // is not from muon FSR
      } // association exists
      else {
        ATH_MSG_WARNING( "Jet without association found!" );
      }
    } // jet loop

    ATH_MSG_DEBUG("Number of selected jets: " << dec_constitObjLinks(*metJet).size());

    if(metSoftTrk) {
      dec_constitObjLinks(*metSoftTrk) = softJetLinks;
      ATH_MSG_DEBUG("Number of softtrk jets: " << dec_constitObjLinks(*metSoftTrk).size());
    }

    if(metSoftClus) {
      dec_constitObjLinks(*metSoftClus) = softJetLinks;
      ATH_MSG_DEBUG("Number of softclus jets: " << dec_constitObjLinks(*metSoftClus).size());
    }

    if(softConst) ATH_MSG_DEBUG(softConst->size() << " soft constituents from core term + jets");

    if(metSoftTrk) {
      // supplement track term with any tracks associated to isolated muons
      // these are recorded in the misc association
      const MissingETAssociation* assoc = map->getMiscAssociation();
      if(assoc) {
        MissingETBase::Types::constvec_t trkvec = assoc->overlapTrkVec(helper);
        double opx = trkvec.cpx();
        double opy = trkvec.cpy();
        double osumpt = trkvec.sumpt();
        ATH_MSG_VERBOSE( "Misc track px, py, sumpt = " << opx << ", " << opy << ", " << osumpt );
        metSoftTrk->add(opx,opy,osumpt);
        ATH_MSG_VERBOSE("Final soft track mpx " << metSoftTrk->mpx()
                        << ", mpy " << metSoftTrk->mpy()
                        << " sumet " << metSoftTrk->sumet());
      }
    }

    if(metSoftClus) {
      // supplement cluster term with any clusters associated to isolated e/gamma
      // these are recorded in the misc association
      const MissingETAssociation* assoc = map->getMiscAssociation();
      if(assoc) {
        float total_eloss(0.);
        MissingETBase::Types::bitmask_t muons_selflags(0);
        MissingETBase::Types::constvec_t calvec = assoc->overlapCalVec(helper);
        double opx = calvec.cpx();
        double opy = calvec.cpy();
        double osumpt = calvec.sumpt();
        for(const auto& obj : assoc->objects()) {
          if (!obj) continue;
          if(obj->type()==xAOD::Type::Muon) {
            const xAOD::Muon* mu_test(static_cast<const xAOD::Muon*>(obj));
            if(acc_originalObject.isAvailable(*mu_test)) mu_test = static_cast<const xAOD::Muon*>(*acc_originalObject(*mu_test));
            if(MissingETComposition::objSelected(helper,mu_test)) { //
              float mu_Eloss = acc_Eloss(*mu_test);
              switch(mu_test->energyLossType()) {
              case xAOD::Muon::Parametrized:
              case xAOD::Muon::MOP:
              case xAOD::Muon::Tail:
              case xAOD::Muon::FSRcandidate:
              case xAOD::Muon::NotIsolated:
                // For now don't differentiate the behaviour
                // Remove the Eloss assuming the parameterised value
                // The correction is limited to the selected clusters
                total_eloss += mu_Eloss;
                muons_selflags |= (1<<assoc->findIndex(mu_test));
              }
              ATH_MSG_VERBOSE("Mu index " << mu_test->index());
            }
          }
        }
        ATH_MSG_VERBOSE("Mu selection flags " << muons_selflags);
        ATH_MSG_VERBOSE("Mu total eloss " << total_eloss);

        MissingETBase::Types::constvec_t mu_calovec;
        // borrowed from overlapCalVec
        for(size_t iKey = 0; iKey < assoc->sizeCal(); iKey++) {
          bool selector = (muons_selflags & assoc->calkey()[iKey]);
          ATH_MSG_VERBOSE("This key: " << assoc->calkey()[iKey] << ", selector: " << selector
                          << " this calvec E: " << assoc->calVec(iKey).ce());
          if(selector) mu_calovec += assoc->calVec(iKey);
        }
        if(m_muEloss){
          mu_calovec *= std::max<float>(0.,1-(total_eloss/mu_calovec.ce()));
          opx += mu_calovec.cpx();
          opy += mu_calovec.cpy();
          osumpt += mu_calovec.sumpt();
        }
        ATH_MSG_VERBOSE("Mu cluster sumpt " << mu_calovec.sumpt());

        ATH_MSG_VERBOSE( "Misc cluster px, py, sumpt = " << opx << ", " << opy << ", " << osumpt );
        metSoftClus->add(opx,opy,osumpt);
        ATH_MSG_VERBOSE("Final soft cluster mpx " << metSoftClus->mpx()
                        << ", mpy " << metSoftClus->mpy()
                        << " sumet " << metSoftClus->sumet());
      }
    }

    return StatusCode::SUCCESS;
  }

  StatusCode METMaker::rebuildTrackMET(xAOD::MissingET* metJet,
                                       const xAOD::JetContainer* jets,
                                       xAOD::MissingETAssociationHelper& helper,
                                       xAOD::MissingET* metSoftTrk,
                                       const xAOD::MissingET* coreSoftTrk,
                                       bool doJetJVT) {
    return rebuildJetMET(metJet,jets,helper,nullptr,nullptr,metSoftTrk,coreSoftTrk,doJetJVT,true);
  }

  // **** Remove objects and any overlaps from MET calculation ****

  StatusCode METMaker::markInvisible(const xAOD::IParticleContainer* collection,
                                     xAOD::MissingETAssociationHelper& helper,
                                     xAOD::MissingETContainer* metCont)
  {

    MissingET* met = nullptr;
    if( fillMET(met,metCont, "Invisibles" , invisSource) != StatusCode::SUCCESS) {
      ATH_MSG_ERROR("failed to fill MET term \"Invisibles\"");
      return StatusCode::FAILURE;
    }

    return rebuildMET(met,collection,helper,MissingETBase::UsageHandler::PhysicsObject);
  }



  // Retrieve non overlapping constituents
  ////////////////////////////////////////

  // Fill OverlapRemovedCHSParticleFlowObjects
  // and  OverlapRemovedCHSCharged/NeutralParticleFlowObjects
  StatusCode METMaker::retrieveOverlapRemovedConstituents(const xAOD::PFOContainer* cpfo, const xAOD::PFOContainer* npfo,
			  xAOD::MissingETAssociationHelper& metHelper,
			  xAOD::PFOContainer *OR_cpfos,
			  xAOD::PFOContainer *OR_npfos,
			  bool retainMuon,
			  const xAOD::IParticleContainer* muonCollection)//,
			  //MissingETBase::UsageHandler::Policy p); //
  {

    const xAOD::PFOContainer *OR_cpfos_tmp = retrieveOverlapRemovedConstituents(cpfo, metHelper,retainMuon,muonCollection);
    const xAOD::PFOContainer *OR_npfos_tmp = retrieveOverlapRemovedConstituents(npfo, metHelper,retainMuon,muonCollection);

    for (auto tmp_constit : static_cast<xAOD::PFOContainer>(*cpfo)){
      xAOD::PFO* constit=new xAOD::PFO();
      OR_cpfos->push_back(constit);
      *constit=*tmp_constit;

      bool keep=false;
      for (const auto *const ORconstit : *OR_cpfos_tmp){
	if (ORconstit->index()==tmp_constit->index() && ORconstit->charge()==tmp_constit->charge()) {keep=true;}
      }
      if (!keep){constit->setP4(0., 0., 0., 0.);}

      ATH_MSG_VERBOSE("Constituent with index " << tmp_constit->index() << ", charge " << tmp_constit->charge()<< " pT " << tmp_constit->pt() << ((keep==true) ? "" : " not ") <<" in OverlapRemovedCHSParticleFlowObjects");
    } // end cPFO loop

    for (auto tmp_constit : static_cast<xAOD::PFOContainer>(*npfo)){
      xAOD::PFO* constit=new xAOD::PFO();
      OR_npfos->push_back(constit);
      *constit=*tmp_constit;

      bool keep=false;
      for (const auto *const ORconstit : *OR_npfos_tmp){
	if (ORconstit->index()==tmp_constit->index() && ORconstit->charge()==tmp_constit->charge()) {keep=true;}
      }
      if (!keep){ constit->setP4(0., 0., 0., 0.); }

      ATH_MSG_VERBOSE("Constituent with index " << tmp_constit->index() << ", charge " << tmp_constit->charge()<< " pT " << tmp_constit->pt() << ((keep==true) ? "" : " not ") <<" in OverlapRemovedCHSParticleFlowObjects");
    } // end nPFO loop/



    return StatusCode::SUCCESS;
  }

  // Fill OverlapRemovedCHSParticleFlowObjects
  StatusCode METMaker::retrieveOverlapRemovedConstituents(const xAOD::PFOContainer* pfo,
			  xAOD::MissingETAssociationHelper& metHelper,
			  const xAOD::PFOContainer **OR_pfos,
			  bool retainMuon,
			  const xAOD::IParticleContainer* muonCollection)//,
			  //MissingETBase::UsageHandler::Policy p); //
  {
     *OR_pfos=retrieveOverlapRemovedConstituents(pfo,metHelper,retainMuon,muonCollection);
    return StatusCode::SUCCESS;
  }


  const xAOD::PFOContainer* METMaker::retrieveOverlapRemovedConstituents(const xAOD::PFOContainer* signals,  xAOD::MissingETAssociationHelper& helper, bool retainMuon, const xAOD::IParticleContainer* muonCollection, MissingETBase::UsageHandler::Policy p)
  {

    ATH_MSG_VERBOSE("Policy " << p <<" " <<MissingETBase::UsageHandler::ParticleFlow);
    const xAOD::MissingETAssociationMap* map = helper.map();


    // If muon is selected, flag it as non selected to retain its constituents in OR jets (to recover std. muon-jet overlap)
    std::vector<size_t> muon_index;
    if (retainMuon){
      bool originalInputs = !acc_originalObject.isAvailable(*muonCollection->front());

      for(const auto *const obj : *muonCollection) {
	const IParticle* orig = obj;
	if(!originalInputs) { orig = *acc_originalObject(*obj); }
	std::vector<const xAOD::MissingETAssociation*> assocs = xAOD::MissingETComposition::getAssociations(map,orig);
	if(assocs.empty()) {
	  ATH_MSG_WARNING("Object is not in association map. Did you make a deep copy but fail to set the \"originalObjectLinks\" decoration?");
	  ATH_MSG_WARNING("If not, Please apply xAOD::setOriginalObjectLinks() from xAODBase/IParticleHelpers.h");
	}
	if(MissingETComposition::objSelected(helper,orig)) {
	  ATH_MSG_DEBUG("Muon with index "<<orig->index() << " is selected. Flag it as non selected before getOverlapRemovedSignals");
	  muon_index.push_back(orig->index());
	  for(size_t i = 0; i < assocs.size(); i++) helper.setObjSelectionFlag(assocs[i],orig,false);
	}
      }

      /*ATH_MSG_VERBOSE("Check selected muons before getOverlapRemovedSignals");
      for(const auto& obj : *muonCollection) {
	const IParticle* orig = obj;
	if(!originalInputs) { orig = *acc_originalObject(*obj); }
	ATH_MSG_VERBOSE("Muon with index "<<orig->index() << " is " << (MissingETComposition::objSelected(helper,orig) ? "" : "non-") << "selected" );
      }*/
    } // end retainMuon

    const xAOD::PFOContainer* ORsignals =static_cast<const xAOD::PFOContainer*>(map->getOverlapRemovedSignals(helper,signals,p));

    /*for (const auto tmp_const : *signals){ // printout overlap removed constituents
      bool keep=false;
      for (const auto constit : *ORsignals){
	if (constit->index()==tmp_const->index() && constit->charge()==tmp_const->charge()){keep=true;}
      }
      if (keep==false){ANA_MSG_DEBUG("Retrieve OR constituents: DON'T keep " << tmp_const->index() << " with charge " <<  tmp_const->charge() << " and pt "<<tmp_const->pt());}
    }*/

    // Flag back muons as selected
    if (retainMuon && !muon_index.empty()){
      bool originalInputs = !acc_originalObject.isAvailable(*muonCollection->front());
      for(const auto *const obj : *muonCollection) {
	const IParticle* orig = obj;
	if(!originalInputs) { orig = *acc_originalObject(*obj); }
	std::vector<const xAOD::MissingETAssociation*> assocs = xAOD::MissingETComposition::getAssociations(map,orig);
	for (size_t ind=0; ind<muon_index.size();ind++){
	  if(orig->index()==muon_index.at(ind)) {
	    for(size_t i = 0; i < assocs.size(); i++) helper.setObjSelectionFlag(assocs[i],orig,true);
	  }
	}
      }
      /*ATH_MSG_VERBOSE("Check selected muons after getOverlapRemovedSignals");
      for(const auto& obj : *muonCollection) {
	const IParticle* orig = obj;
	if(!originalInputs) { orig = *acc_originalObject(*obj); }
	ATH_MSG_VERBOSE("Muon with index "<<orig->index() << " is selected?" << MissingETComposition::objSelected(helper,orig));
      }*/
    }

    return ORsignals;
  }


  // Accept Track
  ////////////////
  bool METMaker::acceptTrack(const xAOD::TrackParticle* trk, const xAOD::Vertex* vx) const
  {
    return static_cast<bool>(m_trkseltool->accept( *trk, vx ));
  }

  const xAOD::Vertex* METMaker::getPV() const {

    const xAOD::Vertex *pv = nullptr;

    SG::ReadHandle<xAOD::VertexContainer> h_PV(m_PVkey);

    if (!h_PV.isValid()) {

      ATH_MSG_WARNING("Unable to retrieve primary vertex container PrimaryVertices");

    } else if(h_PV->empty()) {

      ATH_MSG_WARNING("Event has no primary vertices!");

    } else {

      ATH_MSG_DEBUG("Successfully retrieved primary vertex container");

      for(const auto *const vx : *h_PV) {

         if(vx->vertexType()==xAOD::VxType::PriVtx) {

           pv = vx; break;

         }

      }

    }

    return pv;

  }


} //> end namespace met
