/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "SpacePointCopier.h"

SpacePointCopier::SpacePointCopier(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

SpacePointCopier::~SpacePointCopier()
{
}

StatusCode SpacePointCopier::initialize()
{
  ATH_CHECK( m_pixelSPKey.initialize() );
  ATH_CHECK( m_SCTSPKey.initialize() );
  ATH_CHECK( m_outputKey.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode SpacePointCopier::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode SpacePointCopier::execute(const EventContext& context) const
{
  auto pixelSPContainer = SG::makeHandle(m_pixelSPKey, context);
  auto SCTSPContainer = SG::makeHandle(m_SCTSPKey, context);

  auto output = std::make_unique<xAOD::BaseContainer>();
  auto outputAux = std::make_unique<xAOD::AuxContainerBase>();
  output->setStore(outputAux.get());

  size_t pixSize = 0;
  for ( auto coll: *pixelSPContainer ) {
      pixSize += coll->size();
  }
  size_t SCTSize = 0;
  for ( auto coll: *SCTSPContainer ) {
      SCTSize += coll->size();
  }
  // avoid recording for large events (for them the output collection will be empty)
  if ( pixSize >= m_maxPixSP 
      or SCTSize >= m_maxSCTSP 
      or pixSize + SCTSize >= m_maxTotalSP) {
    ATH_MSG_DEBUG("SP counts thresholds exceeded, no conversion. N PIX " << pixSize << " N SCT " << SCTSize);
  } else {
    static const SG::AuxElement::Accessor< float > x ("x");
    static const SG::AuxElement::Accessor< float > y ("y");
    static const SG::AuxElement::Accessor< float > z ("z");

    for ( auto coll: *pixelSPContainer ) {
      for ( auto sp: *coll ) {
        auto *item = new SG::AuxElement();
        output->push_back( item );
        x(*item) =  float(sp->globalPosition().x());
        y(*item) =  float(sp->globalPosition().y());
        z(*item) =  float(sp->globalPosition().z());
      }
    }


    for ( auto coll: *SCTSPContainer ) {
      for ( auto sp: *coll ) {
        auto *item = new SG::AuxElement();
        output->push_back( item );
        x(*item) =  float(sp->globalPosition().x());
        y(*item) =  float(sp->globalPosition().y());
        z(*item) =  float(sp->globalPosition().z());
      }
    }
  }



  auto outputHandle = SG::makeHandle(m_outputKey, context);
  ATH_CHECK( outputHandle.record(std::move(output), std::move(outputAux)));

  return StatusCode::SUCCESS;
}

