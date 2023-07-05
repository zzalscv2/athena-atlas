/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonReadoutGeometry/GlobalUtilities.h>
#include <iostream>
int main (int , char**){
    for (std::string_view stName : {"T1F", "T1E" , "T2F", "T2E", "T3F", "T3E",
                                   "T4E"}){

        int etaRange{1};
        if (stName[1] != '4' && stName[2] == 'E'){ 
            etaRange = 5 - (stName[1] == '1'); 
        }
        for (int phiAmdb =1 ; phiAmdb <= 8; ++phiAmdb) {
            for (int eta = -etaRange; eta<= etaRange; ++eta) {
            if (eta == 0) continue;
            
                const int phi = MuonGM::stationPhiTGC(stName,phiAmdb, eta);
                const int backAmdbPhi = MuonGM::amdbPhiTGC(stName,phi, eta);
                if (backAmdbPhi == phiAmdb) continue;
                std::cout<<"AMDB -> Identifier -> AMDB conversion failed for stName: "<<stName<<", eta: "<<eta<<", phi (AMDB): "<<phiAmdb<<", phi(Identifier): "<<phi<<
                ", back translation: "<<backAmdbPhi<<std::endl;
                return EXIT_FAILURE;
             }
        }
    }

    return EXIT_SUCCESS;
}

