/*
   Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "CaloSuperCellAlignCondAlg.h"

#include "CaloDetDescrUtils/CaloDetDescrBuilder.h"
#include "AthenaKernel/getMessageSvc.h"

#include <memory>

// ------------
#include "CaloIdentifier/CaloCell_Base_ID.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloDetDescr/CaloDetDescriptor.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "CaloDetDescr/CaloDetectorElements.h"

namespace {

  inline
  double phi_for_descr (double phi)
  {
    if (phi < 0)
      phi += 2*M_PI;
    return phi;
  }
  
  int descr_index (const CaloDetDescriptor* desc,
		   const CaloDetDescrManager_Base* mgr)
  {
    if (desc->is_tile()) {
      int i= 0;
      for (const CaloDetDescriptor* d : mgr->tile_descriptors_range()) {
	if (d == desc) return mgr->calo_descriptors_size() + i;
	++i;
      }
      return -1;
   }
    else
      return desc->calo_hash();
  }
  
  const  CaloDetDescriptor* get_cell_descriptor (Identifier cell_id,
						 const CaloDetDescrManager_Base* mgr)
  {
    const CaloCell_Base_ID* calo_helper = mgr->getCaloCell_ID();
    Identifier reg_id = calo_helper->region_id (cell_id);
    if (!calo_helper->is_tile (cell_id)) {
      assert (reg_id.is_valid());
      return mgr->get_descriptor (reg_id);
   }
    
    int clayer = calo_helper->sample (cell_id);
    bool is_sc = calo_helper->is_supercell (cell_id);
    for (const CaloDetDescriptor* d : mgr->tile_descriptors_range()) {
      if (d->identify() != reg_id) continue;
     int dlayer = d->layer();
     if (clayer == dlayer) return d;
     if (is_sc && dlayer == 0 && clayer != 2) return d;
    }
    return nullptr;
  }
  
  const CaloDetDescriptor* get_reg_descriptor (Identifier reg_id,
					       const CaloDetDescrManager_Base* mgr)
  {
    const CaloCell_Base_ID* calo_helper = mgr->getCaloCell_ID();
    if (!calo_helper->is_tile (reg_id)) {
      assert (reg_id.is_valid());
      return mgr->get_descriptor (reg_id);
    }
    
    for (const CaloDetDescriptor* d : mgr->tile_descriptors_range()) {
      if (d->identify() == reg_id) return d;
    }
    return nullptr;
  }
  
} // anonymous namespace

StatusCode CaloSuperCellAlignCondAlg::initialize()
{
  ATH_MSG_DEBUG("initialize " << name());
  
  ATH_CHECK(m_condSvc.retrieve());
  ATH_CHECK(m_scidTool.retrieve());
  ATH_CHECK(m_readCaloMgrKey.initialize());
  ATH_CHECK(m_writeCaloSuperCellMgrKey.initialize());
  // Register Write Cond Handle
  if(m_condSvc->regHandle(this, m_writeCaloSuperCellMgrKey).isFailure()) {
    ATH_MSG_ERROR("unable to register WriteCondHandle " << m_writeCaloSuperCellMgrKey.fullKey() << " with CondSvc");
    return StatusCode::FAILURE;
  }
  
  return StatusCode::SUCCESS;
}

StatusCode CaloSuperCellAlignCondAlg::execute()
{
  const EventContext& ctx = Gaudi::Hive::currentContext();
  // ____________ Construct Write Cond Handle and check its validity ____________
  SG::WriteCondHandle<CaloSuperCellDetDescrManager> writeCaloSuperCellMgrHandle{m_writeCaloSuperCellMgrKey,ctx};
  if (writeCaloSuperCellMgrHandle.isValid()) {
    ATH_MSG_DEBUG("Found valid write handle");
    return StatusCode::SUCCESS;
  }
  
  // ____________ Get Read Cond Object ____________
  SG::ReadCondHandle<CaloDetDescrManager> readCaloMgrHandle{m_readCaloMgrKey,ctx};
  ATH_CHECK(readCaloMgrHandle.isValid());
  ATH_MSG_DEBUG("Retrieved CaloDetDescrManager object form the Condition Store");
  writeCaloSuperCellMgrHandle.addDependency(readCaloMgrHandle);
  
  // ____________ Build new CaloSuperCellDetDescrManager _________________
  std::unique_ptr<CaloSuperCellDetDescrManager> mgr = std::make_unique<CaloSuperCellDetDescrManager>();

  const CaloIdManager* caloId_mgr{nullptr};
  if(detStore()->retrieve(caloId_mgr, "CaloIdManager")!=StatusCode::SUCCESS)
    throw std::runtime_error("... failed to acquire a pointer to CaloIdManager helper");
 
  mgr->set_helper(caloId_mgr->getCaloCell_SuperCell_ID());
  mgr->set_helper(caloId_mgr);
  mgr->initialize();
 
  createDescriptors(mgr.get());
  createElements(mgr.get());

  // ____________ Apply Alignment Corrections ____________________________
  ATH_CHECK(updateElements(mgr.get(),*readCaloMgrHandle));
  updateDescriptors(mgr.get(),*readCaloMgrHandle);

  ATH_CHECK(writeCaloSuperCellMgrHandle.record(std::move(mgr)));
  ATH_MSG_INFO("recorded new CaloSuperCellDetDescr Manager condition object with key " << writeCaloSuperCellMgrHandle.key() 
	       << " and range " << writeCaloSuperCellMgrHandle.getRange());
  
  return StatusCode::SUCCESS;
}

void CaloSuperCellAlignCondAlg::createDescriptors(CaloSuperCellDetDescrManager* mgr)
{
  const CaloCell_Base_ID* calo_helper = mgr->getCaloCell_ID();
  for (Identifier reg_id : calo_helper->reg_range()) {
    if (! calo_helper->is_tile (reg_id)) {
      mgr->add (new CaloDetDescriptor (reg_id, 0, calo_helper));
    }
    else {
      mgr->add_tile (new CaloDetDescriptor
		     (reg_id, calo_helper->tile_idHelper(), calo_helper,
		      (CaloCell_ID::CaloSample)calo_helper->calo_sample(reg_id), 0));
      // NB. CaloDetDescrElement::getSampling adds the tile cell sampling
      //     index to the descriptor's sampling, so don't add 2 here
      //     to calo_sample.
      mgr->add_tile (new CaloDetDescriptor
		     (reg_id, calo_helper->tile_idHelper(), calo_helper,
		      (CaloCell_ID::CaloSample)(calo_helper->calo_sample(reg_id)), 2));
    }
  }
}

void CaloSuperCellAlignCondAlg::createElements(CaloSuperCellDetDescrManager* mgr)
{
  const CaloCell_Base_ID* calo_helper = mgr->getCaloCell_ID();
  for (Identifier cell_id : calo_helper->cell_range()) {
    int subCalo = -1;
    IdentifierHash subCaloHash =
      calo_helper->subcalo_cell_hash (cell_id, subCalo);
    
    const CaloDetDescriptor* desc = get_cell_descriptor (cell_id, mgr);
    assert (desc);
    mgr->add (new CaloSuperCellDetectorElement (subCaloHash, desc));
  }
}

StatusCode CaloSuperCellAlignCondAlg::updateElements(CaloSuperCellDetDescrManager* mgr
						     , const CaloDetDescrManager* cellmgr)
{
  // For each supercell, we make a list of the corresponding cells.
   // Then we pass that list to the supercell's @c update method.
 
   for (CaloDetDescrElement* elt : mgr->element_range_nonconst()) {
     if (!elt) continue;
     CaloSuperCellDetectorElement* selt =
       dynamic_cast<CaloSuperCellDetectorElement*> (elt);
     if (!selt) continue;
 
     std::vector<Identifier> ids =
       m_scidTool->superCellToOfflineID (elt->identify());
     assert (!ids.empty());
 
     const CaloCell_Base_ID* idhelper      = mgr->getCaloCell_ID();
     const CaloCell_Base_ID* cell_idhelper = cellmgr->getCaloCell_ID();
 
     std::vector<const CaloDetDescrElement*> fromelts;
     fromelts.reserve (ids.size());
     for (Identifier id : ids) {
       // For tile tower sums, exclude D-layer cells
       // (they have a different size).
       if (cell_idhelper->sub_calo(id) == CaloCell_Base_ID::TILE &&
           cell_idhelper->sample(id) == 2 &&
           idhelper->sample(elt->identify()) != 2)
         continue;
       const CaloDetDescrElement* fromelt = cellmgr->get_element (id);
       assert (fromelt != nullptr);
       fromelts.push_back (fromelt);
     }
     CHECK( selt->update (fromelts) );
   }
  return StatusCode::SUCCESS;
}

void CaloSuperCellAlignCondAlg::updateDescriptors(CaloSuperCellDetDescrManager* mgr
						  , const CaloDetDescrManager* cellmgr)
{
  size_t maxdesc = mgr->calo_descriptors_size() + mgr->tile_descriptors_size();
  std::vector<DescrMinMax> descr_minmax (maxdesc);
  
  // Loop over cells and record range limits for each descriptor.
  for (const CaloDetDescrElement* elt : mgr->element_range()) {
    if (!elt) continue;
    CaloDetDescriptor* desc = const_cast<CaloDetDescriptor*>(elt->descriptor());
    int ndx = descr_index (desc, mgr);
    assert (ndx >= 0 && ndx < (int)descr_minmax.size());
    DescrMinMax& mm = descr_minmax[ndx];
    
    int dz_den = desc->is_lar_hec() ? 1 : 2;
    
    mm.eta.add (std::abs(elt->eta_raw()), elt->deta()/2);
    mm.phi.add (phi_for_descr (elt->phi_raw()), elt->dphi()/2);
    mm.r.add (elt->r(), elt->dr()/2);
    mm.z.add (std::abs(elt->z_raw()), elt->dz()/dz_den);
  }
  
  // Loop over each descriptor and update.
  size_t i = 0;
  for (CaloDetDescriptor* desc : mgr->calo_descriptors_range_nonconst()) {
    updateDescriptor (desc, descr_minmax[i], cellmgr);
    ++i;
  }
  for (CaloDetDescriptor* desc : mgr->tile_descriptors_range_nonconst()) {
    updateDescriptor (desc, descr_minmax[i], cellmgr);
    ++i;
  }
}

void CaloSuperCellAlignCondAlg::updateDescriptor(CaloDetDescriptor* desc
						 , const DescrMinMax& mm
						 , const CaloDetDescrManager* cellmgr)
{
  if (!desc) return;
  
  if (desc->calo_eta_max() == 0) {
    // Only do this the first time.
    desc->setCaloEtaMin (mm.eta.min);
    desc->setCaloEtaMax (mm.eta.max);
    desc->setCaloPhiMin (mm.phi.min);
    desc->setCaloPhiMax (mm.phi.max);
    desc->setCaloRMin (mm.r.min);
    desc->setCaloRMax (mm.r.max);
    desc->setCaloZMin (mm.z.min);
    desc->setCaloZMax (mm.z.max);
    
    std::vector<Identifier> cell_regions =
      m_scidTool->superCellToOfflineRegion (desc->identify());
    assert (!cell_regions.empty());
    
    const CaloDetDescriptor* celldesc =
      get_reg_descriptor (cell_regions[0], cellmgr);
    
    if (celldesc) {
      if (desc->is_tile()) {
	desc->set_eta_phi_granularity (celldesc->n_eta(),
				       celldesc->deta(),
				       celldesc->n_phi(),
				       celldesc->dphi());
      }
      
      std::vector<double> depths;
      desc->set_n_calo_depth (celldesc->n_calo_depth());
      celldesc->get_depth_in (depths);
      desc->set_depth_in (depths);
      celldesc->get_depth_out (depths);
      desc->set_depth_out (depths);
      
      desc->set_transform (celldesc->transform());
    }
  }
  
  // Do this each realignment.
  desc->setLArEtaMin (mm.eta.min);
  desc->setLArPhiMin (mm.phi.min);
  if (desc->calo_sign() > 0) {
    desc->setLArRegMin (mm.eta.min);
    desc->setLArRegMax (mm.eta.max);
  }
  else {
    desc->setLArRegMin (-mm.eta.max);
    desc->setLArRegMax (-mm.eta.min);
  }
}
