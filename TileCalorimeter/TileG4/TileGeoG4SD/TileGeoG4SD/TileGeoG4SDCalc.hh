/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

//************************************************************
// Filename : TileGeoG4SDCalc.hh
// Author   : Sergey Karpov <Sergey.Karpov@cern.ch>
// Created  : July, 2013
//
// DESCRIPTION:
//   Sensitive Detector initialisation for TileCal G4 simulations
//   of both ordinary Hits and CalibHits
//
// HISTORY:
//   Nov 2013 - Work with U-shape was added (Sasha Solodkov)
//
//************************************************************

#ifndef TTileGeoG4SDCalc_H
#define TileGeoG4SDCalc_H 

#include "FadsSensitiveDetector/FadsSensitiveDetector.h"
#include "CaloIdentifier/TileID.h"
#include "CaloIdentifier/TileTBID.h"

#include <vector>
#include <memory>

using namespace FADS;

class TileGeoG4LookupBuilder;

class G4Step;
class G4HCofThisEvent;
class G4String;

class TileRow;
class TileGeoG4Section;
class TileGeoG4Cell;

class IMessageSvc;
class StoreGateSvc;
class MsgStream;


struct TileMicroHit
{
    Identifier  pmt_up,     pmt_down;
    G4double    e_up,       e_down;
    double      time_up,    time_down;
    //int         period,     tilerow; // prepared for future use
};



class TileGeoG4SDCalc {
public:
    static TileGeoG4SDCalc * instance(bool is_ctb=false);
    TileGeoG4SDCalc (bool is_ctb);
   ~TileGeoG4SDCalc ();

    G4bool       FindTileScinSection (const G4Step*);
    G4bool       MakePmtEdepTime     (const G4Step*);
    TileMicroHit GetTileMicroHit     (const G4Step*);

    G4bool ManageScintHit ();
    void   CreateScintHit (int);
    void   UpdateScintHit (int);

    /** @brief function to give PMT responce as a function of distance
        from tile center in mm (along phi direction) */
    static G4double Tile_1D_profileAsym    (int row, G4double x, G4double y, int PMT, int nDetector, int nSide); //asymmetric U-shape
    static G4double Tile_1D_profileSym     (int row, G4double x, G4double y, int PMT, int nDetector, int nSide); //Single Lookup table is included; average of central 17 bins is normalized to 0.5
    static G4double Tile_1D_profileFunc    (int row, G4double x, G4double y, int PMT, int nDetector, int nSide); //Single Lookup table is included; average of all bins is normalized to 0.5
    static G4double Tile_1D_profileRescaled(int row, G4double x, G4double y, int PMT, int nDetector, int nSide);
    static G4double Tile_1D_profileRescaled_zeroBins(int row, G4double x, G4double y, int PMT, int nDetector, int nSide, int nZeroBins=0);

// the default function which is used in simulation in the case Ushape=1
#define Tile_1D_profile Tile_1D_profileRescaled

    StoreGateSvc* m_sgSvc;
    StoreGateSvc* m_detStore;

    TileGeoG4LookupBuilder* m_lookup;
    TileGeoG4Section*       _section;
    TileGeoG4Cell*          _cell;

    //variables to identify Hit objects
    int nModule;
    int nDetector;
    int nTower;
    int nSample;
    int nSide;

    Identifier m_invalid_id;
    Identifier pmtID_up,        pmtID_down;
    G4double   edep_up,         edep_down;
    double     scin_Time_up,    scin_Time_down;

    /** @brief true if producing calibration hits signed with primary particle ID */
    bool   m_doCalibHitParticleID;

private:
    // Private copy-constructor
    TileGeoG4SDCalc& operator=( const TileGeoG4SDCalc& ) = delete;
    TileGeoG4SDCalc ( const TileGeoG4SDCalc& ) = delete;
    IMessageSvc* m_msgSvc;
    MsgStream*   m_log;

    bool    m_debug;
    bool    m_verbose;

    int     nrOfPMT;
    int     tilesize;
    int     tileperiod;
    bool    is_negative;
    double  totalTime_up;
    double  totalTime_down;

    /** @brief granularity in time for hits */
    double m_deltaT;
    std::vector<double> m_deltaTvec;

    /** @brief max allowed time for hits */
    double m_timeCut;

    /** selected U-shape function (!=0 means U-shape enabled) */
    int m_Ushape;

    /** @brief set to true to apply Birks' law */
    bool   m_doBirk;

    /** @brief function to calculate Birks correction */
    G4double BirkLaw(const G4Step* aStep) const;

    /** @brief true if hits in every time row stored separetely  in addtion to normal hits */
    bool   m_doTileRow;

    /** @brief Structure holding the attenuation lengths */
    std::unique_ptr<TileRow> m_row;

    /** @brief true if time of flight correction is applied
        for particles coming from ATLAS center */
    bool   m_doTOFCorr;

    /** @brief pointer to TileID helper class */
    const  TileID* m_tileID;

    /** @brief pointer to TileTBID helper class */
    const  TileTBID* m_tileTBID;

    /** @brief hits in different tile rows are shifted in time by this amount of ns */
    double m_tilesize_deltaT;

    /** @brief time for hits which are above m_timeCut threshold
        it is equal to m_tilesize_deltaT - m_deltaT */
    double m_lateHitTime;

    /** @brief function to provide correct deltaT bin width for given time */
    inline double deltaT(double time) const {
    unsigned int i=0;
    double delta = m_deltaTvec[i++];
        while (i<m_deltaTvec.size()) {
            if (time>m_deltaTvec[i++] && time<m_deltaTvec[i++]) break;
            delta = m_deltaTvec[i++];
        }
    return delta;
    }

};  //class TileGeoG4SDCalc

#endif

