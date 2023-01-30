/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFexTowerBuilder  -  description:
//       Builds an eFexTowerContainer from a CaloCellContainer (for supercells) and TriggerTowerContainer (for ppm tile towers)
//                              -------------------
//     begin                : 06 12 2022
//     email                : will@cern.ch
//***************************************************************************/


// MyPackage includes
#include "eFexTowerBuilder.h"

#include "xAODTrigL1Calo/eFexTowerAuxContainer.h"

#include "CaloIdentifier/CaloCell_SuperCell_ID.h"

#undef R__HAS_VDT
#include "ROOT/RVec.hxx"

#include "TFile.h"
#include "TTree.h"


namespace LVL1 {

eFexTowerBuilder::eFexTowerBuilder( const std::string& name, ISvcLocator* pSvcLocator ) : AthReentrantAlgorithm( name, pSvcLocator ){


}

StatusCode eFexTowerBuilder::initialize() {
    ATH_MSG_INFO ("Initializing " << name() << "...");

    CHECK( m_ddmKey.initialize(true) );
    CHECK( m_ttKey.initialize(true) );
    CHECK( m_scellKey.initialize(true) );
    CHECK( m_outKey.initialize(true) );



    return StatusCode::SUCCESS;
}


StatusCode eFexTowerBuilder::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("Executing " << name() << "...");
    setFilterPassed(true, ctx);

    SG::ReadCondHandle<CaloSuperCellDetDescrManager> ddm{m_ddmKey,ctx};

    SG::ReadHandle<xAOD::TriggerTowerContainer> tTowers(m_ttKey,ctx);
    if(!tTowers.isValid()){
        ATH_MSG_FATAL("Could not retrieve collection " << m_ttKey.key() );
        return StatusCode::FAILURE;
    }
    SG::ReadHandle<CaloCellContainer> scells(m_scellKey,ctx);
    if(!scells.isValid()){
        ATH_MSG_FATAL("Could not retrieve collection " << m_scellKey.key() );
        return StatusCode::FAILURE;
    }

    SG::WriteHandle<xAOD::eFexTowerContainer> eTowers = SG::WriteHandle<xAOD::eFexTowerContainer>(m_outKey,ctx);
    ATH_CHECK( eTowers.record(std::make_unique<xAOD::eFexTowerContainer>(),std::make_unique<xAOD::eFexTowerAuxContainer>()) );

    struct TowerSCells {
        std::vector<int> ps;
        std::vector<std::pair<float,int>> l1;
        std::vector<std::pair<float,int>> l2;
        std::vector<int> l3;
        std::vector<int> had;
        std::vector<int> other;
    };

    auto calToFex = [&](int calEt) {
        if (m_mappingVerificationMode.value()) return calEt;
        if(calEt == std::numeric_limits<int>::max()) return 1024; // indicates masked channel
        if(calEt<448) return std::max((calEt&~1)/2+32,1);
        if(calEt<1472) return (calEt-448)/4+256;
        if(calEt<3520) return (calEt-1472)/8+512;
        if(calEt<11584) return (calEt-3520)/32+768;
        return 1020;
    };
    auto etaIndex = [](float eta) { return int( eta*10 ) + ((eta<0) ? -1 : 1); };
    auto phiIndex = [](float phi) { return int( phi*32./ROOT::Math::Pi() ) + (phi<0 ? -1 : 1); };
    std::map<std::pair<int,int>,TowerSCells> towers;
    std::map<std::pair<int,int>,std::set<int>> towerIDs;
    // sort cells by eta, phi, into layers
    // deliberately leaving next line commented to restore for debugging in future
    //TFile f1("test.root","RECREATE");TTree* badChans = new TTree("badChans","badChans");uint32_t offId; badChans->Branch("offId",&offId);uint32_t status; badChans->Branch("status",&status);
    for (auto digi: *scells) {
        Identifier id = digi->ID(); // this is if using supercells

        if (auto elem = ddm->get_element(id); elem && std::abs(elem->eta_raw())<2.5) {
            float eta = elem->eta_raw(); // this seems more symmetric
            int sampling = elem->getSampling();
            if(sampling==6 && ddm->getCaloCell_ID()->region(id)==0 && eta<0) eta-=0.01; // nudge this L2 endcap supercell into correct tower (right on boundary)
            auto val =  round(digi->energy()/(12.5*std::cosh(digi->eta())));

            bool isMasked = ((digi)->provenance()&0x80);

            if( isMasked ) {
                // deliberately leaving following lines commented in case we want to restore for tests
                //badChans.SetPoint(badChans.GetN(),eta,elem->phi_raw());
                //offId =  id.get_identifier32().get_compact();
                //status = bcCont->offlineStatus(id).packedData();
                //badChans->Fill();
                val = std::numeric_limits<int>::max();
            }
            if(m_mappingVerificationMode.value()) {
                int towerid = -1;int slot = -1;bool issplit = false;
                CHECK(m_eFEXSuperCellTowerIdProviderTool->geteTowerIDandslot(id.get_compact(), towerid, slot, issplit));
                towerIDs[std::pair(etaIndex(eta), phiIndex(elem->phi_raw()))].insert(towerid);
                // will use slot+1 as the energy value in this mode
                val = slot+1;
            }
            auto& sc = towers[std::pair(etaIndex(eta),phiIndex(elem->phi_raw()))];
            switch(sampling) {
                case 0: case 4: //lar barrel/endcap presampler
                    sc.ps.push_back(val);
                    break;
                case 1: case 5: //lar barrel/endcap l1
                    sc.l1.push_back({elem->eta(),val}); break;
                case 2: case 6: //lar barrel/endcap l2
                    sc.l2.push_back({elem->eta(),val}); break;
                case 3: case 7: //lar barrel/endcap l3
                    sc.l3.push_back(val); break;
                case 8: case 9: case 10: case 11: //lar hec
                    sc.had.push_back(val); break;
                default:
                    sc.other.push_back(val); break;
            }
        }
    }
    //badChans->Write("badchans");f1.Close(); // comment left here so we know how bad channels were verified
    // add tile energies from TriggerTowers
    for(const auto& tTower : *tTowers) {
        if (std::abs(tTower->eta()) > 1.5) continue;
        if (tTower->sampling() != 1) continue;
        double phi = tTower->phi(); if(phi > ROOT::Math::Pi()) phi -= 2.*ROOT::Math::Pi();
        towers[std::pair(etaIndex(tTower->eta()),phiIndex(phi))].had.push_back(m_mappingVerificationMode.value() ? 11 : tTower->cpET());
    }

    // sort (by increasing eta) l1/l2 sc and handle special cases
    for(auto& [coord,sc] : towers) {
        std::sort(sc.l1.begin(),sc.l1.end());
        std::sort(sc.l2.begin(),sc.l2.end());
        // we have 5 l2 cells @ |eta|=1.45 ... put lowest |eta| one in l3 slot
        if (sc.l2.size()==5) {
            if (coord.first >= 0) {
                sc.l3.push_back(sc.l2.front().second);
                sc.l2.erase(sc.l2.begin()); // remove first
            } else {
                sc.l3.push_back(sc.l2.back().second);
                sc.l2.resize(sc.l2.size()-1); // remove last
            }
        }
        if (std::abs(coord.first)==15) { //|eta| = 1.45
            // in the overlap region it seems like the latome id with highest |eta| is swapped with next highest
            // so to compare we swap the first and second (3rd and 4th are fine) if eta < 0, or 3rd and 4th if eta > 0
            if (coord.first<0) {std::swap(sc.l1.at(0),sc.l1.at(1)); }
            else {std::swap(sc.l1.at(2),sc.l1.at(3));}
        }
        // handle case @ |eta|~1.8-2 with 6 L1 cells
        if (sc.l1.size()==6) {
            sc.l1.at(0).second += int(sc.l1.at(1).second>>1);
            sc.l1.at(1).second = int(sc.l1.at(1).second>>1) + sc.l1.at(2).second;
            sc.l1.at(2).second = sc.l1.at(3).second + int(sc.l1.at(4).second>>1);
            sc.l1.at(3).second = int(sc.l1.at(4).second>>1) + sc.l1.at(5).second;
            sc.l1.resize(4);
        }

        // for |eta|>2.4 there's only 1 l1 sc, to match hardware this should be compared placed in the 'last' l1 input
        if (sc.l1.size()==1) {
            auto tmp = sc.l1.at(0); sc.l1.clear(); sc.l1.resize(4,std::pair(0,0)); sc.l1.at(3) = tmp;
        }

        // create an eFexTower ... use module number = -1, fpga = -1
        std::vector<uint16_t> counts; counts.reserve(11);
        counts.push_back(sc.ps.empty() ? 0 : calToFex(sc.ps.at(0)));
        for(size_t i=0;i<4;i++) counts.push_back((sc.l1.size() > i) ? calToFex(sc.l1.at(i).second) : 0);
        for(size_t i=0;i<4;i++) counts.push_back((sc.l2.size() > i) ? calToFex(sc.l2.at(i).second) : 0);
        counts.push_back(sc.l3.empty() ? 0 : calToFex(sc.l3.at(0)));
        counts.push_back(sc.had.empty() ? 0 : (std::abs(coord.first) <= 15 ? sc.had.at(0) : calToFex(sc.had.at(0))) ); // tile needs no convert


        if(m_mappingVerificationMode.value()) {
            std::cout << coord.first << " " << coord.second << " : ";
            for(auto& id : towerIDs[coord]) std::cout << id << ",";
            std::cout << " : ";
            // counts(-1) should typically be just 0,1,2,3,...
            // -1 indicates no input
            // exceptions are:
            //   |eta|=25 : the 4th l1 will be a "0" (treating as ps)
            //   |eta|=18,19,20 :
            for(size_t i=0;i<counts.size();i++) {
                std::cout << counts[i]-1 << ",";
            }
            std::cout << std::endl;
            continue;
        }

        eTowers->push_back( std::make_unique<xAOD::eFexTower>() );
        eTowers->back()->initialize( ( (coord.first<0 ? 0.5:-0.5) + coord.first)*0.1 ,
                                     ( (coord.second<0 ? 0.5:-0.5) + coord.second)*ROOT::Math::Pi()/32,
                                     counts,
                                     -1, /* module number */
                                     -1, /* fpga number */
                                     0,0 /* status flags ... could use to indicate which cells were actually present?? */);

    }


    return StatusCode::SUCCESS;

}

} // LVL1 Namespace