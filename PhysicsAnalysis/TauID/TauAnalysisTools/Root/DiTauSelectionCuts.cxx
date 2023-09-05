/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


// local include(s)
#include "TauAnalysisTools/DiTauSelectionCuts.h"
#include "TauAnalysisTools/DiTauSelectionTool.h"

// framework include(s)
#include "AsgDataHandles/ReadHandle.h"

using namespace TauAnalysisTools;

//______________________________________________________________________________
DiTauSelectionCut::DiTauSelectionCut(const std::string& sName, TauAnalysisTools::DiTauSelectionTool* tDTST)
  : m_sName(sName)
  , m_hHistCutPre(nullptr)
  , m_hHistCut(nullptr)
  , m_tDTST(tDTST)
{
}

//______________________________________________________________________________
DiTauSelectionCut::~DiTauSelectionCut()
{
  // FIXME: could use unique_ptr
  delete m_hHistCutPre;
  delete m_hHistCut;
}

//______________________________________________________________________________
void DiTauSelectionCut::writeControlHistograms()
{
  m_hHistCutPre->Write();
  m_hHistCut->Write();
}

//______________________________________________________________________________
TH1F* DiTauSelectionCut::CreateControlPlot(const char* sName, const char* sTitle, int iBins, double dXLow, double dXUp)
{
  if (m_tDTST->m_bCreateControlPlots)
  {
    TH1F* hHist = new TH1F(sName, sTitle, iBins, dXLow, dXUp);
    hHist->SetDirectory(0);
    return hHist;
  }

  return nullptr;
}

//______________________________________________________________________________
void DiTauSelectionCut::fillHistogramCutPre(const xAOD::DiTauJet& xDiTau)
{
  fillHistogram(xDiTau, *m_hHistCutPre);
}

//______________________________________________________________________________
void DiTauSelectionCut::fillHistogramCut(const xAOD::DiTauJet& xDiTau)
{
  fillHistogram(xDiTau, *m_hHistCut);
}


//______________________________________________________________________________
void DiTauSelectionCut::setProperty(const std::string& name, const std::string& value)
{
  std::map<std::string, std::string&>::iterator it = m_mProperties.find(name);
  if(it == m_mProperties.end() )
    throw std::runtime_error (("Undeclared property: " + name + "\n").c_str());
  it->second = value;
}

//______________________________________________________________________________
void DiTauSelectionCut::declareProperty(const std::string& name, std::string& loc)
{
  std::pair<std::string, std::string&> p(name, loc);
  m_mProperties.insert(p);
}

//______________________________________________________________________________
std::string DiTauSelectionCut::getProperty(const std::string& name)
{
  std::map<std::string, std::string&>::iterator it = m_mProperties.find(name);
  if(it == m_mProperties.end() )
    throw std::runtime_error (("Undeclared property: " + name + "\n").c_str());

  return it->second;
}


//_______________________________SelectionCutPt_________________________________
//______________________________________________________________________________
DiTauSelectionCutPt::DiTauSelectionCutPt(DiTauSelectionTool* tDTST)
  : DiTauSelectionCut("CutPt", tDTST)
{
  m_hHistCutPre = CreateControlPlot("hPt_pre","Pt_pre;di-#tau-p_{T} [GeV]; events",1000,0,1000);
  m_hHistCut = CreateControlPlot("hPt_cut","Pt_cut;di-#tau-p_{T} [GeV]; events",1000,0,1000);
}

//______________________________________________________________________________
void DiTauSelectionCutPt::fillHistogram(const xAOD::DiTauJet& xDiTau, TH1F& hHist) const
{
  hHist.Fill(xDiTau.pt()/1000.);
}

//______________________________________________________________________________
void DiTauSelectionCutPt::setAcceptInfo(asg::AcceptInfo& info) const
{
  info.addCut( "Pt",
               "Selection of ditaus according to their transverse momentum" );
}
//______________________________________________________________________________
bool DiTauSelectionCutPt::accept(const xAOD::DiTauJet& xDiTau,
                            asg::AcceptData& acceptData)
{
  // save ditau pt in GeV
  double pt = xDiTau.pt() / 1000.;
  // in case of only one entry in vector, run for lower limits
  if (m_tDTST->m_vPtRegion.size() == 1)
  {
    if ( pt >= m_tDTST->m_vPtRegion.at(0) )
    {
      acceptData.setCutResult( "Pt", true );
      return true;
    }
  }
  unsigned int iNumPtRegion = m_tDTST->m_vPtRegion.size()/2;
  for( unsigned int iPtRegion = 0; iPtRegion < iNumPtRegion; iPtRegion++ )
  {
    if ( pt >= m_tDTST->m_vPtRegion.at(iPtRegion*2) and pt <= m_tDTST->m_vPtRegion.at(iPtRegion*2+1))
    {
      acceptData.setCutResult( "Pt", true );
      return true;
    }
  }
  m_tDTST->msg() << MSG::VERBOSE << "DiTau failed pt requirement, ditau pt [GeV]: " << pt << endmsg;
  return false;
}

//_____________________________SelectionCutAbsEta_______________________________
//______________________________________________________________________________
DiTauSelectionCutAbsEta::DiTauSelectionCutAbsEta(DiTauSelectionTool* tDTST)
  : DiTauSelectionCut("CutAbsEta", tDTST)
{
  m_hHistCutPre = CreateControlPlot("hEta_pre","Eta_pre;di-#tau-#eta; events",100,-3,3);
  m_hHistCut = CreateControlPlot("hEta_cut","Eta_cut;di-#tau-#eta; events",100,-3,3);
}

//______________________________________________________________________________
void DiTauSelectionCutAbsEta::fillHistogram(const xAOD::DiTauJet& xDiTau, TH1F& hHist) const
{
  hHist.Fill(xDiTau.eta());
}

//______________________________________________________________________________
void DiTauSelectionCutAbsEta::setAcceptInfo(asg::AcceptInfo& info) const
{
  info.addCut( "AbsEta",
               "Selection of ditaus according to their absolute pseudorapidity" );
}
//______________________________________________________________________________
bool DiTauSelectionCutAbsEta::accept(const xAOD::DiTauJet& xDiTau,
                                asg::AcceptData& acceptData)
{
  // check regions of eta, if ditau is in one region then return true; false otherwise
  unsigned int iNumEtaRegion = m_tDTST->m_vAbsEtaRegion.size()/2;
  for( unsigned int iEtaRegion = 0; iEtaRegion < iNumEtaRegion; iEtaRegion++ )
  {
    if ( std::abs( xDiTau.eta() ) >= m_tDTST->m_vAbsEtaRegion.at(iEtaRegion*2) and std::abs( xDiTau.eta() ) <= m_tDTST->m_vAbsEtaRegion.at(iEtaRegion*2+1))
    {
      acceptData.setCutResult( "AbsEta", true );
      return true;
    }
  }
  m_tDTST->msg() << MSG::VERBOSE << "DiTau failed eta requirement, ditau eta: " << xDiTau.eta() << endmsg;
  return false;
}

