/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


/////////////////////////////////////////////////////////////////
//                                                             //
//  Header file for class TouchedMuonChamberHelper             //
//                                                             //
//  Description: For keeping track of muon chambers with       //
//               (shown) objects in them, and for ensuring     //
//               that systems emit signals as appropriate      //
//                                                             //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)   //
//  Initial version: January 2008                              //
//                                                             //
/////////////////////////////////////////////////////////////////

#ifndef TOUCHEDMUONCHAMBERHELPER_H
#define TOUCHEDMUONCHAMBERHELPER_H

// GeoModel
#include "GeoPrimitives/GeoPrimitives.h"
//
#include "GeoModelKernel/GeoVPhysVol.h"

#include <QObject>
#include <set>

class TouchedMuonChamberHelper : public QObject {

  Q_OBJECT

public:

  TouchedMuonChamberHelper(QObject * parent = 0);
  virtual ~TouchedMuonChamberHelper();

  void incrementNumberOfObjectsForPV(const GeoPVConstLink& chamberPV);
  void decrementNumberOfObjectsForPV(const GeoPVConstLink& chamberPV);

  bool isTouchedByTrack(const GeoPVConstLink& chamberPV);//!< Returns true if the passed chamber link has a track or segment.
  void eraseEventData();

  void updateTouchedByTracks(const std::set<GeoPVConstLink>&);

signals:
  void touchedMuonChambersChanged(const std::set<GeoPVConstLink>&);
  void muonChambersTouchedByTracksChanged(void);

private Q_SLOTS:
  void checkForChangeInTouchedChambers();

private:
  class Imp;
  Imp * m_d;

};

#endif
