/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file TileDetectorFactory.h
 *
 * @brief Definition of TileDetectorFactory class
 *
 * @author Vakho Tsulaia
 *
 */
#ifndef TileDetectorFactory_h
#define TileDetectorFactory_h 1

#include "GeoModelKernel/GeoVDetectorFactory.h"
#include "TileDetDescr/TileDetDescrManager.h"
#include "TileGeoModel/TileSwitches.h"

class MsgStream;
class StoreGateSvc;


class TileDetectorFactory : public GeoVDetectorFactory
{
 public:

  /** Constructor */
  TileDetectorFactory(StoreGateSvc *pDetStore, TileDetDescrManager * manager,
                      const TileSwitches & switches, MsgStream *log);

  /** Destructor */
  ~TileDetectorFactory();

  /** Creation of Tile geometry */
  virtual void create(GeoPhysVol *world);

  /** Access function to TileDetDescr geometry data */
  virtual const TileDetDescrManager * getDetectorManager() const { return m_detectorManager; }

 private:

  /** Detector pointer to Store Gate service */
  StoreGateSvc              *m_detectorStore;

  /** Detector pointer to TileDetDescrManager */
  TileDetDescrManager       *m_detectorManager;

  /** Get message SVC */
  MsgStream                 *m_log;

  /** all switches */
  TileSwitches m_switches;

  /** Flag for activation verbose level for debugging */
  bool                       m_verbose;
};

#endif
