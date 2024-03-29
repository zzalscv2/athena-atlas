/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILECALIBALG_TILELASERDEFAULTCALIBTOOL_H
#define TILECALIBALG_TILELASERDEFAULTCALIBTOOL_H 

// Tile includes
#include "TileCalibAlgs/ITileCalibTool.h"
#include "TileEvent/TileDQstatus.h"
#include "TileConditions/TileCablingService.h"
#include "TileConditions/TileCondToolEmscale.h"
#include "TileConditions/ITileBadChanTool.h"
#include "TileConditions/ITileDCSTool.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileLaserObject.h"
#include "TileMonitoring/ITileStuckBitsProbsTool.h"


// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadHandleKey.h"

#include <string>
#include <map>
#include <cmath>


#define NDIODES 10
#define NDIODES_LASER1 4
#define NMONITORS 4
#define NPMTS 2

#define NGAINS 2
#define NPARTITIONS 4 
#define NDRAWERS 64
#define NCOUPLES 22 
#define NCHANNELS 48
#define NSLICES 100
#define NFIBERS 2


class TileRawChannelContainer;
class TileLaserObject;
class TileHWID;
class TFile;
class RunningStat;

class TileLaserDefaultCalibTool : public AthAlgTool, virtual public ITileCalibTool
{

 public:
  TileLaserDefaultCalibTool(const std::string& type, const std::string& name,const IInterface* pParent);
  virtual ~TileLaserDefaultCalibTool();
 
  virtual StatusCode initialize() override;
  virtual StatusCode initNtuple(int runNumber, int runType, TFile * rootfile) override;
  virtual StatusCode execute() override;
  virtual StatusCode finalizeCalculations() override;
  virtual StatusCode writeNtuple(int runNumber, int runType, TFile * rootfile) override;
  virtual StatusCode finalize() override;

 private:

  // Name of ntuple of the Tool
  std::string m_toolNtuple;

  std::string m_rawChannelContainerName;
  std::string m_laserContainerName;
	
  bool m_pisaMethod2;
  bool m_isLaserCalib;
  
  const TileHWID* m_tileHWID;    
  const TileCablingService* m_cabling;
  ToolHandle<TileCondToolEmscale> m_tileToolEmscale{this,
    "TileCondToolEmscale", "TileCondToolEmscale", "Tile em scale tool"};
  ToolHandle<ITileBadChanTool> m_tileBadChanTool{this,
    "TileBadChanTool", "TileBadChanTool", "Tile bad channel tool"};
  ToolHandle<ITileStuckBitsProbsTool> m_stuckBitsProbs{this,
    "StuckBitsProbsTool", "", "Tile stuck bits probabilities tool"};

  SG::ReadHandleKey<TileDQstatus> m_dqStatusKey;
  SG::ReadHandleKey<TileRawChannelContainer> m_rawChannelContainerKey{this,
      "TileRawChannelContainer", "TileRawChannelOpt2", "Tile raw channel container"};
  SG::ReadHandleKey<TileLaserObject> m_laserContainerKey{this,
      "TileLaserObject", "TileLaserObj", "Tile laser object"};

  ToolHandle<ITileDCSTool> m_tileDCS{this, "TileDCSTool", "TileDCSTool", "Tile DCS tool"};

  // Parameter which will end up in the ROOTuple
  //
  // Details concerning this parameters could be found on the LASER webpage :
  // 
  // http://atlas-tile-laser.web.cern.ch/
  //

  int m_toolRunNo;                   // The run number
  int m_ADC_problem;                 // Expert tag for LastROD problem noticfication
  int m_las_filter;                  // Filter wheel position
  float m_las_requ_amp;             // Requested amplitude
  float m_hrate;                    // Humidity rate (in %)
  float m_flow;                     // Gas flow in diodes box (in L/h)
  float m_head_temp;                // Temperature of the LASER head
  float m_las_time;                 // Event time

  // LASERII
  float (*m_ratio_LASERII)[NGAINS][NPARTITIONS][NDRAWERS][NCHANNELS][NGAINS];       // Calib coefficients computed for this run (for all diodes, PMTs, phocal, CIS)
  float (*m_ratio_S_LASERII)[NGAINS][NPARTITIONS][NDRAWERS][NCHANNELS][NGAINS];     // Corresponding RMS
  float (*m_ratio_LASERII_good)[NGAINS][NPARTITIONS][NDRAWERS][NCHANNELS][NGAINS];       // Calib coefficients computed for this run (for all diodes, PMTs, phocal, CIS)
  float (*m_ratio_S_LASERII_good)[NGAINS][NPARTITIONS][NDRAWERS][NCHANNELS][NGAINS];     // Corresponding RMS

  RunningStat* (*m_rs_ratio_LASERII)[NGAINS][NPARTITIONS][NDRAWERS][NCHANNELS][NGAINS];
  RunningStat* (*m_rs_ratio_LASERII_good)[NGAINS][NPARTITIONS][NDRAWERS][NCHANNELS][NGAINS];

  // FIRST PART OF DATA FRAGMENT
  /*  float m_chan_LAS[32];                 // Mean value for monitoring diodes, PMTs, phocal, CIS in Laser runs
  float m_chan_PED[32];                 // Mean value for monitoring diodes, PMTs, phocal, CIS in Pedestal runs
  float m_chan_LED[32];                 // Mean value for monitoring diodes, PMTs, phocal, CIS in LED runs
  float m_chan_APH[32];                 // Mean value for monitoring diodes, PMTs, phocal, CIS in Alpha runs
  float m_chan_LIN[32];                 // Mean value for monitoring diodes, PMTs, phocal, CIS un Linearity runs
  float m_chan_S_LAS[32];               // Corresponding RMS in Laser runs
  float m_chan_S_PED[32];               // Corresponding RMS in Pedestal runs
  float m_chan_S_LED[32];               // Corresponding RMS in LED runs
  float m_chan_S_APH[32];               // Corresponding RMS in Alpha runs
  float m_chan_S_LIN[32];               // Corresponding RMS in Linearity runs
  */

  // SECOND PART OF DATA FRAGMENT
  float (*m_PMT_LASERII)[NGAINS];             // Mean value for box PMTs
  float (*m_PMT_S_LASERII)[NGAINS];           // Corresponding RMS
  RunningStat* (*m_rs_PMT_signal_LASERII)[NGAINS];

  float (*m_diode_LASERII)[NGAINS];         // Mean value for box Photodiodes
  float (*m_diode_S_LASERII)[NGAINS];       // Corresponding RMS
  int (*m_entries_diode_LASERII)[NGAINS]; //Number of DIODE events collected for one diode (and a particular gain)

  RunningStat* (*m_rs_diode_signal_LASERII)[NGAINS];


  float (*m_diode_Ped_LASERII)[NGAINS];     // Corresponding pedestal values
  float (*m_diode_Ped_S_LASERII)[NGAINS];   // Sigma of pedestal values
  float (*m_diode_Alpha_LASERII)[NGAINS];     // Corresponding pedestal values
  float (*m_diode_Alpha_S_LASERII)[NGAINS];   // Sigma of pedestal values
  float (*m_diode_Led_LASERII)[NGAINS];     // Corresponding pedestal values
  float (*m_diode_Led_S_LASERII)[NGAINS];   // Sigma of pedestal values
  // float m_diode_Lin_LASERII[NDIODES][NGAINS];     // Corresponding pedestal values
  // float m_diode_Lin_S_LASERII[NDIODES][NGAINS];   // Sigma of pedestal values
  // float m_diode_Las_LASERII[NDIODES][NGAINS];     // Corresponding pedestal values
  // float m_diode_Las_S_LASERII[NDIODES][NGAINS];   // Sigma of pedestal values
  float (*m_PMT_Ped_LASERII)[NGAINS];     // Corresponding pedestal values
  float (*m_PMT_Ped_S_LASERII)[NGAINS];   // Sigma of pedestal values


  // LASERI
  float m_PMT[NPMTS];                                // Mean value for box PMTs
  float m_PMT_S[NPMTS];                              // Corresponding RMS
  float m_diode[NDIODES_LASER1];                     // Mean value for box Photodiodes
  float m_diode_S[NDIODES_LASER1];                   // Corresponding RMS
  float m_diode_Ped[NDIODES_LASER1];                 // Corresponding pedestal values
  float m_diode_Alpha[NDIODES_LASER1];               // Corresponding alpha peaks
  float m_diode_SPed[NDIODES_LASER1];                // Sigma of pedestals
  float m_diode_SAlpha[NDIODES_LASER1];              // RMS of alpha spectra
  float (*m_ratio)[NPARTITIONS][NDRAWERS][NCHANNELS][NGAINS];       // Calib coefficients computed for this run (for all diodes)
  float (*m_ratio_S)[NPARTITIONS][NDRAWERS][NCHANNELS][NGAINS];     // Corresponding RMS
  float (*m_ratio_good)[NPARTITIONS][NDRAWERS][NCHANNELS][NGAINS];       // Calib coefficients computed for this run (for all diodes)
  float (*m_ratio_good_S)[NPARTITIONS][NDRAWERS][NCHANNELS][NGAINS];     // Corresponding RMS
  float (*m_pmt_ratios)[NDRAWERS][NCHANNELS][NGAINS];       // Calib coefficients computed for this run (for all diodes)
  float (*m_pmt_S_ratios)[NDRAWERS][NCHANNELS][NGAINS];     // Corresponding RMS

  float (*m_diode_ratio_low)[NDIODES];
  float (*m_diode_ratio_high)[NDIODES];
  float (*m_diode_ratio_sigma_low)[NDIODES];
  float (*m_diode_ratio_sigma_high)[NDIODES];


  RunningStat* m_rs_diode_signal[NDIODES_LASER1];
  RunningStat* m_rs_PMT_signal[NPMTS];
  RunningStat* (*m_rs_ratio)[NPARTITIONS][NDRAWERS][NCHANNELS][NGAINS];
  RunningStat* (*m_rs_ratio_good)[NPARTITIONS][NDRAWERS][NCHANNELS][NGAINS];
  RunningStat* (*m_rs_pmt_ratios)[NDRAWERS][NCHANNELS][NGAINS]; 
 
  RunningStat* (*m_rs_diode_ratio_low)[NDIODES];
  RunningStat* (*m_rs_diode_ratio_high)[NDIODES];

  float (*m_meantime)[NGAINS];         // Mean time computed for this run per partition (to remove 25ns jitter) 
  float (*m_time)[NDRAWERS][NCHANNELS][NGAINS];           // Mean time computed for this run
  float (*m_time_S)[NDRAWERS][NCHANNELS][NGAINS];         // Corresponding RMS

  float (*m_mean)[NDRAWERS][NCHANNELS][NGAINS];           // Mean signal computed for this run
  float (*m_mean_S)[NDRAWERS][NCHANNELS][NGAINS];         // Corresponding RMS
  float (*m_raw_mean)[NDRAWERS][NCHANNELS][NGAINS];       // Mean signal computed for this run
  float (*m_raw_mean_S)[NDRAWERS][NCHANNELS][NGAINS];     // Corresponding RMS
  int   (*m_entries)[NDRAWERS][NCHANNELS][NGAINS];        // Number of LASER events collected for one channel (and a particular gain)
  float (*m_kappa)[NDRAWERS][NFIBERS][NGAINS];          // Kappa correction term
  float (*m_mean_slice)[NDRAWERS][NCHANNELS][NSLICES][NGAINS];
  float (*m_variance_slice)[NDRAWERS][NCHANNELS][NSLICES][NGAINS];
  short  (*m_status)[NDRAWERS][NCHANNELS][NGAINS];        // Status of the channel in the DB
  float  (*m_HV)[NDRAWERS][NCHANNELS]; 
  float  (*m_HVSet)[NDRAWERS][NCHANNELS]; 

  // Local results - not sent to ROOTuple
  int m_PMT1_ADC_prev;
  int m_PMT2_ADC_prev;

  bool m_LASERII;
  long long m_evtNr;
  int m_have_pedestals{0};
  int m_have_alpha{0};
  int m_have_led{0};
  int m_have_linearity{0};
  int m_have_laser{0};

  RunningStat* (*m_rs_meantime)[NGAINS];
  RunningStat* (*m_rs_time)[NDRAWERS][NCHANNELS][NGAINS];
  RunningStat* (*m_rs_signal)[NDRAWERS][NCHANNELS][NGAINS];
  RunningStat* (*m_rs_raw_signal)[NDRAWERS][NCHANNELS][NGAINS];
  RunningStat* (*m_rs_reducedKappa)[NDRAWERS][NCOUPLES-1][NCOUPLES][NGAINS][NFIBERS];

  // Functions
  static std::pair<unsigned int, unsigned int> getCoupleOfPMT(int ros, int couple);
  short isCellBad(int ros, int drawer, int channel, int gain);
  
  inline int chanIsConnected(int ros, int chan) {
    if(m_cabling->channel2hole(ros,chan)<0) return 0; //negative means not connected
    return 1;
  }
};


// Small class to compute mean & variance in a efficient way
class RunningStat {
	
public:
	// Default constructor
	RunningStat() : m_n(0), m_oldM(0), m_newM(0), m_oldS(0), m_newS(0), m_nTotEntries(0),
	                m_n_slice(0), m_oldM_slice(0), m_newM_slice(0), m_oldS_slice(0), m_newS_slice(0), m_eventsPerSlice(1000) {}

	void Clear() {
		m_n = 0;
	}
	
	// See Knuth TAOCP vol 2, 3rd edition, page 232
	void Push(double x) {
		m_n++;
		m_n_slice++;
		m_nTotEntries++;
		
		if (m_n == 1) {
			m_oldM = m_newM = x;
			m_oldS = 0.0;
		} else {
			m_newM = m_oldM + (x - m_oldM)/m_n;
			m_newS = m_oldS + (x - m_oldM)*(x - m_newM);
   
			// set up for next iteration
			m_oldM = m_newM; 
			m_oldS = m_newS;
		}

		//--- for slicing
		if (m_n_slice == 1) {
			m_oldM_slice = m_newM_slice = x;
			m_oldS_slice = 0.0;
		} else {
			m_newM_slice = m_oldM_slice + (x - m_oldM_slice)/m_n_slice;
			m_newS_slice = m_oldS_slice + (x - m_oldM_slice)*(x - m_newM_slice);
			m_oldM_slice = m_newM_slice; 
			m_oldS_slice = m_newS_slice;
		}
		
		if( m_n_slice==m_eventsPerSlice ) {
			m_vectMean.push_back( (m_n_slice > 0) ? m_newM_slice : 0.0 );
			m_vectVar.push_back( (m_n_slice > 1) ? m_newS_slice/(m_n_slice - 1) : 0.0 );
			m_n_slice=0;
		}
	}

	inline int NumDataValues() const { return m_nTotEntries;}
	inline double Mean() const { return (m_n > 0) ? m_newM : 0.0; }
	inline double Variance() const { return ( (m_n > 1) ? m_newS/(m_n - 1) : 0.0 ); }
	inline double StandardDeviation() const { return sqrt( Variance() ); }
	inline int EventsPerSlice() { return m_eventsPerSlice;}
	
	inline double Mean(unsigned int iSlice){ 
		if ( iSlice<m_vectMean.size() ) return m_vectMean[iSlice];
		return 0;
	} 

	inline double Variance(unsigned int iSlice){ 
		if (iSlice<m_vectVar.size()) return m_vectVar[iSlice];
		return 0;
	}
	
	inline int GetNSlices(){
		return m_vectVar.size();
	}


private:
	int m_n;
	double m_oldM, m_newM, m_oldS, m_newS;
	int m_nTotEntries;
	int m_n_slice;
	double m_oldM_slice, m_newM_slice, m_oldS_slice, m_newS_slice;
	int m_eventsPerSlice;
	std::vector<double> m_vectMean, m_vectVar;
	
};



#endif // #ifndef TILECALIBALG_TILELASERDEFAULTCALIBTOOL_H
