/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonCondData/Defs.h>

namespace MuonCond {

DcsFsmState getFsmStateEnum(const std::string& fsmState){
    if(fsmState=="UNKNOWN"  ) return DcsFsmState::UNKNOWN;
    if(fsmState=="ON"       ) return DcsFsmState::ON;
    if(fsmState=="OFF"      ) return DcsFsmState::OFF;
    if(fsmState=="STANDBY"  ) return DcsFsmState::STANDBY;
    if(fsmState=="DEAD"     ) return DcsFsmState::DEAD;
    if(fsmState=="UNPLUGGED") return DcsFsmState::UNPLUGGED;
    if(fsmState=="RAMP_UP"  ) return DcsFsmState::RAMP_UP;
    if(fsmState=="RAMP_DOWN") return DcsFsmState::RAMP_DOWN;
    if(fsmState=="TRIP"     ) return DcsFsmState::TRIP;
    if(fsmState=="RECOVERY" ) return DcsFsmState::RECOVERY;
    if(fsmState=="LOCKED"   ) return DcsFsmState::LOCKED;
    return DcsFsmState::NONE;
}
std::string getFsmStateStrg(DcsFsmState fsmState) {
    switch (fsmState) {
        case DcsFsmState::UNKNOWN:
            return "UNKNOWN";
        case DcsFsmState::ON:
            return "ON";
        case DcsFsmState::OFF:
            return "OFF";
        case DcsFsmState::STANDBY:
            return "STANDBY";
        case DcsFsmState::DEAD:
            return "DEAD";
        case DcsFsmState::UNPLUGGED:
            return "UNPLUGGED";
        case DcsFsmState::RAMP_UP:
            return "RAMP_UP";
        case DcsFsmState::RAMP_DOWN:
            return "RAMP_DOWN";
        case DcsFsmState::TRIP:
            return "TRIP";
        case DcsFsmState::RECOVERY:
            return "RECOVERY" ;
        case DcsFsmState::LOCKED:
            return "LOCKED";
        case DcsFsmState::NONE:
            return "NONE";
    }
    return "NONE";
}

}