/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 global stuff
 -----------------------------------------
 ***************************************************************************/

#include "MuonReadoutGeometry/GlobalUtilities.h"

#include <charconv>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>

namespace MuonGM {

    std::string buildString(int i, int ncha) {
        if (ncha <= 0) {
            return std::to_string(i);
        } else {
            int ij = i;
            std::ostringstream ostigg;
            for (int j = ncha - 1; j >= 0; j--) {
                ostigg << int(ij / std::pow(10.0, j));
                ij = int(ij % int(std::pow(10.0, j)));
            }
            return ostigg.str();
        }
    }

    int strtoint(std::string_view str, unsigned int istart, unsigned int length) {
        std::string_view s(str.substr(istart, length));
        int result = -999;
        std::from_chars(s.data(), s.data() + s.size(), result);
        return result;
    }

    int stationPhiTGC(std::string_view stName, int fi, int zi_input) {
        // fi and zi_imput are the amdb indices (1, ... 8) and (-8, 8) are their ranges
        std::string_view stName3 = stName.substr(0, 3);
        int stphi = 0;

        int zi = std::abs(zi_input);
        --fi;  // start from 0
        
        if (stName3 == "T4E") {
            switch (fi) {
                case 0: stphi = zi; break;
                case 1: stphi = zi + 3; break;
                case 2: stphi = zi + 6; break;
                case 3:
                    stphi = zi + 9;  // zi are numbered in order, i.e. 1 and 2
                    break;
                case 4: stphi = zi + 11; break;
                case 5: stphi = zi + 14; break;
                case 6: stphi = zi + 16; break;
                case 7: stphi = zi + 19; break;
                default: stphi = 0;
            }
            // minumum stPhi at phi 0
            stphi = stphi - 1;
            if (stphi < 1) stphi = stphi + 21;
        } else {
            int nch = 3;
            if (stName[2] == 'E') nch = 6;
            int fioff = std::abs(zi);
            if (fioff > 3 && stName[2] == 'F') fioff = fioff - 3;
            // minumum stPhi at phi 0
            if (stName[2] == 'F')
                fioff = fioff - 1;
            else
                fioff = fioff - 2;
            stphi = fi * nch + fioff;
            if (stphi < 1) {
                if (stName[2] == 'F')
                    stphi = stphi + 24;
                else
                    stphi = stphi + 48;
            }
        }
        return stphi;
    }
    int amdbPhiTGC(std::string_view stName, int phiIndex, int eta_index) {        
        if (stName != "T4E") {
            const int nch = 3*( stName[2] == 'E' ? 2 : 1);
            int fioff = std::abs(eta_index);
            if (fioff > 3 && stName[2] == 'F') fioff -= 3;
            if (stName[2] == 'F') fioff -=1;
            else fioff-=2;            
            int amdbPhi = ( (phiIndex - fioff) / nch + 1 ) % 8;
            if (amdbPhi <= 0) amdbPhi = 8;
            return amdbPhi;
        }
        /// For T4E the devil was certainly involved in the game. The backward conversion does not make
        /// any sense at all. Hardcode it...
        switch (phiIndex) {            
            case 21: return 1;
            case 3 : return 2;
            case 6: return 3;
            case 9: return 4;
            case 11: return 5;
            case 14: return 6;
            case 16: return 7;
            case 19: return 8;
            default: return 0;
        }
        return 0;
    }


}  // namespace MuonGM
