/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//*****************************************************************************
//  Filename : TileTBAANtuple.cxx
//  Author   : Luca Fiorini
//  Created  : Mar, 2007
//
//  DESCRIPTION:
//     Implement the algorithm to save TB Beam ROD dataX
//
//  HISTORY:
//
//  BUGS:
//
//*****************************************************************************

//Gaudi Includes
//#include "GaudiKernel/ITHistSvc.h"

//Event info
#include "xAODEventInfo/EventInfo.h"

#include "PathResolver/PathResolver.h"
#include "StoreGate/ReadCondHandle.h"

#include <cmath>

//Calo includes
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloID.h"

//TileCalo includes
#include "CaloIdentifier/TileID.h"
#include "TileIdentifier/TileHWID.h"
#include "TileIdentifier/TileTBFrag.h"
#include "TileCalibBlobObjs/TileCalibUtils.h"
#include "TileConditions/TileCablingService.h"
#include "TileConditions/TileInfo.h"
#include "TileEvent/TileDigitsContainer.h"
#include "TileEvent/TileBeamElemContainer.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileLaserObject.h"
#include "TileByteStream/TileBeamElemContByteStreamCnv.h"
#include "TileByteStream/TileLaserObjByteStreamCnv.h"
#include "TileEvent/TileHitContainer.h"
#include "TileSimEvent/TileHitVector.h"
#include "TileTBRec/TileTBAANtuple.h"

#include <fstream>
#include <sstream>

#define WRONG_SAMPLE(frag,chan,size)                                    \
msg(MSG::ERROR) << "Wrong no. of samples (" << size                     \
                << ") for channel " << chan                             \
                << " in frag 0x"<<MSG::hex << frag << MSG::dec          \
                << " - " << BeamFragName[frag&0x1F] << endmsg;

#define WRONG_CHANNEL(frag,chan)                                        \
msg(MSG::ERROR) << "Wrong channel " << chan                             \
                << " in frag 0x"<<MSG::hex << frag << MSG::dec          \
                << " - " << BeamFragName[frag&0x1F] << endmsg;

#define FRAG_FOUND(frag,chan,size)                                      \
if (msgLvl(MSG::DEBUG))                                                 \
  msg(MSG::DEBUG) << "Found channel " << chan                           \
                  << " in frag 0x"<<MSG::hex << frag << MSG::dec        \
                  << " - " << BeamFragName[frag&0x1F]                   \
                  << " of size " << size << endmsg;


#define SIGNAL_FOUND(frag,chan,amplitude)                               \
if (msgLvl(MSG::DEBUG))                                                 \
  msg(MSG::DEBUG) << "Found channel " << chan                           \
                  << " in frag 0x"<<MSG::hex << frag << MSG::dec        \
                  << " - " << BeamFragName[frag&0x1F]                   \
                  << " with amp=" << amplitude << endmsg;

#define MAX_DRAWERS 256
#define N_CHANS 48
#define N_DMUS 16

// Constructor & deconstructor
/** @class TileTBAANtuple
 *  @brief class to produce TileCal commissioning ntuples
 */

TileTBAANtuple::TileTBAANtuple(const std::string& name, ISvcLocator* pSvcLocator)
  : AthAlgorithm(name, pSvcLocator)
  , m_thistSvc("THistSvc", name)
  , m_ntuplePtr(0)
  , m_ntupleCreated(false)
  , m_evTime(0)
  , m_run(0)
  , m_evt(0)
  , m_trigType(0)
  , m_dspFlags(0)
  , m_l1ID(0)
  , m_l1Type(0)
  , m_evBCID(0)
  , m_evType(0)
  , m_frBCID(0)
  , m_muBackHit(0.0F)
  , m_muBackSum(0.0F)
  , m_muBack(0)
  , m_muCalib(0)
  , m_ecal(0)
  , m_qdc(nullptr)
  , m_las_BCID(0)
  , m_las_Filt(0)
  , m_las_ReqAmp(0.0)
  , m_las_MeasAmp(0.0)
  , m_las_D1_ADC(0)
  , m_las_D2_ADC(0)
  , m_las_D3_ADC(0)
  , m_las_D4_ADC(0)
  , m_las_D1_Ped(0.0)
  , m_las_D2_Ped(0.0)
  , m_las_D3_Ped(0.0)
  , m_las_D4_Ped(0.0)
  , m_las_D1_Ped_RMS(0.0)
  , m_las_D2_Ped_RMS(0.0)
  , m_las_D3_Ped_RMS(0.0)
  , m_las_D4_Ped_RMS(0.0)
  , m_las_D1_Alpha(0.0)
  , m_las_D2_Alpha(0.0)
  , m_las_D3_Alpha(0.0)
  , m_las_D4_Alpha(0.0)
  , m_las_D1_Alpha_RMS(0.0)
  , m_las_D2_Alpha_RMS(0.0)
  , m_las_D3_Alpha_RMS(0.0)
  , m_las_D4_Alpha_RMS(0.0)
  , m_las_D1_AlphaPed(0.0)
  , m_las_D2_AlphaPed(0.0)
  , m_las_D3_AlphaPed(0.0)
  , m_las_D4_AlphaPed(0.0)
  , m_las_D1_AlphaPed_RMS(0.0)
  , m_las_D2_AlphaPed_RMS(0.0)
  , m_las_D3_AlphaPed_RMS(0.0)
  , m_las_D4_AlphaPed_RMS(0.0)
  , m_las_PMT1_ADC(0)
  , m_las_PMT2_ADC(0)
  , m_las_PMT1_TDC(0)
  , m_las_PMT2_TDC(0)
  , m_las_PMT1_Ped(0.0)
  , m_las_PMT2_Ped(0.0)
  , m_las_PMT1_Ped_RMS(0.0)
  , m_las_PMT2_Ped_RMS(0.0)
  , m_las_Temperature(0.0)
  , m_lasFlag(0)
  , m_las0(0.0F)
  , m_las1(0.0F)
  , m_las2(0.0F)
  , m_las3(0.0F)
  , m_lasExtra(0)
  , m_commonPU(0)
  , m_adder(0)
  , m_eneAdd(0)
  , m_timeAdd(0)
  , m_s1cou(0)
  , m_s2cou(0)
  , m_s3cou(0)
  , m_cher1(0)
  , m_cher2(0)
  , m_cher3(0)
  , m_muTag(0)
  , m_muHalo(0)
  , m_muVeto(0)
  , m_s2extra(0)
  , m_s3extra(0)
  , m_sc1(0)
  , m_sc2(0)
  , m_tof(0)
  , m_btdc1(0)
  , m_btdc2(0)
  , m_scaler(0)
  , m_btdc(0)
  , m_tjitter(0)
  , m_tscTOF(0)
  , m_xChN2(0.0F)
  , m_yChN2(0.0F)
  , m_xChN1(0.0F)
  , m_yChN1(0.0F)
  , m_xCha0(0.0F)
  , m_yCha0(0.0F)
  , m_xCha1(0.0F)
  , m_yCha1(0.0F)
  , m_xCha2(0.0F)
  , m_yCha2(0.0F)
  , m_xCha1_0(0.0F)
  , m_yCha1_0(0.0F)
  , m_xCha2_0(0.0F)
  , m_yCha2_0(0.0F)
  , m_xImp(0.0F)
  , m_yImp(0.0F)
  , m_xImp_0(0.0F)
  , m_yImp_0(0.0F)
  , m_xImp_90(0.0F)
  , m_yImp_90(0.0F)
  , m_xImp_min90(0.0F)
  , m_yImp_min90(0.0F)
  , m_LarEne(0)
  , m_BarEne(0)
  , m_ExtEne(0)
  , m_GapEne(0)
  , m_coincTrig1(0)
  , m_coincTrig2(0)
  , m_coincTrig3(0)
  , m_coincTrig4(0)
  , m_coincTrig5(0)
  , m_coincTrig6(0)
  , m_coincTrig7(0)
  , m_coincTrig8(0)
  , m_coincFlag1(0)
  , m_coincFlag2(0)
  , m_coincFlag3(0)
  , m_coincFlag4(0)
  , m_coincFlag5(0)
  , m_coincFlag6(0)
  , m_coincFlag7(0)
  , m_coincFlag8(0)
  , m_calibrateEnergyThisEvent(false)
  , m_rchUnit(TileRawChannelUnit::MegaElectronVolts)
  , m_dspUnit(TileRawChannelUnit::ADCcounts)
  , m_adderFilterAlgTool("TileRawChannelBuilderFlatFilter/TileAdderFlatFilter",this)
  , m_tileToolEmscale("TileCondToolEmscale")
  , m_beamCnv(0)
  , m_currentFileNum(0)
  , m_eventsPerFile(0)

{
  declareProperty("TileInfoName", m_infoName = "TileInfo");
  declareProperty("TileCondToolEmscale", m_tileToolEmscale);
  declareProperty("TileDigitsContainer", m_digitsContainer = "TileDigitsCnt");
  declareProperty("TileDigitsContainerFlx", m_digitsContainerFlx = "");
  declareProperty("TileBeamElemContainer", m_beamElemContainer = "TileBeamElemCnt");
  // declareProperty("TileRawChannelContainerFlat", m_flatRawChannelContainer = "TileRawChannelFlat");
  declareProperty("TileRawChannelContainerFlat", m_flatRawChannelContainer = ""); // don't create by default
  // declareProperty("TileLaserObject", m_laserObject = "TileLaserObj");
  declareProperty("TileLaserObject", m_laserObject = ""); // don't create by default
  declareProperty("TileRawChannelContainerFit", m_fitRawChannelContainer = ""); // don't create by default
  declareProperty("TileRawChannelContainerOpt", m_optRawChannelContainer = "TileRawChannelOpt2");
  declareProperty("TileRawChannelContainerFitFlx", m_flxFitRawChannelContainer = ""); // don't create by default
  declareProperty("TileRawChannelContainerOptFlx", m_flxOptRawChannelContainer = ""); // don't create by default
  declareProperty("TileRawChannelContainerFitCool", m_fitcRawChannelContainer = ""); // don't create by default
  //declareProperty("TileRawChannelContainerOpt", m_optRawChannelContainer = ""); // don't create by default
  declareProperty("TileRawChannelContainerDsp", m_dspRawChannelContainer = ""); //
  declareProperty("CalibrateEnergy", m_calibrateEnergy = true);
  declareProperty("UseDspUnits", m_useDspUnits = false);
  declareProperty("OfflineUnits", m_finalUnit = TileRawChannelUnit::MegaElectronVolts);
  declareProperty("CalibMode", m_calibMode = false);
  declareProperty("UnpackAdder", m_unpackAdder = false);
  declareProperty("CompleteNtuple", m_completeNtuple = true);
  declareProperty("CommitNtuple", m_commitNtuple = true);
  declareProperty("BSInput", m_bsInput = true);
  declareProperty("PMTOrder", m_pmtOrder = true);

  m_hitVector = "TileHitVec";
  m_hitContainer="TileHitCnt";
  declareProperty("TileHitVector", m_hitVector);
  declareProperty("TileHitContainer", m_hitContainer);

  declareProperty("TBperiod", m_TBperiod = 2016); // tuned for 2016 testbeam by default

  declareProperty("NTupleLoc", m_ntupleLoc = "/NTUPLES/FILE%d/TileRec");
  declareProperty("NTupleID", m_ntupleID = "h1000");
  declareProperty("TreeSize", m_treeSize = 16000000000LL);

  declareProperty("NSamples", m_nSamples = NOT_SETUP);
  declareProperty("NSamplesFelix", m_nSamplesFlx = NOT_SETUP);
  declareProperty("NDrawers", m_nDrawers = 6);

  declareProperty("EventsPerFile", m_eventsPerFile = 200000);

  declareProperty("StreamName", m_streamName = "AANT");

//  m_drawerList.resize(6);    m_drawerType.resize(6);
//  m_drawerList[0] = "0x200"; m_drawerType[0] = 2; // barrel neg
//  m_drawerList[1] = "0x201"; m_drawerType[1] = 2; // barrel neg
//  m_drawerList[2] = "0x202"; m_drawerType[2] = 4; // ext barrel neg
//  m_drawerList[3] = "0x100"; m_drawerType[3] = 1; // barrel pos
//  m_drawerList[4] = "0x101"; m_drawerType[4] = 1; // barrel pos
//  m_drawerList[5] = "0x102"; m_drawerType[5] = 3; // ext barrel pos

  m_drawerList.resize(1);  m_drawerList[0] = "-1"; // setup drawer list from data
  m_drawerType.clear();    // take drawer type from Frag ID (doesn't work for 2003)
  m_drawerMap.clear();     // for 2003 need to use mapping given above

  char frg[6] = "0x000";
  m_beamFragList.clear();
  for (unsigned int i=0; i<sizeof(m_beamIdList)/sizeof(bool); ++i) {
    m_beamIdList[i] = false;
    // no coins trig by default (this is the case for 2004)
    if (i <= ECAL_ADC_FRAG || i >= COMMON_ADC1_FRAG ) {
      sprintf(frg,"0x%3.3x",i);
      m_beamFragList.push_back((std::string)frg);
    }
  }

  m_rosName.push_back("B"); // 0 - this is beam rod name
  m_rosName.push_back("A"); m_rosName.push_back("C");
  m_rosName.push_back("D"); m_rosName.push_back("E");

  declareProperty("rosName", m_rosName);
  declareProperty("drawerList", m_drawerList);
  declareProperty("drawerType", m_drawerType);
  declareProperty("beamFragList", m_beamFragList);

  declareProperty("BN2X1",m_beamBN2X1 = 0.0);
  declareProperty("BN2X2",m_beamBN2X2 = 0.2);
  declareProperty("BN2Y1",m_beamBN2Y1 = 0.0);
  declareProperty("BN2Y2",m_beamBN2Y2 = 0.2);
  declareProperty("BN2Z", m_beamBN2Z  = 1.0);

  declareProperty("BN1X1",m_beamBN1X1 = 0.0);
  declareProperty("BN1X2",m_beamBN1X2 = 0.2);
  declareProperty("BN1Y1",m_beamBN1Y1 = 0.0);
  declareProperty("BN1Y2",m_beamBN1Y2 = 0.2);
  declareProperty("BN1Z", m_beamBN1Z  = 1.0);

  declareProperty("BC0X1",m_beamBC0X1 = 0.0);
  declareProperty("BC0X2",m_beamBC0X2 = 0.2);
  declareProperty("BC0Y1",m_beamBC0Y1 = 0.0);
  declareProperty("BC0Y2",m_beamBC0Y2 = 0.2);
  declareProperty("BC0Z", m_beamBC0Z  = 17138.0);

  declareProperty("BC1X1",m_beamBC1X1 = NOT_SETUP - 1);
  declareProperty("BC1X2",m_beamBC1X2 = NOT_SETUP - 1);
  declareProperty("BC1Y1",m_beamBC1Y1 = NOT_SETUP - 1);
  declareProperty("BC1Y2",m_beamBC1Y2 = NOT_SETUP - 1);
  declareProperty("BC1Z", m_beamBC1Z  = NOT_SETUP - 1);
  declareProperty("BC1Z_0", m_beamBC1Z_0  = NOT_SETUP - 1);
  declareProperty("BC1Z_90", m_beamBC1Z_90  = NOT_SETUP - 1);
  declareProperty("BC1Z_min90", m_beamBC1Z_min90  = NOT_SETUP - 1);

  declareProperty("BC2X1",m_beamBC2X1 = NOT_SETUP - 1);
  declareProperty("BC2X2",m_beamBC2X2 = NOT_SETUP - 1);
  declareProperty("BC2Y1",m_beamBC2Y1 = NOT_SETUP - 1);
  declareProperty("BC2Y2",m_beamBC2Y2 = NOT_SETUP - 1);
  declareProperty("BC2Z", m_beamBC2Z  = NOT_SETUP - 1);
  declareProperty("BC2Z_0", m_beamBC2Z_0  = NOT_SETUP - 1);
  declareProperty("BC2Z_90", m_beamBC2Z_90  = NOT_SETUP - 1);
  declareProperty("BC2Z_min90", m_beamBC2Z_min90  = NOT_SETUP - 1);

  // for CTB 2004 only
  declareProperty("BC2Zperiod1", m_beamBC2Zperiod1 = 9411.0);
  declareProperty("BC2Zperiod2", m_beamBC2Zperiod2 = 12188.0);

  declareProperty("Radius", m_radius = 2280.0);
  declareProperty("EtaFileName", m_etaFileName = "TileEtaCTB.txt");

  m_eta = 0.0;
  m_theta = 0.0;
  m_runNumber = 0;
  m_evtNr = -1;
}

StatusCode TileTBAANtuple::initialize() {
  ATH_CHECK( m_samplingFractionKey.initialize() );
  m_saveFelixData = !(m_digitsContainerFlx.empty() && m_flxOptRawChannelContainer.empty() && m_flxFitRawChannelContainer.empty());
  return StatusCode::SUCCESS;
}

/// Alg standard interface function
/// LF TODO: We could have a problem with the new feature introduced by Sasha that quit empty fragments
///and therefore it can increase or decrease the number of RawChannels fragments dynamically.
/// The code seems to be capable of react if a module is missing during a run,
///but if rawchans for a module appear after the beginning of a run (ntuple varialbes are branched af the beginnning of run),
///its quantities will not be in the ntuple.
StatusCode TileTBAANtuple::ntuple_initialize() {

  CHECK( m_thistSvc.retrieve() );

  // find TileCablingService
  m_cabling = TileCablingService::getInstance();

  // retrieve TileID helper from det store

  CHECK( detStore()->retrieve(m_tileID) );

  CHECK( detStore()->retrieve(m_tileHWID) );

  //=== get TileCondToolEmscale
  CHECK( m_tileToolEmscale.retrieve() );

  if (m_nSamples < 0) {
    m_nSamples = 7;
  }
  if (m_nSamplesFlx < 0) {
    m_nSamplesFlx = m_saveFelixData ? 32 : 0;
  }

  if (m_TBperiod >= 2015)  {
    m_unpackAdder = false;

    if (m_TBperiod == 2015) {
      m_nDrawers = 2;
      m_drawerList.resize(m_nDrawers);    m_drawerType.resize(m_nDrawers);
      m_drawerList[0] = "0x200"; m_drawerType[0] = 2; // barrel neg
      m_drawerList[1] = "0x401"; m_drawerType[1] = 4; // ext.barrel neg
    } else if (m_TBperiod == 2016 || m_TBperiod == 2018 || m_TBperiod == 2021 || m_TBperiod == 2022) {
      m_nDrawers = 5;
      m_drawerList.resize(m_nDrawers);    m_drawerType.resize(m_nDrawers);
      m_drawerList[0] = "0x100"; m_drawerType[0] = 1; // M0 pos
      m_drawerList[1] = "0x101"; m_drawerType[1] = 1; // barrel pos
      m_drawerList[2] = "0x200"; m_drawerType[2] = 2; // M0 neg
      m_drawerList[3] = "0x201"; m_drawerType[3] = 2; // barrel neg
      m_drawerList[4] = "0x402"; m_drawerType[4] = 4; // ext.barrel neg
    } else if (m_TBperiod == 2017) {
      m_nDrawers = 6;
      m_drawerList.resize(m_nDrawers);    m_drawerType.resize(m_nDrawers);
      m_drawerList[0] = "0x100"; m_drawerType[0] = 1; // M0 pos
      m_drawerList[1] = "0x101"; m_drawerType[1] = 1; // barrel pos
      m_drawerList[2] = "0x200"; m_drawerType[2] = 2; // M0 neg
      m_drawerList[3] = "0x201"; m_drawerType[3] = 2; // barrel neg
      m_drawerList[4] = "0x203"; m_drawerType[4] = 2; // barrel neg
      m_drawerList[5] = "0x402"; m_drawerType[5] = 4; // ext.barrel neg
    } else if (m_TBperiod == 2019) {
      m_nDrawers = 7;
      m_drawerList.resize(m_nDrawers);    m_drawerType.resize(m_nDrawers);
      m_drawerList[0] = "0x100"; m_drawerType[0] = 1; // M0 pos
      m_drawerList[1] = "0x101"; m_drawerType[1] = 1; // barrel pos
      m_drawerList[2] = "0x200"; m_drawerType[2] = 2; // M0 neg
      m_drawerList[3] = "0x201"; m_drawerType[3] = 2; // barrel neg
      m_drawerList[4] = "0x203"; m_drawerType[4] = 2; // barrel neg
      m_drawerList[5] = "0x402"; m_drawerType[5] = 4; // ext.barrel neg
      m_drawerList[6] = "0x405"; m_drawerType[6] = 4; // ext.barrel neg
    }

    if (m_TBperiod < 2016) {
      setupBeamChambersTB2015();
    } else if (m_TBperiod < 2021) {
      setupBeamChambersTB2016_2020();
    } else {
      // Strating from 2021 the following properties should be setup via JO
      checkIsPropertySetup(m_beamBC1X1, "BC1X1");
      checkIsPropertySetup(m_beamBC1X2, "BC1X2");
      checkIsPropertySetup(m_beamBC1Y1, "BC1Y1");
      checkIsPropertySetup(m_beamBC1Y2, "BC1Y2");
      checkIsPropertySetup(m_beamBC1Z, "BC1Z");
      checkIsPropertySetup(m_beamBC1Z_0, "BC1Z_0");
      checkIsPropertySetup(m_beamBC1Z_90, "BC1Z_90");
      checkIsPropertySetup(m_beamBC1Z_min90, "BC1Z_min90");

      checkIsPropertySetup(m_beamBC2X1, "BC2X1");
      checkIsPropertySetup(m_beamBC2X2, "BC2X2");
      checkIsPropertySetup(m_beamBC2Y1, "BC2Y1");
      checkIsPropertySetup(m_beamBC2Y2, "BC2Y2");
      checkIsPropertySetup(m_beamBC2Z, "BC2Z");
      checkIsPropertySetup(m_beamBC2Z_0, "BC2Z_0");
      checkIsPropertySetup(m_beamBC2Z_90, "BC2Z_90");
      checkIsPropertySetup(m_beamBC2Z_min90, "BC2Z_min90");
    }

  } else {
    setupBeamChambersBeforeTB2015();
  }


  if (m_unpackAdder) {
    // get TileRawChannelBuilderFlatFilter for adder energy calculation
    CHECK( m_adderFilterAlgTool.retrieve() );

    StatusCode sc;
    sc &= m_adderFilterAlgTool->setProperty("TileRawChannelContainer", "TileAdderFlat");
    sc &= m_adderFilterAlgTool->setProperty("calibrateEnergy", "true");

    sc &= m_adderFilterAlgTool->setProperty("PedStart", "0");
    sc &= m_adderFilterAlgTool->setProperty("PedLength", "1");
    sc &= m_adderFilterAlgTool->setProperty("PedOffset", "0");
    sc &= m_adderFilterAlgTool->setProperty("SignalStart", "1");
    sc &= m_adderFilterAlgTool->setProperty("SignalLength", "15");
    sc &= m_adderFilterAlgTool->setProperty("FilterLength", "5");
    sc &= m_adderFilterAlgTool->setProperty("FrameLength", "16");
    sc &= m_adderFilterAlgTool->setProperty("DeltaCutLo", "9.5");
    sc &= m_adderFilterAlgTool->setProperty("DeltaCutHi", "9.5");
    sc &= m_adderFilterAlgTool->setProperty("RMSCutLo", "1.0");
    sc &= m_adderFilterAlgTool->setProperty("RMSCutHi", "1.0");
    if (sc.isFailure()) {
      ATH_MSG_ERROR("Failure setting properties of " << m_adderFilterAlgTool);
      return StatusCode::FAILURE;
    }
  }

  if (m_finalUnit < TileRawChannelUnit::ADCcounts
      || m_finalUnit > TileRawChannelUnit::OnlineMegaElectronVolts) {

    m_finalUnit = -1;
    if (!m_useDspUnits && m_calibrateEnergy) {
      m_useDspUnits = true;
      ATH_MSG_INFO( "Final offline units are not set, will use DSP units" );
    }
  }

  if (!m_calibrateEnergy) {
    if (m_useDspUnits) {
      ATH_MSG_INFO( "calibrateEnergy is disabled, don't want to use DSP units"  );
      m_useDspUnits = false;
    }
  }

  ATH_MSG_INFO( "TestBeam period " << m_TBperiod );
  ATH_MSG_INFO( "calibMode " << m_calibMode );
  ATH_MSG_INFO( "calibrateEnergy " << m_calibrateEnergy );
  ATH_MSG_INFO( "offlineUnits " << m_finalUnit );
  ATH_MSG_INFO( "useDspUnits " << m_useDspUnits );
  ATH_MSG_INFO( "number of samples " << m_nSamples );


  msg(MSG::INFO) << "drawerList " << MSG::hex;
  unsigned int size = m_drawerList.size();
  for (unsigned int dr = 0; dr < size; ++dr) {
    int frag = strtol(m_drawerList[dr].data(), NULL, 0);
    if (frag >= 0) {
      m_drawerMap[frag] = dr;
      if (dr == m_drawerType.size()) m_drawerType.push_back(frag >> 8);
      msg(MSG::INFO) << " 0x" << frag;
    } else {
      msg(MSG::INFO) << " " << m_drawerList[dr];
      // put negative number in first element (flag to read frag ID from data)
      m_drawerList[0] = m_drawerList[dr];
      m_drawerList.resize(1);
      // m_drawerType.clear();
      m_drawerMap.clear();
      size = 0;
      break;
    }
  }
  if (size == 0) {
    if (m_drawerList.size() > 0)
      msg(MSG::INFO) << " - negative number, will read frag IDs from the data" << MSG::dec << endmsg;
    else
      msg(MSG::INFO) << "is empty, no drawer fragments in ntuple" << MSG::dec << endmsg;
  } else {
    msg(MSG::INFO) << MSG::dec << endmsg;

    size = m_drawerType.size();
    if (size < m_nDrawers) {
      m_drawerType.resize(m_nDrawers);
      for (; size < m_nDrawers; ++size)
        m_drawerType[size] = 0;
    }

    msg(MSG::INFO) << MSG::INFO << "drawerType ";
    for (unsigned int dr = 0; dr < size; ++dr)
      msg(MSG::INFO) << " " << m_drawerType[dr];
    msg(MSG::INFO) << endmsg;

    if (size > m_nDrawers) {
      ATH_MSG_INFO( "increasing m_nDrawers from " << m_nDrawers << " to " << size );
      m_nDrawers = size;
    }
  }

  msg(MSG::INFO) << MSG::INFO << "Beam Frag List " << MSG::hex;
  size = m_beamFragList.size();
  for (unsigned int dr = 0; dr < size; ++dr) {
    int frag = strtol(m_beamFragList[dr].data(), NULL, 0);
    if (frag >= 0) {
      m_beamIdList[frag & 0x1F] = true;
      msg(MSG::INFO) << " 0x" << frag;
    }
  }
  if (size == 0) {
    msg(MSG::INFO) << "is empty, no beam fragments in ntuple" << MSG::dec << endmsg;
  } else {
    msg(MSG::INFO) << MSG::dec << endmsg;
  }

  // set event number to 0 before first event
  m_evtNr = 0;
  m_currentFileNum = 0;

  m_calibrateEnergyThisEvent = m_calibrateEnergy;


  // find event and beam ROD header, calib mode for digitizers

  /// if first event: determine mode (normal or calib)
  /// by examining the first TileDigitCollection
  /// initialize NTuple accordingly
  if (m_evtNr == 0) {

    if (m_bsInput) {
      ServiceHandle<IConversionSvc> cnvSvc("ByteStreamCnvSvc", "");
      if (cnvSvc.retrieve().isFailure()) {
        ATH_MSG_ERROR( " Can't get ByteStreamCnvSvc " );
        m_beamCnv = NULL;

      } else {

        m_beamCnv = dynamic_cast<TileBeamElemContByteStreamCnv *>(cnvSvc->converter( ClassID_traits<TileBeamElemContainer>::ID()));

        if (m_beamCnv == NULL) {
          ATH_MSG_ERROR( " Can't get TileBeamElemContByteStreamCnv " );
        }
      }
    } else {
      m_beamCnv = NULL;
    }
    if (m_unpackAdder) // in 2003 event number starts from 1
      ++m_evtNr;
  }

  if (initList().isFailure()) {
    ATH_MSG_ERROR( " Error during drawer list initialization"  );
  }

  if (m_saveFelixData && initListFlx().isFailure()) {
    ATH_MSG_ERROR( " Error during drawer list initialization"  );
  }

  if (initNTuple().isFailure()) {
    ATH_MSG_ERROR( " Error during ntuple initialization" );
  }

  ATH_MSG_INFO( "initialization completed" );
  return StatusCode::SUCCESS;
}

StatusCode TileTBAANtuple::execute() {

  if (m_evtNr < 0) {

    //bool calibMode  = (m_beamInfo->calibMode() == 1);
    //if ( calibMode != m_calibMode ) {
    //  ATH_MSG_INFO( "Calib mode from data is " << calibMode );
    //  ATH_MSG_INFO( "  Overwriting calib mode " );
    //  m_calibMode = calibMode;
    //}

    if (ntuple_initialize().isFailure()) {
      ATH_MSG_ERROR( "ntuple_initialize failed" );
    }

  }

  m_dspFlags = 0;
  if (ntuple_clear().isFailure()) {
    ATH_MSG_ERROR( "ntuple_clear failed" );
  }

  if (m_evtNr % 1000 == 0)
    ATH_MSG_INFO( m_evtNr << " events processed so far" );

  if (m_beamCnv) {
    try {
      m_evTime = m_beamCnv->eventFragment()->bc_time_seconds();
      m_evt = m_beamCnv->eventFragment()->global_id();
      // m_runNo = m_beamCnv->eventFragment()->run_no();  // run_no is 0 here
    } catch (...) {
      m_evTime = 0;
      m_evt = m_evtNr;
    }
    if ( m_beamCnv->validBeamFrag() )
      m_run = m_beamCnv->robFragment()->rod_run_no();   // take it from beam ROD header
    else
      m_run = 0;
  } else {
    m_evTime = 0;
    m_evt = m_evtNr;
    m_run = 0;
  }


  // new way to set run/event/lumi block
  // Retrieve eventInfo from StoreGate
  const xAOD::EventInfo* eventInfo(nullptr);
  if (evtStore()->retrieve(eventInfo).isSuccess()){
    //Get run and event numbers
    m_run = eventInfo->runNumber();
    m_evt = eventInfo->eventNumber();
    //Get timestamp of the event
    if (eventInfo->timeStamp() > 0) {
      m_evTime = eventInfo->timeStamp();
    }
  }


  // store BeamElements
  bool empty = storeBeamElements().isFailure();

  //store Laser Object
  if (m_laserObject.size() > 0) {
    empty &= storeLaser().isFailure();
  }

  if (m_drawerMap.size() > 0) {

    // store TileDigits
    if (m_nSamples > 0) {
      empty &= (storeDigits().isFailure());
    }
    if (m_nSamplesFlx > 0) {
       empty &= (storeDigitsFlx().isFailure());
    }

    // store TileRawChannels
    // start from DSP channels - so we can find out what is the DSP units

    empty &= (storeRawChannels(m_dspRawChannelContainer,m_calibMode, &m_eDspVec, &m_tDspVec, &m_chi2DspVec, 0, &m_ROD_GlobalCRCVec, &m_ROD_DMUBCIDVec, &m_ROD_DMUmemoryErrVec, &m_ROD_DMUSstrobeErrVec, &m_ROD_DMUDstrobeErrVec, &m_ROD_DMUHeadformatErrVec, &m_ROD_DMUHeadparityErrVec, &m_ROD_DMUDataformatErrVec, &m_ROD_DMUDataparityErrVec, &m_ROD_DMUMaskVec).isFailure());
    empty &= (storeRawChannels(m_fitRawChannelContainer, m_calibMode, &m_efitVec, &m_tfitVec, &m_chi2Vec, &m_pedfitVec, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0).isFailure());
    empty &= (storeRawChannels(m_flxFitRawChannelContainer, true, &m_eflxfitVec, &m_tflxfitVec, &m_chi2flxfitVec, &m_pedflxfitVec, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0).isFailure());
    empty &= (storeRawChannels(m_optRawChannelContainer,m_calibMode, &m_eOptVec, &m_tOptVec, &m_chi2OptVec, &m_pedOptVec, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0).isFailure());
    empty &= (storeRawChannels(m_flxOptRawChannelContainer, true, &m_eflxoptVec, &m_tflxoptVec, &m_chi2flxoptVec, &m_pedflxoptVec, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0).isFailure());
    empty &= (storeRawChannels(m_fitcRawChannelContainer,m_calibMode, &m_efitcVec, &m_tfitcVec, &m_chi2cVec, &m_pedfitcVec, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0).isFailure());
    empty &= (storeRawChannels(m_flatRawChannelContainer,m_calibMode, &m_eneVec, &m_timeVec, &m_chi2FlatVec, &m_pedFlatVec, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0).isFailure());

    empty &= (storeHitVector().isFailure());
    empty &= (storeHitContainer().isFailure());
  }


  if ( m_completeNtuple && m_TBperiod < 2015) {
    // store energy per sampling from all calorimeters
    empty &= (storeCells().isFailure());
  }


  // increase event nr
  // this number can be different from real event number if we skip events

  if (empty) {
    ATH_MSG_WARNING( "Error in execute " );
  }


  //  m_thistSvc->getTree("/"+m_streamName+"/h1000",m_ntuplePtr);
  m_ntuplePtr->Fill();

  ++m_evtNr;

  // Execution completed.
  ATH_MSG_DEBUG( "execute() completed successfully" );

  return StatusCode::SUCCESS;
}

//
// Here the LASER object is opened and corresponding variable are stored
//


StatusCode TileTBAANtuple::storeLaser() {

  const TileLaserObject* laserObj;
  CHECK( evtStore()->retrieve(laserObj, m_laserObject) );


  m_las_BCID   = laserObj->getBCID();

  m_las_Filt = laserObj->getFiltNumber();
  m_las_ReqAmp = laserObj->getDiodeCurrOrd();
  m_las_MeasAmp = laserObj->getDiodeCurrMeas();


  m_las_D1_ADC = laserObj->getDiodeADC(0);
  m_las_D2_ADC = laserObj->getDiodeADC(1);
  m_las_D3_ADC = laserObj->getDiodeADC(2);
  m_las_D4_ADC = laserObj->getDiodeADC(3);

  m_las_D1_Ped = laserObj->getDiodePedestal(0);
  m_las_D2_Ped = laserObj->getDiodePedestal(1);
  m_las_D3_Ped = laserObj->getDiodePedestal(2);
  m_las_D4_Ped = laserObj->getDiodePedestal(3);

  m_las_D1_Ped_RMS = laserObj->getDiodeSigmaPedestal(0);
  m_las_D2_Ped_RMS = laserObj->getDiodeSigmaPedestal(1);
  m_las_D3_Ped_RMS = laserObj->getDiodeSigmaPedestal(2);
  m_las_D4_Ped_RMS = laserObj->getDiodeSigmaPedestal(3);

  m_las_D1_Alpha = laserObj->getAlpha(0);
  m_las_D2_Alpha = laserObj->getAlpha(1);
  m_las_D3_Alpha = laserObj->getAlpha(2);
  m_las_D4_Alpha = laserObj->getAlpha(3);

  m_las_D1_Alpha_RMS = laserObj->getSigmaAlpha(0);
  m_las_D2_Alpha_RMS = laserObj->getSigmaAlpha(1);
  m_las_D3_Alpha_RMS = laserObj->getSigmaAlpha(2);
  m_las_D4_Alpha_RMS = laserObj->getSigmaAlpha(3);

  m_las_D1_AlphaPed = laserObj->getPedestalAlpha(0);
  m_las_D2_AlphaPed = laserObj->getPedestalAlpha(1);
  m_las_D3_AlphaPed = laserObj->getPedestalAlpha(2);
  m_las_D4_AlphaPed = laserObj->getPedestalAlpha(3);

  m_las_D1_AlphaPed_RMS = laserObj->getSigmaPedAlpha(0);
  m_las_D2_AlphaPed_RMS = laserObj->getSigmaPedAlpha(1);
  m_las_D3_AlphaPed_RMS = laserObj->getSigmaPedAlpha(2);
  m_las_D4_AlphaPed_RMS = laserObj->getSigmaPedAlpha(3);

  m_las_PMT1_ADC = laserObj->getPMADC(0);
  m_las_PMT2_ADC = laserObj->getPMADC(1);

  m_las_PMT1_TDC = laserObj->getTDC(0);
  m_las_PMT2_TDC = laserObj->getTDC(1);

  m_las_PMT1_Ped = laserObj->getPMPedestal(0);
  m_las_PMT2_Ped = laserObj->getPMPedestal(1);

  m_las_PMT1_Ped_RMS = laserObj->getPMSigmaPedestal(0);
  m_las_PMT2_Ped_RMS = laserObj->getPMSigmaPedestal(1);

  m_las_Temperature = laserObj->getPumpDiodeTemp();

  ATH_MSG_DEBUG( "storeLaser() completed" );

  return StatusCode::SUCCESS;
}

StatusCode TileTBAANtuple::storeBeamElements() {


  if ( m_beamElemContainer.size() == 0)
    return StatusCode::SUCCESS;

  const TileBeamElemContainer* BeamElemCnt;
  CHECK( evtStore()->retrieve(BeamElemCnt, m_beamElemContainer) );

  TileBeamElemContainer::const_iterator collItr=BeamElemCnt->begin();
  TileBeamElemContainer::const_iterator lastColl=BeamElemCnt->end();

  if ( m_completeNtuple ) {
    // Store ROD header info from collection (just from first one)
    if ( collItr!=lastColl ) {
      m_l1ID->at(m_nDrawers) = (*collItr)->getLvl1Id();
      m_l1Type->at(m_nDrawers) = (*collItr)->getLvl1Type();
      m_evType->at(m_nDrawers) = (*collItr)->getDetEvType();
      m_evBCID->at(m_nDrawers) = (*collItr)->getRODBCID();
    } else {
      m_l1ID->at(m_nDrawers) = 0xFFFFFFFF;
      m_l1Type->at(m_nDrawers) = 0xFFFFFFFF;
      m_evType->at(m_nDrawers) = 0xFFFFFFFF;
      m_evBCID->at(m_nDrawers) = 0xFFFFFFFF;
    }
  }


  m_trigType = 0;

  for(; collItr != lastColl; ++collItr) {

    TileBeamElemCollection::const_iterator beamItr=(*collItr)->begin();
    TileBeamElemCollection::const_iterator lastBeam=(*collItr)->end();

    if (msgLvl(MSG::VERBOSE)) {

      for (; beamItr != lastBeam; ++beamItr) {
        HWIdentifier id = (*beamItr)->adc_HWID();
        std::vector<uint32_t> digits = (*beamItr)->get_digits();
        msg(MSG::VERBOSE) << " --- TileBeamElem -- Identifier " << m_tileHWID->to_string(id)
                          << MSG::hex << " frag: 0x" << (*collItr)->identify()
                          << MSG::dec << " channel " << m_tileHWID->channel(id)
                          << " digits size " << digits.size() << endmsg;
        msg(MSG::VERBOSE) << " --- TileBeamElem -- BeamElem : ";
        for (unsigned int k = 0; k < digits.size(); k++)
          msg(MSG::VERBOSE) << digits[k] << " ";
        msg(MSG::VERBOSE) << endmsg;
      }
      //restore iterator
      beamItr = (*collItr)->begin();
    }

    int frag = (*collItr)->identify();
    ATH_MSG_DEBUG( " frag: " << frag );
    ATH_MSG_DEBUG( " trigType " << (*collItr)->getLvl1Type() );

    if ( m_trigType == 0 && (*collItr)->getLvl1Type() != 0 ) // take it from the ROD header
      m_trigType = (*collItr)->getLvl1Type();

    // unpack only fragments which we want to store in ntuple
    if ( m_beamIdList[frag&0x1F] ) {

      for (; beamItr != lastBeam; ++beamItr) {

        HWIdentifier id = (*beamItr)->adc_HWID();
        std::vector<uint32_t> digits = (*beamItr)->get_digits();
        int cha = m_tileHWID->channel(id);
        int dsize = digits.size();

        if ( dsize <= 0 ) {

          WRONG_SAMPLE(frag,cha,dsize);

        } else if ( dsize != 16 && frag == ADD_FADC_FRAG ) {

          WRONG_SAMPLE(frag,cha,dsize);

        } else if ( dsize != 1              && frag != ADD_FADC_FRAG    &&
                    frag != BEAM_TDC_FRAG   && frag != COMMON_TDC1_FRAG &&
                    frag != COMMON_TOF_FRAG && frag != COMMON_TDC2_FRAG &&
                    !(frag == ECAL_ADC_FRAG)) {

          WRONG_SAMPLE(frag,cha,dsize);

        } else {

          uint32_t amplitude = digits[0];
          SIGNAL_FOUND(frag, cha, amplitude);

          switch (frag) {

          case BEAM_TDC_FRAG:

            FRAG_FOUND(frag,cha,dsize);
            if(cha < 8) m_btdc1[cha] = amplitude;
            else if(cha < 16) m_btdc2[cha-8] = amplitude;
            else WRONG_CHANNEL(frag,cha);
            break;

          case BEAM_ADC_FRAG:
	    
            if ( m_TBperiod >= 2015 ) {
              switch(cha) {
                  // BEAM
                case 0: m_s1cou = amplitude; break;
                case 1: m_s2cou = amplitude; break;
                case 2: m_s3cou = amplitude; break;
                case 3: m_cher1 = amplitude; break; // ATH_MSG_VERBOSE("load beam adc " << m_cher1); break;
                case 4: m_cher2 = amplitude; break;
                case 5: m_muTag = amplitude; break;
                case 6: m_muHalo= amplitude; break;
                case 7: m_muVeto= amplitude; break;
                default: WRONG_CHANNEL(frag, cha);
              }
            } else if ( m_unpackAdder ) {
              switch(cha) {
                  // BEAM
                case 0: m_s1cou = amplitude; break;
                case 1: m_s2cou = amplitude; break;
                case 2: m_s3cou = amplitude; break;
                case 3: m_cher1 = amplitude; break; // swap of Cher1
                case 4: m_muTag = amplitude; break; // and S4 in 2003 data
                case 5: m_cher2 = amplitude; break;
                case 6: m_muHalo= amplitude; break;
                case 7: m_muVeto= amplitude; break;
                // LASER
                case 8:  m_las0 = amplitude; break;
                case 9:  m_las1 = amplitude; break;
                case 10: m_las2 = amplitude; break;
                case 11: m_las3 = amplitude; break;
                case 12: m_lasExtra[0] = amplitude; break;
                case 13: m_lasExtra[1] = amplitude; break;
                case 14: m_lasExtra[2] = amplitude; break;
                case 15: m_lasExtra[3] = amplitude; break;
                default: WRONG_CHANNEL(frag, cha);
              }
            } else { // 2004 data
              switch(cha) {
                // BEAM
                case 0: m_sc1 = amplitude; break;
                case 1: m_sc2 = amplitude; break;
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7: break;
                default: WRONG_CHANNEL(frag, cha);
              }
            }
            break;

          case MUON_ADC_FRAG:

            // first 14 are m_muBack
            if(cha < 14) m_muBack[cha] = amplitude;
            // last 2 are m_muCalib
            else if (cha < 16) m_muCalib[cha - 14] = amplitude;
            else WRONG_CHANNEL(frag, cha);
            break;

          case ADDR_ADC_FRAG:

            // second half of Muon Wall in 2004
            if(cha < 6) m_muBack[cha + 8] = amplitude;
            // last 2 are m_muCalib
            else if (cha < 8) m_muCalib[cha - 6] = amplitude;
            else WRONG_CHANNEL(frag,cha);
            break;

          case LASE_PTN_FRAG:

            if (cha == 0) {
              // laser pattern unit
              m_lasFlag  = amplitude;
              if (amplitude & 0xFF00) m_trigType = amplitude >> 8;
            } else {
              WRONG_CHANNEL(frag, cha);
            }
            break;

          case LASE_ADC_FRAG:

            // laser in 2004
            switch(cha) {
              case 0: m_las0 = amplitude; break;
              case 1: m_las1 = amplitude; break;
              case 2: m_las2 = amplitude; break;
              case 3: m_las3 = amplitude; break;
              case 4: m_lasExtra[0] = amplitude; break;
              case 5: m_lasExtra[1] = amplitude; break;
              case 6: m_lasExtra[2] = amplitude; break;
              case 7: m_lasExtra[3] = amplitude; break;
              default: WRONG_CHANNEL(frag,cha);
            }
            break;

          case ADD_FADC_FRAG:

            if (m_unpackAdder) {

              for (int k = 0; k < dsize; k++) {
                //m_addx[k]=k;
                //check how the matrix is filled
                m_adder[cha][k] = digits[k];
              }
                // FlatFiler adders
              double ene, tim;
              m_adderFilterAlgTool->flatFilter(digits, 0, ene, tim);
              m_eneAdd[cha] = ene;
              m_timeAdd[cha] = tim;
            }

            break;

          case ECAL_ADC_FRAG:

            if (m_TBperiod > 2015) {

              if(cha < 15) {
                m_qdc[cha] = amplitude;
                ATH_MSG_VERBOSE( "QDC: " << cha << " amp: " << amplitude);
              } else if (cha == 15) {
                for (int idx = 0; idx < dsize && idx < 18; ++idx) {
                  m_qdc[idx + 15] = digits[idx];
                  ATH_MSG_VERBOSE("QDC2: " << cha << " amp: " << amplitude);
                }
              } else {
                WRONG_CHANNEL(frag, cha);
              }

            } else {
              if(cha < 8) m_ecal[cha] = amplitude;
              else WRONG_CHANNEL(frag, cha);
            }

            break;

          case DIGI_PAR_FRAG:

            if(cha < 16) m_cispar[cha] = amplitude; //m_cispar->at(cha)=amplitude;
            else WRONG_CHANNEL(frag,cha);
            break;

          case COMMON_ADC1_FRAG:
            if (m_TBperiod > 2015) {
              if (cha < 16) {
                if (m_run > 2211444) {
                  switch(cha) {
                    // BEAM
                    case 0: m_s1cou = amplitude; break;
                    case 1: m_s2cou = amplitude; break;
                    case 2: { 
		      if (m_run < 2310000) {
			m_muBack[10] = amplitude;
		      } else {
			m_s3cou = amplitude;
		      }
		    }
		      break;
                    case 3: m_cher1 = amplitude; break;
                    case 4: m_cher2 = amplitude; break;
                    case 5: m_cher3 = amplitude; break;
                    default: m_muBack[cha - 6] = amplitude;
                  }
                } else {
                  switch(cha) {
                    // BEAM
                    case 0: m_s1cou = amplitude; break;
                    case 1: m_s2cou = amplitude; break;
                    case 2: m_s3cou = amplitude; break;
                    case 3: m_cher1 = amplitude; break;
                    case 4: m_cher2 = amplitude; break;
                    case 5: m_cher3 = amplitude; break;
                  }
                }
              } else {
                WRONG_CHANNEL(frag, cha);
              }
            } else {
              switch(cha) {
                // BEAM
                case 0: m_s1cou = amplitude; break;
                case 1: m_s2cou = amplitude; break;
                case 2: m_s3cou = amplitude; break;
                case 3: m_muTag = amplitude; break;
                case 4: m_cher1 = amplitude; break;
                case 5: m_cher2 = amplitude; break;
                case 6: m_muHalo= amplitude; break;
                case 7: m_muVeto= amplitude; break;
                default: WRONG_CHANNEL(frag, cha);
              }
            }
            break;

          case COMMON_ADC2_FRAG:

            if (m_TBperiod > 2015) {
              if(cha < 14) {
                m_muBack[cha] = amplitude;
              } else {
                WRONG_CHANNEL(frag, cha);
              }
            } else {
              if ( ! m_unpackAdder ) {
                switch(cha) {
                  // BEAM
                  case 0: break;
                  case 1: m_s2extra = amplitude; break;
                  case 2: m_s3extra = amplitude; break;
                  case 3:
                  case 4:
                  case 5:
                  case 6:
                  case 7: break;
                  default: WRONG_CHANNEL(frag, cha);
                }
              }
            }
            break;

          case COMMON_PTN_FRAG:
	    if (m_run > 2310000 && cha < 16) {
	      m_scaler[cha] = amplitude;
	    } else if (cha == 0) {
	      m_commonPU = amplitude;
	    } else {
	      WRONG_CHANNEL(frag, cha);
	    }
            break;

          case COMMON_TOF_FRAG:

            if (m_TBperiod >= 2022) {
              if (cha > 11) { // The first 12 channels are (can be) connected to BC1 and BC2, the last 4 channels are supposed to be TOF
                if(cha < 16) {
                  m_tof[cha] = amplitude;
                  ATH_MSG_VERBOSE( "TOF: " << cha << " amp: " << amplitude);
                } else {
                  WRONG_CHANNEL(frag, cha);
                }
                break;
              }
              // Fall through to case COMMON_TDC1_FRAG to unpack the first 12 channels of BC1 and BC2
              [[fallthrough]]; // silent the warning on fall through
            } else if (m_TBperiod > 2015) {

              if(cha < 16) {
                m_tof[cha] = amplitude;
                ATH_MSG_VERBOSE( "TOF: " << cha << " amp: " << amplitude);
              } else {
                WRONG_CHANNEL(frag, cha);
              }
              break;
            } else {
              if(cha < 8) m_tof[cha] = amplitude;
              else WRONG_CHANNEL(frag, cha);
              break;
            }

          case COMMON_TDC1_FRAG:

            FRAG_FOUND(frag,cha,dsize);
            if ((cha > 11) && (cha < 16) && (m_run > 2211136)) {
              m_tof[cha] = amplitude;
              ATH_MSG_VERBOSE( "TOF: " << cha << " amp: " << amplitude);
            } if(cha < 16) {
              m_btdc1[cha] = amplitude;
              ATH_MSG_VERBOSE( "TDC: " << cha << " amp: " << amplitude);
              if (m_btdcNhit[cha]==0) {
                m_btdc2[cha] = amplitude;
              }
              (*m_btdc)[cha].push_back(amplitude);
              ++m_btdcNhit[cha];
            } else WRONG_CHANNEL(frag, cha);
            break;

          case COMMON_TDC2_FRAG:

            FRAG_FOUND(frag,cha,dsize);
            if(cha < 16) m_btdc2[cha] = amplitude;
            else WRONG_CHANNEL(frag, cha);
            break;

          case COIN_TRIG1_FRAG:

            if(cha < 3) {
              int idx = cha * 32;
              for (int ibit = 0; ibit < 32; ++ibit){
                m_coincTrig1[idx++] = (amplitude >> ibit) & 1;
              }
            } else if (cha == 3) {
              m_coincFlag1 = amplitude;
            } else WRONG_CHANNEL(frag, cha);

            break;

          case COIN_TRIG2_FRAG:

            if(cha < 3) {
              int idx = cha * 32;
              for (int ibit=0; ibit < 32; ++ibit){
                m_coincTrig2[idx++] = (amplitude >> ibit) & 1;
              }
            } else if (cha == 3) {
              m_coincFlag2 = amplitude;
            } else WRONG_CHANNEL(frag, cha);

            break;

          case COIN_TRIG3_FRAG:

            if(cha < 3) {
              int idx = cha * 32;
              for (int ibit = 0; ibit < 32; ++ibit){
                m_coincTrig3[idx++] = (amplitude >> ibit) & 1;
              }
            } else if (cha == 3) {
              m_coincFlag3 = amplitude;
            } else WRONG_CHANNEL(frag,cha);

            break;

          case COIN_TRIG4_FRAG:

            if(cha < 3) {
              int idx = cha * 32;
              for (int ibit = 0; ibit < 32; ++ibit){
                m_coincTrig4[idx++] = (amplitude >> ibit) & 1;
              }
            } else if (cha == 3) {
              m_coincFlag4 = amplitude;
            } else WRONG_CHANNEL(frag, cha);

            break;

          case COIN_TRIG5_FRAG:

            if(cha < 3) {
              int idx = cha * 32;
              for (int ibit = 0; ibit < 32; ++ibit){
                m_coincTrig5[idx++] = (amplitude >> ibit) & 1;
              }
            } else if (cha == 3) {
              m_coincFlag5 = amplitude;
            } else WRONG_CHANNEL(frag, cha);

            break;

          case COIN_TRIG6_FRAG:

            if(cha < 3) {
              int idx = cha * 32;
              for (int ibit = 0; ibit < 32; ++ibit){
                m_coincTrig6[idx++] = (amplitude >> ibit) & 1;
              }
            } else if (cha == 3) {
              m_coincFlag6 = amplitude;
            } else WRONG_CHANNEL(frag, cha);

            break;

          case COIN_TRIG7_FRAG:

            if(cha < 3) {
              int idx = cha * 32;
              for (int ibit = 0; ibit < 32; ++ibit){
                m_coincTrig7[idx++] = (amplitude >> ibit) & 1;
              }
            } else if (cha == 3) {
              m_coincFlag7 = amplitude;
            } else WRONG_CHANNEL(frag,cha);

            break;

          case COIN_TRIG8_FRAG:

            if(cha < 3) {
              int idx = cha * 32;
              for (int ibit = 0; ibit < 32; ++ibit){
                m_coincTrig8[idx++] = (amplitude >> ibit) & 1;
              }
            } else if (cha == 3) {
              m_coincFlag8 = amplitude;
            } else WRONG_CHANNEL(frag, cha);

            break;

          default:
            break;
          }
        }
      }
    }
  }

  for (int i=0; i<8; ++i) {
    if (m_btdcNhit[i] > 1) ++m_btdcNchMultiHit[i>>2];
  }
  // calculate beam coords in Beam Chambers
  if ( m_TBperiod >= 2015 ) {

//      For BC1
//      -------
//      m_xCha1 = -0.0462586 + (-0.175666)*(m_btdc1[1] - m_btdc1[0]);
//      m_yCha1 = -0.051923 + (-0.176809)*(m_btdc1[2] - m_btdc1[3]);
//
//      For BC2
//      -------
//      m_xCha2 = 0.25202 + (-0.18053)*(m_btdc1[5] - m_btdc1[4]);
//      m_yCha2 = 0.0431688 + (-0.181128)*(m_btdc1[6] - m_btdc1[7]);

      if (m_run > 2211444) {
        m_xCha1 = m_beamBC1X1 + m_beamBC1X2*(m_btdc1[8] - m_btdc1[0]);
        m_yCha1 = m_beamBC1Y1 + m_beamBC1Y2*(m_btdc1[9] - m_btdc1[3]);
      } else {
        m_xCha1 = m_beamBC1X1 + m_beamBC1X2*(m_btdc1[1] - m_btdc1[0]);
        m_yCha1 = m_beamBC1Y1 + m_beamBC1Y2*(m_btdc1[2] - m_btdc1[3]);
      }
      if (m_run > 612543 && m_run< 614141) {
        m_xCha2 = m_beamBC2X1 + m_beamBC2X2*(m_btdc1[5] - m_btdc1[6]);
        m_yCha2 = m_beamBC2Y1 + m_beamBC2Y2*(m_btdc1[4] - m_btdc1[7]);
      } else {
        m_xCha2 = m_beamBC2X1 + m_beamBC2X2*(m_btdc1[5] - m_btdc1[4]);
        m_yCha2 = m_beamBC2Y1 + m_beamBC2Y2*(m_btdc1[6] - m_btdc1[7]);
      }

      // Using the first value from the TDC
      if (m_run > 2211444) {
        m_xCha1_0 = m_beamBC1X1 + m_beamBC1X2*(m_btdc2[8] - m_btdc2[0]);
        m_yCha1_0 = m_beamBC1Y1 + m_beamBC1Y2*(m_btdc2[9] - m_btdc2[3]);
      } else {
        m_xCha1_0 = m_beamBC1X1 + m_beamBC1X2*(m_btdc2[1] - m_btdc2[0]);
        m_yCha1_0 = m_beamBC1Y1 + m_beamBC1Y2*(m_btdc2[2] - m_btdc2[3]);
      }
      m_xCha2_0 = m_beamBC2X1 + m_beamBC2X2*(m_btdc2[5] - m_btdc2[4]);
      m_yCha2_0 = m_beamBC2Y1 + m_beamBC2Y2*(m_btdc2[6] - m_btdc2[7]);

      m_tjitter = m_btdc1[8];
      m_tscTOF  = m_btdc1[14];

      m_xImp  = m_xCha2 + (m_xCha2 - m_xCha1)*m_beamBC2Z/(m_beamBC1Z - m_beamBC2Z);
      m_yImp  = m_yCha2 + (m_yCha2 - m_yCha1)*m_beamBC2Z/(m_beamBC1Z - m_beamBC2Z);

// Work in progress

      m_xImp_0  = m_xCha2_0 + (m_xCha2_0 - m_xCha1_0)*m_beamBC2Z_0/(m_beamBC1Z_0 - m_beamBC2Z_0);
      m_yImp_0  = m_yCha2_0 + (m_yCha2_0 - m_yCha1_0)*m_beamBC2Z_0/(m_beamBC1Z_0 - m_beamBC2Z_0);
      m_xImp_90  = m_xCha2_0 + (m_xCha2_0 - m_xCha1_0)*m_beamBC2Z_90/(m_beamBC1Z_90 - m_beamBC2Z_90);
      m_yImp_90  = m_yCha2_0 + (m_yCha2_0 - m_yCha1_0)*m_beamBC2Z_90/(m_beamBC1Z_90 - m_beamBC2Z_90);
      m_xImp_min90  = m_xCha2_0 + (m_xCha2_0 - m_xCha1_0)*m_beamBC2Z_min90/(m_beamBC1Z_min90 - m_beamBC2Z_min90);
      m_yImp_min90  = m_yCha2_0 + (m_yCha2_0 - m_yCha1_0)*m_beamBC2Z_min90/(m_beamBC1Z_min90 - m_beamBC2Z_min90);
////////////////////

      if (m_run > 2211444) {
        ATH_MSG_DEBUG( "BC1x  : ( "<< m_btdc1[0] <<" - "<< m_btdc1[8] <<" )\t" << m_xCha1 );
        ATH_MSG_DEBUG( "BC1y  : ( "<< m_btdc1[2] <<" - "<< m_btdc1[9] <<" )\t" << m_yCha1 );
      } else {
        ATH_MSG_DEBUG( "BC1x  : ( "<< m_btdc1[0] <<" - "<< m_btdc1[1] <<" )\t" << m_xCha1 );
        ATH_MSG_DEBUG( "BC1y  : ( "<< m_btdc1[2] <<" - "<< m_btdc1[3] <<" )\t" << m_yCha1 );
      }
      ATH_MSG_DEBUG( "BC2x  : ( "<< m_btdc1[4] <<" - "<< m_btdc1[5] <<" )\t" << m_xCha2 );
      ATH_MSG_DEBUG( "BC2y  : ( "<< m_btdc1[6] <<" - "<< m_btdc1[7] <<" )\t" << m_yCha2 );

  } else if ( m_unpackAdder ) { // this is 2003 data

    if ( m_beamIdList[BEAM_TDC_FRAG] ) {
      m_xCha1 = m_beamBC1X1 + m_beamBC1X2*(m_btdc1[6] - m_btdc1[7]); // last two channels of TDC !!!
      m_yCha1 = m_beamBC1Y1 + m_beamBC1Y2*(m_btdc1[2] - m_btdc1[3]);
      m_xCha2 = m_beamBC2X1 + m_beamBC2X2*(m_btdc2[0] - m_btdc2[1]);
      m_yCha2 = m_beamBC2Y1 + m_beamBC2Y2*(m_btdc2[2] - m_btdc2[3]);

      m_xImp  = m_xCha2 + (m_xCha2 - m_xCha1)*m_beamBC2Z/(m_beamBC1Z - m_beamBC2Z);
      m_yImp  = m_yCha2 + (m_yCha2 - m_yCha1)*m_beamBC2Z/(m_beamBC1Z - m_beamBC2Z);
    }

  } else { // this is 2004 data

    if ( m_beamIdList[COMMON_TDC1_FRAG] ) {
      m_yChN2 = m_beamBN2Y1 + m_beamBN2Y2*(m_btdc1[0] - m_btdc1[1]);
      m_xChN2 = m_beamBN2X1 + m_beamBN2X2*(m_btdc1[2] - m_btdc1[3]);
      m_yChN1 = m_beamBN1Y1 + m_beamBN1Y2*(m_btdc1[4] - m_btdc1[5]);
      m_xChN1 = m_beamBN1X1 + m_beamBN1X2*(m_btdc1[6] - m_btdc1[7]);

      m_yCha0 = m_beamBC0Y1 + m_beamBC0Y2*(m_btdc1[8] - m_btdc1[9]);
      m_xCha0 = m_beamBC0X1 + m_beamBC0X2*(m_btdc1[10]- m_btdc1[11]);

      ATH_MSG_DEBUG( "BC-2x : ( "<< m_btdc1[2] <<" - "<< m_btdc1[3] <<" )\t" <<m_xChN2  );
      ATH_MSG_DEBUG( "BC-2y : ( "<< m_btdc1[0] <<" - "<< m_btdc1[1] <<" )\t" <<m_yChN2 );
      ATH_MSG_DEBUG( "BC-1x : ( "<< m_btdc1[6] <<" - "<< m_btdc1[7] <<" )\t" <<m_xChN1 );
      ATH_MSG_DEBUG( "BC-1y : ( "<< m_btdc1[4] <<" - "<< m_btdc1[5] <<" )\t" <<m_yChN1 );
      ATH_MSG_DEBUG( "BC0x  : ( "<< m_btdc1[10] <<" - "<< m_btdc1[11] <<" )\t" <<m_xCha0 );
      ATH_MSG_DEBUG( "BC0y  : ( "<< m_btdc1[8] <<" - "<< m_btdc1[9] <<" )\t"   <<m_yCha0 );

    }

    if ( m_beamIdList[COMMON_TDC2_FRAG] ) {
      m_yCha1 = m_beamBC1Y1 + m_beamBC1Y2*(m_btdc2[0] - m_btdc2[1]);
      m_xCha1 = m_beamBC1X1 + m_beamBC1X2*(m_btdc2[2] - m_btdc2[3]);
      m_yCha2 = m_beamBC2Y1 + m_beamBC2Y2*(m_btdc2[4] - m_btdc2[5]);
      m_xCha2 = m_beamBC2X1 + m_beamBC2X2*(m_btdc2[6] - m_btdc2[7]);

      m_xImp = 0.0;
      m_yImp = 0.0;

      ATH_MSG_DEBUG( "BC1x  : ( "<< m_btdc2[2] <<" - "<< m_btdc2[3] <<" )\t" <<m_xCha1 );
      ATH_MSG_DEBUG( "BC1y  : ( "<< m_btdc2[0] <<" - "<< m_btdc2[1] <<" )\t" <<m_yCha1 );
      ATH_MSG_DEBUG( "BC2x  : ( "<< m_btdc2[6] <<" - "<< m_btdc2[7] <<" )\t" <<m_xCha2 );
      ATH_MSG_DEBUG( "BC2y  : ( "<< m_btdc2[4] <<" - "<< m_btdc2[5] <<" )\t" <<m_yCha2 );

    }

    if ( m_beamIdList[COMMON_TDC1_FRAG] && m_beamIdList[COMMON_TDC2_FRAG] ) {

      ////////////////////////////////////////////////
      // Get run number and eta
      if ( 0==m_runNumber ){
        const xAOD::EventInfo* eventInfo(0);
        if (evtStore()->retrieve(eventInfo).isFailure()){
          ATH_MSG_ERROR( "No EventInfo object found! Can't read run number!" );
          m_runNumber = -1;
        } else {
          m_runNumber = eventInfo->runNumber();
          getEta();
        }
      }
      ////////////////////////////////////////////////

      // Computation of X,Y imp on TileCal/LAr front
      float tanBx = (m_xCha1-m_xCha0) / (m_beamBC0Z - m_beamBC1Z);
      float tanBy = (m_yCha1-m_yCha0) / (m_beamBC0Z - m_beamBC1Z);

      float Ximp = m_xCha1 + m_beamBC1Z * tanBx;
      float Yimp = m_yCha1 + m_beamBC1Z * tanBy;

      if ( 0.0 != cos(m_theta) * (1 + tanBx * tan(m_theta)) ){
        m_xImp = (Ximp + m_radius *(tanBx * (cos(m_theta) - 1) -sin(m_theta))) / (cos(m_theta) * (1 + tanBx * tan(m_theta)));
        m_yImp = Yimp + tanBy * (m_radius * (1 - cos(m_theta)) -Ximp * sin(m_theta)) / (cos(m_theta) * (1+tanBx * tan(m_theta)));
      }
    }
  }


  // do not apply Cesium and Laser calibration for CIS events
  // m_calibrateEnergyThisEvent = m_calibrateEnergy && (m_trigType != 8);

  return StatusCode::SUCCESS;
}


/**
/// Fill ntuple with data from TRC.
/// Default TRC container contains flat filtered and
/// named container fitted/opt filtered
 */
StatusCode TileTBAANtuple::storeRawChannels(std::string containerId
                                            , bool calib_mode
                                            , std::vector<float*>* eneVec
                                            , std::vector<float*>* timeVec
                                            , std::vector<float*>* chi2Vec
                                            , std::vector<float*>* pedVec
                                            , std::vector<short*>* ROD_GlobalCRCVec
                                            , std::vector<short*>* ROD_DMUBCIDVec
                                            , std::vector<short*>* ROD_DMUmemoryErrVec
                                            , std::vector<short*>* ROD_DMUSstrobeErrVec
                                            , std::vector<short*>* ROD_DMUDstrobeErrVec
                                            , std::vector<short*>* ROD_DMUHeadformatErrVec
                                            , std::vector<short*>* ROD_DMUHeadparityErrVec
                                            , std::vector<short*>* ROD_DMUDataformatErrVec
                                            , std::vector<short*>* ROD_DMUDataparityErrVec
                                            , std::vector<short*>* ROD_DMUMaskVec)
{


  if (containerId.size() == 0) // empty name, nothing to do
    return StatusCode::FAILURE;


  // get named container
  const TileRawChannelContainer* rcCnt = nullptr;
  CHECK( evtStore()->retrieve(rcCnt, containerId) );

  TileRawChannelUnit::UNIT rChUnit = rcCnt->get_unit();
  ATH_MSG_DEBUG( "RawChannel unit is " << rChUnit );


  if (rChUnit >= TileRawChannelUnit::OnlineADCcounts) { // this is container with DSP results
    m_dspUnit = rChUnit;
    m_dspFlags = rcCnt->get_bsflags() >> 16;
    ATH_MSG_DEBUG( "DSP flag is 0x" << MSG::hex << m_dspFlags << MSG::dec  << " DSP unit is " << m_dspUnit );

  } else if ((m_useDspUnits || m_finalUnit >= TileRawChannelUnit::OnlineADCcounts)
             && rChUnit != TileRawChannelUnit::ADCcounts) {

      ATH_MSG_ERROR( "RawChannel units are not ADC counts, can't apply DSP-like calibration" );
      return StatusCode::FAILURE;
  }

  if (m_calibrateEnergyThisEvent) {
    if (m_useDspUnits) { // calibrate a-la online
      m_rchUnit = m_dspUnit;
    } else { // convert to final units
      m_rchUnit = (TileRawChannelUnit::UNIT)m_finalUnit;
    }
  } else {
    m_rchUnit = rChUnit;
  }

  ATH_MSG_DEBUG( "Final RawChannel unit is " << m_rchUnit );

  // Get iterator for all TRCColl in TRCCont
  TileRawChannelContainer::const_iterator itColl = (*rcCnt).begin();
  TileRawChannelContainer::const_iterator itCollEnd = (*rcCnt).end();

  TileRawChannelCollection::const_iterator it, itEnd;

  // type is 0 - m_nDrawers-1, fragType is 1-4 B+/B-/EB+/EB-
  int type, fragType;

  // Go through all TileRawChannelCollections
  for(; itColl != itCollEnd; ++itColl) {
    // determine type
    int fragId = (*itColl)->identify();
    int drawerIdx = TileCalibUtils::getDrawerIdxFromFragId(fragId);
    drawerMap_iterator itr = m_drawerMap.find(fragId);
    if ( itr != m_drawerMap.end() ) {
      type = (*itr).second;
    } else {
      type= -1;
    }

    if (type < 0) {
      if ( (*itColl)->begin() != (*itColl)->end() )
          //ATH_MSG_WARNING( "frag id 0x"<<MSG::hex<<fragId<<MSG::dec <<" was not found among valid frag IDs when storing TRC!" );
        ATH_MSG_DEBUG( "frag id 0x"<<MSG::hex<<fragId<<MSG::dec <<" was not found among valid frag IDs when storing TRC!" );
    } else {
      fragType = m_drawerType[type];
      ATH_MSG_DEBUG( "TRC (" << containerId
                    << ") Event# " << m_evtNr
                    << " Frag id 0x" << MSG::hex << fragId << MSG::dec
                    << " index "<< type );

      // go through all TileRawChannels in collection
      it = (*itColl)->begin();
      itEnd = (*itColl)->end();

      for (; it != itEnd; ++it) {
        const TileRawChannel* rch = (*it);
        int index = type;
        HWIdentifier hwid = rch->adc_HWID();

        // determine channel and gain
        int channel = m_tileHWID->channel(hwid);
        int gain = m_tileHWID->adc(hwid);

        if (calib_mode) {
          // gain, if hi add m_nDrawers to index
          if (gain == 1) index += m_nDrawers;
        }

        /// final calibration
        double energy = rch->amplitude();
        if (m_rchUnit != rChUnit) {
          if (m_rchUnit < TileRawChannelUnit::OnlineOffset)
            energy = m_tileToolEmscale->channelCalib(drawerIdx, channel, gain, energy, rChUnit, m_rchUnit);
          else
            energy = m_tileToolEmscale->channelCalibOnl(drawerIdx, channel, gain, energy, m_rchUnit);
        }

        // cabling for testbeam (convert to pmt#-1)
        if ((m_TBperiod < 2015 ||
             (m_TBperiod==2015 && fragType<3) ||
             ((m_TBperiod==2016 || m_TBperiod==2021) && ((fragId&0xFF)<4 && fragId != 0x201)) ||
             (m_TBperiod==2017 && ((fragId&0xFF)<4 && !(fragId == 0x201 || fragId == 0x203))) ||
             (m_TBperiod==2018 && ((fragId&0xFF)<4 && !(fragId == 0x201 || fragId == 0x402))) ||
             (m_TBperiod==2019 && ((fragId&0xFF)<5 && !(fragId == 0x201 || fragId == 0x203 || fragId >= 0x402))) ||
             (m_TBperiod==2022 && ((fragId&0xFF)<4 && !(fragId == 0x201 || (m_run >= 2210456 && fragId == 0x402)))))
            && fragType > 0 && m_pmtOrder)
          channel = digiChannel2PMT(fragType, channel);

        //     if  ( int((eneVec->at(index))->size()) < (channel+1) ) (eneVec->at(index))->resize(channel+1); //ugly
        (eneVec->at(index))[channel] = energy;
        //     if  ( int((timeVec->at(index))->size()) < (channel+1) ) (timeVec->at(index))->resize(channel+1); //ugly
        (timeVec->at(index))[channel] = rch->time();
        if (chi2Vec != 0) {
          //     if  ( int((chi2Vec->at(index))->size()) < (channel+1) ) (chi2Vec->at(index))->resize(channel+1); //ugly
          (chi2Vec->at(index))[channel] = rch->quality();
        }
        if (pedVec != 0) {
          //     if  ( int((pedVec->at(index))->size()) < (channel+1) ) (pedVec->at(index))->resize(channel+1); //ugly
          (pedVec->at(index))[channel] = rch->pedestal();
        }

        ATH_MSG_DEBUG( "TRC ene=" << energy
                       << " time=" << rch->time()
                       << " chi2=" << rch->quality()
                       << " ped=" << rch->pedestal()
                       << " pmt-1=" << channel
                       << " index " << index );

      }

      if (ROD_GlobalCRCVec != 0) {

        int index1 = type, index2 = type + 1;
        if (calib_mode) index2 += m_nDrawers;

        for (int index = index1; index < index2; index += m_nDrawers) {

          (ROD_GlobalCRCVec->at(index))[0] = (*itColl)->getFragGlobalCRC() & 1;
          (ROD_DMUMaskVec->at(index))[0] = (*itColl)->getFragRODChipMask();
          (ROD_DMUMaskVec->at(index))[1] = (*itColl)->getFragFEChipMask();

          for (unsigned int dmu = 0; dmu < N_DMUS; dmu++) {

            (ROD_DMUBCIDVec->at(index))[dmu] = ((*itColl)->getFragBCID() >> dmu) & 1;
            (ROD_DMUmemoryErrVec->at(index))[dmu] = ((*itColl)->getFragMemoryPar() >> dmu) & 1;
            (ROD_DMUSstrobeErrVec->at(index))[dmu] = ((*itColl)->getFragSstrobe() >> dmu) & 1;
            (ROD_DMUDstrobeErrVec->at(index))[dmu] = ((*itColl)->getFragDstrobe() >> dmu) & 1;
            (ROD_DMUHeadformatErrVec->at(index))[dmu] = ((*itColl)->getFragHeaderBit() >> dmu) & 1;
            (ROD_DMUHeadparityErrVec->at(index))[dmu] = ((*itColl)->getFragHeaderPar() >> dmu) & 1;
            (ROD_DMUDataformatErrVec->at(index))[dmu] = ((*itColl)->getFragSampleBit() >> dmu) & 1;
            (ROD_DMUDataparityErrVec->at(index))[dmu] = ((*itColl)->getFragSamplePar() >> dmu) & 1;

          }
        }
      }
    }
  }

  return StatusCode::SUCCESS;
}


/**
/// Fill Ntuple with info from TileDigits
/// Return true if the collection is empty,
/// which means that there are no RawChanels either.
 */
StatusCode TileTBAANtuple::storeDigits() {

  bool emptyColl = true;

  // Read Digits from TDS
  const TileDigitsContainer *digitsCnt;
  CHECK( evtStore()->retrieve(digitsCnt, m_digitsContainer) );

  // Get iterator for all TDColl in TDCont
  TileDigitsContainer::const_iterator itColl = (*digitsCnt).begin();
  TileDigitsContainer::const_iterator itCollEnd = (*digitsCnt).end();

  TileDigitsCollection::const_iterator it, itEnd;

  // type is 0 - m_nDrawers-1, fragType is 1-4 B+/B-/EB+/EB-
  int n, type, fragType, channel;

  std::vector<float> sampleVec;
  std::vector<uint32_t> headerVec;
  std::vector<uint32_t> headerVecHi;
  uint32_t CRCmask;
  uint32_t fe_crc;
  uint32_t rod_crc;

  // Go through all TileDigitsCollections
  for(; itColl != itCollEnd; ++itColl) {
    // determine type of frag
    int fragId = (*itColl)->identify();
    drawerMap_iterator itr = m_drawerMap.find(fragId);
    if ( itr != m_drawerMap.end() ) {
      type = (*itr).second;
    } else {
      type= -1;
    }

    if (type < 0) {
      if ( (*itColl)->begin() != (*itColl)->end() )
          //ATH_MSG_WARNING( "frag id 0x" << MSG::hex << fragId << MSG::dec <<" was not found among valid frag IDs when storing TRC!" );
        ATH_MSG_DEBUG( "frag id 0x" << MSG::hex << fragId << MSG::dec <<" was not found among valid frag IDs when storing TRC!" );

    } else {
      fragType = m_drawerType[type];

      ATH_MSG_DEBUG( "Event# " << m_evtNr
                     << " Frag id 0x" << MSG::hex << fragId << MSG::dec
                     << " index " << type
                     << " Calib " << m_calibMode );

      ATH_MSG_DEBUG( "       Size=" << (*itColl)->getFragSize()
                    << " BCID=" << (*itColl)->getFragBCID()<<MSG::hex
                    << " CRC=0x" << ((*itColl)->getFragCRC()&0xffff)
                    << " DMUMask=0x" << ((*itColl)->getFragDMUMask()&0xffff)<<MSG::dec );

      ATH_MSG_DEBUG( "       Lvl1ID=" << (*itColl)->getLvl1Id()
                     << " Lvl1Type=" << (*itColl)->getLvl1Type()
                     << " EvBCID=" << (*itColl)->getRODBCID()
                     << " EvType=" << (*itColl)->getDetEvType() );

      ATH_MSG_DEBUG( "       Header=" << (*itColl)->getFragChipHeaderWords() );

      if (m_completeNtuple) {
        /// Store ROD header info from collection
        /// (should be just one per ROD,
        /// but we don't know how many RODs we have,
        /// so store it for every collection)
        m_l1ID->at(type) = (*itColl)->getLvl1Id();
        m_l1Type->at(type) = (*itColl)->getLvl1Type();
        m_evType->at(type) = (*itColl)->getDetEvType();
        m_evBCID->at(type) = (*itColl)->getRODBCID();
        // store FrBCID
        m_frBCID->at(type) = (*itColl)->getFragBCID();
      }

      if(m_calibMode) {
        // Digits in calib mode
        // check gain for first digits in collection

        it = (*itColl)->begin();
        itEnd = (*itColl)->end();

        int dcnt=0;
        // non empty collection
        if(it != itEnd) {
          // store evtnr, bcid,crc, size
          // Same for lo and hi, because they come from the same fragment
          *(m_rodBCIDVec.at(type)) = (*itColl)->getRODBCID();
          *(m_sizeVec.at(type)) = (*itColl)->getFragSize();
          *m_sizeVec[type + m_nDrawers] = (*itColl)->getFragSize();
          *m_evtVec[type] = m_evtNr;
          *m_evtVec[type + m_nDrawers] = m_evtNr;

          headerVec = (*itColl)->getFragChipHeaderWords();
          headerVecHi = (*itColl)->getFragChipHeaderWordsHigh();
          CRCmask = (*itColl)->getFragDMUMask(); //mask of FE+ROD DMU crc check (16bit+16bit) 0xffffffff == All ok
          fe_crc = CRCmask & 0xFFFF;
          rod_crc = CRCmask >> 16;

          unsigned int headsize = std::min(16U, static_cast<unsigned int>(headerVec.size()));
          unsigned int headsizehi = std::min(16U, static_cast<unsigned int>(headerVecHi.size()));

          for (unsigned int ih = 0; ih < headsize; ++ih) {

            (m_bcidVec.at(type))[ih] = (headerVec[ih] & 0xFFF);
            (m_DMUheaderVec.at(type))[ih] = headerVec[ih]; /// Full DMU header, stored for debugging
            (m_DMUformatErrVec.at(type))[ih] = CheckDMUFormat(headerVec[ih]); /// bit_31==1 and bit_17==0
            (m_DMUparityErrVec.at(type))[ih] = CheckDMUParity(headerVec[ih]); /// parity must be an odd number
            (m_DMUmemoryErrVec.at(type))[ih] = (headerVec[ih] >> 25 & 0x1); /// memory parity error bit_25
            (m_DMUSstrobeErrVec.at(type))[ih] = (headerVec[ih] >> 24 & 0x1); /// single strobe error bit_24 (it is recovered)
            (m_DMUDstrobeErrVec.at(type))[ih] = (headerVec[ih] >> 23 & 0x1); /// double strobe error bit_23 (cannot be recovered)

            (m_feCRCVec.at(type))[ih] = (fe_crc >> ih & 0x1);
            (m_rodCRCVec.at(type))[ih] = (rod_crc >> ih & 0x1);
          }

          for (unsigned int ihhi = 0; ihhi < headsizehi; ++ihhi) {
            (m_bcidVec.at(type + m_nDrawers))[ihhi] = (headerVecHi[ihhi] & 0xFFF);
            (m_DMUheaderVec.at(type + m_nDrawers))[ihhi] = headerVecHi[ihhi]; /// Full DMU header, stored for debugging
            (m_DMUformatErrVec.at(type + m_nDrawers))[ihhi] = CheckDMUFormat(headerVecHi[ihhi]); /// bit_31==1 and bit_17==0
            (m_DMUparityErrVec.at(type + m_nDrawers))[ihhi] = CheckDMUParity(headerVecHi[ihhi]); /// parity must be an odd number
            (m_DMUmemoryErrVec.at(type + m_nDrawers))[ihhi] = (headerVecHi[ihhi] >> 25 & 0x1); /// memory parity error bit_25
            (m_DMUSstrobeErrVec.at(type + m_nDrawers))[ihhi] = (headerVecHi[ihhi] >> 24 & 0x1); /// single strobe error bit_24 (it is recovered)
            (m_DMUDstrobeErrVec.at(type + m_nDrawers))[ihhi] = (headerVecHi[ihhi] >> 23 & 0x1); /// double strobe error bit_23 (cannot be recovered)
            (m_feCRCVec.at(type))[ihhi] = -1; //Variables must be filled anyway, empty variables are not allowed
            (m_rodCRCVec.at(type))[ihhi] = -1; //Variables must be filled anyway, empty variables are not allowed
          }

          (m_slinkCRCVec.at(type))[0] = ((*itColl)->getFragCRC() >> 16) & 0xffff;
          (m_dmuMaskVec.at(type))[0] = ((*itColl)->getFragDMUMask() >> 16) & 0xffff;
          (m_slinkCRCVec.at(type))[1] = (*itColl)->getFragCRC() & 0xffff;
          (m_dmuMaskVec.at(type))[1] = (*itColl)->getFragDMUMask() & 0xffff;

          (m_slinkCRCVec.at(type + m_nDrawers))[0] = ((*itColl)->getFragCRC() >> 16) & 0xffff;
          (m_dmuMaskVec.at(type + m_nDrawers))[0] = ((*itColl)->getFragDMUMask() >> 16) & 0xffff;
          (m_slinkCRCVec.at(type + m_nDrawers))[1] = (*itColl)->getFragCRC() & 0xffff;
          (m_dmuMaskVec.at(type + m_nDrawers))[1] = (*itColl)->getFragDMUMask() & 0xffff;

          // go through all TileDigits in collection
          for (; it != itEnd; ++it) {
            emptyColl = false;

            HWIdentifier hwid = (*it)->adc_HWID();
            // determine gain
            int gain = m_tileHWID->adc(hwid);
            // add m_nDrawers to index if hi gain
            int index = type;
            if (gain == 1) index += m_nDrawers;

            // determine channel
            channel = m_tileHWID->channel(hwid);
            // cabling for testbeam (convert to pmt#-1)

            if ((m_TBperiod < 2015 ||
                 (m_TBperiod==2015 && fragType<3) ||
                 ((m_TBperiod==2016 || m_TBperiod==2021) && ((fragId&0xFF)<4 && fragId != 0x201)) ||
                 (m_TBperiod==2017 && ((fragId&0xFF)<4 && !(fragId == 0x201 || fragId == 0x203))) ||
                 (m_TBperiod==2018 && ((fragId&0xFF)<4 && !(fragId == 0x201 || fragId == 0x402))) ||
                 (m_TBperiod==2019 && ((fragId&0xFF)<5 && !(fragId == 0x201 || fragId == 0x203 || fragId >= 0x402))) ||
                 (m_TBperiod==2022 && ((fragId&0xFF)<4 && !(fragId == 0x201 || (m_run >= 2210456 && fragId == 0x402)))))
                && fragType > 0 && m_pmtOrder)

              channel = digiChannel2PMT(fragType, channel);

            /*     if  ( int((m_gainVec.at(type))->size()) < (channel+1) ) {
             (m_gainVec.at(type))->resize(channel+1); //ugly
             }*/

            // gain determined for all digits in collection
            (m_gainVec.at(index))[channel] = gain;
            ATH_MSG_DEBUG( "Storing TD for channel: " << channel
                           << " with gain " << m_tileHWID->adc(hwid)
                           << " index " << index );

            // get digits
            sampleVec = (*it)->samples();
            int siz = sampleVec.size();

            if (msgLvl(MSG::DEBUG)) {
              msg(MSG::DEBUG) << "Digits(" << siz << ")." << (dcnt++) << " {";
              for (int i = 0; i < siz; i++) {
                msg(MSG::DEBUG) << (int) sampleVec[i] << " ";
              }

              if (siz > m_nSamples) {
                msg(MSG::DEBUG) << "} ONLY " << m_nSamples << " digits saved to ntuple" << endmsg;
              } else {
                msg(MSG::DEBUG) << "}" << endmsg;
              }
            }

            if (siz > m_nSamples) siz = m_nSamples;
            for (n = 0; n < siz; n++) {
              (m_sampleVec.at(index))[channel][n] = (int) sampleVec[n];
              // (*m_sampleVec[index])[channel][n] = sampleVec[n];
            }
          }
        }
      } else {
        // Digits in normal mode

        // store evtnr, bcid,crc, size
        //log << MSG::INFO << "Normal mode, filling variables" << endmsg;
        *m_rodBCIDVec[type] = (*itColl)->getRODBCID();
        *m_sizeVec[type] = (*itColl)->getFragSize();
        *m_evtVec[type] = m_evtNr;

        headerVec = (*itColl)->getFragChipHeaderWords();
        CRCmask = (*itColl)->getFragDMUMask(); //mask of FE+ROD DMU crc check (16bit+16bit) 0xffffffff == All ok
        fe_crc = CRCmask & 0xFFFF;
        rod_crc = CRCmask >> 16;

        int headsize = headerVec.size();

        for (int ih = 0; ih < headsize; ih++) {
          (m_bcidVec.at(type))[ih] = (headerVec[ih] & 0xFFF);
          (m_DMUheaderVec.at(type))[ih] = headerVec[ih];   /// Full DMU header, stored for debugging
          (m_DMUformatErrVec.at(type))[ih] = CheckDMUFormat(headerVec[ih]); /// bit_31==1 and bit_17==0
          (m_DMUparityErrVec.at(type))[ih] = CheckDMUParity(headerVec[ih]); /// parity must be an odd number
          (m_DMUmemoryErrVec.at(type))[ih] = (headerVec[ih] >> 25 & 0x1); /// memory parity error bit_25
          (m_DMUSstrobeErrVec.at(type))[ih] = (headerVec[ih] >> 24 & 0x1); /// single strobe error bit_24 (it is recovered)
          (m_DMUDstrobeErrVec.at(type))[ih] = (headerVec[ih] >> 23 & 0x1); /// double strobe error bit_23 (cannot be recovered)
          (m_feCRCVec.at(type))[ih] = (fe_crc >> ih & 0x1);
          (m_rodCRCVec.at(type))[ih] = (rod_crc >> ih & 0x1);
        }


        (m_slinkCRCVec.at(type))[0] = ((*itColl)->getFragCRC() >> 16) & 0xffff;
        (m_dmuMaskVec.at(type))[0] = ((*itColl)->getFragDMUMask() >> 16) & 0xffff;
        (m_slinkCRCVec.at(type))[1] = (*itColl)->getFragCRC() & 0xffff;
        (m_dmuMaskVec.at(type))[1] = (*itColl)->getFragDMUMask() & 0xffff;

        // go through all TileDigits in collection
        it = (*itColl)->begin();
        itEnd = (*itColl)->end();

        int dcnt = 0;
        for (; it != itEnd; ++it) {
          emptyColl = false;

          HWIdentifier hwid = (*it)->adc_HWID();
          // determine channel
          channel = m_tileHWID->channel(hwid);
          // cabling for testbeam
          if ((m_TBperiod < 2015 ||
               (m_TBperiod==2015 && fragType<3) ||
               ((m_TBperiod==2016 || m_TBperiod==2021) && ((fragId&0xFF)<4 && fragId != 0x201)) ||
               (m_TBperiod==2017 && ((fragId&0xFF)<4 && !(fragId == 0x201 || fragId == 0x203))) ||
               (m_TBperiod==2018 && ((fragId&0xFF)<4 && !(fragId == 0x201 || fragId == 0x402))) ||
               (m_TBperiod==2019 && ((fragId&0xFF)<5 && !(fragId == 0x201 || fragId == 0x203 || fragId >= 0x402))) ||
               (m_TBperiod==2022 && ((fragId&0xFF)<4 && !(fragId == 0x201 || (m_run >= 2210456 && fragId == 0x402)))))
              && fragType > 0 && m_pmtOrder)
            channel = digiChannel2PMT(fragType, channel);

          // gain
          /*if ( int((m_gainVec.at(type))->size()) < (channel+1) ) {
           (m_gainVec.at(type))->resize(channel+1); //ugly
           }*/
          (m_gainVec.at(type))[channel] = m_tileHWID->adc(hwid);
          ATH_MSG_DEBUG( "Storing TD for channel: " << channel
                         << " with gain " << m_tileHWID->adc(hwid) );

          // get digits
          sampleVec = (*it)->samples();
          int siz = sampleVec.size();
          if (msgLvl(MSG::DEBUG)) {
            msg(MSG::DEBUG) << "Digits(" << siz << ")." << (dcnt++) << " {";

            for (int i = 0; i < siz; i++) {
              msg(MSG::DEBUG) << (int) sampleVec[i] << " ";
            }

            msg(MSG::DEBUG) << "}" << endmsg;
          }

          for (n = 0; n < siz; n++) {
            //((m_sampleVec.at(type))->at(channel)).push_back((int)sampleVec[n]);
            //(*m_sampleVec[type])[channel][n] = sampleVec[n];
            (m_sampleVec.at(type))[channel][n] = (int) sampleVec[n];
          }

        }
      }
    }
    // next container
  }

  if (emptyColl)
    return StatusCode::FAILURE;
  else
    return StatusCode::SUCCESS;
}


StatusCode TileTBAANtuple::storeDigitsFlx() {

  bool emptyColl = true;

  // Read Digits from TDS
  const TileDigitsContainer *digitsCntFlx;
  CHECK( evtStore()->retrieve(digitsCntFlx, m_digitsContainerFlx) );

  // Get iterator for all TDColl in TDCont
  TileDigitsContainer::const_iterator itColl = (*digitsCntFlx).begin();
  TileDigitsContainer::const_iterator itCollEnd = (*digitsCntFlx).end();

  TileDigitsCollection::const_iterator it, itEnd;

  // type is 0 - m_nDrawers-1, fragType is 1-4 B+/B-/EB+/EB-
  int n, type,  channel;

  std::vector<float> sampleVecLo;
  std::vector<float> sampleVecHi;
  std::vector<uint32_t> headerVec;
  std::vector<uint32_t> headerVecHi;


  // Go through all TileDigitsCollections
  for(; itColl != itCollEnd; ++itColl) {
    // determine type of frag
    int fragId = (*itColl)->identify();
    drawerMap_iterator itr = m_drawerMap.find(fragId);
    if ( itr != m_drawerMap.end() ) {
      type = (*itr).second;
    } else {
      type= -1;
    }

    if (type < 0) {
      if ( (*itColl)->begin() != (*itColl)->end() )
          //ATH_MSG_WARNING( "frag id 0x" << MSG::hex << fragId << MSG::dec <<" was not found among valid frag IDs when storing TRC!" );
        ATH_MSG_DEBUG( "frag id 0x" << MSG::hex << fragId << MSG::dec <<" was not found among valid frag IDs when storing TRC!" );

    } else {

      ATH_MSG_DEBUG( "Event# " << m_evtNr
                     << " Frag id 0x" << MSG::hex << fragId << MSG::dec
                     << " index " << type);

      ATH_MSG_DEBUG( "       Size=" << (*itColl)->getFragSize());

      ATH_MSG_DEBUG( "       Lvl1ID=" << (*itColl)->getLvl1Id()
                     << " EvBCID=" << (*itColl)->getRODBCID()
                     << " EvType=" << (*itColl)->getDetEvType() );
      ATH_MSG_DEBUG( "       Headers = "<< (*itColl)->getFragExtraWords() );

      if (m_completeNtuple) {
        /// Store ROD header info from collection
        /// (should be just one per ROD,
        /// but we don't know how many RODs we have,
        /// so store it for every collection)
        m_l1ID->at(type) = (*itColl)->getLvl1Id();
        m_l1Type->at(type) = (*itColl)->getLvl1Type();
        m_evType->at(type) = (*itColl)->getDetEvType();
        m_evBCID->at(type) = (*itColl)->getRODBCID();
        // store FrBCID
        m_frBCID->at(type) = (*itColl)->getFragBCID();
       }


      // Digits in calib mode
        // check gain for first digits in collection

        if (m_mdL1idflxVec.at(type)) {
          std::vector<uint32_t> extraWords = (*itColl)->getFragExtraWords();
          if (extraWords.size() >= 10 * MAX_MINIDRAWERS) {

            int* md[] = {m_mdL1idflxVec.at(type), m_mdBcidflxVec.at(type),
                         m_mdModuleflxVec.at(type), m_mdRunTypeflxVec.at(type),m_mdRunflxVec.at(type), m_mdPedLoflxVec.at(type), m_mdPedHiflxVec.at(type), 
                         m_mdChargeflxVec.at(type), m_mdChargeTimeflxVec.at(type), m_mdCapacitorflxVec.at(type)};

            auto it = extraWords.begin();
            for (int i = 0; i < 10; ++i) {
              std::copy(it + i * MAX_MINIDRAWERS, it + (i + 1) * MAX_MINIDRAWERS, md[i]);
            }
          }
        }


        it = (*itColl)->begin();
        itEnd = (*itColl)->end();

        int dcnt=0;
        // non empty collection
        if(it != itEnd) {
          // store evtnr, bcid,crc, size
          // Same for lo and hi, because they come from the same fragment
          *(m_rodBCIDflxVec.at(type)) = (*itColl)->getRODBCID();
          *(m_sizeflxVec.at(type)) = (*itColl)->getFragSize();
          *m_sizeflxVec[type + m_nDrawers] = (*itColl)->getFragSize();
          *m_evtflxVec[type] = m_evtNr;
          *m_evtflxVec[type + m_nDrawers] = m_evtNr;


          headerVec = (*itColl)->getFragChipHeaderWords();
          headerVecHi = (*itColl)->getFragChipHeaderWordsHigh();

          unsigned int headsize = std::min(16U, static_cast<unsigned int>(headerVec.size()));
          unsigned int headsizehi = std::min(16U, static_cast<unsigned int>(headerVecHi.size()));
          for (unsigned int ih = 0; ih < headsize; ++ih) {

            (m_bcidVec.at(type))[ih] = (headerVec[ih] & 0xFFF);
          }

          for (unsigned int ihhi = 0; ihhi < headsizehi; ++ihhi) {
            (m_bcidVec.at(type + m_nDrawers))[ihhi] = (headerVecHi[ihhi] & 0xFFF);
          }


         // go through all TileDigits in collection
          for (; it != itEnd; ++it) {
            emptyColl = false;
            HWIdentifier hwid = (*it)->adc_HWID();
            // determine gain
            int gain = m_tileHWID->adc(hwid);
            // add m_nDrawers to index if hi gain
            int index = type;
            if (gain == 1) index += m_nDrawers;

            // determine channel
            channel = m_tileHWID->channel(hwid);
            // cabling for testbeam (convert to pmt#-1)


            // gain determined for all digits in collection
            (m_gainflxVec.at(index))[channel] = gain;
            ATH_MSG_DEBUG( "Storing TD for channel: " << channel
                           << " with gain " << m_tileHWID->adc(hwid)
                           << " index " << index );

            // get digits
            if (gain == 0) sampleVecLo = (*it)->samples();
            if (gain == 1) sampleVecHi = (*it)->samples();
            int sizLo = sampleVecLo.size();
            int sizHi = sampleVecHi.size();


            if (msgLvl(MSG::DEBUG)) {
              if (sizLo > 0 ){
                 msg(MSG::DEBUG) << "Low gain Digits(" << sizLo << ")." << (dcnt++) << " {";
                 for (int i = 0; i < sizLo; i++) {
                    msg(MSG::DEBUG) << (int) sampleVecLo[i] << " ";
                 }

                 if (sizLo > m_nSamplesFlx) {
                    msg(MSG::DEBUG) << "} ONLY " << m_nSamplesFlx << " digits saved to ntuple" << endmsg;
                 } else {
                    msg(MSG::DEBUG) << "}" << endmsg;
                 }
              }

              if (sizHi > 0 ){
                 msg(MSG::DEBUG) << "High gain Digits(" << sizHi << ")." << (dcnt++) << " {";
                 for (int i = 0; i < sizHi; i++) {
                    msg(MSG::DEBUG) << (int) sampleVecHi[i] << " ";
                 }

                 if (sizHi > m_nSamplesFlx) {
                    msg(MSG::DEBUG) << "} ONLY " << m_nSamplesFlx << " digits saved to ntuple" << endmsg;
                 } else {
                    msg(MSG::DEBUG) << "}" << endmsg;
                 }
              }


            }
            if (sizLo > m_nSamplesFlx) sizLo = m_nSamplesFlx;
            if (sizHi > m_nSamplesFlx) sizHi = m_nSamplesFlx;
            for (n = 0; n < sizLo; n++) {
              (m_sampleflxVec.at(index))[channel][n] = (int) sampleVecLo[n];
            }
            for (n = 0; n < sizHi; n++) {
              (m_sampleflxVec.at(index))[channel][n] = (int) sampleVecHi[n];
            }
            sampleVecLo.clear();
            sampleVecHi.clear();

          }
        }



    }
    // next container
  }

  if (emptyColl)
    return StatusCode::FAILURE;
  else
    return StatusCode::SUCCESS;
}












/*
/// Fill Ntuple with MC truth info from simulation
/// Namely, hit energies directly from Geant4
 */
StatusCode TileTBAANtuple::storeHitVector() {

  if (m_hitVector.size() == 0) return StatusCode::FAILURE;
  ATH_MSG_DEBUG( "Event# " << m_evtNr << " reading Hit Vector");

  // Read Hit Vector from TDS
  const TileHitVector *hitVec;
  CHECK( evtStore()->retrieve(hitVec, m_hitVector) );

  SG::ReadCondHandle<TileSamplingFraction> samplingFraction(m_samplingFractionKey);
  ATH_CHECK( samplingFraction.isValid() );

  // Go through all TileHit
  for (const TileHit& cinp : *hitVec) {

    // get hits
    HWIdentifier hwid = cinp.pmt_HWID();

    // determine type of frag
    int fragId = m_tileHWID->frag(hwid);
    drawerMap_iterator itr = m_drawerMap.find(fragId);
    int type = ( itr != m_drawerMap.end() ) ? (*itr).second : -1;

    if (type < 0) {
      ATH_MSG_WARNING( "frag id 0x" << MSG::hex << fragId << MSG::dec <<" was not found among valid frag IDs when storing HITS!" );

    } else {
      int fragType = m_drawerType[type];
      storeHit(&cinp,fragType,fragId,m_ehitVec.at(type),m_thitVec.at(type), *samplingFraction);
    }
  }

  if (hitVec->empty())
    return StatusCode::FAILURE;
  else
    return StatusCode::SUCCESS;
}


/**
/// Fill Ntuple with MC truth info from simulation
/// Namely, hit energies corrected by photoelectron statistics and Birks' law
 */
StatusCode TileTBAANtuple::storeHitContainer() {

  if (m_hitContainer.size() == 0) return StatusCode::FAILURE;
  ATH_MSG_DEBUG( "Event# " << m_evtNr << " reading Hit Container");

  // Read Hit Container from TDS
  const TileHitContainer *hitCnt;
  ATH_CHECK( evtStore()->retrieve(hitCnt, m_hitContainer) );

  SG::ReadCondHandle<TileSamplingFraction> samplingFraction(m_samplingFractionKey);
  ATH_CHECK( samplingFraction.isValid() );

  // Get iterator for all collections in container
  TileHitContainer::const_iterator itColl = (*hitCnt).begin();
  TileHitContainer::const_iterator itCollEnd = (*hitCnt).end();
  bool emptyColl = true;

  // Go through all TileHitCollections
  for(; itColl != itCollEnd; ++itColl) {

    // determine type of frag
    int fragId = (*itColl)->identify();
    drawerMap_iterator itr = m_drawerMap.find(fragId);
    int type = ( itr != m_drawerMap.end() ) ? (*itr).second : -1;

    if (type < 0) {
      if ( (*itColl)->begin() != (*itColl)->end() )
        ATH_MSG_WARNING( "frag id 0x" << MSG::hex << fragId << MSG::dec <<" was not found among valid frag IDs when storing HITS!" );

    } else {
      int fragType = m_drawerType[type];

      ATH_MSG_DEBUG( "Event# " << m_evtNr
                     << " Frag id 0x" << MSG::hex << fragId << MSG::dec
                     << " index " << type );

      // go through all TileHit in collection
      TileHitCollection::const_iterator it = (*itColl)->begin();
      TileHitCollection::const_iterator itEnd = (*itColl)->end();
      if (emptyColl) emptyColl = (it==itEnd);

      for (; it != itEnd; ++it) {
        // get hits
        const TileHit * cinp = (*it);
        storeHit(cinp,fragType,fragId,m_ehitCnt.at(type),m_thitCnt.at(type),*samplingFraction);
      }
    }
  }

  if (emptyColl)
    return StatusCode::FAILURE;
  else
    return StatusCode::SUCCESS;
}

void TileTBAANtuple::storeHit(const TileHit *cinp, int fragType, int fragId, float* ehitVec, float* thitVec,
                              const TileSamplingFraction* samplingFraction) {

  // determine channel
  HWIdentifier hwid = cinp->pmt_HWID();
  int channel = m_tileHWID->channel(hwid);

  int size = cinp->size();
  if (msgLvl(MSG::VERBOSE)) {
    msg(MSG::VERBOSE) << "hit hwid="
                      << m_tileHWID->to_string(hwid, -1) << " ener=";

    for (int i = 0; i < size; ++i)
      msg(MSG::VERBOSE) << cinp->energy(i) << " ";

    msg(MSG::VERBOSE) << "time=";
    for (int i = 0; i < size; ++i)
      msg(MSG::VERBOSE) << cinp->time(i) << " ";

    msg(MSG::VERBOSE) << endmsg;
  }

  double ehit=0.0, thit=0.0;
  for(int i=0;i<size;++i) {

    double e = cinp->energy(i);
    double t = cinp->time(i);

    if (-75.<t && t<75.) {
      ehit += e;
      thit += e*t;
    }
  }

  if (ehit!=0) {
    thit /= ehit;
    // conversion factor from hit energy to final energy units 
    int drawerIdx = TileCalibUtils::getDrawerIdxFromFragId(fragId);
    ehit *= samplingFraction->getSamplingFraction(drawerIdx, channel);
    if (m_rchUnit != TileRawChannelUnit::MegaElectronVolts) {
      ehit /= m_tileToolEmscale->channelCalib(drawerIdx, channel, TileID::HIGHGAIN, 1.,
                                              m_rchUnit, TileRawChannelUnit::MegaElectronVolts);
    }
  } else {
    thit=0.0;
  }

  // cabling for testbeam
  if ((m_TBperiod < 2015 || 
       (m_TBperiod==2015 && fragType<3) ||
       ((m_TBperiod==2016 || m_TBperiod==2021) && ((fragId&0xFF)<4 && fragId != 0x201)) ||
       (m_TBperiod==2017 && ((fragId&0xFF)<4 && !(fragId == 0x201 || fragId == 0x203))) ||
       (m_TBperiod==2018 && ((fragId&0xFF)<4 && !(fragId == 0x201 || fragId == 0x402))) ||
       (m_TBperiod==2019 && ((fragId&0xFF)<5 && !(fragId == 0x201 || fragId == 0x203 || fragId >= 0x402))) ||
       (m_TBperiod==2022 && ((fragId&0xFF)<4 && !(fragId == 0x201 || (m_run >= 2210456 && fragId == 0x402)))))
      && fragType > 0 && m_pmtOrder)
    channel = digiChannel2PMT(fragType, channel);

  ehitVec[channel] = ehit;
  thitVec[channel] = thit;

  ATH_MSG_DEBUG( "HIT ene=" << ehit
                 << " time=" << thit
                 << " pmt-1=" << channel
                 << " index " << m_drawerMap.find(fragId)->second );

}
StatusCode TileTBAANtuple::ntuple_clear() {

  TRIGGER_clearBranch();

  if (m_beamIdList[MUON_ADC_FRAG] || m_beamIdList[ADDR_ADC_FRAG]) {
    MUON_clearBranch();
  }

  if (m_beamIdList[ECAL_ADC_FRAG]) {
    ECAL_clearBranch();
    if (m_TBperiod > 2015) QDC_clearBranch();
  }

  LASER_clearBranch();

  if (m_unpackAdder) {
    if (m_beamIdList[ADD_FADC_FRAG]) {
      ADDER_clearBranch();
    }
  }

  if (m_beamIdList[DIGI_PAR_FRAG & 0x1F]) {
    CISPAR_clearBranch();
  }

  if (((m_TBperiod >= 2015 || m_unpackAdder) && m_beamIdList[BEAM_ADC_FRAG])
      || (!m_unpackAdder && m_beamIdList[COMMON_ADC1_FRAG])
      || (m_TBperiod >= 2022 && m_beamIdList[COMMON_TOF_FRAG])) {
    BEAM_clearBranch();
    ATH_MSG_VERBOSE( "clear branch");
  }

  if (m_completeNtuple && m_TBperiod < 2015) {
    ENETOTAL_clearBranch();
  }

  COINCBOARD_clearBranch();

  DIGI_clearBranch(); // working now
  if (m_saveFelixData) {
    FELIX_clearBranch();
  }

  HIT_clearBranch();
  ATH_MSG_DEBUG( "clear() successfully" );

  return StatusCode::SUCCESS;
}


StatusCode TileTBAANtuple::initNTuple(void) {
  MsgStream log(msgSvc(), name());

  m_evtVec.clear();
  m_bcidVec.clear();
  m_DMUheaderVec.clear();
  m_DMUformatErrVec.clear();
  m_DMUparityErrVec.clear();
  m_DMUmemoryErrVec.clear();
  m_DMUDstrobeErrVec.clear();
  m_DMUSstrobeErrVec.clear();
  m_rodBCIDVec.clear();
  m_sizeVec.clear();
  m_dmuMaskVec.clear();
  m_slinkCRCVec.clear();
  m_gainVec.clear();
  m_sampleVec.clear();
  m_feCRCVec.clear();
  m_rodCRCVec.clear();

  m_evtflxVec.clear();
  m_rodBCIDflxVec.clear();
  m_sizeflxVec.clear();
  m_gainflxVec.clear();
  m_sampleflxVec.clear();

  m_eneVec.clear();
  m_timeVec.clear();
  m_pedFlatVec.clear();
  m_chi2FlatVec.clear();

  m_efitVec.clear();
  m_tfitVec.clear();
  m_pedfitVec.clear();
  m_chi2Vec.clear();

  m_efitcVec.clear();
  m_tfitcVec.clear();
  m_pedfitcVec.clear();
  m_chi2cVec.clear();

  m_eOptVec.clear();
  m_tOptVec.clear();
  m_pedOptVec.clear();
  m_chi2OptVec.clear();

  m_eflxfitVec.clear();
  m_tflxfitVec.clear();
  m_pedflxfitVec.clear();
  m_chi2flxfitVec.clear();

  m_eflxoptVec.clear();
  m_tflxoptVec.clear();
  m_pedflxoptVec.clear();
  m_chi2flxoptVec.clear();

  m_eDspVec.clear();
  m_tDspVec.clear();
  m_chi2DspVec.clear();

  m_ROD_GlobalCRCVec.clear();
  m_ROD_DMUBCIDVec.clear();
  m_ROD_DMUmemoryErrVec.clear();
  m_ROD_DMUSstrobeErrVec.clear();
  m_ROD_DMUDstrobeErrVec.clear();
  m_ROD_DMUHeadformatErrVec.clear();
  m_ROD_DMUHeadparityErrVec.clear();
  m_ROD_DMUDataformatErrVec.clear();
  m_ROD_DMUDataparityErrVec.clear();
  m_ROD_DMUMaskVec.clear();

  //Ntuple creation

  m_ntuplePtr = new TTree("h1000", "TileBEAM-Ntuple");
  m_ntuplePtr->SetMaxTreeSize(m_treeSize);

  if (m_thistSvc->regTree("/" + m_streamName + "/h1000", m_ntuplePtr).isFailure()) {
    ATH_MSG_ERROR( "Problem registering TileRec Tree" );
    m_ntupleCreated = false;
  } else {
    m_ntupleCreated = true;
  }

  TRIGGER_addBranch();
  MUON_addBranch();

  if (m_TBperiod < 2015) {
    ECAL_addBranch();
    LASER_addBranch();
    ADDER_addBranch();
    ENETOTAL_addBranch();
    COINCBOARD_addBranch();
  }

  if (m_TBperiod > 2015) {
    QDC_addBranch();
  }

  CISPAR_addBranch();
  BEAM_addBranch();
  DIGI_addBranch(); //working now
  if (m_saveFelixData) {
    FELIX_addBranch();
  }

  HIT_addBranch();

  return StatusCode::SUCCESS;
}

StatusCode TileTBAANtuple::storeCells() {


  const CaloCellContainer* cellContainer;
  //Retrieve Cell collection from SG

  ATH_MSG_DEBUG( "retrieve cellContainer from SG" );

  CHECK( evtStore()->retrieve(cellContainer, "AllCalo") );

  //Loop over all cells in container. Sum up the Energy and fill 2DHistograms
  ATH_MSG_DEBUG( "succeeded retrieving cellContainer from SG" );

  CaloCellContainer::const_iterator ifirst = cellContainer->begin();
  CaloCellContainer::const_iterator ilast = cellContainer->end();
  ATH_MSG_DEBUG( "TileTBAANtuple : about to iterate over CaloCells" );

  m_LarEne[0] = m_LarEne[1] = m_LarEne[2] = m_LarEne[3] = 0.0;
  m_BarEne[0] = m_BarEne[1] = m_BarEne[2] = 0.0;
  m_ExtEne[0] = m_ExtEne[1] = m_ExtEne[2] = 0.0;
  m_GapEne[0] = m_GapEne[1] = m_GapEne[2] = 0.0;

  for (; ifirst != ilast; ++ifirst) {
    //Decode cell information
    const CaloCell* cell_ptr = *ifirst;    // pointer to cell object
    const double energy = cell_ptr->energy();
    const CaloDetDescrElement* caloDDE_ptr = cell_ptr->caloDDE(); //pointer to the DetectorDescriptionElement
    const CaloCell_ID::CaloSample sampl = caloDDE_ptr->getSampling(); //To which sampling belongs this cell?
//    const CaloCell_ID::SUBCALO subcalo= caloDDE_ptr->getSubCalo();

    if (sampl == CaloCell_ID::PreSamplerB) {
      m_LarEne[0] += energy;
    } else if (sampl == CaloCell_ID::EMB1) {
      m_LarEne[1] += energy;
    } else if (sampl == CaloCell_ID::EMB2) {
      m_LarEne[2] += energy;
    } else if (sampl == CaloCell_ID::EMB3) {
      m_LarEne[3] += energy;
    } else if (sampl == CaloCell_ID::TileBar0) {
      m_BarEne[0] += energy;
    } else if (sampl == CaloCell_ID::TileBar1) {
      m_BarEne[1] += energy;
    } else if (sampl == CaloCell_ID::TileBar2) {
      m_BarEne[2] += energy;
    } else if (sampl == CaloCell_ID::TileExt0) {
      m_ExtEne[0] += energy;
    } else if (sampl == CaloCell_ID::TileExt1) {
      m_ExtEne[1] += energy;
    } else if (sampl == CaloCell_ID::TileExt2) {
      m_ExtEne[2] += energy;
    } else if (sampl == CaloCell_ID::TileGap1) {
      m_GapEne[1] += energy;
    } else if (sampl == CaloCell_ID::TileGap2) {
      m_GapEne[2] += energy;
    } else if (sampl == CaloCell_ID::TileGap3) {
      m_GapEne[0] += energy;
    }
  }

  return StatusCode::SUCCESS;
}



StatusCode TileTBAANtuple::initList() {

  unsigned int size = m_drawerList.size();

  if (size > 0) {

    int frag = strtol(m_drawerList[0].data(), NULL, 0);

    if (frag < 0) { // setup frags IDs from the data

      // Read Digits from TDS
      const TileDigitsContainer *digitsCnt;
      if (evtStore()->retrieve(digitsCnt, m_digitsContainer).isFailure()) {
        ATH_MSG_ERROR( "can't retrieve Digits from TDS" );
        ATH_MSG_ERROR( "can't set up fragment list for ntuple" );

        if (m_nSamples != 0) {
          ATH_MSG_WARNING( "Disable digit samples in ntuple" );
          m_nSamples = 0;
        }

        return StatusCode::SUCCESS;
      }

      // Get iterator for all TDColl in TDCont
      TileDigitsContainer::const_iterator itColl = (*digitsCnt).begin();
      TileDigitsContainer::const_iterator itCollEnd = (*digitsCnt).end();

      std::vector<unsigned int> frags;
      // Go through all TileDigitsCollections
      for (; itColl != itCollEnd; ++itColl) {
        // determine type of frag
        if ((*itColl)->begin() != (*itColl)->end()) frags.push_back((*itColl)->identify());
      }
      size = frags.size();

      if (size > 0) {

        if (size < m_nDrawers) {
          ATH_MSG_INFO( "decreasing m_nDrawers from " << m_nDrawers << " to " << size );
          m_nDrawers = size;
        }

        unsigned int rosOrder[5] = { 2, 1, 3, 4, 0 };
        unsigned int dr = 0;
        char frg[6] = "0x000";

        m_drawerList.clear();
        // m_drawerType.clear();
        m_drawerMap.clear();

        msg(MSG::INFO) << "setting drawerList from data " << MSG::hex;
        for (unsigned int ir = 0; ir < 5; ++ir) {
          for (unsigned int i = 0; i < size; ++i) {
            unsigned int frag = frags[i];
            if (frag >> 8 == rosOrder[ir]) {
              sprintf(frg, "0x%3.3x", frag);
              m_drawerList.push_back((std::string) frg);
              if (dr == m_drawerType.size()) m_drawerType.push_back(frag >> 8);
              m_drawerMap[frag] = dr;
              msg(MSG::INFO) << " 0x" << frag;
              ++dr;
            }
          }
        }

        msg(MSG::INFO) << MSG::dec << endmsg;

        size = m_drawerType.size();
        if (size < m_nDrawers) {
          m_drawerType.resize(m_nDrawers);
          for (; size < m_nDrawers; ++size)
            m_drawerType[size] = 0;
        }

        msg(MSG::INFO) << MSG::INFO << "drawerType ";
        for (unsigned int dr = 0; dr < size; ++dr)
          msg(MSG::INFO) << " " << m_drawerType[dr];
        msg(MSG::INFO) << endmsg;

        if (size > m_nDrawers) {
          ATH_MSG_INFO( "increasing m_nDrawers from " << m_nDrawers << " to " << size );
          m_nDrawers = size;
        }

        if (size < 1) size = 1;
        if (m_eventsPerFile == 0) {
          m_eventsPerFile = (int) (200 / size) * 1000;
          ATH_MSG_INFO( "Number of events per file was 0, set it to 200k/" << size << " = " << m_eventsPerFile );
        }

      } else {

        ATH_MSG_ERROR( "can't find any TileDigits collections" );
        ATH_MSG_ERROR( "can't set up fragment list for ntuple" );
      }
    }


    // once again - check number of samples in the data
    // but do not print any ERRORs now

    // Read Digits from TDS
    const TileDigitsContainer *digitsCnt;

    if (evtStore()->retrieve(digitsCnt, m_digitsContainer).isSuccess()) {

      // Get iterator for all TDColl in TDCont
      TileDigitsContainer::const_iterator itColl = (*digitsCnt).begin();
      TileDigitsContainer::const_iterator itCollEnd = (*digitsCnt).end();

      // Go through all TileDigitsCollections
      for (; itColl != itCollEnd; ++itColl) {
        if ((*itColl)->begin() != (*itColl)->end()) {
          int siz = (*(*itColl)->begin())->samples().size();
          m_nSamplesInDrawerMap[(*itColl)->identify()] = siz;
          if (siz > m_nSamples && m_nSamples != 0) {
            ATH_MSG_WARNING( "Increasing number of digit samples in ntuple from " << m_nSamples << " to " << siz );
            m_nSamples = siz;
          }
        }
      }
    } else {
      if (m_nSamples != 0) {
        ATH_MSG_WARNING( "Disable digit samples in ntuple" );
        m_nSamples = 0;
      }
    }
  }

  return StatusCode::SUCCESS;
}
















StatusCode TileTBAANtuple::initListFlx() {

  unsigned int size = m_drawerList.size();

  if (size > 0) {

    int frag = strtol(m_drawerList[0].data(), NULL, 0);

    if (frag < 0) { // setup frags IDs from the data

      // Read Digits from TDS

      const TileDigitsContainer *digitsCntFlx;
      if (evtStore()->retrieve(digitsCntFlx, m_digitsContainerFlx).isFailure()) {
        ATH_MSG_ERROR( "can't retrieve Felix Digits from TDS" );
        ATH_MSG_ERROR( "can't set up fragment list for ntuple" );

        if (m_nSamplesFlx != 0) {
          ATH_MSG_WARNING( "Disable Felix digit samples in ntuple" );
          m_nSamplesFlx = 0;
        }

        return StatusCode::SUCCESS;
      }



      // Get iterator for all TDColl in TDCont
      TileDigitsContainer::const_iterator itColl = (*digitsCntFlx).begin();
      TileDigitsContainer::const_iterator itCollEnd = (*digitsCntFlx).end();

      std::vector<unsigned int> frags;
      // Go through all TileDigitsCollections
      for (; itColl != itCollEnd; ++itColl) {
        // determine type of frag
        if ((*itColl)->begin() != (*itColl)->end()) frags.push_back((*itColl)->identify());
      }
      size = frags.size();

      if (size > 0) {

        if (size < m_nDrawers) {
          ATH_MSG_INFO( "decreasing m_nDrawers from " << m_nDrawers << " to " << size );
          m_nDrawers = size;
        }

        unsigned int rosOrder[5] = { 2, 1, 3, 4, 0 };
        unsigned int dr = 0;
        char frg[6] = "0x000";

        m_drawerList.clear();
        // m_drawerType.clear();
        m_drawerMap.clear();

        msg(MSG::INFO) << "setting drawerList from data " << MSG::hex;
        for (unsigned int ir = 0; ir < 5; ++ir) {
          for (unsigned int i = 0; i < size; ++i) {
            unsigned int frag = frags[i];
            if (frag >> 8 == rosOrder[ir]) {
              sprintf(frg, "0x%3.3x", frag);
              m_drawerList.push_back((std::string) frg);
              if (dr == m_drawerType.size()) m_drawerType.push_back(frag >> 8);
              m_drawerMap[frag] = dr;
              msg(MSG::INFO) << " 0x" << frag;
              ++dr;
            }
          }
        }

        msg(MSG::INFO) << MSG::dec << endmsg;

        size = m_drawerType.size();
        if (size < m_nDrawers) {
          m_drawerType.resize(m_nDrawers);
          for (; size < m_nDrawers; ++size)
            m_drawerType[size] = 0;
        }

        msg(MSG::INFO) << MSG::INFO << "drawerType ";
        for (unsigned int dr = 0; dr < size; ++dr)
          msg(MSG::INFO) << " " << m_drawerType[dr];
        msg(MSG::INFO) << endmsg;

        if (size > m_nDrawers) {
          ATH_MSG_INFO( "increasing m_nDrawers from " << m_nDrawers << " to " << size );
          m_nDrawers = size;
        }

        if (size < 1) size = 1;
        if (m_eventsPerFile == 0) {
          m_eventsPerFile = (int) (200 / size) * 1000;
          ATH_MSG_INFO( "Number of events per file was 0, set it to 200k/" << size << " = " << m_eventsPerFile );
        }

      } else {

        ATH_MSG_ERROR( "can't find any TileDigits collections" );
        ATH_MSG_ERROR( "can't set up fragment list for ntuple" );
      }
    }


    // once again - check number of samples in the data
    // but do not print any ERRORs now

    // Read Digits from TDS

    const TileDigitsContainer *digitsCntFlx;
    if (evtStore()->retrieve(digitsCntFlx, m_digitsContainerFlx).isSuccess()) {
       TileDigitsContainer::const_iterator itColl = (*digitsCntFlx).begin();
       TileDigitsContainer::const_iterator itCollEnd = (*digitsCntFlx).end();

       for (; itColl != itCollEnd; ++itColl) {
        if ((*itColl)->begin() != (*itColl)->end()) {
          int siz = (*(*itColl)->begin())->samples().size();
          m_nSamplesInDrawerMap[(*itColl)->identify()] = siz;
          if (siz > m_nSamplesFlx && m_nSamplesFlx != 0) {
            ATH_MSG_WARNING( "Increasing number of FELIX  digit samples in ntuple from " << m_nSamplesFlx << " to " << siz );
            m_nSamplesFlx = siz;
          }
        }
      }
    } else {
      if (m_nSamplesFlx != 0) {
        ATH_MSG_WARNING( "Disable FELIX digit samples in ntuple" );
        m_nSamplesFlx = 0;
      }
    }


  }

  return StatusCode::SUCCESS;
}



//////////////////////////////////////////////////////////////////////////////
void TileTBAANtuple::getEta() {

  // Get eta from an ASCII file with the following structure :
  // runnumber eta

  ATH_MSG_INFO( "Get eta for run " << m_runNumber );

  // Find the full path to filename:
  std::string fileName = PathResolver::find_file(m_etaFileName, "DATAPATH");
  ATH_MSG_INFO( "Reading file  " << fileName );

  if (fileName.size() == 0) {

    ATH_MSG_WARNING( "Could not find input file " << m_etaFileName );
    ATH_MSG_WARNING( "Skip reading of eta value " );

  } else {

    std::ifstream etafile;
    etafile.open(fileName.c_str());

    if (etafile.good()) {

      int runNumber = 0;
      float eta = 0;
      while ((runNumber != m_runNumber) && (!etafile.eof())) {
        etafile >> runNumber >> eta;
      }

      if (runNumber != m_runNumber) {
        ATH_MSG_INFO( "Run " << m_runNumber << " has not been found, keep eta and theta at zero" );

        m_eta = m_theta = 0.0;
      } else {
        m_eta = eta;
        m_theta = (M_PI_2 - 2 * atan(exp(m_eta)));
        ATH_MSG_INFO( "Run " << m_runNumber << " has been found with eta=" << m_eta << ", theta =" << m_theta );

      }

    } else {

      ATH_MSG_WARNING( "Problem with file " << fileName );
      ATH_MSG_WARNING( "Skip reading of eta value " );

    }

    etafile.close();
  }
}

/**
/////////////////////////////////////////////////////////////////////////////
//
///    Variables Legenda
///
///  - C : a character string terminated by the 0 character
///  - B : an 8 bit signed integer
///  - b : an 8 bit unsigned integer                    2^8=256
///  - S : a 16 bit signed integer (i.e. a "short")
///  - s : a 16 bit unsigned integer                    2^16=65536
///  - I : a 32 bit signed integer (i.e an "int")
///  - i : a 32 bit unsigned integer                    2^32=4294967296
///  - F : a 32 bit floating point (i.e. a "float")
///  - D : a 64 bit floating point (i.e. a "double")
///  - L : a 64 bit signed integer
///  - l : a 64 bit unsigned integer
///  - O : a boolean
//
*/
/**
//////////////////////////////////////////////////////////////////////////////
///Add TRIGGER variables to the Tree
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::TRIGGER_addBranch(void)
{

  m_ntuplePtr->Branch("EvTime",&m_evTime,"EvTime/I");
  m_ntuplePtr->Branch("Run",&m_run,"Run/I");
  m_ntuplePtr->Branch("Evt",&m_evt,"Evt/I");
  m_ntuplePtr->Branch("Trig",&m_trigType,"Trig/S");
  m_ntuplePtr->Branch("DSPflags",&m_dspFlags,"DSPflags/I");
  m_ntuplePtr->Branch("DSPunits",&m_dspUnit,"DSPunits/S");
  m_ntuplePtr->Branch("OFLunits",&m_rchUnit,"OFLunits/S");

  if ( m_completeNtuple ) {
    // Info from ROD headers
    // m_nDrawers drawers separately (i.e. with duplications) + Beam ROD
    m_l1ID = new std::vector<int>;
    m_l1Type = new std::vector<int>;
    m_evType = new std::vector<int>;
    m_evBCID = new std::vector<int>;
    m_frBCID = new std::vector<int>;

    m_ntuplePtr->Branch("L1ID",&m_l1ID);
    m_ntuplePtr->Branch("L1Type",&m_l1Type);
    m_ntuplePtr->Branch("EvType",&m_evType);
    m_ntuplePtr->Branch("EvBCID",&m_evBCID);
    m_ntuplePtr->Branch("FrBCID",&m_frBCID);
  }


}

/**
//////////////////////////////////////////////////////////////////////////////
///Add MUON variables to the Tree
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::MUON_addBranch(void)
{

  if ( m_beamIdList[MUON_ADC_FRAG] ||
       m_beamIdList[ADDR_ADC_FRAG] )
    {
      m_ntuplePtr->Branch("MuBackHit",&m_muBackHit,"MuBackHit/F");
      m_ntuplePtr->Branch("MuBackSum",&m_muBackSum,"MuBackSum/F");

      m_muBack = new float[14];
      m_muCalib = new float[2];
      if (m_TBperiod < 2015) {
        m_ntuplePtr->Branch("MuBack",m_muBack,"m_muBack[14]");
        m_ntuplePtr->Branch("MuCalib",m_muCalib,"m_muCalib[2]");
      } else if (m_TBperiod == 2015) {
        m_ntuplePtr->Branch("MuBack",m_muBack,"MuBack[8]/F");
      } else {
        m_ntuplePtr->Branch("MuBack",m_muBack,"MuBack[12]/F");
      }
    }


}

/**
//////////////////////////////////////////////////////////////////////////////
///Add ECAL variables to the Tree
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::ECAL_addBranch(void)
{

  if (m_beamIdList[ECAL_ADC_FRAG]) {
    m_ecal = new  float[8];
    m_ntuplePtr->Branch("Ecal",m_ecal,"m_ecal[8]/F");
  }

}

/**
//////////////////////////////////////////////////////////////////////////////
///Add QDC variables to the Tree
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::QDC_addBranch(void)
{

  if (m_beamIdList[ECAL_ADC_FRAG]) {
    m_qdc = new  uint32_t[33];
    for(unsigned i=0; i<33; ++i){ m_qdc[i]=0.0; }
    m_ntuplePtr->Branch("qdc", m_qdc, "qdc[33]/i");
  }

}


/**
//////////////////////////////////////////////////////////////////////////////
//Clear Tree TRIGGER variables
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::TRIGGER_clearBranch(void) {

  m_evTime = 0;
  m_run = 0;
  m_evt = 0;
  m_trigType = 0;

  if (m_completeNtuple) {

    m_l1ID->clear();
    m_l1Type->clear();
    m_evType->clear();
    m_evBCID->clear();
    m_frBCID->clear();

    if (m_nDrawers > 0) {

      m_l1ID->resize(m_nDrawers + 1);
      m_l1Type->resize(m_nDrawers + 1);
      m_evType->resize(m_nDrawers + 1);
      m_evBCID->resize(m_nDrawers + 1);
      m_frBCID->resize(m_nDrawers);
    }

  }
}

/**
//////////////////////////////////////////////////////////////////////////////
//Clear Tree MUON variables
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::MUON_clearBranch(void) {
  m_muBackHit = 0.;
  m_muBackSum = 0.;

}

/**
//////////////////////////////////////////////////////////////////////////////
//Clear Tree ECAL variables
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::ECAL_clearBranch(void)
{

}

/**
//////////////////////////////////////////////////////////////////////////////
//Clear Tree QDC variables
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::QDC_clearBranch(void)
{
  for(unsigned i=0; i<33; ++i){ m_qdc[i]=0.0; }
}


/**
//////////////////////////////////////////////////////////////////////////////
///Add Tree LASER variables Tree
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::LASER_addBranch(void) {

  if (m_laserObject.size() > 0) {

    m_ntuplePtr->Branch("LASER_BCID", &m_las_BCID, "LASER_BCID/I");

    m_ntuplePtr->Branch("LASER_FILTER", &m_las_Filt, "LASER_FILTER/I");
    m_ntuplePtr->Branch("LASER_REQAMP", &m_las_ReqAmp, "LASER_REQAMP/D");
    m_ntuplePtr->Branch("LASER_MEASAMP", &m_las_MeasAmp, "LASER_MEASAMP/D");

    m_ntuplePtr->Branch("LASER_Diode_1_ADC", &m_las_D1_ADC, "LASER_Diode_1_ADC/I");
    m_ntuplePtr->Branch("LASER_Diode_2_ADC", &m_las_D2_ADC, "LASER_Diode_2_ADC/I");
    m_ntuplePtr->Branch("LASER_Diode_3_ADC", &m_las_D3_ADC, "LASER_Diode_3_ADC/I");
    m_ntuplePtr->Branch("LASER_Diode_4_ADC", &m_las_D4_ADC, "LASER_Diode_4_ADC/I");

    m_ntuplePtr->Branch("LASER_Diode_1_Ped", &m_las_D1_Ped, "LASER_Diode_1_Ped/D");
    m_ntuplePtr->Branch("LASER_Diode_2_Ped", &m_las_D2_Ped, "LASER_Diode_2_Ped/D");
    m_ntuplePtr->Branch("LASER_Diode_3_Ped", &m_las_D3_Ped, "LASER_Diode_3_Ped/D");
    m_ntuplePtr->Branch("LASER_Diode_4_Ped", &m_las_D4_Ped, "LASER_Diode_4_Ped/D");

    m_ntuplePtr->Branch("LASER_Diode_1_Ped_RMS", &m_las_D1_Ped_RMS, "LASER_Diode_1_Ped_RMS/D");
    m_ntuplePtr->Branch("LASER_Diode_2_Ped_RMS", &m_las_D2_Ped_RMS, "LASER_Diode_2_Ped_RMS/D");
    m_ntuplePtr->Branch("LASER_Diode_3_Ped_RMS", &m_las_D1_Ped_RMS, "LASER_Diode_3_Ped_RMS/D");
    m_ntuplePtr->Branch("LASER_Diode_4_Ped_RMS", &m_las_D1_Ped_RMS, "LASER_Diode_4_Ped_RMS/D");

    m_ntuplePtr->Branch("LASER_Diode_1_Alpha", &m_las_D1_Alpha, "LASER_Diode_1_Alpha/D");
    m_ntuplePtr->Branch("LASER_Diode_2_Alpha", &m_las_D2_Alpha, "LASER_Diode_2_Alpha/D");
    m_ntuplePtr->Branch("LASER_Diode_3_Alpha", &m_las_D3_Alpha, "LASER_Diode_3_Alpha/D");
    m_ntuplePtr->Branch("LASER_Diode_4_Alpha", &m_las_D4_Alpha, "LASER_Diode_4_Alpha/D");

    m_ntuplePtr->Branch("LASER_Diode_1_Alpha_RMS", &m_las_D1_Alpha_RMS, "LASER_Diode_1_Alpha_RMS/D");
    m_ntuplePtr->Branch("LASER_Diode_2_Alpha_RMS", &m_las_D2_Alpha_RMS, "LASER_Diode_2_Alpha_RMS/D");
    m_ntuplePtr->Branch("LASER_Diode_3_Alpha_RMS", &m_las_D3_Alpha_RMS, "LASER_Diode_3_Alpha_RMS/D");
    m_ntuplePtr->Branch("LASER_Diode_4_Alpha_RMS", &m_las_D4_Alpha_RMS, "LASER_Diode_4_Alpha_RMS/D");

    m_ntuplePtr->Branch("LASER_Diode_1_AlphaPed", &m_las_D1_AlphaPed, "LASER_Diode_1_AlphaPed/D");
    m_ntuplePtr->Branch("LASER_Diode_2_AlphaPed", &m_las_D2_AlphaPed, "LASER_Diode_2_AlphaPed/D");
    m_ntuplePtr->Branch("LASER_Diode_3_AlphaPed", &m_las_D3_AlphaPed, "LASER_Diode_3_AlphaPed/D");
    m_ntuplePtr->Branch("LASER_Diode_4_AlphaPed", &m_las_D4_AlphaPed, "LASER_Diode_4_AlphaPed/D");

    m_ntuplePtr->Branch("LASER_Diode_1_AlphaPed_RMS", &m_las_D1_AlphaPed_RMS, "LASER_Diode_1_AlphaPed_RMS/D");
    m_ntuplePtr->Branch("LASER_Diode_2_AlphaPed_RMS", &m_las_D2_AlphaPed_RMS, "LASER_Diode_2_AlphaPed_RMS/D");
    m_ntuplePtr->Branch("LASER_Diode_3_AlphaPed_RMS", &m_las_D3_AlphaPed_RMS, "LASER_Diode_3_AlphaPed_RMS/D");
    m_ntuplePtr->Branch("LASER_Diode_4_AlphaPed_RMS", &m_las_D4_AlphaPed_RMS, "LASER_Diode_4_AlphaPed_RMS/D");

    m_ntuplePtr->Branch("LASER_PMT_1_ADC", &m_las_PMT1_ADC, "LASER_PMT_1_ADC/I");
    m_ntuplePtr->Branch("LASER_PMT_2_ADC", &m_las_PMT2_ADC, "LASER_PMT_2_ADC/I");

    m_ntuplePtr->Branch("LASER_PMT_1_TDC", &m_las_PMT1_TDC, "LASER_PMT_1_TDC/I");
    m_ntuplePtr->Branch("LASER_PMT_2_TDC", &m_las_PMT2_TDC, "LASER_PMT_2_TDC/I");

    m_ntuplePtr->Branch("LASER_PMT_1_Ped", &m_las_PMT1_Ped, "LASER_PMT_1_Ped/D");
    m_ntuplePtr->Branch("LASER_PMT_2_Ped", &m_las_PMT2_Ped, "LASER_PMT_2_Ped/D");

    m_ntuplePtr->Branch("LASER_PMT_1_Ped_RMS", &m_las_PMT1_Ped_RMS, "LASER_PMT_1_Ped_RMS/D");
    m_ntuplePtr->Branch("LASER_PMT_2_Ped_RMS", &m_las_PMT2_Ped_RMS, "LASER_PMT_2_Ped_RMS/D");

    m_ntuplePtr->Branch("LASER_HEAD_Temp", &m_las_Temperature, "LASER_HEAD_Temp/D");
  }

  if (m_beamIdList[LASE_PTN_FRAG]) {
    m_ntuplePtr->Branch("LasFlag", &m_lasFlag, "LasFlag/s");
  }

  if ((m_unpackAdder && m_beamIdList[BEAM_ADC_FRAG])
      || (!m_unpackAdder && m_beamIdList[LASE_ADC_FRAG])) {
    m_ntuplePtr->Branch("Las0", &m_las0, "Las0/F");
    m_ntuplePtr->Branch("Las1", &m_las1, "Las1/F");
    m_ntuplePtr->Branch("Las2", &m_las2, "Las2/F");
    m_ntuplePtr->Branch("Las3", &m_las3, "Las3/F");
    m_lasExtra = new float[4];
    m_ntuplePtr->Branch("LasExtra", m_lasExtra, "m_lasExtra[4]/F");
  }

}

/**
//////////////////////////////////////////////////////////////////////////////
//Clear Tree LASER variables
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::LASER_clearBranch(void)
{

  if (m_laserObject.size() > 0) {

    m_las_BCID = 0;
    m_las_D1_ADC = 0;
    m_las_D2_ADC = 0;
    m_las_D3_ADC = 0;
    m_las_D4_ADC = 0;

    m_las_D1_Ped = 0;
    m_las_D2_Ped = 0;
    m_las_D3_Ped = 0;
    m_las_D4_Ped = 0;

    m_las_D1_Alpha = 0;
    m_las_D2_Alpha = 0;
    m_las_D3_Alpha = 0;
    m_las_D4_Alpha = 0;

    m_las_PMT1_ADC = 0;
    m_las_PMT2_ADC = 0;

    m_las_PMT1_TDC = 0;
    m_las_PMT2_TDC = 0;

    m_las_PMT1_Ped = 0;
    m_las_PMT2_Ped = 0;
  }

  if (m_beamIdList[LASE_PTN_FRAG]) {
    m_lasFlag=0;
  }

  if ((m_unpackAdder && m_beamIdList[BEAM_ADC_FRAG])
      || (!m_unpackAdder && m_beamIdList[LASE_ADC_FRAG])) {

    m_las0 = 0.;
    m_las1 = 0.;
    m_las2 = 0.;
    m_las3 = 0.;

  }
}

/**
//////////////////////////////////////////////////////////////////////////////
///Add Tree ADDER variables Tree
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::ADDER_addBranch(void) {

  if (m_unpackAdder) {
    if (m_beamIdList[ADD_FADC_FRAG]) {
      m_adder = (int**) malloc(16 * sizeof(int *));
      m_adder[0] = (int*) malloc(16 * 16 * sizeof(int));
      for (int j = 1; j < 16; j++) {
        m_adder[j] = m_adder[0] + j * 16;
      }

      m_eneAdd = new float[16];
      m_timeAdd = new float[16];

      m_ntuplePtr->Branch("Adder", *m_adder, "m_adder[16][16]/I");
      m_ntuplePtr->Branch("EneAdd", m_eneAdd, "m_eneAdd[16]/F");
      m_ntuplePtr->Branch("TimeAdd", m_timeAdd, "m_timeAdd[16]/F");
    }
  }
}

/**
//////////////////////////////////////////////////////////////////////////////
//Clear Tree ADDER variables
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::ADDER_clearBranch(void)
{


}

/**
//////////////////////////////////////////////////////////////////////////////
///Add Tree CISPAR variables Tree
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::CISPAR_addBranch(void) {

  if (m_beamIdList[DIGI_PAR_FRAG & 0x1F]) {

    m_ntuplePtr->Branch("cispar", m_cispar, "cispar[16]/I");
  }
}

/**
//////////////////////////////////////////////////////////////////////////////
//Clear Tree CISPAR variables
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::CISPAR_clearBranch(void)
{
  memset(m_cispar,-1,sizeof(m_cispar));
}


/**
//////////////////////////////////////////////////////////////////////////////
///Add Tree BEAM variables Tree
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::BEAM_addBranch(void) {

    if (((m_TBperiod >= 2015 || m_unpackAdder) && m_beamIdList[BEAM_ADC_FRAG])
	|| ((!m_unpackAdder || m_TBperiod >= 2022) && m_beamIdList[COMMON_ADC1_FRAG])) {

    m_ntuplePtr->Branch("S1cou", &m_s1cou, "S1cou/S");
    m_ntuplePtr->Branch("S2cou", &m_s2cou, "S2cou/S");
    if (m_TBperiod >= 2016 && m_TBperiod < 2022) {
      m_ntuplePtr->Branch("Cher3", &m_s3cou, "Cher3/S"); // dropped S3 in favor of Cher3 in September 2016 TB
     // m_ntuplePtr->Branch("SpmFinger", &m_muVeto, "SmpFinger/S"); //  muVeto replaced by SPM for testing
      m_ntuplePtr->Branch("SiPM1", &m_muTag, "SiPM1/S"); //  muTag replaced by SiPM1 for testing
      m_ntuplePtr->Branch("SiPM2", &m_muHalo, "SiPM2/S");// muHalo repalce by SiPM1 for testing
    } else {
      m_ntuplePtr->Branch("S3cou", &m_s3cou, "S3cou/S");
    }

    m_ntuplePtr->Branch("Cher1", &m_cher1, "Cher1/S");
    m_ntuplePtr->Branch("Cher2", &m_cher2, "Cher2/S");
    if (m_TBperiod >= 2022) {
      m_ntuplePtr->Branch("Cher3", &m_cher3, "Cher3/S");
    }

    if (m_TBperiod < 2015) {
      m_ntuplePtr->Branch("MuTag", &m_muTag, "MuTag/S");
      m_ntuplePtr->Branch("MuHalo", &m_muHalo, "MuHalo/S");
      m_ntuplePtr->Branch("MuVeto", &m_muVeto, "MuVeto/S");
    //} else if (m_TBperiod == 2015) {
      // nothing
    } else if (m_TBperiod >= 2016) {
      m_ntuplePtr->Branch("SCalo1", &m_muTag, "SCalo1/S");
      m_ntuplePtr->Branch("SCalo2", &m_muHalo, "SCalo2/S");
      //m_ntuplePtr->Branch("SCalo3", &m_muVeto, "SCalo3/S");
    }
  }

  if (m_TBperiod >= 2015) {
    if (m_beamIdList[COMMON_TDC1_FRAG] || (m_TBperiod >= 2022 && m_beamIdList[COMMON_TOF_FRAG])) {
      m_btdc1 = new int[16];
      m_btdc2 = new int[16];
      m_btdc = new std::vector<std::vector<int> >(16);
      m_ntuplePtr->Branch("btdc1", m_btdc1, "m_btdc1[16]/I");
      m_ntuplePtr->Branch("btdc2", m_btdc2, "m_btdc2[16]/I");
      m_ntuplePtr->Branch("btdc", &m_btdc);
      m_ntuplePtr->Branch("tjitter", &m_tjitter, "tjitter/I");
      m_ntuplePtr->Branch("tscTOF", &m_tscTOF, "tscTOF/I");
      m_ntuplePtr->Branch("btdcNhit", m_btdcNhit, "btdcNhit[16]/I");
      m_ntuplePtr->Branch("btdcNchMultiHit", m_btdcNchMultiHit, "btdcNchMultiHit[2]/I");
    }
    if (m_beamIdList[COMMON_TOF_FRAG]) {
      m_tof = new int[16];
      m_ntuplePtr->Branch("tof", m_tof, "m_tof[16]/I");
    }
    if (m_beamIdList[COMMON_PTN_FRAG]) {
      m_scaler = new int[16];
      m_ntuplePtr->Branch("scaler", m_scaler, "m_scaler[16]/I");
    }
  } else if (!m_unpackAdder) {
    if (m_beamIdList[COMMON_ADC2_FRAG]) {
      m_ntuplePtr->Branch("S2extra", &m_s2extra, "S2extra/S");
      m_ntuplePtr->Branch("S2cou", &m_s3extra, "S3extra/S");
    }
    if (m_beamIdList[BEAM_ADC_FRAG]) {
      m_ntuplePtr->Branch("SC1", &m_sc1, "SC1/S");
      m_ntuplePtr->Branch("SC2", &m_sc2, "SC2/S");
    }
    if (m_beamIdList[COMMON_PTN_FRAG]) {
      m_ntuplePtr->Branch("pu", &m_commonPU, "pu/S");
    }
    if (m_beamIdList[COMMON_TOF_FRAG]) {
      m_tof = new int[8];
      m_ntuplePtr->Branch("tof", m_tof, "m_tof[8]/I");
    }
    if (m_beamIdList[COMMON_TDC1_FRAG]) {
      m_btdc1 = new int[16];
      m_ntuplePtr->Branch("btdc1", m_btdc1, "m_btdc1/I");
    }
    if (m_beamIdList[COMMON_TDC2_FRAG]) {
      m_btdc2 = new int[16];
      m_ntuplePtr->Branch("btdc2", m_btdc2, "m_btdc2/I");
    }

    if (m_beamIdList[COMMON_TDC1_FRAG]) {
      m_ntuplePtr->Branch("XchN2", &m_xChN2, "XchN2/F");
      m_ntuplePtr->Branch("YchN2", &m_yChN2, "YchN2/F");
      m_ntuplePtr->Branch("XchN1", &m_xChN1, "XchN1/F");
      m_ntuplePtr->Branch("YchN1", &m_yChN1, "YchN1/F");
    }
    if (m_beamIdList[COMMON_TDC2_FRAG]) {
      m_ntuplePtr->Branch("Xcha0", &m_xCha0, "Xcha0/F");
      m_ntuplePtr->Branch("Ycha0", &m_yCha0, "Ycha0/F");
    }
  } else {
    if (m_beamIdList[BEAM_TDC_FRAG]) {
      m_btdc1 = new int[16];
      m_btdc2 = new int[16];
      m_ntuplePtr->Branch("btdc1", m_btdc1, "m_btdc1[16]/I");
      m_ntuplePtr->Branch("btdc2", m_btdc2, "m_btdc2[16]/I");
    }
  }

  if ((m_TBperiod >= 2015 && m_beamIdList[COMMON_TDC1_FRAG])
      || (m_unpackAdder && m_beamIdList[BEAM_TDC_FRAG])
      || (!m_unpackAdder && m_beamIdList[COMMON_TDC2_FRAG])
      || (m_TBperiod >= 2022 && m_beamIdList[COMMON_TOF_FRAG])) {

    m_ntuplePtr->Branch("Xcha1", &m_xCha1, "Xcha1/F");
    m_ntuplePtr->Branch("Ycha1", &m_yCha1, "Ycha1/F");
    m_ntuplePtr->Branch("Xcha2", &m_xCha2, "Xcha2/F");
    m_ntuplePtr->Branch("Ycha2", &m_yCha2, "Ycha2/F");
    m_ntuplePtr->Branch("Xcha1_0", &m_xCha1_0, "Xcha1_0/F");
    m_ntuplePtr->Branch("Ycha1_0", &m_yCha1_0, "Ycha1_0/F");
    m_ntuplePtr->Branch("Xcha2_0", &m_xCha2_0, "Xcha2_0/F");
    m_ntuplePtr->Branch("Ycha2_0", &m_yCha2_0, "Ycha2_0/F");
    m_ntuplePtr->Branch("Ximp", &m_xImp, "Ximp/F");
    m_ntuplePtr->Branch("Yimp", &m_yImp, "Yimp/F");
    m_ntuplePtr->Branch("Ximp_0", &m_xImp_0, "Ximp_0/F");
    m_ntuplePtr->Branch("Yimp_0", &m_yImp_0, "Yimp_0/F");
    m_ntuplePtr->Branch("Ximp_90", &m_xImp_90, "Ximp_90/F");
    m_ntuplePtr->Branch("Yimp_90", &m_yImp_90, "Yimp_90/F");
    m_ntuplePtr->Branch("Ximp_min90", &m_xImp_min90, "Ximp_min90/F");
    m_ntuplePtr->Branch("Yimp_min90", &m_yImp_min90, "Yimp_min90/F");
  }

}

/**
//////////////////////////////////////////////////////////////////////////////
//Clear Tree BEAM variables
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::BEAM_clearBranch(void) {

  if ((m_unpackAdder && m_beamIdList[BEAM_ADC_FRAG])
      || (!m_unpackAdder && m_beamIdList[COMMON_ADC1_FRAG])) {

    m_s1cou = 0;
    m_s2cou = 0;
    m_s3cou = 0;
    m_cher1 = 0;
    m_cher2 = 0;
    m_cher3 = 0;
    m_muTag = 0;
    m_muHalo = 0;
    m_muVeto = 0;
  }
  if (!m_unpackAdder) {
    if (m_beamIdList[COMMON_ADC2_FRAG]) {

      m_s2extra = 0;
      m_s3extra = 0;
    }
    if (m_beamIdList[BEAM_ADC_FRAG]) {
      m_sc1 = 0;
      m_sc2 = 0;
    }
    if (m_beamIdList[COMMON_PTN_FRAG]) {
      m_commonPU = 0;
    }
    if (m_beamIdList[COMMON_TDC1_FRAG]) {
      m_xChN2 = 0.;
      m_yChN2 = 0.;
      m_xChN1 = 0.;
      m_yChN1 = 0.;
    }
    if (m_beamIdList[COMMON_TDC2_FRAG]) {
      m_xCha0 = 0.;
      m_yCha0 = 0.;
    }
  }

  if ((m_TBperiod >= 2015 && m_beamIdList[COMMON_TDC1_FRAG])
      || (m_unpackAdder && m_beamIdList[BEAM_TDC_FRAG])
      || (!m_unpackAdder && m_beamIdList[COMMON_TDC2_FRAG])
      || (m_TBperiod >= 2022 && m_beamIdList[COMMON_TOF_FRAG])) {

    m_xCha1 = 0.;
    m_yCha1 = 0.;
    m_xCha2 = 0.;
    m_yCha2 = 0.;
    m_xCha1_0 = 0.;
    m_yCha1_0 = 0.;
    m_xCha2_0 = 0.;
    m_yCha2_0 = 0.;
    m_xImp = 0.;
    m_yImp = 0.;
  }

  if (m_tof) {
    for (int i=0; i<16; i+=2) {
      m_tof[i] = +0xFFFF;
      m_tof[i+1] = -0xFFFF;
    }
  }

  if (m_btdc1) {
    for (int i=0; i<16; i+=2) {
      m_btdc1[i] = +0xFFFF;
      m_btdc1[i+1] = -0xFFFF;
    }
  }
  if (m_btdc2) {
    for (int i=0; i<16; i+=2) {
      m_btdc2[i] = +0xFFFF;
      m_btdc2[i+1] = -0xFFFF;
    } 
  } 
  if (m_btdc) {
    for (std::vector<int>& btdc_amplitudes : *m_btdc) {
      btdc_amplitudes.clear();
    }
  }

  if (m_scaler) {
    memset(m_scaler, 0, sizeof(*m_scaler));
  }

  memset(m_btdcNhit,0,sizeof(m_btdcNhit));
  memset(m_btdcNchMultiHit,0,sizeof(m_btdcNchMultiHit));
}

/**
//////////////////////////////////////////////////////////////////////////////
///Add Tree ENETOTAL variables Tree
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::ENETOTAL_addBranch(void)
{
  if (m_completeNtuple) {
    m_LarEne = new float[4];
    m_BarEne = new float[3];
    m_ExtEne = new float[3];
    m_GapEne = new float[3];
    m_ntuplePtr->Branch("LarSmp", m_LarEne, ",m_LarEne[4]/F");
    m_ntuplePtr->Branch("BarSmp", m_BarEne, "m_BarEne[3]/F");
    m_ntuplePtr->Branch("ExtSmp", m_ExtEne, "m_ExtEne[3]/F");
    m_ntuplePtr->Branch("GapSmp", m_GapEne, "m_GapEne[3]/F");
  }
}

/**
//////////////////////////////////////////////////////////////////////////////
//Clear Tree ENETOTAL variables
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::ENETOTAL_clearBranch(void)
{


}

/**
//////////////////////////////////////////////////////////////////////////////
///Add Tree COINCBOARD variables Tree
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::COINCBOARD_addBranch(void) {
  if (m_beamIdList[COIN_TRIG1_FRAG]) {
    m_coincTrig1 = new uint32_t[96];
    m_ntuplePtr->Branch("CoincTrig1", m_coincTrig1, "m_coincTrig1[96]/I");
    m_ntuplePtr->Branch("CoincFlag1", &m_coincFlag1, "CoincFlag1/I");
  }

  if (m_beamIdList[COIN_TRIG2_FRAG]) {
    m_coincTrig2 = new uint32_t[96];
    m_ntuplePtr->Branch("CoincTrig2", m_coincTrig2, "m_coincTrig2[96]/I");
    m_ntuplePtr->Branch("CoincFlag2", &m_coincFlag2, "CoincFlag2/I");
  }

  if (m_beamIdList[COIN_TRIG3_FRAG]) {
    m_coincTrig3 = new uint32_t[96];
    m_ntuplePtr->Branch("CoincTrig3", m_coincTrig3, "m_coincTrig3[96]/I");
    m_ntuplePtr->Branch("CoincFlag3", &m_coincFlag3, "CoincFlag3/I");
  }

  if (m_beamIdList[COIN_TRIG4_FRAG]) {
    m_coincTrig4 = new uint32_t[96];
    m_ntuplePtr->Branch("CoincTrig4", m_coincTrig4, "m_coincTrig4[96]/I");
    m_ntuplePtr->Branch("CoincFlag4", &m_coincFlag4, "CoincFlag4/I");
  }
  if (m_beamIdList[COIN_TRIG5_FRAG]) {
    m_coincTrig5 = new uint32_t[96];
    m_ntuplePtr->Branch("CoincTrig5", m_coincTrig5, "m_coincTrig5[96]/I");
    m_ntuplePtr->Branch("CoincFlag5", &m_coincFlag5, "CoincFlag5/I");
  }

  if (m_beamIdList[COIN_TRIG6_FRAG]) {
    m_coincTrig6 = new uint32_t[96];
    m_ntuplePtr->Branch("CoincTrig6", m_coincTrig6, "m_coincTrig6[96]/I");
    m_ntuplePtr->Branch("CoincFlag6", &m_coincFlag6, "CoincFlag6/I");
  }

  if (m_beamIdList[COIN_TRIG7_FRAG]) {
    m_coincTrig7 = new uint32_t[96];
    m_ntuplePtr->Branch("CoincTrig7", m_coincTrig7, "m_coincTrig7[96]/I");
    m_ntuplePtr->Branch("CoincFlag7", &m_coincFlag7, "CoincFlag7/I");
  }

  if (m_beamIdList[COIN_TRIG8_FRAG]) {
    m_coincTrig8 = new uint32_t[96];
    m_ntuplePtr->Branch("CoincTrig8", m_coincTrig8, "m_coincTrig8[96]/I");
    m_ntuplePtr->Branch("CoincFlag8", &m_coincFlag8, "CoincFlag8/I");
  }

}

/**
//////////////////////////////////////////////////////////////////////////////
//Clear Tree COINCBOARD variables
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::COINCBOARD_clearBranch(void)
{



}

/**
//////////////////////////////////////////////////////////////////////////////
///Add Tree DIGI variables Tree
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::DIGI_addBranch(void)
{


  /// Reserve is needed, because, if the vector is displaced, then the position declared in the Branch i not longer valid.
  /// Can be done a cleaner way?
  m_bcidVec.reserve(MAX_DRAWERS);
  m_DMUheaderVec.reserve(MAX_DRAWERS);
  m_DMUformatErrVec.reserve(MAX_DRAWERS);
  m_DMUparityErrVec.reserve(MAX_DRAWERS);
  m_DMUmemoryErrVec.reserve(MAX_DRAWERS);
  m_DMUDstrobeErrVec.reserve(MAX_DRAWERS);
  m_DMUSstrobeErrVec.reserve(MAX_DRAWERS);
  m_dmuMaskVec.reserve(MAX_DRAWERS);
  m_slinkCRCVec.reserve(MAX_DRAWERS);
  m_gainVec.reserve(MAX_DRAWERS);
  m_sampleVec.reserve(MAX_DRAWERS);
  m_feCRCVec.reserve(MAX_DRAWERS);
  m_rodCRCVec.reserve(MAX_DRAWERS);
  m_eneVec.reserve(MAX_DRAWERS);
  m_timeVec.reserve(MAX_DRAWERS);
  m_pedFlatVec.reserve(MAX_DRAWERS);
  m_chi2FlatVec.reserve(MAX_DRAWERS);
  m_efitVec.reserve(MAX_DRAWERS);
  m_tfitVec.reserve(MAX_DRAWERS);
  m_pedfitVec.reserve(MAX_DRAWERS);
  m_chi2Vec.reserve(MAX_DRAWERS);
  m_efitcVec.reserve(MAX_DRAWERS);
  m_tfitcVec.reserve(MAX_DRAWERS);
  m_pedfitcVec.reserve(MAX_DRAWERS);
  m_chi2cVec.reserve(MAX_DRAWERS);
  m_eOptVec.reserve(MAX_DRAWERS);
  m_tOptVec.reserve(MAX_DRAWERS);
  m_pedOptVec.reserve(MAX_DRAWERS);
  m_chi2OptVec.reserve(MAX_DRAWERS);
  m_eDspVec.reserve(MAX_DRAWERS);
  m_tDspVec.reserve(MAX_DRAWERS);
  m_chi2DspVec.reserve(MAX_DRAWERS);
  m_ROD_GlobalCRCVec.reserve(MAX_DRAWERS);
  m_ROD_DMUBCIDVec.reserve(MAX_DRAWERS);
  m_ROD_DMUmemoryErrVec.reserve(MAX_DRAWERS);
  m_ROD_DMUSstrobeErrVec.reserve(MAX_DRAWERS);
  m_ROD_DMUDstrobeErrVec.reserve(MAX_DRAWERS);
  m_ROD_DMUHeadformatErrVec.reserve(MAX_DRAWERS);
  m_ROD_DMUHeadparityErrVec.reserve(MAX_DRAWERS);
  m_ROD_DMUDataformatErrVec.reserve(MAX_DRAWERS);
  m_ROD_DMUDataparityErrVec.reserve(MAX_DRAWERS);
  m_ROD_DMUMaskVec.reserve(MAX_DRAWERS);

  std::ostringstream oss;
  oss << m_nSamples;
  std::string nSampStr=oss.str();

  unsigned int listSize = std::min (m_nDrawers, static_cast<unsigned int>(m_drawerMap.size()) );

  if (listSize > 0) {

    std::string digit[10] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
    std::vector<std::string> suffixArr;
    unsigned int length;
    bool testbeam = TileCablingService::getInstance()->getTestBeam();

    if (m_calibMode) {

      length = 2 * m_nDrawers;
      suffixArr.resize(length);

      for (unsigned int i = 0; i < listSize; ++i) {
        unsigned int ros = m_drawerType[i];
        unsigned int drawer = strtol(m_drawerList[i].data(), NULL, 0) & 0x3F;
        std::string digits;
        if (m_TBperiod >= 2010) {
          ++drawer; // count modules from 1
          digits = digit[drawer / 10] + digit[drawer % 10];
        } else if (testbeam) {
          digits = digit[drawer & 7];
        } else {
          ++drawer; // count modules from 1
          digits = digit[drawer / 10] + digit[drawer % 10];
        }

        if (ros == 0) {
          std::string suff = m_drawerList[i];
          suff.replace(suff.find("0x"), 2, "");
          suffixArr[i] = suff + "lo";
          suffixArr[i + m_nDrawers] = suff + "hi";
        } else {
          suffixArr[i] = m_rosName[ros] + digits + "lo";
          suffixArr[i + m_nDrawers] = m_rosName[ros] + digits + "hi";
        }
      }
    } else {

      length = m_nDrawers;
      suffixArr.resize(length);

      for (unsigned int i = 0; i < listSize; ++i) {
        unsigned int ros = m_drawerType[i];
        unsigned int drawer = strtol(m_drawerList[i].data(), NULL, 0) & 0x3F;
        std::string digits;
        if (m_TBperiod >= 2010) {
          ++drawer; // count modules from 1
          digits = digit[drawer / 10] + digit[drawer % 10];
        } else if (testbeam) {
          digits = digit[drawer & 7];
        } else {
          ++drawer; // count modules from 1
          digits = digit[drawer / 10] + digit[drawer % 10];
        }

        if (ros == 0) {
          std::string suff = m_drawerList[i];
          suff.replace(suff.find("0x"), 2, "");
          suffixArr[i] = suff;
        } else {
          suffixArr[i] = m_rosName[ros] + digits;
        }
      }
    }

    //    m_ntuplePtr->Branch("Samples",&m_nSamples,"m_nSamples/I");

    for (unsigned int i = 0; i < length; i++) {

      int nSamplesInDrawer(0);
      int frag = std::stoi(m_drawerList[i%m_nDrawers], nullptr, 0);

      auto it = m_nSamplesInDrawerMap.find(frag);
      if (it != m_nSamplesInDrawerMap.end()) {
	nSamplesInDrawer = it->second;
      }

      if (i % m_nDrawers < listSize)
        ATH_MSG_DEBUG( "Adding items for " << suffixArr[i] );

      int *evt = new int;
      short *rodBCID = new short;
      short *size = new short;

      int *bcid = new int[N_DMUS];
      uint32_t *DMUheader = new uint32_t[N_DMUS];
      short *DMUformatErr = new short[N_DMUS];
      short *DMUparityErr = new short[N_DMUS];
      short *DMUmemoryErr = new short[N_DMUS];
      short *DMUDstrobeErr = new short[N_DMUS];
      short *DMUSstrobeErr = new short[N_DMUS];

      short *ROD_GlobalCRC = new short[1];
      short *ROD_DMUBCID = new short[N_DMUS];
      short *ROD_DMUmemoryErr = new short[N_DMUS];
      short *ROD_DMUSstrobeErr = new short[N_DMUS];
      short *ROD_DMUDstrobeErr = new short[N_DMUS];
      short *ROD_DMUHeadformatErr = new short[N_DMUS];
      short *ROD_DMUHeadparityErr = new short[N_DMUS];
      short *ROD_DMUDataformatErr = new short[N_DMUS];
      short *ROD_DMUDataparityErr = new short[N_DMUS];
      short *ROD_DMUMask = new short[2];

      int *dmu = new int[2];
      int *crc = new int[2];
      int *gain = new int[N_CHANS];

      int **sample = (int**) malloc(N_CHANS * sizeof(int *));
      sample[0] = (int*) malloc(N_CHANS * m_nSamples * sizeof(int));
      memset(sample[0], -1, sizeof(int) * m_nSamples * N_CHANS);
      if (m_calibMode) nSamplesInDrawer = std::max(m_nSamples,nSamplesInDrawer);
      for (int j = 1; j < N_CHANS; j++) {
        sample[j] = sample[0] + j * nSamplesInDrawer;
      }

      int *fe_crc = new int[N_DMUS];
      int *rod_crc = new int[N_DMUS];

      float *ene = new float[N_CHANS];
      float *time = new float[N_CHANS];
      float *pedflat = new float[N_CHANS];
      float *chi2Flat = new float[N_CHANS];

      float *efit = new float[N_CHANS];
      float *tfit = new float[N_CHANS];
      float *pedfit = new float[N_CHANS];
      float *chi2 = new float[N_CHANS];

      float *efitc = new float[N_CHANS];
      float *tfitc = new float[N_CHANS];
      float *pedfitc = new float[N_CHANS];
      float *chi2c = new float[N_CHANS];

      float *eOpt = new float[N_CHANS];
      float *tOpt = new float[N_CHANS];
      float *pedOpt = new float[N_CHANS];
      float *chi2Opt = new float[N_CHANS];

      float *eDsp = new float[N_CHANS];
      float *tDsp = new float[N_CHANS];
      float *chi2Dsp = new float[N_CHANS];


     m_evtVec.push_back( evt ); // I
     m_bcidVec.push_back( bcid ); // U
     m_DMUheaderVec.push_back( DMUheader ); // U32
     m_DMUformatErrVec.push_back( DMUformatErr ); // U32
     m_DMUparityErrVec.push_back( DMUparityErr ); // U32
     m_DMUmemoryErrVec.push_back( DMUmemoryErr ); // U32
     m_DMUDstrobeErrVec.push_back( DMUDstrobeErr ); // U32
     m_DMUSstrobeErrVec.push_back( DMUSstrobeErr ); // U32

     m_rodBCIDVec.push_back( rodBCID  ); // U
     m_sizeVec.push_back( size  ); // U
     m_dmuMaskVec.push_back( dmu ); // U(2)
     m_slinkCRCVec.push_back( crc ); // U(2)
     m_gainVec.push_back( gain ); // U(48/96)
     m_sampleVec.push_back( sample ); // U(48/96,9)
     m_feCRCVec.push_back( fe_crc ); //U
     m_rodCRCVec.push_back( rod_crc ); //U

     m_eneVec.push_back( ene );
     m_timeVec.push_back( time );
     m_pedFlatVec.push_back( pedflat );
     m_chi2FlatVec.push_back( chi2Flat );

     m_efitVec.push_back( efit );
     m_tfitVec.push_back( tfit );
     m_pedfitVec.push_back( pedfit );
     m_chi2Vec.push_back( chi2 );

     m_efitcVec.push_back( efitc );
     m_tfitcVec.push_back( tfitc );
     m_pedfitcVec.push_back( pedfitc );
     m_chi2cVec.push_back( chi2c );

     m_eOptVec.push_back( eOpt );
     m_tOptVec.push_back( tOpt );
     m_pedOptVec.push_back( pedOpt );
     m_chi2OptVec.push_back( chi2Opt );

     m_eDspVec.push_back( eDsp );
     m_tDspVec.push_back( tDsp );
     m_chi2DspVec.push_back( chi2Dsp );

     m_ROD_GlobalCRCVec.push_back( ROD_GlobalCRC );
     m_ROD_DMUBCIDVec.push_back( ROD_DMUBCID );
     m_ROD_DMUmemoryErrVec.push_back( ROD_DMUmemoryErr );
     m_ROD_DMUSstrobeErrVec.push_back( ROD_DMUSstrobeErr );
     m_ROD_DMUDstrobeErrVec.push_back( ROD_DMUDstrobeErr );
     m_ROD_DMUHeadformatErrVec.push_back( ROD_DMUHeadformatErr );
     m_ROD_DMUHeadparityErrVec.push_back( ROD_DMUHeadparityErr );
     m_ROD_DMUDataformatErrVec.push_back( ROD_DMUDataformatErr );
     m_ROD_DMUDataparityErrVec.push_back( ROD_DMUDataparityErr );
     m_ROD_DMUMaskVec.push_back( ROD_DMUMask );

     if (i%m_nDrawers < listSize) {
       // create ntuple layout
       ATH_MSG_DEBUG( "Adding Leaf to Event '" << ("Evt" + suffixArr[i]) << "' @" << evt );

      if (m_bsInput) {
         m_ntuplePtr->Branch(("Evt"+suffixArr[i]).c_str(),evt,("Evt"+suffixArr[i]+"/I").c_str()); // int
         m_ntuplePtr->Branch(("rodBCID"+suffixArr[i]).c_str(),rodBCID,("rodBCID"+suffixArr[i]+"/S").c_str());//int
         m_ntuplePtr->Branch(("Size"+suffixArr[i]).c_str(),size,("Size"+suffixArr[i]+"/S").c_str()); // short
         m_ntuplePtr->Branch(("BCID"+suffixArr[i]).c_str(),m_bcidVec.back(),("bcid"+suffixArr[i]+"[16]/I").c_str()); // int
         m_ntuplePtr->Branch(("DMUheader"+suffixArr[i]).c_str(),m_DMUheaderVec.back(),("DMUheader"+suffixArr[i]+"[16]/i").c_str()); // uint32
         m_ntuplePtr->Branch(("DMUformatErr"+suffixArr[i]).c_str(),m_DMUformatErrVec.back(),("DMUformatErr"+suffixArr[i]+"[16]/S").c_str()); // short
         m_ntuplePtr->Branch(("DMUparityErr"+suffixArr[i]).c_str(),m_DMUparityErrVec.back(),("DMUparityErr"+suffixArr[i]+"[16]/S").c_str()); // short
         m_ntuplePtr->Branch(("DMUmemoryErr"+suffixArr[i]).c_str(),m_DMUmemoryErrVec.back(),("DMUmemoryErr"+suffixArr[i]+"[16]/S").c_str()); // short
         m_ntuplePtr->Branch(("DMUSstrobeErr"+suffixArr[i]).c_str(),m_DMUSstrobeErrVec.back(),("DMUSstrobeErr"+suffixArr[i]+"[16]/S").c_str()); // short
         m_ntuplePtr->Branch(("DMUDstrobeErr"+suffixArr[i]).c_str(),m_DMUDstrobeErrVec.back(),("DMUDstrobeErr"+suffixArr[i]+"[16]/S").c_str()); // short
         m_ntuplePtr->Branch(("DMUMask"+suffixArr[i]).c_str(),m_dmuMaskVec.back(),("dmumask"+suffixArr[i]+"[2]/I").c_str()); // int
         m_ntuplePtr->Branch(("SlinkCRC"+suffixArr[i]).c_str(),m_slinkCRCVec.back(),("crc"+suffixArr[i]+"[2]/I").c_str()); // int
      }

      m_ntuplePtr->Branch(("Gain"+suffixArr[i]).c_str(),m_gainVec.back(),("gain"+suffixArr[i]+"[48]/I").c_str()); // int

       if (nSamplesInDrawer > 0) {
	 
	 nSampStr = std::to_string(nSamplesInDrawer);

	 m_ntuplePtr->Branch(("Sample" + suffixArr[i]).c_str(), *sample,
			     ("sample" + suffixArr[i] + "[48]["+nSampStr+"]/I").c_str()); // size m_nsample and type int
          //m_ntuplePtr->Branch(("Sample"+suffixArr[i]).c_str(),&(m_sampleVec.back())); // size m_nsample and type double (but int in reality)
        }

        if (m_bsInput) {
          m_ntuplePtr->Branch(("feCRC" + suffixArr[i]).c_str(), m_feCRCVec.back(), ("fe_crc" + suffixArr[i] + "[16]/I").c_str()); // int
          m_ntuplePtr->Branch(("rodCRC" + suffixArr[i]).c_str(), m_rodCRCVec.back(), ("rod_crc" + suffixArr[i] + "[16]/I").c_str()); // int
        }

        if (m_flatRawChannelContainer.size() > 0) {

          m_ntuplePtr->Branch(("Ene" + suffixArr[i]).c_str(), m_eneVec.back(), ("ene" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Time" + suffixArr[i]).c_str(), m_timeVec.back(), ("time" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Ped" + suffixArr[i]).c_str(), m_pedFlatVec.back(), ("pedflat" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Chi2ene" + suffixArr[i]).c_str(), m_chi2FlatVec.back(), ("chiflat" + suffixArr[i] + "[48]/F").c_str()); // float

        }

       if (m_fitRawChannelContainer.size() > 0) {

          m_ntuplePtr->Branch(("Efit" + suffixArr[i]).c_str(), m_efitVec.back(), ("efit" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Tfit" + suffixArr[i]).c_str(), m_tfitVec.back(), ("tfit" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Pedfit" + suffixArr[i]).c_str(), m_pedfitVec.back(), ("pedfit" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Chi2fit" + suffixArr[i]).c_str(), m_chi2Vec.back(), ("chifit" + suffixArr[i] + "[48]/F").c_str()); // float

        }

        if (m_fitcRawChannelContainer.size() > 0) {

          m_ntuplePtr->Branch(("Efitc" + suffixArr[i]).c_str(), m_efitcVec.back(), ("efitc" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Tfitc" + suffixArr[i]).c_str(), m_tfitcVec.back(), ("tfitc" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Pedfitc" + suffixArr[i]).c_str(), m_pedfitcVec.back(), ("pedfitc" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Chi2fitc" + suffixArr[i]).c_str(), m_chi2cVec.back(), ("chifitc" + suffixArr[i] + "[48]/F").c_str()); // float

        }

       if (m_optRawChannelContainer.size() > 0) {

         m_ntuplePtr->Branch(("Eopt"+suffixArr[i]).c_str(),m_eOptVec.back(),("eOpt"+suffixArr[i]+"[48]/F").c_str()); // float
         m_ntuplePtr->Branch(("Topt"+suffixArr[i]).c_str(),m_tOptVec.back(),("tOpt"+suffixArr[i]+"[48]/F").c_str()); // float
         m_ntuplePtr->Branch(("Pedopt"+suffixArr[i]).c_str(),m_pedOptVec.back(),("pedOpt"+suffixArr[i]+"[48]/F").c_str()); // float
         m_ntuplePtr->Branch(("Chi2opt"+suffixArr[i]).c_str(),m_chi2OptVec.back(),("chiOpt"+suffixArr[i]+"[48]/F").c_str()); // float

       }

       if (m_dspRawChannelContainer.size() > 0) {

         m_ntuplePtr->Branch(("Edsp"+suffixArr[i]).c_str(),m_eDspVec.back(),("eDsp"+suffixArr[i]+"[48]/F").c_str()); // float
         m_ntuplePtr->Branch(("Tdsp"+suffixArr[i]).c_str(),m_tDspVec.back(),("tDsp"+suffixArr[i]+"[48]/F").c_str()); // float
         m_ntuplePtr->Branch(("Chi2dsp"+suffixArr[i]).c_str(),m_chi2DspVec.back(),("chiDsp"+suffixArr[i]+"[48]/F").c_str()); // float
         m_ntuplePtr->Branch(("ROD_GlobalCRC"+suffixArr[i]).c_str(),m_ROD_GlobalCRCVec.back(),("ROD_GlobalCRC"+suffixArr[i]+"/s").c_str()); // unsigned short
         m_ntuplePtr->Branch(("ROD_DMUBCID"+suffixArr[i]).c_str(),m_ROD_DMUBCIDVec.back(),("ROD_DMUBCID"+suffixArr[i]+"[16]/s").c_str()); // unsigned short
         m_ntuplePtr->Branch(("ROD_DMUmemoryErr"+suffixArr[i]).c_str(),m_ROD_DMUmemoryErrVec.back(),("ROD_DMUmemoryErr"+suffixArr[i]+"[16]/s").c_str()); // unsigned short
         m_ntuplePtr->Branch(("ROD_DMUSstrobeErr"+suffixArr[i]).c_str(),m_ROD_DMUSstrobeErrVec.back(),("ROD_DMUSstrobeErr"+suffixArr[i]+"[16]/s").c_str()); // unsigned short
         m_ntuplePtr->Branch(("ROD_DMUDstrobeErr"+suffixArr[i]).c_str(),m_ROD_DMUDstrobeErrVec.back(),("ROD_DMUDstrobeErr"+suffixArr[i]+"[16]/s").c_str()); // unsigned short
         m_ntuplePtr->Branch(("ROD_DMUHeadformatErr"+suffixArr[i]).c_str(),m_ROD_DMUHeadformatErrVec.back(),("ROD_DMUHeadformatErr"+suffixArr[i]+"[16]/s").c_str()); // unsigned short
         m_ntuplePtr->Branch(("ROD_DMUHeadparityErr"+suffixArr[i]).c_str(),m_ROD_DMUHeadparityErrVec.back(),("ROD_DMUHeadparityErr"+suffixArr[i]+"[16]/s").c_str()); // unsigned short
         m_ntuplePtr->Branch(("ROD_DMUDataformatErr"+suffixArr[i]).c_str(),m_ROD_DMUDataformatErrVec.back(),("ROD_DMUDataformatErr"+suffixArr[i]+"[16]/s").c_str()); // unsigned short
         m_ntuplePtr->Branch(("ROD_DMUDataparityErr"+suffixArr[i]).c_str(),m_ROD_DMUDataparityErrVec.back(),("ROD_DMUDataparityErr"+suffixArr[i]+"[16]/s").c_str()); // unsigned short
         m_ntuplePtr->Branch(("ROD_DMUMask"+suffixArr[i]).c_str(),m_ROD_DMUMaskVec.back(),("ROD_DMUMask"+suffixArr[i]+"[2]/s").c_str()); // unsigned short

       }

     }

    }

  }

}

/**
//////////////////////////////////////////////////////////////////////////////
///Clear Tree DIGI variables
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::DIGI_clearBranch(void)
{
  clear_int(m_evtVec,1);
  clear_short(m_rodBCIDVec,1);
  clear_short(m_sizeVec,1);

  clear_int(m_bcidVec,N_DMUS);
  clear_uint32(m_DMUheaderVec,N_DMUS);
  clear_short(m_DMUformatErrVec,N_DMUS);
  clear_short(m_DMUparityErrVec,N_DMUS);
  clear_short(m_DMUmemoryErrVec,N_DMUS);
  clear_short(m_DMUDstrobeErrVec,N_DMUS);
  clear_short(m_DMUSstrobeErrVec,N_DMUS);

  clear_short(m_ROD_GlobalCRCVec,1);
  clear_short(m_ROD_DMUBCIDVec,N_DMUS);
  clear_short(m_ROD_DMUmemoryErrVec,N_DMUS);
  clear_short(m_ROD_DMUSstrobeErrVec,N_DMUS);
  clear_short(m_ROD_DMUDstrobeErrVec,N_DMUS);
  clear_short(m_ROD_DMUHeadformatErrVec,N_DMUS);
  clear_short(m_ROD_DMUHeadparityErrVec,N_DMUS);
  clear_short(m_ROD_DMUDataformatErrVec,N_DMUS);
  clear_short(m_ROD_DMUDataparityErrVec,N_DMUS);
  clear_short(m_ROD_DMUMaskVec,2);

  clear_int(m_dmuMaskVec,2);
  clear_int(m_slinkCRCVec,2);
  clear_int(m_gainVec,N_CHANS);
  clear_intint(m_sampleVec,N_CHANS);
  clear_int(m_feCRCVec,N_DMUS);
  clear_int(m_rodCRCVec,N_DMUS);

  clear_float(m_eneVec);
  clear_float(m_timeVec);
  clear_float(m_pedFlatVec);
  clear_float(m_chi2FlatVec);
  clear_float(m_efitVec);
  clear_float(m_tfitVec);
  clear_float(m_pedfitVec);
  clear_float(m_chi2Vec);
  clear_float(m_efitcVec);
  clear_float(m_tfitcVec);
  clear_float(m_pedfitcVec);
  clear_float(m_chi2cVec);
  clear_float(m_eOptVec);
  clear_float(m_tOptVec);
  clear_float(m_pedOptVec);
  clear_float(m_chi2OptVec);
  clear_float(m_eDspVec);
  clear_float(m_tDspVec);
  clear_float(m_chi2DspVec);

}


void TileTBAANtuple::FELIX_addBranch(void)
{
  m_eflxfitVec.reserve(MAX_DRAWERS);
  m_tflxfitVec.reserve(MAX_DRAWERS);
  m_chi2flxfitVec.reserve(MAX_DRAWERS);
  m_pedflxfitVec.reserve(MAX_DRAWERS);
  m_eflxoptVec.reserve(MAX_DRAWERS);
  m_tflxoptVec.reserve(MAX_DRAWERS);
  m_chi2flxoptVec.reserve(MAX_DRAWERS);
  m_pedflxoptVec.reserve(MAX_DRAWERS);
  m_gainflxVec.reserve(MAX_DRAWERS);
  m_sampleflxVec.reserve(MAX_DRAWERS);

  std::ostringstream oss;
  oss << m_nSamplesFlx;
  std::string nSampStr_Flx=oss.str();

  unsigned int listSize = std::min (m_nDrawers, static_cast<unsigned int>(m_drawerMap.size()) );

      std::string digit[10] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
    std::vector<std::string> suffixArr;
    unsigned int length;
    bool testbeam = TileCablingService::getInstance()->getTestBeam();
      length = 2 * m_nDrawers;
      suffixArr.resize(length);

      for (unsigned int i = 0; i < listSize; ++i) {
        unsigned int ros = m_drawerType[i];
        unsigned int drawer = strtol(m_drawerList[i].data(), NULL, 0) & 0x3F;
        std::string digits;
        if (m_TBperiod >= 2010) {
          ++drawer; // count modules from 1
          digits = digit[drawer / 10] + digit[drawer % 10];
        } else if (testbeam) {
          digits = digit[drawer & 7];
        } else {
          ++drawer; // count modules from 1
          digits = digit[drawer / 10] + digit[drawer % 10];
        }


        if (ros == 0) {
          std::string suff = m_drawerList[i];
          suff.replace(suff.find("0x"), 2, "");
          suffixArr[i] = suff + "lo";
          suffixArr[i + m_nDrawers] = suff + "hi";
        } else {
          suffixArr[i] = m_rosName[ros] + digits + "lo";
          suffixArr[i + m_nDrawers] = m_rosName[ros] + digits + "hi";
        }
      }

  for (unsigned int i = 0; i < length; i++) {

      int nSamplesInDrawer(0);
      int frag = std::stoi(m_drawerList[i%m_nDrawers], nullptr, 0);

      auto it = m_nSamplesInDrawerMap.find(frag);
      if (it != m_nSamplesInDrawerMap.end()) {
        nSamplesInDrawer = it->second;
      }

      if (i % m_nDrawers < listSize)
        ATH_MSG_DEBUG( "Adding items for " << suffixArr[i] );

      int *evt = new int;
      short *size = new short;
      short *rodBCID = new short;
      int *gain = new int[N_CHANS];



      int **sample_flx = (int**) malloc(N_CHANS * sizeof(int *));
      sample_flx[0] = (int*) malloc(N_CHANS * m_nSamplesFlx * sizeof(int));
      memset(sample_flx[0], -1, sizeof(int) * m_nSamplesFlx * N_CHANS);
      nSamplesInDrawer = std::max(m_nSamplesFlx,nSamplesInDrawer);
      for (int j = 1; j < N_CHANS; j++) {
        sample_flx[j] = sample_flx[0] + j * nSamplesInDrawer;
      }

      float *eflxfit = new float[N_CHANS];
      float *tflxfit = new float[N_CHANS];
      float *pedflxfit = new float[N_CHANS];
      float *chi2flxfit = new float[N_CHANS];

      float *eflxOpt = new float[N_CHANS];
      float *tflxOpt = new float[N_CHANS];
      float *pedflxOpt = new float[N_CHANS];
      float *chi2flxOpt = new float[N_CHANS];


      int* mdL1idflx(nullptr);
      int* mdBcidflx(nullptr);
      int* mdModuleflx(nullptr);
      int* mdRunTypeflx(nullptr);
      int* mdPedLoflx(nullptr);
      int* mdPedHiflx(nullptr);
      int* mdRunflx(nullptr);
      int* mdChargeflx(nullptr);
      int* mdChargeTimeflx(nullptr);
      int* mdCapacitorflx(nullptr);


      mdL1idflx = new int[MAX_MINIDRAWERS];
      mdBcidflx = new int[MAX_MINIDRAWERS];
      mdModuleflx = new int[MAX_MINIDRAWERS];
      mdRunTypeflx = new int[MAX_MINIDRAWERS];
      mdPedLoflx = new int[MAX_MINIDRAWERS];
      mdPedHiflx = new int[MAX_MINIDRAWERS];
      mdRunflx = new int[MAX_MINIDRAWERS];
      mdChargeflx = new int[MAX_MINIDRAWERS];
      mdChargeTimeflx = new int[MAX_MINIDRAWERS];
      mdCapacitorflx = new int[MAX_MINIDRAWERS];



      m_eflxfitVec.push_back( eflxfit );
      m_tflxfitVec.push_back( tflxfit );
      m_pedflxfitVec.push_back( pedflxfit );
      m_chi2flxfitVec.push_back( chi2flxfit );

      m_eflxoptVec.push_back( eflxOpt );
      m_tflxoptVec.push_back( tflxOpt );
      m_pedflxoptVec.push_back( pedflxOpt );
      m_chi2flxoptVec.push_back( chi2flxOpt );

     m_rodBCIDflxVec.push_back( rodBCID);
     m_sizeflxVec.push_back( size);
     m_evtflxVec.push_back( evt);

     m_mdL1idflxVec.push_back(mdL1idflx);
     m_mdBcidflxVec.push_back(mdBcidflx);
     m_mdModuleflxVec.push_back(mdModuleflx);
     m_mdRunTypeflxVec.push_back(mdRunTypeflx);
     m_mdPedLoflxVec.push_back(mdPedLoflx);
     m_mdPedHiflxVec.push_back(mdPedHiflx);
     m_mdRunflxVec.push_back(mdRunflx);
     m_mdChargeflxVec.push_back(mdChargeflx);
     m_mdChargeTimeflxVec.push_back(mdChargeTimeflx);
     m_mdCapacitorflxVec.push_back(mdCapacitorflx);


     m_gainflxVec.push_back( gain );
     m_sampleflxVec.push_back( sample_flx ); // U(48/96,9)

     if (i%m_nDrawers < listSize) {

        ATH_MSG_DEBUG( "Adding Leaf to Event '" << ("Evt" + suffixArr[i]+"Flx") << "' @" << evt );

        if (m_bsInput) {
         m_ntuplePtr->Branch(("Flx_Evt"+suffixArr[i]).c_str(),evt,("Flx_Evt"+suffixArr[i]+"/I").c_str()); // int
         m_ntuplePtr->Branch(("Flx_rodBCID"+suffixArr[i]).c_str(),rodBCID,("Flx_rodBCID"+suffixArr[i]+"/S").c_str());//int
         m_ntuplePtr->Branch(("Flx_Size"+suffixArr[i]).c_str(),size,("Flx_Size"+suffixArr[i]+"/S").c_str()); // short
        }

        if (mdL1idflx) {
         m_ntuplePtr->Branch(("mdL1ID"+suffixArr[i]).c_str(),m_mdL1idflxVec.back(),("mdL1id"+suffixArr[i]+"[4]/I").c_str()); // int
         m_ntuplePtr->Branch(("mdBCID"+suffixArr[i]).c_str(),m_mdBcidflxVec.back(),("mdBcid"+suffixArr[i]+"[4]/I").c_str()); // int
         m_ntuplePtr->Branch(("mdModule"+suffixArr[i]).c_str(),m_mdModuleflxVec.back(),("mdModule"+suffixArr[i]+"[4]/I").c_str()); // int
         m_ntuplePtr->Branch(("mdRunType"+suffixArr[i]).c_str(),m_mdRunTypeflxVec.back(),("mdRunType"+suffixArr[i]+"[4]/I").c_str()); // int
         m_ntuplePtr->Branch(("mdPedLo"+suffixArr[i]).c_str(),m_mdPedLoflxVec.back(),("mdPedLo"+suffixArr[i]+"[4]/I").c_str()); // int
         m_ntuplePtr->Branch(("mdPedHi"+suffixArr[i]).c_str(),m_mdPedHiflxVec.back(),("mdPedHi"+suffixArr[i]+"[4]/I").c_str()); // int
         m_ntuplePtr->Branch(("mdChargeTime"+suffixArr[i]).c_str(),m_mdChargeTimeflxVec.back(),("mdChargeTime"+suffixArr[i]+"[4]/I").c_str()); // int
         m_ntuplePtr->Branch(("mdCapacitor"+suffixArr[i]).c_str(),m_mdCapacitorflxVec.back(),("mdCapacitor"+suffixArr[i]+"[4]/I").c_str()); // int
        }

       m_ntuplePtr->Branch(("Flx_Gain"+suffixArr[i]).c_str(),m_gainflxVec.back(),("Flx_gain"+suffixArr[i]+"[48]/I").c_str()); // int

       if (nSamplesInDrawer > 0) {

         nSampStr_Flx = std::to_string(nSamplesInDrawer);
         m_ntuplePtr->Branch(("Flx_Sample" + suffixArr[i]).c_str(), *sample_flx,
                             ("Flx_sample" + suffixArr[i] + "[48]["+nSampStr_Flx+"]/I").c_str()); // size m_nsample and type int



        }

        if (m_flxFitRawChannelContainer.size() > 0) {
          m_ntuplePtr->Branch(("Flx_Efit" + suffixArr[i]).c_str(), m_eflxfitVec.back(), ("Flx_eflxfit" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Flx_Tfit" + suffixArr[i]).c_str(), m_tflxfitVec.back(), ("Flx_tflxfit" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Flx_Pedfit" + suffixArr[i]).c_str(), m_pedflxfitVec.back(), ("Flx_pedflxfit" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Flx_Chi2fit" + suffixArr[i]).c_str(), m_chi2flxfitVec.back(), ("Flx_chiflxfit" + suffixArr[i] + "[48]/F").c_str()); // float

        }

        if (m_flxOptRawChannelContainer.size() > 0) {
          m_ntuplePtr->Branch(("Flx_EOpt" + suffixArr[i]).c_str(), m_eflxoptVec.back(), ("Flx_eflxOpt" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Flx_TOpt" + suffixArr[i]).c_str(), m_tflxoptVec.back(), ("Flx_tflxOpt" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Flx_PedOpt" + suffixArr[i]).c_str(), m_pedflxoptVec.back(), ("Flx_pedflxOpt" + suffixArr[i] + "[48]/F").c_str()); // float
          m_ntuplePtr->Branch(("Flx_Chi2Opt" + suffixArr[i]).c_str(), m_chi2flxoptVec.back(), ("Flx_chiflxOpt" + suffixArr[i] + "[48]/F").c_str()); // float

        }
     }
   }
}



void TileTBAANtuple::FELIX_clearBranch(void)
{
  clear_int(m_evtflxVec,1);
  clear_short(m_rodBCIDflxVec,1);
  clear_short(m_sizeflxVec,1);

  clear_intint(m_sampleflxVec,N_CHANS);

  clear_float(m_eflxfitVec);
  clear_float(m_tflxfitVec );
  clear_float(m_chi2flxfitVec);
  clear_float(m_pedflxfitVec);
  clear_float(m_eflxoptVec);
  clear_float(m_tflxoptVec);
  clear_float(m_chi2flxoptVec);
  clear_float(m_pedflxoptVec);

  clear_int(m_mdL1idflxVec, MAX_MINIDRAWERS);
  clear_int(m_mdBcidflxVec, MAX_MINIDRAWERS);
  clear_int(m_mdModuleflxVec, MAX_MINIDRAWERS);
  clear_int(m_mdRunTypeflxVec, MAX_MINIDRAWERS);
  clear_int(m_mdRunflxVec, MAX_MINIDRAWERS);
  clear_int(m_mdChargeflxVec, MAX_MINIDRAWERS);
  clear_int(m_mdChargeTimeflxVec, MAX_MINIDRAWERS);
  clear_int(m_mdCapacitorflxVec, MAX_MINIDRAWERS);
  clear_int(m_mdPedLoflxVec, MAX_MINIDRAWERS);
  clear_int(m_mdPedHiflxVec, MAX_MINIDRAWERS);

}

/**
//////////////////////////////////////////////////////////////////////////////
///Add Tree HIT variables Tree
//
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::HIT_addBranch(void)
{
  m_ehitVec.clear();
  m_thitVec.clear();
  m_ehitCnt.clear();
  m_thitCnt.clear();

  m_ehitVec.reserve(MAX_DRAWERS);
  m_thitVec.reserve(MAX_DRAWERS);
  m_ehitCnt.reserve(MAX_DRAWERS);
  m_thitCnt.reserve(MAX_DRAWERS);

  unsigned int listSize = std::min (m_nDrawers, static_cast<unsigned int>(m_drawerMap.size()) );

  if (listSize > 0) {

    std::string digit[10] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9" };
    std::vector<std::string> suffixArr;
    unsigned int length;
    bool testbeam = TileCablingService::getInstance()->getTestBeam();

    length = m_nDrawers;
    suffixArr.resize(length);

    for (unsigned int i = 0; i < listSize; ++i) {
      unsigned int ros = m_drawerType[i];
      unsigned int drawer = strtol(m_drawerList[i].data(), NULL, 0) & 0x3F;
      std::string digits;
      if (m_TBperiod >= 2010) {
        ++drawer; // count modules from 1
        digits = digit[drawer / 10] + digit[drawer % 10];
      } else if (testbeam) {
        digits = digit[drawer & 7];
      } else {
        ++drawer; // count modules from 1
        digits = digit[drawer / 10] + digit[drawer % 10];
      }

      if (ros == 0) {
        std::string suff = m_drawerList[i];
        suff.replace(suff.find("0x"), 2, "");
        suffixArr[i] = suff;
      } else {
        suffixArr[i] = m_rosName[ros] + digits;
      }
    }

    for (unsigned int i = 0; i < length; i++) {
      
      if (i % m_nDrawers < listSize)
        ATH_MSG_DEBUG( "Adding items for " << suffixArr[i] );
      
      float *ehitVec = new float[N_CHANS];
      float *thitVec = new float[N_CHANS];
      float *ehitCnt = new float[N_CHANS];
      float *thitCnt = new float[N_CHANS];

      m_ehitVec.push_back( ehitVec );
      m_thitVec.push_back( thitVec );
      m_ehitCnt.push_back( ehitCnt );
      m_thitCnt.push_back( thitCnt );


      if (i%m_nDrawers < listSize) {

        if (m_hitVector.size() > 0) {

          if (i % m_nDrawers < listSize)
            ATH_MSG_DEBUG( "Adding G4 hit info for " << suffixArr[i] );
      
          m_ntuplePtr->Branch(("EhitG4"+suffixArr[i]).c_str(),m_ehitVec.back(),("eHitG4"+suffixArr[i]+"[48]/F").c_str());
          m_ntuplePtr->Branch(("ThitG4"+suffixArr[i]).c_str(),m_thitVec.back(),("tHitG4"+suffixArr[i]+"[48]/F").c_str());
        }

        if (m_hitContainer.size() > 0) {

          if (i % m_nDrawers < listSize)
            ATH_MSG_DEBUG( "Adding G4 corrected hit info for " << suffixArr[i] );
      
          m_ntuplePtr->Branch(("EhitSim"+suffixArr[i]).c_str(),m_ehitCnt.back(),("eHitSim"+suffixArr[i]+"[48]/F").c_str());
          m_ntuplePtr->Branch(("ThitSim"+suffixArr[i]).c_str(),m_thitCnt.back(),("tHitSim"+suffixArr[i]+"[48]/F").c_str());
        }
      }
    }
  }
}

/**
//////////////////////////////////////////////////////////////////////////////
///Clear Tree HIT variables
//////////////////////////////////////////////////////////////////////////////
*/
void TileTBAANtuple::HIT_clearBranch(void)
{
  clear_float(m_ehitVec);
  clear_float(m_thitVec);
  clear_float(m_ehitCnt);
  clear_float(m_thitCnt);
}
void TileTBAANtuple::clear_short(std::vector<short*> & vec, int nchan)
{
  for (std::vector<short*>::iterator it=vec.begin(); it!=vec.end(); ++it) {
    short * ptr = *it;
    memset(ptr,-1,sizeof(short)*nchan);
  }
}

void TileTBAANtuple::clear_int(std::vector<int*> & vec, int nchan)
{
  for (std::vector<int*>::iterator it=vec.begin(); it!=vec.end(); ++it) {
    int* ptr = *it;
    if (ptr) memset(ptr,-1,sizeof(int)*nchan);
  }
}

void TileTBAANtuple::clear_uint32(std::vector<uint32_t*> & vec, int nchan)
{
  for (std::vector<uint32_t*>::iterator it=vec.begin(); it!=vec.end(); ++it) {
    uint32_t * ptr = *it;
    memset(ptr,0,sizeof(uint32_t)*nchan);
  }
}

void TileTBAANtuple::clear_float(std::vector<float*> & vec, int nchan)
{
  for (std::vector<float*>::iterator it=vec.begin(); it!=vec.end(); ++it) {
    float * ptr = *it;
    memset(ptr,0,sizeof(float)*nchan);
  }
}

void TileTBAANtuple::clear_intint(std::vector<int**> & vec, int nchan)
{
  for (std::vector<int**>::iterator it=vec.begin(); it!=vec.end(); ++it) {
    int **ptr = *it;
    memset(ptr[0], -1, sizeof(int) * m_nSamples * nchan);
  }
}

void TileTBAANtuple::setupBeamChambersBeforeTB2015(void) {
  // Setup default value for the following properties, if they are not setup via JO
  setupPropertyDefaultValue(m_beamBC1X1, -0.938, "BC1X1");
  setupPropertyDefaultValue(m_beamBC1X2, 0.1747, "BC1X2");
  setupPropertyDefaultValue(m_beamBC1Y1, 0.125, "BC1Y1");
  setupPropertyDefaultValue(m_beamBC1Y2, 0.1765, "BC1Y2");
  setupPropertyDefaultValue(m_beamBC1Z, 13788.0, "BC1Z");
  setupPropertyDefaultValue(m_beamBC1Z_0, 13788.0, "BC1Z_0");
  setupPropertyDefaultValue(m_beamBC1Z_90, 13788.0, "BC1Z_90");
  setupPropertyDefaultValue(m_beamBC1Z_min90, 13788.0, "BC1Z_min90");

  setupPropertyDefaultValue(m_beamBC2X1, -0.9369, "BC2X1");
  setupPropertyDefaultValue(m_beamBC2X2, 0.191, "BC2X2");
  setupPropertyDefaultValue(m_beamBC2Y1, -1.29, "BC2Y1");
  setupPropertyDefaultValue(m_beamBC2Y2, 0.187, "BC2Y2");
  setupPropertyDefaultValue(m_beamBC2Z, 9411.0, "BC2Z");
  setupPropertyDefaultValue(m_beamBC2Z_0, 9411.0, "BC2Z_0");
  setupPropertyDefaultValue(m_beamBC2Z_90, 9411.0, "BC2Z_90");
  setupPropertyDefaultValue(m_beamBC2Z_min90, 9411.0, "BC2Z_min90");
}

void TileTBAANtuple::setupBeamChambersTB2015(void) {
  setupPropertyDefaultValue(m_beamBC1X1, -0.0462586, "BC1X1");
  setupPropertyDefaultValue(m_beamBC1X2, -0.175666, "BC1X2");
  setupPropertyDefaultValue(m_beamBC1Y1, -0.051923, "BC1Y1");
  setupPropertyDefaultValue(m_beamBC1Y2, -0.176809, "BC1Y2");
  setupPropertyDefaultValue(m_beamBC1Z, 13000. + 2760, "BC1Z");
  setupPropertyDefaultValue(m_beamBC1Z_0, 13788.0, "BC1Z_0");
  setupPropertyDefaultValue(m_beamBC1Z_90, 13788.0, "BC1Z_90");
  setupPropertyDefaultValue(m_beamBC1Z_min90, 13788.0, "BC1Z_min90");

  setupPropertyDefaultValue(m_beamBC2X1, 0.25202, "BC2X1");
  setupPropertyDefaultValue(m_beamBC2X2, -0.18053, "BC2X2");
  setupPropertyDefaultValue(m_beamBC2Y1, 0.0431688, "BC2Y1");
  setupPropertyDefaultValue(m_beamBC2Y2, -0.181128, "BC2Y2");
  setupPropertyDefaultValue(m_beamBC2Z, 2760, "BC2Z");
  setupPropertyDefaultValue(m_beamBC2Z_0, 9411.0, "BC2Z_0");
  setupPropertyDefaultValue(m_beamBC2Z_90, 9411.0, "BC2Z_90");
  setupPropertyDefaultValue(m_beamBC2Z_min90, 9411.0, "BC2Z_min90");
}

void TileTBAANtuple::setupBeamChambersTB2016_2020(void) {
  // 2016 settings:
  //
  // https://pcata007.cern.ch/elog/TB2016/88
  //
  //
  // The calibration has been done with the following runs :
  // BC1:
  // Center : 610212
  // Left/up : 610210
  // Right/down : 610209
  //
  // BC2:
  // Center : 610256
  // Left/up : 610321
  // Right/down : 610320
  //
  // Here are the new constants :
  //
  // BC1
  // horizontal slope = -0.171928
  // horizontal offset = -0.047624
  //
  // vertical slope = -0.172942
  // vertical offset = -0.0958677
  //
  // BC2
  // horizontal slope = -0.175698
  // horizontal offset = -1.04599
  //
  // vertical slope = -0.174535
  // vertical offset = -3.10666

  // June 2016 calibration
  //m_beamBC1X1 = -0.047624;
  //m_beamBC1X2 = -0.171928;
  //m_beamBC1Y1 = -0.0958677;
  //m_beamBC1Y2 = -0.172942;
  //m_beamBC1Z  = 13000. + 2760 /* 2600. */;

  //m_beamBC2X1 = -1.04599;
  //m_beamBC2X2 = -0.175698;
  //m_beamBC2Y1 = -3.10666;
  //m_beamBC2Y2 = -0.174535;
  //m_beamBC2Z  = 2760 /* 2600. */;

  // September 2016 calibration, https://pcata007.cern.ch/elog/TB2016/300 (joakim.olsson@cern.ch)
  //m_beamBC1X1 = 0.100857923042;
  //m_beamBC1X2 = -0.172098;
  //m_beamBC1Y1 = -0.133045996607;
  //m_beamBC1Y2 = -0.172855178323;
  //m_beamBC1Z  = 13000. + 2760 /* 2600. */;
  //
  //m_beamBC2X1 = 0.271555258578 ;
  //m_beamBC2X2 = -0.173463 ;
  //m_beamBC2Y1 = 0.305483228502;
  //m_beamBC2Y2 = -0.173805131744 ;
  //m_beamBC2Z  = 2760 /* 2600. */;

  // June 2017 calibration, https://pcata007.cern.ch/elog/TB2017/550 (schae@cern.ch)
  //m_beamBC1X1 =  0.153584934082;
  //m_beamBC1X2 = -0.175220;
  //m_beamBC1Y1 = -0.493246053303;
  //m_beamBC1Y2 = -0.176567356723;
  //m_beamBC1Z  = 13000. + 2760 /* 2600. */;
  //
  //m_beamBC2X1 = 0.414611893278;
  //m_beamBC2X2 = -0.176122;
  //m_beamBC2Y1 = 0.150807740888;
  //m_beamBC2Y2 = -0.173472808704;
  //m_beamBC2Z  = 2760 /* 2600. */;

  // August 2017 calibration, https://pcata007.cern.ch/elog/TB2017/550 (schae@cern.ch)
  //m_beamBC1X1 =  0.181797;
  //m_beamBC1X2 = -0.175657;
  //m_beamBC1Y1 = -0.128910;
  //m_beamBC1Y2 = -0.175965;
  //m_beamBC1Z  = 13000. + 2760 /* 2600. */;
  //
  //m_beamBC2X1 = 0.611502;
  //m_beamBC2X2 = -0.183116;
  //m_beamBC2Y1 = 0.541212;
  //m_beamBC2Y2 = -0.183115;
  //m_beamBC2Z  = 2760 /* 2600. */;

  // September 2017 calibration, https://pcata007.cern.ch/elog/TB2017/550 (schae@cern.ch)
  //m_beamBC1X1 =  0.181797;
  //m_beamBC1X2 = -0.175657;
  //m_beamBC1Y1 = -0.128910;
  //m_beamBC1Y2 = -0.175965;
  //m_beamBC1Z  = 13000. + 2760 /* 2600. */;

  //m_beamBC2X1 = 0.622896039922;
  //m_beamBC2X2 = -0.176735;
  //m_beamBC2Y1 = 0.195954125116;
  //m_beamBC2Y2 = -0.176182117624;
  //m_beamBC2Z  = 2760 /* 2600. */;

	// September 2017 calibration with additional precision from Survey
  setupPropertyDefaultValue(m_beamBC1X1, 0.681797, "BC1X1");
  setupPropertyDefaultValue(m_beamBC1X2, -0.175657, "BC1X2");
  setupPropertyDefaultValue(m_beamBC1Y1, -2.02891, "BC1Y1");
  setupPropertyDefaultValue(m_beamBC1Y2, -0.175965, "BC1Y2");
  setupPropertyDefaultValue(m_beamBC1Z, 17348.8, "BC1Z");
  setupPropertyDefaultValue(m_beamBC1Z_0, 17348.8, "BC1Z_0");
  setupPropertyDefaultValue(m_beamBC1Z_90, 15594.05, "BC1Z_90");
  setupPropertyDefaultValue(m_beamBC1Z_min90, 15571.8, "BC1Z_min90");

  setupPropertyDefaultValue(m_beamBC2X1, -24.377104, "BC2X1");
  setupPropertyDefaultValue(m_beamBC2X2, -0.176735, "BC2X2");
  setupPropertyDefaultValue(m_beamBC2Y1, 17.895954, "BC2Y1");
  setupPropertyDefaultValue(m_beamBC2Y2, -0.176182117624, "BC2Y2");
  setupPropertyDefaultValue(m_beamBC2Z, 4404.2, "BC2Z");
  setupPropertyDefaultValue(m_beamBC2Z_0, 4420.7, "BC2Z_0");
  setupPropertyDefaultValue(m_beamBC2Z_90, 2649.45, "BC2Z_90");
  setupPropertyDefaultValue(m_beamBC2Z_min90, 2627.2, "BC2Z_min90");

}
