/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkTrackSummary/TrackSummary.h"
#include "TrkTrackSummary/MuonTrackSummary.h"
#include "TrkEventTPCnv/TrkTrackSummary/TrackSummaryCnv_p2.h"

void TrackSummaryCnv_p2::dbgPrint( const Trk::TrackSummary *t){

    std::cout << "-------------------------------------------" <<std::endl;
    std::cout << "m_idHitPattern:\t" << t->m_idHitPattern << std::endl;

    std::cout << " std::vector m_information size: "<< t->m_information.size() <<std::endl;
    for (int i : t->m_information) std::cout<<"\t "<<i;
    std::cout<<std::endl;

    if(t->m_muonTrackSummary){
        std::cout << " m_muonTrackSummary->m_nscatterers: "<< t->m_muonTrackSummary->nscatterers() <<std::endl;
        std::cout << " m_muonTrackSummary->m_npseudoMeasurements: "<< t->m_muonTrackSummary->npseudoMeasurements() <<std::endl;
        std::cout << " std::vector m_muonTrackSummary->m_chamberHitSummary size: "<< t->m_muonTrackSummary->chamberHitSummary().size() <<std::endl;
        for (const Trk::MuonTrackSummary::ChamberHitSummary& s :
               t->m_muonTrackSummary->chamberHitSummary())
        {
            std::cout<<"\t m_chId           "<<s.chamberId()        <<std::endl;
            std::cout<<"\t m_isMdt          "<<s.isMdt()            <<std::endl;
            std::cout<<"\t m_first.nhits      "<<s.mdtMl1().nhits     <<std::endl;
            std::cout<<"\t m_first.nholes     "<<s.mdtMl1().nholes    <<std::endl;
            std::cout<<"\t m_first.noutliers  "<<s.mdtMl1().noutliers <<std::endl;
            std::cout<<"\t m_first.ndeltas    "<<s.mdtMl1().ndeltas   <<std::endl;
            std::cout<<"\t m_first.ncloseHits "<<s.mdtMl1().ncloseHits<<std::endl;
            std::cout<<"\t m_second.nhits     "<<s.mdtMl2().nhits     <<std::endl;
            std::cout<<"\t m_second.nholes    "<<s.mdtMl2().nholes    <<std::endl;
            std::cout<<"\t m_second.noutliers "<<s.mdtMl2().noutliers <<std::endl;
            std::cout<<"\t m_second.ndeltas   "<<s.mdtMl2().ndeltas   <<std::endl;
            std::cout<<"\t m_second.ncloseHits"<<s.mdtMl2().ncloseHits<<std::endl;
        }
    }

}


void TrackSummaryCnv_p2::persToTrans( const Trk::TrackSummary_p2 *persObj, Trk::TrackSummary *transObj, MsgStream &/*log*/ ){
    transObj->m_information       = persObj->m_information;
    if (transObj->m_information.size() < Trk::numberOfTrackSummaryTypes)
      transObj->m_information.resize(Trk::numberOfTrackSummaryTypes,
                                     Trk::TrackSummary::SummaryTypeNotSet);

    if (persObj->m_information.size() <= Trk::numberOfInnermostPixelLayerHits) {
      transObj->m_information[Trk::numberOfInnermostPixelLayerHits] =
        transObj->m_information[Trk::numberOfBLayerHits];
    }

    if (persObj->m_information.size() <= Trk::legacy_numberOfInnermostPixelLayerSharedHits) {
      transObj->m_information[Trk::legacy_numberOfInnermostPixelLayerSharedHits] =
        transObj->m_information[Trk::legacy_numberOfBLayerSharedHits];
    }


    transObj->m_idHitPattern      = persObj->m_idHitPattern;

    std::size_t s = persObj->m_muonTrackSummary.size();
    if(s){  // MUON TRACK SUMMARY
        Trk::MuonTrackSummary *ts= new Trk::MuonTrackSummary();
        std::vector<unsigned int>::const_iterator i = persObj->m_muonTrackSummary.begin();

        ts->m_nscatterers = *i; ++i;
        ts->m_npseudoMeasurements = *i; ++i;

        size_t size=(s-2)/12;
        ts->m_chamberHitSummary.reserve(size);

        for (size_t sc=0;  sc<size ; ++sc ){
            ts->m_chamberHitSummary.emplace_back(Identifier(*i), bool(*(++i)));
            ++i;
            ts->m_chamberHitSummary[sc].m_first.nhits      =(*i);++i;
            ts->m_chamberHitSummary[sc].m_first.nholes     =(*i);++i;
            ts->m_chamberHitSummary[sc].m_first.noutliers  =(*i);++i;
            ts->m_chamberHitSummary[sc].m_first.ndeltas    =(*i);++i;
            ts->m_chamberHitSummary[sc].m_first.ncloseHits =(*i);++i;
            ts->m_chamberHitSummary[sc].m_second.nhits     =(*i);++i;
            ts->m_chamberHitSummary[sc].m_second.nholes    =(*i);++i;
            ts->m_chamberHitSummary[sc].m_second.noutliers =(*i);++i;
            ts->m_chamberHitSummary[sc].m_second.ndeltas   =(*i);++i;
            ts->m_chamberHitSummary[sc].m_second.ncloseHits=(*i);++i;
        }

        transObj->m_muonTrackSummary.reset(ts);
    }

    // dbgPrint(transObj);
}


void TrackSummaryCnv_p2::transToPers( const Trk::TrackSummary    *transObj, Trk::TrackSummary_p2 *persObj, MsgStream & /*log*/ ){

    // dbgPrint(transObj);

    persObj->m_information       = transObj->m_information;
    persObj->m_idHitPattern      = transObj->m_idHitPattern;

    if(transObj->m_muonTrackSummary){  // MUON TRACK SUMMARY

        size_t size = transObj->m_muonTrackSummary->m_chamberHitSummary.size();

        persObj->m_muonTrackSummary.reserve(2 + 12 * size);
        persObj->m_muonTrackSummary.push_back(transObj->m_muonTrackSummary->m_nscatterers);
        persObj->m_muonTrackSummary.push_back(transObj->m_muonTrackSummary->m_npseudoMeasurements);

        for (const Trk::MuonTrackSummary::ChamberHitSummary& s :
               transObj->m_muonTrackSummary->chamberHitSummary())
        {
            persObj->m_muonTrackSummary.push_back(s.chamberId().get_identifier32().get_compact() );
            persObj->m_muonTrackSummary.push_back(s.isMdt()  ); // these are just bits and should be compressed by us
            persObj->m_muonTrackSummary.push_back(s.mdtMl1().nhits      );
            persObj->m_muonTrackSummary.push_back(s.mdtMl1().nholes     );
            persObj->m_muonTrackSummary.push_back(s.mdtMl1().noutliers  );
            persObj->m_muonTrackSummary.push_back(s.mdtMl1().ndeltas    );
            persObj->m_muonTrackSummary.push_back(s.mdtMl1().ncloseHits );
            persObj->m_muonTrackSummary.push_back(s.mdtMl2().nhits      );
            persObj->m_muonTrackSummary.push_back(s.mdtMl2().nholes     );
            persObj->m_muonTrackSummary.push_back(s.mdtMl2().noutliers  );
            persObj->m_muonTrackSummary.push_back(s.mdtMl2().ndeltas    );
            persObj->m_muonTrackSummary.push_back(s.mdtMl2().ncloseHits );
        }

    }
}
