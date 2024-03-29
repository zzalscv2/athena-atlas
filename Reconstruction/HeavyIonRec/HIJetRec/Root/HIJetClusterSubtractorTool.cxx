/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "HIJetClusterSubtractorTool.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODEventInfo/EventInfo.h"
#include "HIEventUtils/HIEventShapeIndex.h"
#include "HIJetRec/HIJetRecDefs.h"
#include "HIJetRec/IHIUEModulatorTool.h"
#include "PathResolver/PathResolver.h"
#include <TH3F.h>
#include <TFile.h>
#include <iomanip>

HIJetClusterSubtractorTool::HIJetClusterSubtractorTool(const std::string& myname) : HIJetSubtractorToolBase(myname),
										    m_init(false),
										    m_h3W(nullptr),
										    m_h3Eta(nullptr),
										    m_h3Phi(nullptr),
										    m_useSamplings(true)
{
	declareProperty("UseSamplings",m_useSamplings=true);
  setUseCells(false);
}

void HIJetClusterSubtractorTool::subtract(xAOD::IParticle::FourMom_t& subtr_mom, const xAOD::IParticle* cl_in, const xAOD::HIEventShapeContainer* shape, const HIEventShapeIndex* index, const ToolHandle<IHIUEModulatorTool>& modulator, const xAOD::HIEventShape* eshape) const
{

  const xAOD::CaloCluster* cl=static_cast<const xAOD::CaloCluster*>(cl_in);
  float E_unsubtr=cl->e(HIJetRec::unsubtractedClusterState());
  float eta_unsubtr=cl->eta(HIJetRec::unsubtractedClusterState());
  float phi_unsubtr=cl->phi(HIJetRec::unsubtractedClusterState());

  float E_subtr=0;
  float E_eta_subtr=0;
  float E_phi_subtr=0;
  const float eta0=cl->eta0();
  const float phi0=cl->phi0();
  float mod=1;
  if(modulator) mod=modulator->getModulation(phi0, eshape);

  float energy=E_unsubtr;
  float eta=eta0;
  float phi=phi0;

  if(cl->isAvailable<float>("HIEtaPhiWeight"))
  {
    float DF_weight=cl->auxdataConst<float>("HIEtaPhiWeight");
    if(DF_weight!=0.) mod/=DF_weight;
  }

  if(shape==nullptr)
  {
    ATH_MSG_ERROR("No HIEventShape supplied, cannot do subtraction");
    return;
  }

  if(m_useSamplings)
  {
    if(index==nullptr)
    {
      ATH_MSG_ERROR("No HIEventShapeIndex supplied, cannot do subtraction");
      return;
    }

    for(unsigned int sample=0; sample<24; sample++)
    {
      CaloSampling::CaloSample s=static_cast<CaloSampling::CaloSample>(sample);
      if(!cl->hasSampling(s)) continue;

      float nCells=index->getShape(eta0,sample,shape)->nCells();
      float rho=0;
      if(nCells!=0.) rho=index->getShape(eta0,sample,shape)->rho()/nCells;
      rho*=mod*std::cosh(eta0);

      E_subtr+=rho*getWeight(eta0,phi0,sample);
      E_eta_subtr+=rho*getWeightEta(eta0,phi0,sample);
      E_phi_subtr+=rho*getWeightPhi(eta0,phi0,sample);
    }

    energy=E_unsubtr-E_subtr;
    if(energy!=0.)
    {
      eta=eta_unsubtr+(eta_unsubtr*E_subtr-E_eta_subtr)/energy;
      phi=phi_unsubtr+(phi_unsubtr*E_subtr-E_phi_subtr)/energy;
    }
    setSubtractedEtaPhi(energy,eta,phi,eta0,phi0,energy/E_subtr);
  }
  else
  {
    unsigned int es_bin=HI::TowerBins::findBinEta(eta0);

    float area=shape->at(es_bin)->area();
    float rho=(area==0.) ? 0. : shape->at(es_bin)->et()/area;

    rho*=mod*std::cosh(eta0);
    E_subtr=rho*HI::TowerBins::getBinArea();
    energy=E_unsubtr-E_subtr;
    eta=eta_unsubtr;
    phi=phi_unsubtr;

    ATH_MSG_VERBOSE("Subtracting"
		    << std::setw(8) << eta0
		    << std::setw(8) << phi0
		    << std::setw(10) << std::setprecision(3) << modulator->getModulation(phi0, eshape)
		    << std::setw(10) << std::setprecision(3) << mod
		    << std::setw(10) << std::setprecision(3) << modulator->getModulation(phi0, eshape)/mod
		    << std::setw(10) << std::setprecision(3) << area
		    << std::setw(10) << std::setprecision(3) << rho*area
		    << std::setw(10) << std::setprecision(3) << E_unsubtr*1e-3
		    << std::setw(10) << std::setprecision(3) << E_subtr*1e-3
		    << std::setw(10) << std::setprecision(3) << E_unsubtr*1e-3/std::cosh(eta0)
		    << std::setw(10) << std::setprecision(3) << E_subtr*1e-3/std::cosh(eta0));




  }
  float ET=energy/std::cosh(eta);
  subtr_mom.SetPxPyPzE(ET*std::cos(phi),ET*std::sin(phi),ET*std::sinh(eta),energy);
}

void HIJetClusterSubtractorTool::updateUsingCluster(xAOD::HIEventShapeContainer* shape, const HIEventShapeIndex* index, const xAOD::CaloCluster* cl) const
{

  float eta0=cl->eta0();
  float phi0=cl->phi0();

  if(m_useSamplings)
  {
    for(unsigned int sample=0; sample<24; sample++)
    {
      CaloSampling::CaloSample s=static_cast<CaloSampling::CaloSample>(sample);
      if(!cl->hasSampling(s)) continue;
      float esamp=cl->eSample(s);
      float ET=esamp/std::cosh(eta0);
      xAOD::HIEventShape* slice=shape->at(index->getIndex(eta0,sample));
      float area_cluster=getWeight(eta0,phi0,sample);
      //update members
      updateSlice(slice,ET,phi0,area_cluster);
    }
  }
  else
  {
    unsigned int es_bin=HI::TowerBins::findBinEta(eta0);
    xAOD::HIEventShape* slice=shape->at(es_bin);
    constexpr float area_cluster=HI::TowerBins::getBinArea();
    float ET=cl->e(HIJetRec::unsubtractedClusterState())/std::cosh(eta0);
    if(cl->isAvailable<float>("HIEtaPhiWeight"))
    {
      float HI_weight=cl->auxdataConst<float>("HIEtaPhiWeight");
      ET*=HI_weight;
    }
    //update members
    updateSlice(slice,ET,phi0,area_cluster);
  }
}


void HIJetClusterSubtractorTool::subtractWithMoments(xAOD::CaloCluster* cl, const xAOD::HIEventShapeContainer* shape, const HIEventShapeIndex* index, const ToolHandle<IHIUEModulatorTool>& modulator, const xAOD::HIEventShape* eshape) const
{
  xAOD::IParticle::FourMom_t subtr_mom;
  subtract(subtr_mom, cl, shape, index, modulator, eshape);
  HIJetRec::setClusterP4(subtr_mom,cl,HIJetRec::subtractedClusterState());
}

StatusCode HIJetClusterSubtractorTool::initializeTool()
{
  if(m_useSamplings)
  {
    std::string local_path=static_cast<std::string>(m_configDir)+m_inputFile;
    std::string full_path=PathResolverFindCalibFile(local_path);
    ATH_MSG_INFO("Reading input file "<< m_inputFile << " from " << full_path);
    TFile* f=TFile::Open(full_path.c_str());
    if(f==nullptr)
    {
      ATH_MSG_FATAL("Cannot read config file " << full_path );
      return StatusCode::FAILURE;
    }

    m_h3W=(TH3F*)f->GetObjectChecked("h3_w","TH3F");
    if(m_h3W==nullptr)
    {
      ATH_MSG_FATAL("Cannot find TH3F m_h3W in config file " << full_path );
      return StatusCode::FAILURE;
    }

    m_h3Eta=(TH3F*)f->GetObjectChecked("h3_eta","TH3F");
    if(m_h3Eta==nullptr)
    {
      ATH_MSG_FATAL("Cannot find TH3F m_h3Eta in config file " << full_path );
      return StatusCode::FAILURE;
    }

    m_h3Phi=(TH3F*)f->GetObjectChecked("h3_phi","TH3F");
    if(m_h3Phi==nullptr)
    {
      ATH_MSG_FATAL("Cannot find TH3F m_h3Phi in config file " << full_path );
      return StatusCode::FAILURE;
    }
    m_h3W->SetDirectory(0);
    m_h3Eta->SetDirectory(0);
    m_h3Phi->SetDirectory(0);

    f->Close();
  }
  m_init=true;
  return StatusCode::SUCCESS;

}


StatusCode HIJetClusterSubtractorTool::initialize()
{
  return initializeTool();

}


float HIJetClusterSubtractorTool::getWeight(float eta, float phi, int sample) const
{
  return m_h3W->GetBinContent(m_h3W->FindFixBin(eta,phi,sample));
}
float HIJetClusterSubtractorTool::getWeightEta(float eta, float phi, int sample) const
{
  return m_h3Eta->GetBinContent(m_h3Eta->FindFixBin(eta,phi,sample));
}
float HIJetClusterSubtractorTool::getWeightPhi(float eta, float phi, int sample) const
{
  return m_h3Phi->GetBinContent(m_h3Phi->FindFixBin(eta,phi,sample));
}

void HIJetClusterSubtractorTool::updateSlice(xAOD::HIEventShape* slice, float ET, float phi0, float area_cluster) const
{
  if(area_cluster==0.)
  {
    area_cluster=1.;

    if(ET==0.)
    {
      ATH_MSG_INFO("Provided cluster area was 0, check if the input file " << m_inputFile << " from " << PathResolverFindCalibFile(static_cast<std::string>(m_configDir)+m_inputFile) << " was generated with the currently used geometry. See ATLHI-472");
    }
    else
    {
      ATH_MSG_ERROR("Provided cluster area was 0, the HIEventShape will be incorrect! Check if the input file " << m_inputFile << " from " << PathResolverFindCalibFile(static_cast<std::string>(m_configDir)+m_inputFile) << " was generated with the currently used geometry. See ATLHI-472");
    }
  }
  
  float area_slice=slice->area();
  float area_sf=1.;

  if(area_slice!=0.) area_sf-=area_cluster/area_slice;
  slice->setNCells( std::floor(area_sf*slice->nCells()) );
  slice->setEt(slice->et()-ET);
  slice->setArea(area_slice-area_cluster);

  slice->setRho(slice->rho() - ET/area_cluster);


  for(unsigned int ih=0; ih<slice->etCos().size(); ih++)
  {
    float ih_f=ih+1;
    float tmp_cos = slice->etCos().at(ih);
    slice->etCos()[ih] = tmp_cos + cos(ih_f*phi0)*ET;

    float tmp_sin = slice->etSin().at(ih);
    slice->etSin()[ih] = tmp_sin + sin(ih_f*phi0)*ET;
  }
}
