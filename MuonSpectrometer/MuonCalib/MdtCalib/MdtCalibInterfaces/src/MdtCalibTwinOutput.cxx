/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MdtCalibInterfaces/MdtCalibTwinOutput.h>

using MdtDriftCircleStatus = MdtCalibTwinOutput::MdtDriftCircleStatus;
std::ostream& operator<<(std::ostream& ostr, const MdtCalibTwinOutput& output) {
    ostr <<"primary tdc: "<<output.primaryTdc()<<",";
    ostr<<"twin td: "<<output.twinTdc()<<", ";
    ostr<<"primary adc: "<<output.primaryAdc()<<", ";
    ostr<<"twin adc: "<<output.twinAdc()<<", ";
    ostr<<"primary drift R: "<<output.primaryDriftR()<<" pm "<<output.uncertPrimaryR()<<", ";
    ostr<<"twin drift R: "<<output.twinDriftR()<<" pm "<<output.uncertTwinR()<<", ";
    ostr<<"local Z: "<<output.locZ()<<" pm "<<output.sigmaZ()<<", ";    
    return ostr;
}
MdtCalibTwinOutput::MdtCalibTwinOutput(const MdtCalibInput& primHit,
                                       const MdtCalibInput& twinHit,
                                       const MdtCalibOutput& primRes,
                                       const MdtCalibOutput& twinRes):
    m_primId{primHit.identify()},
    m_primAdc{primHit.adc()},
    m_primTdc{primHit.tdc()},
    m_primDriftR{primRes.driftRadius()},
    m_primDriftUncert{primRes.driftRadiusUncert()},    
    m_primStatus{primRes.status()},
    m_twinId{twinHit.identify()},
    m_twinAdc{twinHit.adc()},
    m_twinTdc{twinHit.tdc()},
    m_twinDriftR{twinRes.driftRadius()},
    m_twinDriftUncert{twinRes.driftRadiusUncert()},
    m_twinStatus{twinRes.status()}{}

int MdtCalibTwinOutput::primaryAdc() const{ return m_primAdc; }
int MdtCalibTwinOutput::primaryTdc() const { return m_primTdc; }

int MdtCalibTwinOutput::twinAdc() const{ return m_twinAdc; }
int MdtCalibTwinOutput::twinTdc() const { return m_twinTdc; }

Identifier MdtCalibTwinOutput::primaryID() const { return m_primId; }
Identifier MdtCalibTwinOutput::twinID() const { return m_twinId; }

void MdtCalibTwinOutput::setLocZ(const double locZ, const double locZuncert) {
    m_locZ = locZ;
    m_locZuncert = locZuncert;
}

double MdtCalibTwinOutput::locZ() const { return m_locZ; }
double MdtCalibTwinOutput::sigmaZ() const { return m_locZuncert; }
double MdtCalibTwinOutput::primaryDriftR() const { return m_primDriftR; }
double MdtCalibTwinOutput::twinDriftR() const { return m_twinDriftR;}
double MdtCalibTwinOutput::uncertPrimaryR() const { return m_primDriftUncert; }
double MdtCalibTwinOutput::uncertTwinR() const { return m_twinDriftUncert; }
MdtDriftCircleStatus MdtCalibTwinOutput::primaryStatus() const {return m_primStatus;}
MdtDriftCircleStatus MdtCalibTwinOutput::twinStatus() const {return m_twinStatus;}