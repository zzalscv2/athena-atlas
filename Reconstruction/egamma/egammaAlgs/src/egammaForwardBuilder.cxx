/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "egammaForwardBuilder.h"
#include "egammaInterfaces/IegammaBaseTool.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloClusterAuxContainer.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/ElectronAuxContainer.h"
#include "xAODEgamma/Electron.h"

#include "EgammaAnalysisInterfaces/IAsgForwardElectronIsEMSelector.h"
#include "PATCore/AcceptData.h"

#include <algorithm>
#include <cmath>

egammaForwardBuilder::egammaForwardBuilder(const std::string& name,
                                           ISvcLocator* pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{}

egammaForwardBuilder::~egammaForwardBuilder() = default;

StatusCode egammaForwardBuilder::initialize()
{
  // the data handle keys
  ATH_CHECK(m_topoClusterKey.initialize());
  ATH_CHECK(m_electronOutputKey.initialize());
  ATH_CHECK(m_outClusterContainerKey.initialize());
  m_outClusterContainerCellLinkKey = m_outClusterContainerKey.key() + "_links";
  ATH_CHECK(m_outClusterContainerCellLinkKey.initialize());

  // retrieve object quality tool
  if (!m_objectQualityTool.empty()) {
    ATH_CHECK(m_objectQualityTool.retrieve());
  } else {
    m_objectQualityTool.disable();
  }

  // retrieve 4-mom builder:
  if (!m_fourMomBuilder.empty()) {
    ATH_CHECK(m_fourMomBuilder.retrieve());
  } else {
    m_fourMomBuilder.disable();
  }

  ATH_CHECK(m_forwardElectronIsEMSelectors.retrieve());

  if (m_forwardElectronIsEMSelectors.size() !=
      m_forwardElectronIsEMSelectorResultNames.size()) {
    ATH_MSG_ERROR("The number of selectors does not match the number of given "
                  "fwd-electron selector names");
    return StatusCode::FAILURE;
  }
  
  // retrieve track match builder
  ATH_CHECK(RetrieveEMTrackMatchBuilder());

  ATH_MSG_DEBUG("Initialization completed successfully");

  return StatusCode::SUCCESS;
}

StatusCode egammaForwardBuilder::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode egammaForwardBuilder::execute(const EventContext& ctx) const
{

  // create an egamma container and register it
  SG::WriteHandle<xAOD::ElectronContainer> xaodFrwd(m_electronOutputKey, ctx);
  ATH_CHECK(xaodFrwd.record(std::make_unique<xAOD::ElectronContainer>(),
                            std::make_unique<xAOD::ElectronAuxContainer>()));

  // Create the relevant cluster output and register it
  SG::WriteHandle<xAOD::CaloClusterContainer> outClusterContainer(
    m_outClusterContainerKey, ctx);
  ATH_CHECK(outClusterContainer.record(
    std::make_unique<xAOD::CaloClusterContainer>(),
    std::make_unique<xAOD::CaloClusterAuxContainer>()));

  SG::WriteHandle<CaloClusterCellLinkContainer> outClusterContainerCellLink(
    m_outClusterContainerCellLinkKey, ctx);
  ATH_CHECK(outClusterContainerCellLink.record(
    std::make_unique<CaloClusterCellLinkContainer>()));

  // Topo cluster Container
  SG::ReadHandle<xAOD::CaloClusterContainer> inputClusters(m_topoClusterKey, ctx);

  // check is only used for serial running; remove when MT scheduler used
  if(!inputClusters.isValid()) {
    ATH_MSG_FATAL("egammaForwardBuilder::Could not retrieve Cluster container");
    return StatusCode::FAILURE;
  }

  EgammaRecContainer egammaRecsFwd;

  // loop over input cluster container and create fwd electrons
  xAOD::CaloClusterContainer::const_iterator clus_begin =
    inputClusters->begin();
  xAOD::CaloClusterContainer::const_iterator clus_end = inputClusters->end();
  static const SG::AuxElement::Accessor<
    std::vector<ElementLink<xAOD::CaloClusterContainer>>>
    caloClusterLinks("constituentClusterLinks");
  
  size_t origClusterIndex = 0;
  for (; clus_begin!=clus_end; ++clus_begin,++origClusterIndex) {
 
    //Preselection cuts
    if((*clus_begin)->et() < m_ETcut||
       std::abs((*clus_begin)->eta())<m_etacut){
      continue;
    }

    auto newCluster = new xAOD::CaloCluster(**clus_begin);
    outClusterContainer->push_back(newCluster);  

    // Create links back to the original clusters
    std::vector<ElementLink<xAOD::CaloClusterContainer>> constituentLinks;
    constituentLinks.emplace_back(*inputClusters, origClusterIndex, ctx);
    caloClusterLinks(*newCluster) = constituentLinks;
   
    int index = outClusterContainer->size() - 1;    
    const ElementLink<xAOD::CaloClusterContainer> clusterLink(
      *outClusterContainer, index, ctx);
    const std::vector<ElementLink<xAOD::CaloClusterContainer>>
      clusterLinkVector{ clusterLink };

    auto egRec = std::make_unique<egammaRec>();
    egRec->setCaloClusters(clusterLinkVector);
    egammaRecsFwd.push_back(std::move(egRec));
  }
  
  // Add track-cluster matching information if requested
  if (m_doTrackMatching) {
    ATH_CHECK(m_trackMatchBuilder->executeRec(ctx, &egammaRecsFwd));
  }

  for (const egammaRec* egRec : egammaRecsFwd) {

    if (!egRec) {
      return StatusCode::FAILURE;
    }

    xAOD::Electron* el = new xAOD::Electron();
    xaodFrwd->push_back(el);
    el->setAuthor(xAOD::EgammaParameters::AuthorFwdElectron);

    std::vector<ElementLink<xAOD::CaloClusterContainer>> clusterLinks;
    for (size_t i = 0; i < egRec->getNumberOfClusters(); ++i) {
      clusterLinks.push_back(egRec->caloClusterElementLink(i));
    }
    el->setCaloClusterLinks(clusterLinks);

    ATH_CHECK(m_fourMomBuilder->execute(ctx, el));
    ATH_CHECK(ExecObjectQualityTool(ctx, el));
    // Apply the Forward Electron selectors
    size_t size = m_forwardElectronIsEMSelectors.size();

    for (size_t i = 0; i < size; ++i) {
      asg::AcceptData accept =
        m_forwardElectronIsEMSelectors[i]->accept(ctx, el);
      // save the bool result
      el->setPassSelection(static_cast<bool>(accept),
                           m_forwardElectronIsEMSelectorResultNames[i]);
      // save the isem
      el->setSelectionisEM(accept.getCutResultInverted(),
                           "isEM" +
                             m_forwardElectronIsEMSelectorResultNames[i]);
    }
    if (egRec->getNumberOfTrackParticles() == 0 and m_doTrackMatching) {
      continue;
    }
    std::vector<ElementLink<xAOD::TrackParticleContainer>> trackLinks;
    for (size_t i = 0; i < egRec->getNumberOfTrackParticles(); ++i) {
      trackLinks.push_back(egRec->trackParticleElementLink(i));
    }
    el->setTrackParticleLinks(trackLinks);

    const xAOD::TrackParticle* trackParticle = el->trackParticle();
    if (trackParticle) {
      el->setCharge(trackParticle->charge());
    }
    if (m_doTrackMatching) {
      // Set DeltaEta, DeltaPhi , DeltaPhiRescaled
      std::array<double, 4> deltaEta = egRec->deltaEta();
      std::array<double, 4> deltaPhi = egRec->deltaPhi();
      std::array<double, 4> deltaPhiRescaled = egRec->deltaPhiRescaled();

      el->setTrackCaloMatchValue(static_cast<float>(deltaEta[0]),
                                      xAOD::EgammaParameters::deltaEta0);
      el->setTrackCaloMatchValue(static_cast<float>(deltaPhi[0]),
                                      xAOD::EgammaParameters::deltaPhi0);
      el->setTrackCaloMatchValue(static_cast<float>(deltaPhiRescaled[0]),
                                      xAOD::EgammaParameters::deltaPhiRescaled0);

      el->setTrackCaloMatchValue(static_cast<float>(deltaEta[1]),
                                      xAOD::EgammaParameters::deltaEta1);
      el->setTrackCaloMatchValue(static_cast<float>(deltaPhi[1]),
                                      xAOD::EgammaParameters::deltaPhi1);
      el->setTrackCaloMatchValue(static_cast<float>(deltaPhiRescaled[1]),
                                      xAOD::EgammaParameters::deltaPhiRescaled1);

      static const SG::AuxElement::Accessor<float> pear("deltaEta1PearDistortion");

      el->setTrackCaloMatchValue(static_cast<float>(deltaEta[2]),
                                      xAOD::EgammaParameters::deltaEta2);
      el->setTrackCaloMatchValue(static_cast<float>(deltaPhi[2]),
                                      xAOD::EgammaParameters::deltaPhi2);
      el->setTrackCaloMatchValue(static_cast<float>(deltaPhiRescaled[2]),
                                      xAOD::EgammaParameters::deltaPhiRescaled2);

      el->setTrackCaloMatchValue(static_cast<float>(deltaEta[3]),
                                      xAOD::EgammaParameters::deltaEta3);
      el->setTrackCaloMatchValue(static_cast<float>(deltaPhi[3]),
                                      xAOD::EgammaParameters::deltaPhi3);
      el->setTrackCaloMatchValue(static_cast<float>(deltaPhiRescaled[3]),
                                      xAOD::EgammaParameters::deltaPhiRescaled3);

      float deltaPhiLast = static_cast<float>(egRec->deltaPhiLast());
      el->setTrackCaloMatchValue(
        deltaPhiLast, xAOD::EgammaParameters::deltaPhiFromLastMeasurement);
      }
  }

  // Now finalize the cluster: based on code in CaloClusterStoreHelper::finalizeClusters
  for (xAOD::CaloCluster* cl : *outClusterContainer) {
    cl->setLink(outClusterContainerCellLink.ptr(), ctx);
  }

  return StatusCode::SUCCESS;
}

StatusCode
egammaForwardBuilder::ExecObjectQualityTool(
  const EventContext& ctx,
  xAOD::Egamma* eg) const
{
  //
  // execution of the object quality tools
  //
  // protection in case tool is not available
  // return success as algorithm can run without it
  if (!m_objectQualityTool.isEnabled()) return StatusCode::SUCCESS;
  // execute the tool
  return m_objectQualityTool->execute(ctx,*eg);
}

StatusCode
egammaForwardBuilder::RetrieveEMTrackMatchBuilder()
{
  if (!m_doTrackMatching) {
    m_trackMatchBuilder.disable();
    return StatusCode::SUCCESS;
  }

  if (m_trackMatchBuilder.empty()) {
    ATH_MSG_ERROR(
      "EMTrackMatchBuilder is empty, but track matching is enabled");
    return StatusCode::FAILURE;
  }

  if (m_trackMatchBuilder.retrieve().isFailure()) {
    ATH_MSG_ERROR("Unable to retrieve " << m_trackMatchBuilder);
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}
