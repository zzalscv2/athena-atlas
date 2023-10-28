/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonChamberToolTest.h"

#include <StoreGate/ReadCondHandle.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <MuonReadoutGeometryR4/RpcReadoutElement.h>
#include <GaudiKernel/SystemOfUnits.h>

namespace MuonGMR4 {

    MuonChamberToolTest::MuonChamberToolTest(const std::string& name, ISvcLocator* pSvcLocator):
        AthReentrantAlgorithm{name, pSvcLocator} {}

    StatusCode MuonChamberToolTest::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_geoCtxKey.initialize());
        ATH_CHECK(m_chambTool.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_detVolSvc.retrieve());
        return StatusCode::SUCCESS;
    }

    StatusCode MuonChamberToolTest::execute(const EventContext& ctx) const {
        SG::ReadCondHandle<ActsGeometryContext> gctx{m_geoCtxKey, ctx};
        m_detVolSvc->detector();
        if (!gctx.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve the Acts alignment "<<m_geoCtxKey.fullKey());
            return StatusCode::FAILURE;
        }

        ChamberSet chambers = m_chambTool->buildChambers();
        std::vector<const MuonReadoutElement*> elements = m_detMgr->getAllReadoutElements();
        
        /// Test that all readout elements have a chamber
        for (const MuonReadoutElement* readOut : elements) {
            ChamberSet::const_iterator itr = chambers.find(*readOut);
            if (itr == chambers.end()) {
                ATH_MSG_FATAL("The element "<<m_idHelperSvc->toStringDetEl(readOut->identify())
                            <<" is not attributed with any chamber");
                return StatusCode::FAILURE;
            }

        }

            
        for (const MuonChamber& chamber : chambers) {
                        ATH_MSG_INFO("Muon chamber "<<m_idHelperSvc->mdtIdHelper().stationNameString(chamber.stationName())
                                                    <<", eta: "<<chamber.stationEta()<<", phi: "<<chamber.stationPhi());
                                                    
            unsigned int maxTubesPerLayer{0};
            for(const MuonReadoutElement* readOut : chamber.readOutElements()) {
                if (readOut->detectorType() == ActsTrk::DetectorType::Mdt) {
                    const MdtReadoutElement* mdtMl = static_cast<const MdtReadoutElement*>(readOut);
                    maxTubesPerLayer = std::max(mdtMl->numTubesInLay(), maxTubesPerLayer);
                }
            }

            const Amg::Transform3D chamberRot = Amg::getRotateY3D(M_PI_2) * Amg::getRotateX3D(M_PI_2) * Amg::getRotateZ3D(M_PI_2);
            const std::shared_ptr<Acts::Volume> boundVol = chamber.boundingVolume(**gctx);
            const std::shared_ptr<Acts::Volume> boundVolChamberFrame = std::make_shared<Acts::Volume>(chamberRot, chamber.bounds());
            for(const MuonReadoutElement* readOut : chamber.readOutElements()){

                const Amg::Vector3D boxCenter{boundVolChamberFrame->center()};

                    if (!boundVol->inside(readOut->center(**gctx))) {
                        ATH_MSG_FATAL("The Mdt "<<m_idHelperSvc->toStringDetEl(readOut->identify())<<" "
                                    <<Amg::toString(readOut->center(**gctx), 2)<<" is outside the chamber volume "<<(*boundVol)
                                    <<", volume center: "<<Amg::toString(boundVol->center(), 2));
                        return StatusCode::FAILURE;
                    }

                    if (readOut->detectorType() == ActsTrk::DetectorType::Mdt) {
                        const MdtReadoutElement* mdtMl = static_cast<const MdtReadoutElement*>(readOut);
                        for (unsigned int layer = 1; layer <= mdtMl->numLayers(); layer++) {
                            for (unsigned int tube = 1; tube <= mdtMl->numTubesInLay(); tube++) {

                                const IdentifierHash idHash = mdtMl->measurementHash(layer, tube);
                                const Amg::Vector3D transformToChamberTube = chamber.globalToLocalTrans(**gctx) * mdtMl->globalTubePos(**gctx, idHash);
                                const Amg::Vector3D transformToChamberTubeRO = chamber.globalToLocalTrans(**gctx)* mdtMl->readOutPos(**gctx, idHash);
                            
                                const Amg::Vector3D tubeCenter{chamberRot * transformToChamberTube};
                                const Amg::Vector3D tubeRO{chamberRot * transformToChamberTubeRO}; // readout position
                                const Amg::Vector3D tubeCenterToRO{tubeCenter - tubeRO};
                                const Amg::Vector3D tubeHV{tubeRO + 2 * tubeCenterToRO }; // get the HV poisition

                                //Ensure that all tube centers are inside the box to begin with
                                if(!boundVolChamberFrame->inside(tubeCenter)) {
                                    ATH_MSG_FATAL(__LINE__<<" The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())
                                                    <<" is outside the chamber volume. Tube center is located at: "
                                                    <<Amg::toString(tubeCenter, 2) 
                                                    <<Amg::toString(mdtMl->globalTubePos(**gctx, idHash), 2) 
                                                    
                                                    << ". Chamber center located at:  " << Amg::toString(boxCenter, 2)
                                                    << " with dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                    <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ()
                                                    <<" " <<(*boundVolChamberFrame));
                                    return StatusCode::FAILURE;
                                }
                                else {
                                    ATH_MSG_VERBOSE("The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())
                                                    <<" is inside the chamber volume with tube center located at: " <<Amg::toString(mdtMl->globalTubePos(**gctx, idHash))
                                                    <<". Chamber center located at:  " << Amg::toString(chamber.boundingVolume(**gctx)->center())
                                                    << " with chamber dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                    <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                                }

                            //Check if the first and last tube in each layer are close to the left/right edge of chamber
                            if( tube == 1 || tube == mdtMl->numTubesInLay() ) {
                                //Shift tube to left or right in chamber frame depending on tube number
                                if(tube == 1){
                                    const Amg::Vector3D leftEdgeChamberFrame{tubeCenter - Amg::Vector3D{0,0,mdtMl->tubeRadius()}};
                                    //Shift by a little more we should be outside the box now
                                    const Amg::Vector3D leftEdgeChamberFrameOutside{tubeCenter - Amg::Vector3D{0,0,mdtMl->tubeRadius() + 5 * Gaudi::Units::cm}};
                                    //Check if leftEdgeGlobalFrame is outside the box and that leftEdgeGlobalFrameOutside is inside, fail if either true
                                    if(!boundVolChamberFrame->inside(leftEdgeChamberFrame) || boundVolChamberFrame->inside(leftEdgeChamberFrameOutside)) {
                                        //Check if one multilayer has less tubes
                                        if(boundVolChamberFrame->inside(leftEdgeChamberFrameOutside) && maxTubesPerLayer != mdtMl->numTubesInLay()){
                                            ATH_MSG_VERBOSE("The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())<< " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                            <<" is inside the chamber volume. This multilayer has less tubes than the other one, so this behavior is expected. Tube center is located at: "
                                                            <<Amg::toString(leftEdgeChamberFrame, 2) 
                                                            <<".  Tube left edge shifted outside located at: " << Amg::toString(leftEdgeChamberFrameOutside, 2)
                                                            << ". Chamber center located at:  " << Amg::toString(boxCenter, 2)
                                                            << " with dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                            <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                                        }
                                        else{
                                            ATH_MSG_FATAL(__LINE__<<"The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify()) << " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                        <<" is outside the chamber volume. Tube left edge is located at: "
                                                        <<Amg::toString(leftEdgeChamberFrame, 2) 
                                                        <<".  Tube left edge shifted outside located at: " << Amg::toString(leftEdgeChamberFrameOutside, 2)
                                                        << ". Chamber center located at:  " << Amg::toString(boxCenter, 2)
                                                        << " with dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                        <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                                            return StatusCode::FAILURE;
                                        }
                                    }
                                    else {
                                        ATH_MSG_VERBOSE("The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify()) << " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                        <<" is inside the chamber volume with tube left edge located at: " <<Amg::toString(leftEdgeChamberFrame, 2)
                                                        <<". Tube left edge shifted outside located at: " << Amg::toString(leftEdgeChamberFrameOutside, 2)
                                                        <<". Chamber center located at:  " << Amg::toString(chamber.boundingVolume(**gctx)->center())
                                                        << " with chamber dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                        <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                                    }
                                } 
                                else {
                                    const Amg::Vector3D rightEdgeChamberFrame{tubeCenter + Amg::Vector3D{0,0,mdtMl->tubeRadius()}};
                                    const Amg::Vector3D rightEdgeChamberFrameOutside{tubeCenter + Amg::Vector3D{0,0, mdtMl->tubeRadius() + 5 * Gaudi::Units::cm}};
                                    //Check if inside the boundVol
                                    if(!boundVolChamberFrame->inside(rightEdgeChamberFrame) || boundVolChamberFrame->inside(rightEdgeChamberFrameOutside)) {
                                        if(boundVolChamberFrame->inside(rightEdgeChamberFrameOutside) && maxTubesPerLayer != mdtMl->numTubesInLay()){
                                            ATH_MSG_VERBOSE("The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())<< " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                            <<" is inside the chamber volume. This multilayer has less tubes than the other one, so this behavior is expected. Tube center is located at: "
                                                            <<Amg::toString(rightEdgeChamberFrame, 2) 
                                                            <<".  Tube right edge shifted outside located at: " << Amg::toString(rightEdgeChamberFrameOutside, 2)
                                                            << ". Chamber center located at:  " << Amg::toString(boxCenter, 2)
                                                            << " with dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                            <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                                        }
                                        else{
                                            if(boundVolChamberFrame->inside(rightEdgeChamberFrameOutside) && maxTubesPerLayer == mdtMl->numTubesInLay()){
                                                ATH_MSG_VERBOSE("The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())<< " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                            <<" is inside the chamber volume. This multilayer has equal tubes to the max, so this behavior is expected. Tube center is located at: "
                                                            <<Amg::toString(rightEdgeChamberFrame, 2) 
                                                            <<".  Tube right edge shifted outside located at: " << Amg::toString(rightEdgeChamberFrameOutside, 2)
                                                            << ". Chamber center located at:  " << Amg::toString(boxCenter, 2)
                                                            << " with dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                            <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                                            }
                                            else{
                                                ATH_MSG_FATAL(__LINE__<<"The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())<< " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                                <<" is outside the chamber volume. Tube right edge is located at: "
                                                                <<Amg::toString(rightEdgeChamberFrame, 2) 
                                                                <<".  Tube right edge shifted outside located at: " << Amg::toString(rightEdgeChamberFrameOutside, 2)
                                                                << ". Chamber center located at:  " << Amg::toString(boxCenter, 2)
                                                                << " with dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                                <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                                                return StatusCode::FAILURE;
                                            }
                                        }
                                    }
                                    else {
                                        ATH_MSG_VERBOSE("The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())<< " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                        <<" is inside the chamber volume with tube right edge located at: " <<Amg::toString(rightEdgeChamberFrame, 2)
                                                        <<".  Tube right edge shifted outside located at: " << Amg::toString(rightEdgeChamberFrameOutside, 2)
                                                        <<". Chamber center located at:  " << Amg::toString(chamber.boundingVolume(**gctx)->center())
                                                        << " with chamber dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                        <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                                    }
                                }
                            }
                            //Check if we move tubes in ml 1 down by radius of tube, we are still inside and moving slightly more puts us outside. Also if only 1 multilayer check the top tubes
                            if(mdtMl->multilayer() == 1 && layer == 1){
                                const Amg::Vector3D bottomEdgeChamberFrame{tubeCenter - Amg::Vector3D{0,mdtMl->tubeRadius(),0}};
                                //Shift by a little more we should be outside the box now 20cm is chosen in case of foam + rpcs
                                const Amg::Vector3D bottomEdgeChamberFrameOutside{tubeCenter - Amg::Vector3D{0, mdtMl->tubeRadius() + 20 * Gaudi::Units::cm, 0}};
                                if(!boundVolChamberFrame->inside(bottomEdgeChamberFrame) || boundVolChamberFrame->inside(bottomEdgeChamberFrameOutside)){
                                    ATH_MSG_FATAL(__LINE__<<"The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())<< " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                    <<" is either outside the chamber volume, or not close to the bottom of the chamber. Tube bottom edge is located at: "
                                                    <<Amg::toString(bottomEdgeChamberFrame, 2) 
                                                    <<". Bottom Edge Chamber Frame Outside: " << Amg::toString(bottomEdgeChamberFrameOutside, 2)
                                                    << ". Chamber center located at:  " << Amg::toString(boxCenter, 2)
                                                    << " with dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                    <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ()
                                                    <<" " <<(*boundVolChamberFrame));
                                    return StatusCode::FAILURE;
                                }
                                else {
                                    ATH_MSG_VERBOSE("The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())<< " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                    <<" is inside the chamber volume and is close to the bottom edge. Tube bottom edge located at: " <<Amg::toString(bottomEdgeChamberFrame, 2)
                                                    <<". Bottom Edge Chamber Frame Outside: " << Amg::toString(bottomEdgeChamberFrameOutside, 2)
                                                    <<". Chamber center located at:  " << Amg::toString(chamber.boundingVolume(**gctx)->center())
                                                    << " with chamber dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                    <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                                }
                                const MdtIdHelper& mdtHelperSvc = m_idHelperSvc->mdtIdHelper();
                                if(mdtHelperSvc.numberOfMultilayers(readOut->identify()) == 1){
                                    //Since this chamber has only one multilayer, check the top tubes also
                                    const Amg::Vector3D topEdgeChamberFrame{tubeCenter + Amg::Vector3D{0,mdtMl->tubeRadius(),0}};
                                    //Shift by a little more we should be outside the box now 20cm is chosen in case of foam + rpcs
                                    const Amg::Vector3D topEdgeChamberFrameOutside{tubeCenter + Amg::Vector3D{0, mdtMl->tubeRadius() + 20 * Gaudi::Units::cm, 0}};
                                    if(!boundVolChamberFrame->inside(topEdgeChamberFrame) || boundVolChamberFrame->inside(topEdgeChamberFrameOutside)){
                                        ATH_MSG_FATAL(__LINE__<<"The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())<< " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                        <<" is either outside the chamber volume, or not close to the top of the chamber. Tube top edge is located at: "
                                                        <<Amg::toString(topEdgeChamberFrame, 2) 
                                                        <<". Top Edge Chamber Frame Outside: " << Amg::toString(topEdgeChamberFrameOutside, 2)
                                                        << ". Chamber center located at:  " << Amg::toString(boxCenter, 2)
                                                        << " with dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                        <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                                        return StatusCode::FAILURE;
                                    }
                                    else {
                                        ATH_MSG_VERBOSE("The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())<< " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                        <<" is inside the chamber volume and is close to the top edge. Tube top edge located at: " <<Amg::toString(topEdgeChamberFrame, 2)
                                                        <<". Top Edge Chamber Frame Outside: " << Amg::toString(topEdgeChamberFrameOutside, 2)
                                                        <<". Chamber center located at:  " << Amg::toString(chamber.boundingVolume(**gctx)->center())
                                                        << " with chamber dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                        <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                                    }
                                }
                            }
                            else if (mdtMl->multilayer() == 2 && layer == mdtMl->numLayers()){
                                //Now we are in multilayer 2 and we only need to check the top of the box
                                const Amg::Vector3D topEdgeChamberFrame{tubeCenter + Amg::Vector3D{0,mdtMl->tubeRadius(),0}};
                                //Shift by a little more we should be outside the box now 20cm is chosen in case of foam + rpcs
                                const Amg::Vector3D topEdgeChamberFrameOutside{tubeCenter + Amg::Vector3D{0, mdtMl->tubeRadius() + 20 * Gaudi::Units::cm, 0}};
                                if(!boundVolChamberFrame->inside(topEdgeChamberFrame) || boundVolChamberFrame->inside(topEdgeChamberFrameOutside)){
                                    ATH_MSG_FATAL(__LINE__<<"The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())<< " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                    <<" is either outside the chamber volume, or not close to the top of the chamber. Tube top edge is located at: "
                                                    <<Amg::toString(topEdgeChamberFrame, 2) 
                                                    <<". Top Edge Chamber Frame Outside: " << Amg::toString(topEdgeChamberFrameOutside, 2)
                                                    << ". Chamber center located at:  " << Amg::toString(boxCenter, 2)
                                                    << " with dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                    <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                                    return StatusCode::FAILURE;
                                }
                                else {
                                    ATH_MSG_VERBOSE("The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())<< " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                    <<" is inside the chamber volume and is close to the top edge. Tube top edge located at: " <<Amg::toString(topEdgeChamberFrame, 2)
                                                    <<". Top Edge Chamber Frame Outside: " << Amg::toString(topEdgeChamberFrameOutside, 2)
                                                    <<". Chamber center located at:  " << Amg::toString(chamber.boundingVolume(**gctx)->center())
                                                    << " with chamber dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                    <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                                }
                            }
                            // Check that readOut and HV are in chamber
                            if(!boundVolChamberFrame->inside(tubeRO) || !boundVolChamberFrame->inside(tubeHV)){
                                ATH_MSG_FATAL("The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())<< " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                <<" has its readOut or HV outside the chamber. Tube readout is located at: "
                                                <<Amg::toString(tubeRO, 2) 
                                                <<". Tube HV is located at: " << Amg::toString(tubeHV, 2)
                                                << ". Chamber center located at:  " << Amg::toString(boxCenter, 2)
                                                <<". Tube center located at: " << Amg::toString(tubeCenter, 2)
                                                << " with dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                                return StatusCode::FAILURE;
                            }
                            else {
                                ATH_MSG_VERBOSE("The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())<< " Tube :" << tube << " in layer: " << layer << " in multilayer: " << mdtMl->multilayer()
                                                <<" is inside the chamber volume and is close to the readout and HV. Tube readout is located at: " <<Amg::toString(tubeRO, 2)
                                                <<". Tube HV is located at: " << Amg::toString(tubeHV, 2)
                                                <<". Chamber center located at:  " << Amg::toString(chamber.boundingVolume(**gctx)->center())
                                                << " with chamber dimensions (halfYShort/ halfYLong/ halfX/ halfZ): "
                                                <<chamber.halfYShort()<<"/"<<chamber.halfYLong()<<"/"<<chamber.halfX()<<"/"<<chamber.halfZ());
                            }
                        }
                    }  
                } 
                else if (readOut->detectorType() == ActsTrk::DetectorType::Rpc) {
                        const RpcReadoutElement* rpc = static_cast<const RpcReadoutElement*>(readOut);
                        if (!boundVol->inside(rpc->center(**gctx))){
                            std::cout << rpc->center(**gctx) << std::endl;
                            ATH_MSG_FATAL("The Rpc "<<m_idHelperSvc->toStringDetEl(rpc->identify())<<" is outside the chamber volume");
                            return StatusCode::FAILURE;
                        }
                    }
                else {
                    ATH_MSG_FATAL("The readout element "<<m_idHelperSvc->toStringDetEl(readOut->identify())
                                <<" is not an Mdt or Rpc");
                    return StatusCode::FAILURE;
                }
            }
        }
        return StatusCode::SUCCESS;
    }
}
