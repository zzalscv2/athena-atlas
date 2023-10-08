/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <HIGlobal/HIEventShapeMaker.h>
#include <xAODHIEvent/HIEventShape.h>
#include <xAODHIEvent/HIEventShapeContainer.h>
#include <xAODHIEvent/HIEventShapeAuxContainer.h>

#include <GaudiKernel/ServiceHandle.h>
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

HIEventShapeMaker::HIEventShapeMaker(const std::string& name, ISvcLocator* pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator)
{
}


StatusCode HIEventShapeMaker::initialize()
{
  ATH_MSG_INFO("Inside HIEventShapeMaker::initialize()");

  //First we initialize keys - after initialization they are frozen
  ATH_CHECK(m_towerContainerKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_naviContainerKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_cellContainerKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_outputKey.initialize(!m_summaryOnly));
  ATH_CHECK(m_readExistingKey.initialize(m_summaryOnly));
  ATH_CHECK(m_summaryKey.initialize(!m_summaryKey.key().empty()));

  // assure consistent setup of the input
  ATH_CHECK(m_naviContainerKey.empty() != m_cellContainerKey.empty());

  //Create the HIEventShapeFillerTool
  if (!m_summaryOnly)
  {
    ATH_CHECK(m_HIEventShapeFillerTool.retrieve());
    m_HIEventShapeFillerTool->setContainerName(m_outputKey.key());
    ATH_CHECK(m_HIEventShapeFillerTool->initializeIndex());
  }
  if (m_summaryKey.key().compare("") != 0) ATH_CHECK(m_summaryTool->initialize());

  return StatusCode::SUCCESS;
}

StatusCode HIEventShapeMaker::execute(const EventContext& ctx) const
{
  const xAOD::HIEventShapeContainer* evtShape_const = nullptr;
  if (m_summaryOnly) {
    SG::ReadHandle<xAOD::HIEventShapeContainer>  readHandleEvtShape(m_readExistingKey, ctx);
    if (!readHandleEvtShape.isValid()) {
      ATH_MSG_FATAL("Could not find HI event shape! (" << m_readExistingKey.key() << ").");
      return(StatusCode::FAILURE);
    }
    evtShape_const = readHandleEvtShape.cptr();
  }
  else
  {
    auto evtShape=std::make_unique<xAOD::HIEventShapeContainer>();
    auto evtShapeAux=std::make_unique<xAOD::HIEventShapeAuxContainer>();
    evtShape->setStore(evtShapeAux.get());

    ATH_CHECK(m_HIEventShapeFillerTool->initializeEventShapeContainer(evtShape));

    if (not m_cellContainerKey.empty()) {
      ATH_CHECK(m_HIEventShapeFillerTool->fillCollectionFromCells(evtShape,m_cellContainerKey, ctx));
    } else {
      ATH_CHECK(m_HIEventShapeFillerTool->fillCollectionFromTowers(evtShape,m_towerContainerKey, m_naviContainerKey, ctx));
    }

    SG::WriteHandle<xAOD::HIEventShapeContainer> writeHandleEvtShape(m_outputKey, ctx);
    ATH_CHECK( writeHandleEvtShape.record(std::move(evtShape), std::move(evtShapeAux)) );

    evtShape_const = writeHandleEvtShape.cptr();
  }

  ATH_MSG_DEBUG(PrintHIEventShapeContainer(evtShape_const));

  if (!m_summaryKey.empty())
  {
    SG::WriteHandle<xAOD::HIEventShapeContainer> write_handle_esSummary(m_summaryKey,ctx);
    ATH_CHECK(write_handle_esSummary.record(std::make_unique<xAOD::HIEventShapeContainer>(),
      std::make_unique<xAOD::HIEventShapeAuxContainer>()));
    if (m_summaryOnly) {
      ATH_CHECK(m_summaryTool->summarize(evtShape_const, write_handle_esSummary.ptr()));
    } else {
      ATH_CHECK(m_summaryTool->summarize(evtShape_const, write_handle_esSummary.ptr()));
    }
  }
  return StatusCode::SUCCESS;
}


StatusCode HIEventShapeMaker::finalize()
{
  return StatusCode::SUCCESS;
}


std::string HIEventShapeMaker::PrintHIEventShapeContainer(const xAOD::HIEventShapeContainer* Container) const
{
  std::stringstream buffer;
  buffer << std::endl << "|############|############|############|############|############|" << std::endl;
  buffer << "|" << std::setw(10) << "EtaMin" << "  |"\
    << std::setw(10) << "EtaMax" << "  |"\
    << std::setw(10) << "Layer" << "  |"\
    << std::setw(10) << "NCells" << "  |"\
    << std::setw(10) << "Et" << "  |"<< std::endl;
  unsigned int size = Container->size();
  for (unsigned int i = 0;i < size;i++) {
    const xAOD::HIEventShape* sh = Container->at(i);
    int NCells = sh->nCells();
    int Layer = sh->layer();
    float Et = sh->et();
    float EtaMin = sh->etaMin();
    float EtaMax = sh->etaMax();

    if (Et == 0) continue;
    buffer << "|" << std::setw(10) << EtaMin << "  |"\
      << std::setw(10) << EtaMax << "  |"\
      << std::setw(10) << Layer << "  |"\
      << std::setw(10) << NCells << "  |"\
      << std::setw(10) << Et << "  |" <<std::endl;
  }
  buffer << "|############|############|############|############|############|" << std::endl;

  return buffer.str();
}
