/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// ********************************************************************
//
// NAME:     TileTBCellMonTool.h
// PACKAGE:  TileMonitoring
//
// AUTHORS:   Alexander Solodkov
//	      Luca Fiorini (Luca.Fiorini@cern.ch)
//
// ********************************************************************
#ifndef TILETBMONTOOL_H
#define TILETBMONTOOL_H

#include "TileFatherMonTool.h"
#include <cstdint>
#include <vector>

class TH1F;
class TH2F;

class ITileBadChanTool;
class TileCell;

/** @class TileTBCellMonTool
 *  @brief Class for TileCal monitoring at cell level
 */

class ATLAS_NOT_THREAD_SAFE TileTBCellMonTool: public TileFatherMonTool {  // deprecated: ATLASRECTS-7259

  public:

    TileTBCellMonTool(const std::string & type, const std::string & name, const IInterface* parent);

    virtual ~TileTBCellMonTool();

    virtual StatusCode initialize() final;

    //pure virtual methods
    StatusCode bookHistograms();
    StatusCode fillHistograms();
    StatusCode procHistograms();
    StatusCode checkHists(bool fromFinalize);

    StatusCode bookHistTrig(int trig);
    StatusCode bookHistTrigPart(int trig, int part);

    void cleanHistVec();
	bool m_isFirstEv;
  private:

    void calculateSynch();
    short isCellBad(const TileCell* tile_cell);
    short isAdcBad(int partition, int module, int channel, int gain);
    short nAdcBad(int partition);
    void FirstEvInit();
    void ShiftLumiHist(TProfile2D*, int32_t);
	void fillHitMap(TH2F *hHitMap,int cellHitMap[],double energy);
	//Double_t langaufun(Double_t *x, Double_t *par);
	TF1 *langaufit(TH1F *his, Double_t *fitrange, Double_t *startvalues, Double_t *parlimitslo, Double_t *parlimitshi, Double_t *fitparams, Double_t *fiterrors, Double_t *ChiSqr, Int_t *NDF);
	//Double_t langaufun(Double_t *x, Double_t *par);
	Int_t langaupro(Double_t *params, Double_t &maxx, Double_t &FWHM);


    ToolHandle<ITileBadChanTool> m_tileBadChanTool; //!< Tile Bad Channel tool

    static const int MAX_MODULE = 3;

    bool m_doOnline{};
    double m_Threshold{};
    double m_NegThreshold{};
    double m_energyThresholdForTime{};
    double m_EneBalThreshold{};
    double m_TimBalThreshold{};
    int32_t m_TileCellTrig{};
    int32_t m_old_lumiblock{};
    int32_t m_delta_lumiblock{};
    int32_t m_OldLumiArray1[4][MAX_MODULE][4]={{{0}}};
    int32_t m_OldLumiArray2[4][MAX_MODULE][4]={{{0}}};
    int m_nEventsProcessed[9]={0}; // number of processed events per trigger
    std::string m_cellsContName;

    double m_maxEnergy{};
    double m_maxTotalEnergy{};
    std::vector<float> m_timeRange;

    TH1F* m_tileChannelTimeLBA[MAX_MODULE][48]={};
    TH1F* m_tileChannelTimeLBC[MAX_MODULE][48]={};
    TH1F* m_tileChannelTimeEBA[MAX_MODULE][48]={};
    TH1F* m_tileChannelTimeEBC[MAX_MODULE][48]={};
    TH1F* m_tileChannelEnergyLBA[MAX_MODULE][48]={};
    TH1F* m_tileChannelEnergyLBC[MAX_MODULE][48]={};
    TH1F* m_tileChannelEnergyEBA[MAX_MODULE][48]={};
    TH1F* m_tileChannelEnergyEBC[MAX_MODULE][48]={};

    int m_cellsInPartition[5] = {1151, 1472, 1408, 1151, 5182}; // EBA, LBA, LBC, EBC, ALL
    bool m_fillTimeHistograms{};

    TH1F* m_TileTBTotalEnergyEBA[MAX_MODULE]={};
    TH2F* m_TileTBHitMapEBA[MAX_MODULE]={};
    TH1F* m_TileTBCellEneSumEBA[MAX_MODULE][14]={};
    TH1F* m_TileTBCellEneDiffEBA[MAX_MODULE][14]={};
    TH1F* m_TileTBCellTimeSumEBA[MAX_MODULE][14]={};
    TH1F* m_TileTBCellTimeDiffEBA[MAX_MODULE][14]={};

    TH2F* m_TileTBCellEneLeftVsRightPMTEBA[MAX_MODULE][14]={};
    TH2F* m_TileTBCellTimeLeftVsRightPMTEBA[MAX_MODULE][14]={};
    TH2F* m_TileTBSampleBCvsAEneEBA[MAX_MODULE]={};
    TH1F* m_TileTBSampleDEneEBA[MAX_MODULE]={};

    TH1F* m_TileTBTotalEnergyEBC[MAX_MODULE]={};
    TH2F* m_TileTBHitMapEBC[MAX_MODULE]={};
    TH1F* m_TileTBCellEneSumEBC[MAX_MODULE][14]={};
    TH1F* m_TileTBCellEneDiffEBC[MAX_MODULE][14]={};
    TH1F* m_TileTBCellTimeSumEBC[MAX_MODULE][14]={};
    TH1F* m_TileTBCellTimeDiffEBC[MAX_MODULE][14]={};

    TH2F* m_TileTBCellEneLeftVsRightPMTEBC[MAX_MODULE][14]={};
    TH2F* m_TileTBCellTimeLeftVsRightPMTEBC[MAX_MODULE][14]={};
    TH2F* m_TileTBSampleBCvsAEneEBC[MAX_MODULE]={};
    TH1F* m_TileTBSampleDEneEBC[MAX_MODULE]={};

    TH1F* m_TileTBTotalEnergyLBA[MAX_MODULE]={};
    TH2F* m_TileTBHitMapLBA[MAX_MODULE]={};
    TH1F* m_TileTBCellEneSumLBAD0[MAX_MODULE]={};
    TH1F* m_TileTBCellEneSumLBA[MAX_MODULE][23]={};
    TH1F* m_TileTBCellEneDiffLBA[MAX_MODULE][23]={};
    TH1F* m_TileTBCellTimeSumLBAD0[MAX_MODULE]={};
    TH1F* m_TileTBCellTimeSumLBA[MAX_MODULE][23]={};
    TH1F* m_TileTBCellTimeDiffLBA[MAX_MODULE][23]={};

    TH2F* m_TileTBCellEneLeftVsRightPMTLBA[MAX_MODULE][23]={};
    TH2F* m_TileTBCellTimeLeftVsRightPMTLBA[MAX_MODULE][23]={};
    TH2F* m_TileTBSampleBCvsAEneLBA[MAX_MODULE]={};
    TH1F* m_TileTBSampleDEneLBA[MAX_MODULE]={};

    TH1F* m_TileTBTotalEnergyLBC[MAX_MODULE]={};
    TH2F* m_TileTBHitMapLBC[MAX_MODULE]={};
    TH1F* m_TileTBCellEneSumLBCD0[MAX_MODULE]={};
    TH1F* m_TileTBCellEneSumLBC[MAX_MODULE][23]={};
    TH1F* m_TileTBCellEneDiffLBC[MAX_MODULE][23]={};
    TH1F* m_TileTBCellTimeSumLBCD0[MAX_MODULE]={};
    TH1F* m_TileTBCellTimeSumLBC[MAX_MODULE][23]={};
    TH1F* m_TileTBCellTimeDiffLBC[MAX_MODULE][23]={};

    TH2F* m_TileTBCellEneLeftVsRightPMTLBC[MAX_MODULE][23]={};
    TH2F* m_TileTBCellTimeLeftVsRightPMTLBC[MAX_MODULE][23]={};
    TH2F* m_TileTBSampleBCvsAEneLBC[MAX_MODULE]={};
    TH1F* m_TileTBSampleDEneLBC[MAX_MODULE]={};

	/*cellHitMap[channel][bins]
	*  bins=0 is the lower x bin
	*  bins=1 is the lower y bin
	*  bins=2 is the upper x bin
	*  bins=3 is the upper y bin
	*  bins=4 corresponding name in cellHitMap
	*  bins=5 whether or not there is C cell for long barrel
	*/
    int m_cellHitMapEB[48][5]={{0}};
    int m_cellHitMapLB[48][6]={{0}};
    int m_cellHitMapLB_C[8][4]={{0}};
    std::string m_cellHitMapNameEB[14];
    std::string m_cellHitMapNameLB[23];


};

#endif
