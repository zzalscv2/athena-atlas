/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigT1RPChardware_SectorLogic_H
#define TrigT1RPChardware_SectorLogic_H

#include <array>
#include <cctype>
#include <cerrno>
#include <cfloat>
#include <climits>
#include <clocale>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>

#include "MuonCablingTools/BaseObject.h"
#include "TrigT1RPChardware/Lvl1Def.h"
#include "TrigT1RPChardware/NOBXS.h"

// ### DataFromPad ###
struct DataFromPad {
    int bcid{0};
    int r{0};
    int oveta{0};
    int ovphi{0};
    int opl{0};
    int pt{0};
    int roi{0};
    int ntrig{0};
    int sign{0};
    DataFromPad() = default;
};

/***************************************************************************/

// ### OutputFromSectorLogic ###
// bit map:
//  bit pos.  information
//
//     0      >2 candidates in a sector
//    1-5     ROI1
//    6-7     Reserved1
//    8-9     OVL1
//    10-14   ROI2
//    15-16   Reserved2
//    17-18   OVL2
//    19-21   Pt1
//    22-24   Pt2
//     25     >1 candidate in ROI1
//     26     >1 candidate in ROI2
//    27-29   BCID
//     30     Candidate1 sign
//     31     Candidate2 sign
//
//
class OutputFromSectorLogic {
public:
    int ntrig{0};
    int roi1{0};
    int pad1{0};
    int r1{0};
    int ovf1{0};
    int ove1{0};
    int roi2{0};
    int pad2{0};
    int r2{0};
    int ovf2{0};
    int ove2{0};
    int pt1{0};
    int pt2{0};
    int ntrig1{0};
    int ntrig2{0};
    int bcid{0};
    int sign1{0};
    int sign2{0};
    OutputFromSectorLogic() = default;
    friend std::ostream &operator<<(std::ostream &stream, OutputFromSectorLogic &o);
};
/*****************************************************************************/
// ### InternalRegister ###
class InternalRegister {
public:
    std::array<DataFromPad, 8> pad{};
    CMAword tile{};
    OutputFromSectorLogic out;
    InternalRegister() = default;
    friend std::ostream &operator<<(std::ostream &stream, InternalRegister &o);
};
/******************************************************************************/
// ### SectorLogic ###
class SectorLogic : public BaseObject {
    // identificazione sector logic di Aleandro
    int m_run{0};
    int m_event{0};
    CMAword m_debug{};
    ubit16 m_subsys{0};
    ubit16 m_sect{0};

    /*----------------------------------------------------------------------------

    Formato dei parametri di configurazione della sector logic

    EnableTCCheckLow	8x3		8 pad x 3 soglie basse
    EnableTCCheckHigh	8x3		8 pad x 3 soglie alte
    SetTCCheck		8x6x32		8 pad x 6 soglie x 32 segnali dal TC

    EnableOPLCheck	8x3		8 pad x 3 soglie basse
    SetOPLCheck		8x3x8		8 pad x 3 soglie basse x 8 flag di OPL

    Tutti i numeri riportati sotto sono a 32 bit tranne SetOPLCheck pad[0..7]

                         p .... p
                         d .... d
                         7 .... 0
    EnableTCCheckLow   : 00000000		  -> diviso in 8 pezzi da 4 bit -> 8x3
    EnableTCCheckHigh  : 00000000		  -> diviso in 8 pezzi da 4 bit -> 8x3

                           th6      ...      ...      ...      ...      th1
    SetTCCheck pad[0]  : 00000000 00000000 00000000 00000000 00000000 00000000  -> 6x32 moltiplicando per 8 pad
    SetTCCheck pad[1]  : 00000000 00000000 00000000 00000000 00000000 00000000  -> ottengo 8x6x32
    SetTCCheck pad[2]  : 00000000 00000000 00000000 00000000 00000000 00000000
    SetTCCheck pad[3]  : 00000000 00000000 00000000 00000000 00000000 00000000
    SetTCCheck pad[4]  : 00000000 00000000 00000000 00000000 00000000 00000000
    SetTCCheck pad[5]  : 00000000 00000000 00000000 00000000 00000000 00000000
    SetTCCheck pad[6]  : 00000000 00000000 00000000 00000000 00000000 00000000
    SetTCCheck pad[7]  : 00000000 00000000 00000000 00000000 00000000 00000000

                         p .... p
                         d .... d
                         7 .... 0
    EnableOPLCheck     : 00000000		  -> diviso in 8 pezzi da 4 bit -> 8x3

                         t  ..  t
                         h  ..  h
                         3  ..  1
    SetOPLCheck pad[0] : 00 00 00		  -> 3x8 moltiplicando per 8 pad
    SetOPLCheck pad[1] : 00 00 00		  -> ottengo 8x3x8
    SetOPLCheck pad[2] : 00 00 00
    SetOPLCheck pad[3] : 00 00 00
    SetOPLCheck pad[4] : 00 00 00
    SetOPLCheck pad[5] : 00 00 00
    SetOPLCheck pad[6] : 00 00 00
    SetOPLCheck pad[7] : 00 00 00

    ----------------------------------------------------------------------------*/

    // if SetTCCheck[i][j][k] = 1 the k Tilecal signal is mapped to the
    // j threshold of the i PAD
    // if SetTCCheck[i][j][k] = 0 no mapping is done
    // note thet in this way is possible to map a given TileCal signal to more
    // that one threshold and PAD
    CMAword m_SetTCCheck[8][6];

    // once the geometry of the mapping is specified EnableTCCheck[i][j]
    // tells if the mapping must be done or not
    // if EnableTCCheck[i][j] = 1 a muon candidate in the i PAD with the
    // j threshold is confirmed only if there was an energy deposition in
    // at least one of the mapped zones of the TileCal
    // if EnableTCCheck[i][j] = 0 a muon candidate is always confirmed
    CMAword m_EnableTCCheckLow{0x00000000};
    CMAword m_EnableTCCheckHigh{0x00000000};

    // if SetOPLCheck[i][j][k] = 1 the k OPLFlag signal is mapped to the
    // j low-Pt threshold of the i PAD
    // if SetOPLCheck[i][j][k] = 0 no mapping is done
    // note thet in this way is possible to map a given OPLFlag signal to more
    // that one threshold and PAD
    ubit16 m_SetOPLCheck[8][3];

    // once the geometry of the mapping is specified EnableOPLCheck[i][j]
    // tells if the mapping must be done or not
    // if EnableOPLCheck[i][j] = 1 a muon candidate in the i PAD with the
    // j low-Pt threshold is confirmed only if there was track in one of
    // the PADs belonging to a given Sector
    // if EnableOPLCheck[i][j] = 0 a low-Pt muon candidate is always confirmed
    CMAword m_EnableOPLCheck{0x00000000};

    // input and output data
    DataFromPad m_InFromPad[NOBXS][8];
    std::array<CMAword, NOBXS> m_InFromTileCal{0};
    std::array<OutputFromSectorLogic, NOBXS> m_OutFromSectorLogic{};

    // internal registers of the various steps of the Sector Logic pipeline
    // 1st step registers
    // low Pt filter
    std::array<InternalRegister, NOBXS> m_LowPtFilter_in{};
    std::array<InternalRegister, NOBXS> m_LowPtFilter_out{};
    // 2nd step registers
    std::array<InternalRegister, NOBXS> m_TileCalConfirm_in{};
    std::array<InternalRegister, NOBXS> m_TileCalConfirm_out{};
    // 3rd step registers
    std::array<InternalRegister, NOBXS> m_SolveEtaOverlap_in{};
    std::array<InternalRegister, NOBXS> m_SolveEtaOverlap_out{};
    // 4th step registers
    std::array<InternalRegister, NOBXS> m_SortHighest_in{};
    std::array<InternalRegister, NOBXS> m_SortHighest_out{};
    // 5th step registers
    std::array<InternalRegister, NOBXS> m_Sort2ndHighest_in{};
    std::array<InternalRegister, NOBXS> m_Sort2ndHighest_out{};

    ubit16 m_nBunMax{NOBXS};
    // M.Corradi 1/3/2010
    bool m_oldSimulation{false};

public:
    // constructor and destructor
    SectorLogic(int run, int event, CMAword debug, ubit16 subsys, ubit16 sect, bool oldSimulation);
    ~SectorLogic(void);

    ubit16 numberOfBunches() const { return m_nBunMax; };
    //    ubit16 sectorAddress(){return m_sector;};

    CMAword outputToMuCTPI(int deltaBC = 0);
    // old version
    // CMAword outputToMuCTPI(ubit16 bunchID=BCZERO);

    // init and check
    void init(void);
    void check(void);

    // tile cal check and opl check configuration
    void LoadTCCheck(CMAword EnableTCCheckLow_in, CMAword EnableTCCheckHigh_in, CMAword SetTCCheck_in[8][6]);
    void LoadOPLCheck(CMAword EnableOPLCheck_in, ubit16 SetOPLCheck_in[8][3]);

    // input and output in my format: only for debug
    void dbginput(ubit16 bx, DataFromPad from_pad[8], CMAword from_tile_cal);
    OutputFromSectorLogic dbgoutput(ubit16 bx);

    // input and output in standard format
    void load(ubit16 padAdd, ubit16 BX, ubit16 RoIAdd, ubit16 pT, ubit16 OPL, ubit16 overlapPhi, ubit16 overlapEta, ubit16 RoiAmbiguity,
              ubit16 BCIDcounter);
    CMAword output(ubit16 i);

    // executes trigger algorithm
    void execute(void);

    // overload of operators <<
    friend std::ostream &operator<<(std::ostream &stream, SectorLogic &o);
};

// prints the mask for DataFromPad
std::ostream &dfpa(std::ostream &stream, int indent, int whitesp, int ntimes);

// prints the mask for OutputFromSectorLogic
std::ostream &ofsla(std::ostream &stream, int indent, int whitesp, int ntimes);

#endif /* SectorLogic_H */
