/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class SimHitHandle_TrackRecord            //
//                                                            //
//  Description: Handle for Track Records                     //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: May 2008                                 //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef SIMHITHANDLE_TRACKRECORD_H
#define SIMHITHANDLE_TRACKRECORD_H

#include "VP1TrackSystems/SimHitHandleBase.h"
#include "TrackRecord/TrackRecord.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"


class TrackRecord;
class HepMcParticleLink;

class SimHitHandle_TrackRecord : public SimHitHandleBase {
public:

  SimHitHandle_TrackRecord(const TrackRecord*);
  virtual ~SimHitHandle_TrackRecord();

  virtual QString type() const override { return "TrackRecord"; };

  virtual Amg::Vector3D momentumDirection() const override;
  virtual double actualMomentum() const override;
  virtual Amg::Vector3D posStart() const override;
  virtual Amg::Vector3D posEnd() const override;//We fake this one as a point 0.1mm away from posStart, in the momentumDirection.
  virtual double hitTime() const override;
  virtual int actualPDGCodeFromSimHit() const override;
  virtual const HepMcParticleLink& particleLink() const override;
  virtual Trk::TrackParameters * createTrackParameters() const override;

private:

  TrackRecord* m_trkrecord;//We keep this as non-const pointers due to wrong constness in TrackRecord methods!!
  double m_mom;
  Amg::Vector3D m_momdir;
  const HepMcParticleLink* m_link;
};

///////////////
//  Inlines  //
///////////////

inline Amg::Vector3D SimHitHandle_TrackRecord::momentumDirection() const
{
  return m_momdir;
}

inline double SimHitHandle_TrackRecord::actualMomentum() const
{
  return m_mom;
}

inline Amg::Vector3D SimHitHandle_TrackRecord::posStart() const
{
  return Amg::Hep3VectorToEigen(m_trkrecord->GetPosition());
}

inline Amg::Vector3D SimHitHandle_TrackRecord::posEnd() const
{
  return posStart() + 0.1*CLHEP::mm*(momentumDirection());//faking it... but shouldn't matter.
}

inline double SimHitHandle_TrackRecord::hitTime() const
{
  return m_trkrecord->GetTime();
}

inline int SimHitHandle_TrackRecord::actualPDGCodeFromSimHit() const
{
  return m_trkrecord->GetPDGCode();
}

#endif
