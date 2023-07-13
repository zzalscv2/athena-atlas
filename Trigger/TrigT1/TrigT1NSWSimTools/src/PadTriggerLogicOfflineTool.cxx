/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/ConcurrencyFlags.h"

#include "TrigT1NSWSimTools/PadTriggerLogicOfflineTool.h"
#include "MuonAGDDDescription/sTGCDetectorDescription.h"
#include "MuonAGDDDescription/sTGCDetectorHelper.h"

namespace NSWL1 {
//------------------------------------------------------------------------------
PadTriggerLogicOfflineTool::PadTriggerLogicOfflineTool(const std::string& type, const std::string& name, const IInterface* parent) :
    AthAlgTool(type,name,parent),
    m_etaBandsLargeSector(BandsInEtaLargeSector),
    m_etaBandsSmallSector(BandsInEtaSmallSector),
    m_detManager(nullptr),
    m_tdrLogic()
  {
    declareInterface<NSWL1::IPadTriggerLogicTool>(this);
  }

StatusCode PadTriggerLogicOfflineTool::initialize() {
    ATH_MSG_DEBUG( "initializing " << name() );
    ATH_MSG_DEBUG( name() << " configuration:");

    const IInterface* parent = this->parent();
    const INamedInterface* pnamed = dynamic_cast<const INamedInterface*>(parent);
    const std::string& algo_name = pnamed->name();

    if ( m_doNtuple ) {
        if (Gaudi::Concurrency::ConcurrencyFlags::numConcurrentEvents() > 1) {
            ATH_MSG_ERROR("DoNtuple is not possible in multi-threaded mode");
            return StatusCode::FAILURE;
        }
        m_validation_tree = std::make_unique<PadTriggerValidationTree>();

        ATH_CHECK( m_incidentSvc.retrieve() );
        m_incidentSvc->addListener(this,IncidentType::BeginEvent);

        if ( algo_name=="NSWL1Simulation" ) {
            ITHistSvc* tHistSvc=nullptr;
            ATH_CHECK(service("THistSvc", tHistSvc));

            TTree *tree=nullptr;
            std::string treename = algo_name+"Tree";
            ATH_CHECK(tHistSvc->getTree(treename, tree));
            m_validation_tree->init_tree(tree);
        }
    }

    // retrieve the MuonDetectormanager
    ATH_CHECK( detStore()->retrieve( m_detManager ) );

    fillPhiTable();
    return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------
void PadTriggerLogicOfflineTool::handle(const Incident& inc) {
    if( inc.type()==IncidentType::BeginEvent && m_doNtuple ) {
        // Ntuple can only be enabled in single-threaded mode (see initialize)
        [[maybe_unused]] bool success ATLAS_THREAD_SAFE = m_validation_tree->reset_ntuple_variables();
    }
}

void PadTriggerLogicOfflineTool::fillGeometricInformation(const std::shared_ptr<PadOfflineData>& pod) const {
    const MuonGM::sTgcReadoutElement* rdoEl = m_detManager->getsTgcReadoutElement(pod->Identity());
    const Trk::PlaneSurface &surface = rdoEl->surface(pod->Identity());
    std::array<Amg::Vector2D, 4> local_pad_corners{make_array<Amg::Vector2D, 4>(Amg::Vector2D::Zero())};
    //From MuonPadDesign... read pad local corners
    rdoEl->padCorners(pod->Identity(),local_pad_corners);
    Amg::Vector3D pad_corner_global;
    for(unsigned int i=0; i<4; i++) {
        surface.localToGlobal(local_pad_corners.at(i), pad_corner_global, pad_corner_global);
        pod->m_cornerXyz[i][0] = pad_corner_global.x();
        pod->m_cornerXyz[i][1] = pad_corner_global.y();
        pod->m_cornerXyz[i][2] = pad_corner_global.z();
    }
}

///! helper function: copy pads with a given multiplet
std::vector<std::shared_ptr<PadData>> filterByMultiplet(const std::vector<std::shared_ptr<PadData>> &pads_in, const int multiplet){
    std::vector<std::shared_ptr<PadData>> pads_out;
    pads_out.reserve(0.5*pads_in.size()); // educated guess (half inner multiplet, half outer multiplet)
    for(const auto& p : pads_in)
        if(p->multipletId()==multiplet)
            pads_out.push_back(p);
    return pads_out;
}
///! helper function: copy pads with a given gas gap
std::vector<std::shared_ptr<PadData>> filterByGasGap(const std::vector<std::shared_ptr<PadData>> &pads_in, const int gasgap){
    std::vector<std::shared_ptr<PadData>> pads_out;
    pads_out.reserve(0.25*pads_in.size()); // educated guess (4 gas gaps)
    for(const auto& p : pads_in)
        if(p->gasGapId()==gasgap)
            pads_out.push_back(p);
    return pads_out;
}

std::vector<std::unique_ptr<PadTrigger>> PadTriggerLogicOfflineTool::build4of4SingleWedgeTriggers(const std::vector<std::shared_ptr<PadData>> &pads) {
    std::vector<std::unique_ptr<PadTrigger>> triggers;
    for(const auto& p0 : filterByGasGap(pads, 1)){
        int p0ieta = p0->padEtaId();
        int p0iphi = p0->padPhiId();
        for(const auto& p1 : filterByGasGap(pads, 2)){
            int p1ieta = p1->padEtaId();
            int p1iphi = p1->padPhiId();
            bool p0_p1_match = ((p1ieta == p0ieta || p1ieta == p0ieta+1 ) &&
                                (p1iphi == p0iphi || p1iphi == p0iphi+1 ) );
            if(not p0_p1_match) continue;
            for(const auto& p2 :filterByGasGap(pads, 3) ){
                int p2ieta = p2->padEtaId();
                int p2iphi = p2->padPhiId();
                bool p1_p2_match = ((p2ieta == p1ieta || p2ieta == p1ieta+1 ) &&
                                    (p2iphi == p1iphi || p2iphi == p1iphi+1 ) );
                if(not p1_p2_match) continue;
                for(const auto& p3 : filterByGasGap(pads, 4)){
                    int p3ieta = p3->padEtaId();
                    int p3iphi = p3->padPhiId();
                    bool p2_p3_match = ((p3ieta == p2ieta || p3ieta == p2ieta+1 ) &&
                                        (p3iphi == p2iphi || p3iphi == p2iphi+1 ) );
                    if(p2_p3_match){
                        auto trg=std::make_unique<PadTrigger>();
                        trg->m_pads.push_back(p0);
                        trg->m_pads.push_back(p1);
                        trg->m_pads.push_back(p2);
                        trg->m_pads.push_back(p3);
                        triggers.push_back(std::move(trg));
                    }
                } // for(p3)
            } // for(p2)
        } // for(p1)
    } // for(p0)
    return triggers;
}


StatusCode PadTriggerLogicOfflineTool::compute_pad_triggers(const std::vector<std::shared_ptr<PadData>>& pads,
                                                            std::vector<std::unique_ptr<PadTrigger>> &triggers) const
{
    ATH_MSG_DEBUG(" <N> receiving "<<pads.size()<<" pad data");
    ATH_MSG_DEBUG("calling compute_pad_triggers() (pads.size() "<<pads.size()<<")");
    for(const auto& pad : pads){
        ATH_MSG_DEBUG(" <N> building trig from pad "
                      <<" side "<<pad->sideId()<<""
                      <<", sector "<<pad->sectorId()
                      <<", sector type "<<pad->sectorType()
                      <<", module "<<pad->moduleId()
                      <<", multiplet "<<pad->multipletId()
                      <<", gas gap "<<pad->gasGapId()
                      <<", pad eta "<<pad->padEtaId()
                      <<", pad phi "<<pad->padPhiId());
    }
    for(const size_t side : SIDES){
        for(const size_t sector : SECTORS){
            std::vector<std::shared_ptr<PadData>> sector_pads;
            std::copy_if(pads.begin(), pads.end(),
                    back_inserter(sector_pads),
                    [&](const std::shared_ptr<PadData>& p) { return (p->sideId()==static_cast<int>(side) &&
                                                    (2*p->sectorId()-1-p->sectorType())==static_cast<int>(sector));});

            if(!sector_pads.empty()){
                const std::shared_ptr<PadData> firstPad = sector_pads[0];
                ATH_MSG_DEBUG("<N> side "
                              <<(firstPad->sideId()==0?"A":"C")
                              <<" trigger sector "<< (2*firstPad->sectorId()-1-firstPad->sectorType())
                              <<" : "<<sector_pads.size()<<" pads");
                if(m_useSimple4of4) {
                    const int innerMultiplet(1), outerMultiplet(2); // DG-2015-10-07 move to enum?
                    std::vector<std::shared_ptr<PadData>> padsInner(filterByMultiplet(sector_pads, innerMultiplet));
                    std::vector<std::shared_ptr<PadData>> padsOuter(filterByMultiplet(sector_pads, outerMultiplet));
                    std::vector<std::unique_ptr<PadTrigger>> triggersInner = build4of4SingleWedgeTriggers(padsInner);
                    std::vector<std::unique_ptr<PadTrigger>> triggersOuter = build4of4SingleWedgeTriggers(padsOuter);
                    ATH_MSG_DEBUG("found "
                                  <<triggersInner.size()<<" inner triggers"
                                  <<" and "
                                  <<triggersOuter.size()<<" outer triggers");
                     triggers.reserve(triggers.size() + triggersInner.size()+triggersOuter.size());

                    triggers.insert(triggers.end(),std::make_move_iterator(triggersInner.begin()),std::make_move_iterator(triggersInner.end()));
                    triggers.insert(triggers.end(),std::make_move_iterator(triggersOuter.begin()),std::make_move_iterator(triggersOuter.end()));
                } 
                else {
                  std::vector<std::shared_ptr<PadOfflineData>> trgpads;
                  for(const auto& p : sector_pads){
                     auto pod=std::dynamic_pointer_cast<PadOfflineData> (p);
                       fillGeometricInformation(pod);
                       trgpads.push_back(pod);
                  }
                  const std::vector<SectorTriggerCandidate> candidates = m_tdrLogic.buildSectorTriggers(trgpads,m_Zratio);
                  int index=0;
                  for( const auto& st : candidates){
                     auto p=std::make_unique<PadTrigger>(convert(st));
                     p->m_triggerindex=index;
                     if (p->m_pads.empty()) continue;//don't record null triggers (rejected or empty)
                     triggers.push_back(std::move(p));
                     index++;
                  }
                  ATH_MSG_DEBUG("found " << candidates.size() << " triggers using the tdr logic");
               }
            } // if(sector_pads)
        } // for(sector)
    } // for(side)
    // Fill Ntuple
    if(m_doNtuple) {
      // Ntuple can only be enabled in single-threaded mode (see initialize)
      [[maybe_unused]] bool b1 ATLAS_THREAD_SAFE = m_validation_tree->fill_num_pad_triggers(triggers.size());
      [[maybe_unused]] bool b2 ATLAS_THREAD_SAFE = m_validation_tree->fill_pad_trigger_basics(triggers);
    }
    return StatusCode::SUCCESS;
}
//------------------------------------------------------------------------------

int PadTriggerLogicOfflineTool::ROI2BandId(const float EtaTrigAtCenter, const int SectorType) const {

    switch(SectorType){
      case(1): //L
        if(EtaTrigAtCenter < m_etaBandsLargeSector.back() || EtaTrigAtCenter > m_etaBandsLargeSector.front()) return -999;
        for(unsigned int i=0;i<m_etaBandsLargeSector.size();i++){
          if(EtaTrigAtCenter < m_etaBandsLargeSector.at(i) && EtaTrigAtCenter > m_etaBandsLargeSector.at(i+1) ) return i;
        }
        break;

      case(0): //S
        if(EtaTrigAtCenter < m_etaBandsSmallSector.back() || EtaTrigAtCenter > m_etaBandsSmallSector.front()) return -999;
        for(unsigned int i=0;i<m_etaBandsSmallSector.size();i++){
          if(EtaTrigAtCenter < m_etaBandsSmallSector.at(i) && EtaTrigAtCenter > m_etaBandsSmallSector.at(i+1) ) return i;
        }
        break;

      default:
        ATH_MSG_FATAL(" Unexpected SectorType="<<SectorType);
        break;
    }
    return -999;
}

//------------------------------------------------------------------------------
NSWL1::PadTrigger PadTriggerLogicOfflineTool::convert(const SectorTriggerCandidate &stc) const {

    PadTrigger pt;
    const Polygon roi=stc.triggerRegion3();
    Vertex trgCntr=centroid(roi);
    SingleWedgePadTrigger innertrg = stc.wedgeTrigs().at(0);
    
    //*********** assign central etaphi cordinates of the triggering region *****************
    const float xcntr=coordinate<0>(trgCntr);
    const float ycntr=coordinate<1>(trgCntr);
    const float zcntr=innertrg.pads().at(0)->m_cornerXyz[1][2];
    ROOT::Math::XYZVector trigVector(xcntr,ycntr,zcntr);
    const float etaTrig=trigVector.Eta();
    const float phiTrig=trigVector.Phi();
    pt.m_eta    = etaTrig;
    pt.m_phi    = phiTrig;

    /*
    ===== Recipe for projecting eta of the trigger centroid to the axis of symmetry on a sector =======
    */
    //**************************************************************************************
    auto pad0=innertrg.pads().at(0);
    Identifier idt(pad0->id());
    const Trk::PlaneSurface &surf = m_detManager->getsTgcReadoutElement(idt)->surface(idt);
    Amg::Vector3D global_trgCoordinates(xcntr,ycntr,zcntr);
    Amg::Vector2D local_trgCoordinates;
    surf.globalToLocal(global_trgCoordinates,Amg::Vector3D(),local_trgCoordinates);

    float yloc_trg=local_trgCoordinates.y();
    Amg::Vector2D local_trgCoordinateOnAxis(0,yloc_trg);
    Amg::Vector3D global_trgCoordinateProjectedOnAxis;
    surf.localToGlobal(local_trgCoordinateOnAxis, Amg::Vector3D(),global_trgCoordinateProjectedOnAxis);
    ROOT::Math::XYZVector trgVectorProjectedOnAxis(global_trgCoordinateProjectedOnAxis.x(),global_trgCoordinateProjectedOnAxis.y(),global_trgCoordinateProjectedOnAxis.z());
    float etaProjected=trgVectorProjectedOnAxis.Eta();
    int secType=pad0->sectorType();
    int matchedBandId=ROI2BandId(std::abs(etaProjected),secType);
    pt.m_bandid = (matchedBandId < 0) ? 0 : matchedBandId+2;// Y.R Bands start from 2
    if(pt.m_bandid == 0) ATH_MSG_WARNING("PadTrigger out of current eta bands");

     /* ======== End of band Id matching and assignment ======================= */

    pt.m_multiplet_id = pad0->multipletId();
    pt.m_eta_id = innertrg.halfPadCoordinates().ieta;//this is meaningless and shoiuld be removed
    pt.m_isSmall= int(innertrg.isSmallSector());

    //************** assign extrema of the trigger region coordinates in eta-phi  and some other variables for offline analysis **************
    std::vector<std::pair<float,float>> trg_etaphis;
    for(const auto& v : boost::geometry::exterior_ring(roi)){
        const float xcurr=coordinate<0>(v);
        const float ycurr=coordinate<1>(v);
        const float zcurr=zcntr;
        const float etacurr=eta(xcurr,ycurr,zcurr);
        const float phicurr=phi(xcurr,ycurr,zcurr);
        trg_etaphis.emplace_back(etacurr,phicurr);
    }
    const auto trg_phiminmax=std::minmax_element(trg_etaphis.begin(),trg_etaphis.end(),[](
        const std::pair<float,float>& l,const std::pair<float,float>& r){return l.second<r.second;}
    );
    float trgEtaMin=0;
    float trgEtaMax=0;

    if(pt.m_isSmall && matchedBandId > 0){
        trgEtaMin=m_etaBandsSmallSector.at(matchedBandId+1);
        trgEtaMax=m_etaBandsSmallSector.at(matchedBandId);
    } else if(!pt.m_isSmall && matchedBandId > 0) {
        trgEtaMin=m_etaBandsLargeSector.at(matchedBandId+1);
        trgEtaMax=m_etaBandsLargeSector.at(matchedBandId);
    }

    pt.m_etamin=trgEtaMin;
    pt.m_etamax=trgEtaMax;
    pt.m_phimin=(trg_phiminmax.first)->second;
    pt.m_phimax=(trg_phiminmax.second)->second;
    pt.m_moduleIdInner=-1;
    pt.m_moduleIdOuter=-1;
    
    if(stc.wedgeTrigs().size()==2){//double wedge trigger
        pt.m_moduleIdInner=stc.wedgeTrigs().at(0).pads().at(0)->moduleId();
        pt.m_moduleIdOuter=stc.wedgeTrigs().at(1).pads().at(1)->moduleId();
    }
    else{
        int multId0=stc.wedgeTrigs().at(0).pads().at(0)->multipletId();
        if( multId0==1){
            pt.m_moduleIdInner=stc.wedgeTrigs().at(0).pads().at(0)->moduleId();
        }
        else{
            pt.m_moduleIdOuter=stc.wedgeTrigs().at(0).pads().at(0)->moduleId();
        }
        //one of the module Ids remain as -1 /single wedge trigger 
    }

    //****************************************************************************************

    for(const SingleWedgePadTrigger& swt : stc.wedgeTrigs()){
        int currwedge=swt.pads().at(0)->multipletId();
        std::vector<float> trglocalminY;
        std::vector<float> trglocalmaxY;
        std::vector<int> trgSelectedLayers;
        std::vector<int> trgSelectedBands;
        std::vector<int> trgPadPhiIndices;
        std::vector<int> trgPadEtaIndices;
        std::vector< std::shared_ptr<PadData>> trgPads;
        for(const auto &p : swt.pads()){
            const float padZ=p->m_cornerXyz[0][2];
            Identifier Id( p->id());
            const Trk::PlaneSurface &padsurface = m_detManager->getsTgcReadoutElement(Id)->surface(Id);
            float Phi=p->stationPhiAngle();

            //Find the radial boundaries of the band within the sector axis
            float Rmin=std::abs(padZ*std::tan(2*std::atan(-1*std::exp(pt.m_etamax))) );
            float Rmax=std::abs(padZ*std::tan(2*std::atan(-1*std::exp(pt.m_etamin))) );
            float xmin=Rmin*std::cos(Phi);
            float ymin=Rmin*std::sin(Phi);
            float xmax=Rmax*std::cos(Phi);
            float ymax=Rmax*std::sin(Phi);

            Amg::Vector3D global_trgMinOnAxis(xmin,ymin,padZ);
            Amg::Vector3D global_trgMaxOnAxis(xmax,ymax,padZ);

            Amg::Vector2D local_trgMinOnAxis;
            Amg::Vector2D local_trgMaxOnAxis;
            padsurface.globalToLocal(global_trgMinOnAxis,Amg::Vector3D(),local_trgMinOnAxis);
            padsurface.globalToLocal(global_trgMaxOnAxis,Amg::Vector3D(),local_trgMaxOnAxis);

            float bandLocalMaxY=local_trgMaxOnAxis.y();
            float bandLocalMinY=local_trgMinOnAxis.y();

            trglocalminY.push_back(bandLocalMinY);
            trglocalmaxY.push_back(bandLocalMaxY);
            trgSelectedLayers.push_back(p->gasGapId());
            trgSelectedBands.push_back(matchedBandId+2);
            trgPadPhiIndices.push_back(p->padPhiId());
            trgPadEtaIndices.push_back(p->padEtaId());
            trgPads.push_back(p);
            pt.m_pads.push_back(p);
        } // eof for(p) pads
        //assign variables / wedgewise
        if(currwedge==1){
            pt.m_trglocalminYInner=trglocalminY;
            pt.m_trglocalmaxYInner=trglocalmaxY;
            pt.m_trgSelectedLayersInner=trgSelectedLayers;
            pt.m_trgSelectedBandsInner=trgSelectedBands;
            pt.m_trgPadPhiIndicesInner=trgPadPhiIndices;
            pt.m_trgPadEtaIndicesInner=trgPadEtaIndices;
            pt.m_padsInner=trgPads;
        }
        if(currwedge==2){
            pt.m_trglocalminYOuter=trglocalminY;
            pt.m_trglocalmaxYOuter=trglocalmaxY;
            pt.m_trgSelectedLayersOuter=trgSelectedLayers;
            pt.m_trgSelectedBandsOuter=trgSelectedBands;
            pt.m_trgPadPhiIndicesOuter=trgPadPhiIndices;
            pt.m_trgPadEtaIndicesOuter=trgPadEtaIndices;
            pt.m_padsOuter=trgPads;
        }

        trglocalminY.clear();
        trglocalmaxY.clear();
        trgSelectedLayers.clear();
        trgSelectedBands.clear();
        trgPadPhiIndices.clear();
        trgPadEtaIndices.clear();
        trgPads.clear();
    } // eof for (swt) single wedge trigger

    //Assignment of  Phi Id using 6 bits slicing
    Identifier padIdentifier(pt.m_pads.at(0)->id() );
    IdentifierHash moduleHashId;
    const IdContext ModuleContext = m_detManager->stgcIdHelper()->module_context();

    //get the module Identifier using the pad's
    m_detManager->stgcIdHelper()->get_hash( padIdentifier, moduleHashId, &ModuleContext );
    float stationPhiMin=0.0;
    float stationPhiMax=0.0;
    std::map<IdentifierHash,std::pair<double,double>>::const_iterator itPhi = m_phiTable.find(moduleHashId);
    if (itPhi != m_phiTable.end()) {
      stationPhiMin=(*itPhi).second.first;
      stationPhiMax=(*itPhi).second.second;
    }
    else {
      ATH_MSG_WARNING("Could not find the hash Id: " << moduleHashId << " in the map");
    }

    float trgPhiCntr=pt.m_phi;
    int nPhiSlices=1<<m_phiIdBits;//6 bits for Phi Id; i.e interval of [0,....63]

    // special treatment for the last phi region
    if ( stationPhiMax > M_PI && trgPhiCntr<0 ) {
      trgPhiCntr=2*M_PI-fabs(trgPhiCntr);
    }

    if ( trgPhiCntr<stationPhiMin || trgPhiCntr>stationPhiMax ) {
      ATH_MSG_WARNING("Trigger phi: " << trgPhiCntr << " outside the station. Min, max: " << stationPhiMin << "  " << stationPhiMax );
    }

    float step=(stationPhiMax-stationPhiMin)/nPhiSlices;
    for( int i=0;i<nPhiSlices;i++){
        if(stationPhiMin+i*step>=trgPhiCntr){
            pt.m_phi_id=i;
            break;
        }
    }
    return pt;
}

  // fill the map with the phi ranges
  void PadTriggerLogicOfflineTool::fillPhiTable() {

    const sTgcIdHelper* helper = m_detManager->stgcIdHelper();
    
    std::vector<Identifier>::const_iterator  idfirst = helper->module_begin();
    std::vector<Identifier>::const_iterator  idlast =  helper->module_end();
    
    const IdContext ModuleContext = helper->module_context();
   
    for ( std::vector<Identifier>::const_iterator i=idfirst ; i!=idlast ; ++i ) {

      Identifier     Id = *i;
      IdentifierHash hashId;
      
      helper->get_hash( Id, hashId, &ModuleContext );
      
      const MuonGM::sTgcReadoutElement* module = m_detManager->getsTgcReadoutElement(Id);
      if (!module) continue;
      int multilayer = helper->multilayer(Id);
      
      char side     = module->getStationEta() < 0 ? 'C' : 'A'; 
      char sector_l = module->getStationName().substr(2,1)=="L" ? 'L' : 'S';
 
      sTGCDetectorHelper aHelper;
      sTGCDetectorDescription* md = aHelper.Get_sTGCDetector( sector_l, std::abs(module->getStationEta()), module->getStationPhi(), multilayer, side );
      
      Amg::Vector3D pos = module->center();      
      double swidth = md->sWidth();
      double lwidth = md->lWidth(); 
      double ycutout = md->yCutout(); 
      double length = md->Length();
      double moduleR = std::sqrt( pos.mag()*pos.mag() -  pos.z()*pos.z());
      double dphi1 = std::atan( (0.5*lwidth)/(moduleR+0.5*length) );
      double dphi2 = std::atan( (0.5*swidth)/(moduleR-0.5*length) );
 
      double dphi = ( dphi1 > dphi2 ? dphi1 : dphi2 ); 
      if ( ycutout > 0 ) { 
	double rcutout = moduleR+0.5*length - ycutout;
	double dphicutout = std::atan( (0.5*lwidth)/rcutout );
	if ( dphi < dphicutout ) dphi = dphicutout;
      } 
      double phimin = pos.phi()-dphi;
      double phimax = pos.phi()+dphi;
      
      if ( phimin >  M_PI ) phimin -= 2*M_PI;
      if ( phimin < -M_PI ) phimin += 2*M_PI;      

      std::pair<double,double> phiRange(phimin,phimax);
      m_phiTable[hashId]=phiRange;

	if((sector_l=='L' && m_Zratio.first==0) || (sector_l=='S' && m_Zratio.second==0)) {
	double ratio=1/pos.z();
	Id=helper->multilayerID(Id,2);
	const MuonGM::sTgcReadoutElement* module2 = m_detManager->getsTgcReadoutElement(Id);
	Amg::Vector3D pos2 = module2->center();
	ratio*=pos2.z();
	if(sector_l=='L') m_Zratio.first=ratio;
	else if(sector_l=='S') m_Zratio.second=ratio;
	}
    }

  }


} // NSWL1
