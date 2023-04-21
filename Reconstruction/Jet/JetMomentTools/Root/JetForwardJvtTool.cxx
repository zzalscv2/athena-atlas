///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// JetForwardJvtTool.cxx
// Implementation file for class JetForwardJvtTool
// Author: Matt Klein<matthew.henry.klein@cern.ch>
///////////////////////////////////////////////////////////////////

// JetForwardJvtTool includes
#include "JetMomentTools/JetForwardJvtTool.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "AsgDataHandles/WriteDecorHandle.h"

#include <TString.h>

// Jet EDM

  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////

  // Constructors
  ////////////////
  JetForwardJvtTool::JetForwardJvtTool(const std::string& name) :
    AsgTool(name) {
  }

  // Destructor
  ///////////////
  JetForwardJvtTool::~JetForwardJvtTool()
  {}

  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode JetForwardJvtTool::initialize()
  {
    ATH_MSG_INFO ("Initializing " << name() << "...");
    if (m_tightOP) m_fjvtThresh = 0.4;
    else m_fjvtThresh = 0.5;

    ATH_CHECK(m_vertexContainerName.initialize());
    ATH_CHECK(m_trkMETName.initialize());

    if(m_jetContainerName.empty()) {
      if (m_renounceOutputs) {
        m_jetContainerName = "_dummy";
      }
      else {
        ATH_MSG_ERROR("JetForwardJvtTool needs to have its input jet container configured!");
        return StatusCode::FAILURE;
      }
    }
    if(!m_orKey.key().empty()) m_orKey = m_jetContainerName + "." + m_orKey.key();
    m_outKey = m_jetContainerName + "." + m_outKey.key();
    m_isHSKey = m_jetContainerName + "." + m_isHSKey.key();
    m_isPUKey = m_jetContainerName + "." + m_isPUKey.key();
    m_fjvtDecKey = m_jetContainerName + "." + m_fjvtDecKey.key();
    m_widthKey = m_jetContainerName + "." + m_widthKey.key();
    m_jvtMomentKey = m_jetContainerName + "." + m_jvtMomentKey.key();
    m_sumPtsKey = m_jetContainerName + "." + m_sumPtsKey.key();

    ATH_CHECK(m_orKey.initialize(!m_orKey.key().empty()));
    ATH_CHECK(m_outKey.initialize());
    ATH_CHECK(m_isHSKey.initialize());
    ATH_CHECK(m_isPUKey.initialize());
    ATH_CHECK(m_fjvtDecKey.initialize());
    ATH_CHECK(m_widthKey.initialize());
    ATH_CHECK(m_jvtMomentKey.initialize());
    ATH_CHECK(m_sumPtsKey.initialize());

#ifndef XAOD_STANDALONE
    if (m_renounceOutputs) {
      renounce (m_orKey);
      renounce (m_outKey);
      renounce (m_isHSKey);
      renounce (m_isPUKey);
      renounce (m_fjvtDecKey);
      renounce (m_widthKey);
      renounce (m_jvtMomentKey);
      renounce (m_sumPtsKey);
    }
#endif

    return StatusCode::SUCCESS;
  }

  StatusCode JetForwardJvtTool::decorate(const xAOD::JetContainer& jetCont) const {
    SG::WriteDecorHandle<xAOD::JetContainer, char> outHandle(m_outKey);
    SG::WriteDecorHandle<xAOD::JetContainer, float> fjvtDecHandle(m_fjvtDecKey);

    std::vector<TVector2> pileupMomenta;
    const std::size_t pvind = getPV();
    if (m_recalculateFjvt && jetCont.size() > 0) {
      pileupMomenta = calculateVertexMomenta(&jetCont, pvind);
    }
    for(const auto jetF : jetCont) {
      outHandle(*jetF) = 1;
      if(m_recalculateFjvt) fjvtDecHandle(*jetF) = 0;
      if (!forwardJet(jetF)) continue;
      double fjvt = getFJVT(jetF, pileupMomenta, pvind)/jetF->pt();
      if (fjvt>m_fjvtThresh) outHandle(*jetF) = 0;
      fjvtDecHandle(*jetF) = fjvt;
    }
    return StatusCode::SUCCESS;
  }

  float JetForwardJvtTool::getFJVT(const xAOD::Jet *jet, const std::vector<TVector2>& pileupMomenta, std::size_t pvind) const {
    if(!m_recalculateFjvt){
      SG::WriteDecorHandle<xAOD::JetContainer, float> fjvtDecHandle(m_fjvtDecKey);
      return fjvtDecHandle(*jet);
    } 

    TVector2 fjet(-jet->pt()*cos(jet->phi()),-jet->pt()*sin(jet->phi()));
    double fjvt = 0;
    for (size_t pui = 0; pui < pileupMomenta.size(); pui++) {
      if (pui==pvind) continue;
      double projection = pileupMomenta[pui]*fjet/fjet.Mod();
      if (projection>fjvt) fjvt = projection;
    }
    //fjvt += getCombinedWidth(jet);
    return fjvt;
  }

  std::vector<TVector2> JetForwardJvtTool::calculateVertexMomenta(const xAOD::JetContainer *jets, std::size_t pvind) const {

    SG::ReadHandle<xAOD::MissingETContainer> trkMetHandle(m_trkMETName);
    SG::ReadHandle<xAOD::VertexContainer> vertexContainerHandle(m_vertexContainerName);
    if( !trkMetHandle.isValid() ) {
      ATH_MSG_WARNING("xAOD::MissingETContainer " << m_trkMETName.key() << " is invalid");
      return {};
    }
    if( !vertexContainerHandle.isValid() ) {
      ATH_MSG_WARNING("xAOD::VertexContainer " << m_vertexContainerName.key() << " is invalid");
      return {};
    }

    std::vector<TVector2> pileupMomenta;
    for(const auto vx : *vertexContainerHandle) {
      if(vx->vertexType()!=xAOD::VxType::PriVtx && vx->vertexType()!=xAOD::VxType::PileUp) continue;
      TString vname = "PVTrack_vx";
      vname += vx->index();
      pileupMomenta.push_back((vx->index()==pvind ? 0 : -(1./m_jetScaleFactor)) *
                              TVector2(0.5*(*trkMetHandle)[vname.Data()]->mpx(),
                                       0.5*(*trkMetHandle)[vname.Data()]->mpy()));
    }

    for (const auto jet : *jets) {
      if (!centralJet(jet)) continue;
      int jetvert = getJetVertex(jet);
      if (jetvert>=0) pileupMomenta[jetvert] += TVector2(0.5*jet->pt()*cos(jet->phi()),0.5*jet->pt()*sin(jet->phi()));
    }
    return pileupMomenta;
  }

  float JetForwardJvtTool::getCombinedWidth(const xAOD::Jet *jet) const {
    float Width = 0;
    float CWidth = 0;
    float ptsum = 0;
    SG::ReadDecorHandle<xAOD::JetContainer, float> widthHandle(m_widthKey);
    Width = widthHandle(*jet);
    xAOD::JetConstituentVector constvec = jet->getConstituents();
    for (xAOD::JetConstituentVector::iterator it = constvec.begin(); it != constvec.end(); ++it) {
      const xAOD::CaloCluster *cl = static_cast<const xAOD::CaloCluster*>((*it)->rawConstituent());
      float secondR = cl->getMomentValue(xAOD::CaloCluster::MomentType::SECOND_R);
      float centermag = cl->getMomentValue(xAOD::CaloCluster::MomentType::CENTER_MAG);
      CWidth+=fabs(cl->pt()*atan(sqrt(secondR)/centermag)*cosh(cl->eta()));
      ptsum += cl->pt();
    }
    CWidth /= ptsum;
    return (CWidth + Width);
  }

  bool JetForwardJvtTool::forwardJet(const xAOD::Jet *jet) const {
    if (fabs(jet->eta())<m_etaThresh) return false;
    if (jet->pt()<m_forwardMinPt || jet->pt()>m_forwardMaxPt) return false;
    return true;
  }

  bool JetForwardJvtTool::centralJet(const xAOD::Jet *jet) const {
    if (fabs(jet->eta())>m_etaThresh) return false;
    if (jet->pt()<m_centerMinPt || (m_centerMaxPt>0 && jet->pt()>m_centerMaxPt)) return false;
    if(!m_orKey.key().empty()){
      SG::ReadDecorHandle<xAOD::JetContainer, char> orHandle(m_orKey);
      if(!orHandle(*jet)) return false;
    }
    float jvt = 0;
    SG::ReadDecorHandle<xAOD::JetContainer, float> jvtMomentHandle(m_jvtMomentKey);
    jvt = jvtMomentHandle(*jet);
    if (jvt>m_centerJvtThresh) return false;
    if (jet->pt()<m_maxStochPt && getDrpt(jet)<m_centerDrptThresh) return false;
    return true;
  }

  int JetForwardJvtTool::getJetVertex(const xAOD::Jet *jet) const {
    std::vector<float> sumpts;
    SG::ReadDecorHandle<xAOD::JetContainer, std::vector<float> > sumPtsHandle(m_sumPtsKey);
    sumpts = sumPtsHandle(*jet);
    double firstVal = 0;
    int bestMatch = -1;
    for (size_t i = 0; i < sumpts.size(); i++) {
      if (sumpts[i]>firstVal) {
        bestMatch = i;
        firstVal = sumpts[i];
      }
    }
    return bestMatch;
  }

  float JetForwardJvtTool::getDrpt(const xAOD::Jet *jet) const {
    std::vector<float> sumpts;
    SG::ReadDecorHandle<xAOD::JetContainer, std::vector<float> > sumPtsHandle(m_sumPtsKey);
    sumpts = sumPtsHandle(*jet);
    if (sumpts.size()<2) return 0;

    std::nth_element(sumpts.begin(),sumpts.begin()+sumpts.size()/2,sumpts.end(),std::greater<int>());
    double median = sumpts[sumpts.size()/2];
    std::nth_element(sumpts.begin(),sumpts.begin(),sumpts.end(),std::greater<int>());
    double max = sumpts[0];
    return (max-median)/jet->pt();
  }

  std::size_t JetForwardJvtTool::getPV() const {

    std::size_t pvind = 0;
    auto vertexContainer = SG::makeHandle (m_vertexContainerName);
    if (!vertexContainer.isValid()){
      ATH_MSG_WARNING("Invalid xAOD::VertexContainer datahandle");
      return pvind;
    }
    auto vxCont = vertexContainer.cptr();

    if(vxCont->empty()) {
      ATH_MSG_WARNING("Event has no primary vertices!");
    } else {
      ATH_MSG_DEBUG("Successfully retrieved primary vertex container");
      for(const auto vx : *vxCont) {
        if(vx->vertexType()==xAOD::VxType::PriVtx)
          {pvind = vx->index(); break;}
      }
    }
    return pvind;
  }

  StatusCode JetForwardJvtTool::tagTruth(const xAOD::JetContainer *jets,const xAOD::JetContainer *truthJets) {
    SG::WriteDecorHandle<xAOD::JetContainer, bool> isHSHandle(m_isHSKey);
    SG::WriteDecorHandle<xAOD::JetContainer, bool> isPUHandle(m_isPUKey);
    for(const auto jet : *jets) {
      bool ishs = false;
      bool ispu = true;
      for(const auto tjet : *truthJets) {
        if (tjet->p4().DeltaR(jet->p4())<0.3 && tjet->pt()>10e3) ishs = true;
        if (tjet->p4().DeltaR(jet->p4())<0.6) ispu = false;
      }
      isHSHandle(*jet)=ishs;
      isPUHandle(*jet)=ispu;
    }
    return StatusCode::SUCCESS;
  }

