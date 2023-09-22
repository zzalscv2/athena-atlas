/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MDTCALIBINTEFACES_MDTCALIBTWINOUTPUT_H
#define MDTCALIBINTEFACES_MDTCALIBTWINOUTPUT_H

#include <MdtCalibInterfaces/MdtCalibInput.h>
#include <MdtCalibInterfaces/MdtCalibOutput.h>


class MdtCalibTwinOutput{
    public:
      using MdtDriftCircleStatus = MdtCalibOutput::MdtDriftCircleStatus;
       MdtCalibTwinOutput () = default;
       
       MdtCalibTwinOutput(const MdtCalibInput& primHit,
                          const MdtCalibInput& twinHit,
                          const MdtCalibOutput& primRes,
                          const MdtCalibOutput& twinRes);
       
       int primaryAdc() const;
       int twinAdc() const;

       int primaryTdc() const;
       int twinTdc() const;

       double primaryDriftR() const;
       double twinDriftR() const;

       double uncertPrimaryR() const;
       double uncertTwinR() const;
       
       
       Identifier primaryID() const;
       Identifier twinID() const;


       void setLocZ(const double locZ, const double locZuncert);

       double locZ() const;
       double sigmaZ() const;

       MdtDriftCircleStatus primaryStatus() const;
       MdtDriftCircleStatus twinStatus() const;

    private:
      Identifier m_primId{};
      int m_primAdc{0};
      int m_primTdc{0};
      double m_primDriftR{0.};
      double m_primDriftUncert{0.};
      MdtDriftCircleStatus m_primStatus{MdtDriftCircleStatus::MdtStatusUnDefined};

      Identifier m_twinId{};
      int m_twinAdc{0};
      int m_twinTdc{0};
      double m_twinDriftR{0.};
      double m_twinDriftUncert{0.};
      MdtDriftCircleStatus m_twinStatus{MdtDriftCircleStatus::MdtStatusUnDefined};

      double m_locZ{0.};
      double m_locZuncert{0.};
};

std::ostream& operator<<(std::ostream& ostr, const MdtCalibTwinOutput& output);
#endif