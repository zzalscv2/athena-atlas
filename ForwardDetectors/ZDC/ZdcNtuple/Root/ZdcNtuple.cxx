/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include <TSystem.h>
#include <TFile.h>
#include "xAODRootAccess/tools/Message.h"
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODCore/ShallowCopy.h"

#include <ZdcNtuple/ZdcNtuple.h>

// this is needed to distribute the algorithm to the workers
//ClassImp(ZdcNtuple)

ZdcNtuple :: ZdcNtuple (const std::string& name, ISvcLocator *pSvcLocator)
  : EL::AnaAlgorithm(name, pSvcLocator),
    //m_trigConfigTool("TrigConf::xAODConfigTool/xAODConfigTool", this),
    m_trigDecisionTool ("Trig::TrigDecisionTool/TrigDecisionTool"),
    //    m_trigMatchingTool("Trig::MatchingTool/TrigMatchingTool", this),
    m_grl ("GoodRunsListSelectionTool/grl", this),
    m_zdcAnalysisTool("ZDC::ZdcAnalysisTool/ZdcAnalysisTool", this)
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().
  m_setupTrigHist = false;
  m_eventCounter = 0;

  declareProperty("isMC",  m_isMC = false, "MC mode");
  declareProperty("enableOutputTree",  enableOutputTree = false, "Enable output tree");
  declareProperty("enableOutputSamples",  enableOutputSamples = false, "comment");
  declareProperty("enableTrigger",  enableTrigger = true, "comment");
  declareProperty("writeOnlyTriggers",  writeOnlyTriggers = false, "comment");
  declareProperty("useGRL",  useGRL = true, "comment");
  declareProperty("grlFilename",  grlFilename = "$ROOTCOREBIN/data/ZdcNtuple/data16_hip8TeV.periodAllYear_DetStatus-v86-pro20-19_DQDefects-00-02-04_PHYS_HeavyIonP_All_Good.xml", "comment");
  declareProperty("slimmed",  slimmed = false, "comment");
  declareProperty("zdcCalib",  zdcCalib = false, "comment");
  declareProperty("zdcLaser",  zdcLaser = false, "comment");
  declareProperty("zdcOnly", zdcOnly = false, "comment");
  declareProperty("zdcLowGainOnly",  zdcLowGainOnly = false, "comment");

  declareProperty("flipDelay",  flipDelay = 0, "comment");
  declareProperty("reprocZdc",  reprocZdc = 0, "comment");
  declareProperty("auxSuffix",  auxSuffix = "", "comment");
  declareProperty("nsamplesZdc",  nsamplesZdc = 24, "comment");
  declareProperty("lhcf2022", lhcf2022 = true,"comment");
  declareProperty("zdcConfig", zdcConfig = "PbPb2018", "argument to configure ZdcAnalysisTool");
  declareProperty("doZdcCalib", doZdcCalib = false, "perform ZDC energy calibration");

  m_zdcAnalysisTool.declarePropertyFor (this, "zdcAnalysisTool");

}

/*
StatusCode ZdcNtuple :: setupJob (EL::Job& job)
{
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.

  job.useXAOD ();
  ANA_CHECK( "setupJob()", xAOD::Init() ); // call before opening first file

  return StatusCode::SUCCESS;
}
*/


StatusCode ZdcNtuple :: initialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.

  ANA_MSG_DEBUG("Howdy from Initialize!");

  if (enableOutputTree)
  {

    ANA_CHECK( book(TTree("zdcTree", "ZDC Tree")));
    m_outputTree = tree( "zdcTree" );

    m_outputTree->Branch("runNumber", &t_runNumber, "runNumber/i");
    m_outputTree->Branch("eventNumber", &t_eventNumber, "eventNumber/i");
    m_outputTree->Branch("lumiBlock", &t_lumiBlock, "lumiBlock/i");
    m_outputTree->Branch("bcid", &t_bcid, "bcid/i");
    m_outputTree->Branch("avgIntPerCrossing", &t_avgIntPerCrossing, "avgIntPerCrossing/F");
    m_outputTree->Branch("actIntPerCrossing", &t_actIntPerCrossing, "actIntPerCrossing/F");
    m_outputTree->Branch("trigger", &t_trigger, "trigger/l");
    m_outputTree->Branch("trigger_TBP", &t_trigger_TBP, "trigger_TBP/i");
    m_outputTree->Branch("tbp", &t_tbp, "tbp[16]/i");
    m_outputTree->Branch("tav", &t_tav, "tav[16]/i");
    m_outputTree->Branch("passBits", &t_passBits, "passBits/i");
    m_outputTree->Branch("extendedLevel1ID",&t_extendedLevel1ID,"extendedLevel1ID/i");
    m_outputTree->Branch("timeStamp",&t_timeStamp,"timeStamp/i");
    m_outputTree->Branch("timeStampNSOffset",&t_timeStampNSOffset,"timeStampNSOffset/i");

    if (enableOutputSamples)
    {
      if (nsamplesZdc == 7)
      {
        ANA_MSG_INFO("Setting up for 7 samples");
        m_outputTree->Branch("zdc_raw", &t_raw7, "zdc_raw[2][4][2][2][7]/s"); // 7 samples
      }

      if (nsamplesZdc == 15)
      {
        ANA_MSG_INFO("Setting up for 15 samples");
        m_outputTree->Branch("zdc_raw", &t_raw15, "zdc_raw[2][4][2][2][15]/s"); // 15 samples
      }

      if (nsamplesZdc == 24)
      {
        ANA_MSG_INFO("Setting up forr 24 samples");
        m_outputTree->Branch("zdc_raw", &t_raw24, "zdc_raw[2][4][2][2][24]/s"); // 24 samples
      }
    }

    m_outputTree->Branch("zdc_ZdcAmp", &t_ZdcAmp, "zdc_ZdcAmp[2]/F");
    m_outputTree->Branch("zdc_ZdcAmpErr", &t_ZdcAmpErr, "zdc_ZdcAmpErr[2]/F");
    m_outputTree->Branch("zdc_ZdcEnergy", &t_ZdcEnergy, "zdc_ZdcEnergy[2]/F");
    m_outputTree->Branch("zdc_ZdcEnergyErr", &t_ZdcEnergyErr, "zdc_ZdcEnergyErr[2]/F");
    m_outputTree->Branch("zdc_ZdcTime", &t_ZdcTime, "zdc_ZdcTime[2]/F");
    m_outputTree->Branch("zdc_ZdcStatus", &t_ZdcStatus, "zdc_ZdcStatus[2]/S");
    m_outputTree->Branch("zdc_ZdcTrigEff", &t_ZdcTrigEff, "zdc_ZdcTrigEff[2]/F");
    m_outputTree->Branch("zdc_ZdcModuleMask", &t_ZdcModuleMask, "zdc_ZdcModuleMask/i");
    m_outputTree->Branch("zdc_ZdcLucrodTriggerSideAmp",&t_ZdcLucrodTriggerSideAmp,"zdc_ZdcLucrodTriggerSideAmp[2]/S");

    m_outputTree->Branch("zdc_ZdcModuleAmp", &t_ZdcModuleAmp, "zdc_ZdcModuleAmp[2][4]/F");
    m_outputTree->Branch("zdc_ZdcModuleTime", &t_ZdcModuleTime, "zdc_ZdcModuleTime[2][4]/F");
    m_outputTree->Branch("zdc_ZdcModuleFitAmp", &t_ZdcModuleFitAmp, "zdc_ZdcModuleFitAmp[2][4]/F");
    m_outputTree->Branch("zdc_ZdcModuleFitT0", &t_ZdcModuleFitT0, "zdc_ZdcModuleFitT0[2][4]/F");
    m_outputTree->Branch("zdc_ZdcModuleStatus", &t_ZdcModuleStatus, "zdc_ZdcModuleStatus[2][4]/i");
    m_outputTree->Branch("zdc_ZdcModuleChisq", &t_ZdcModuleChisq, "zdc_ZdcModuleChisq[2][4]/F");
    m_outputTree->Branch("zdc_ZdcModuleCalibAmp", &t_ZdcModuleCalibAmp, "zdc_ZdcModuleCalibAmp[2][4]/F");
    m_outputTree->Branch("zdc_ZdcModuleCalibTime", &t_ZdcModuleCalibTime, "zdc_ZdcModuleCalibTime[2][4]/F");
    m_outputTree->Branch("zdc_ZdcModuleBkgdMaxFraction", &t_ZdcModuleBkgdMaxFraction, "zdc_ZdcModuleBkgdMaxFraction[2][4]/F");
    m_outputTree->Branch("zdc_ZdcModuleAmpError", &t_ZdcModuleAmpError, "zdc_ZdcModuleAmpError[2][4]/F");
    m_outputTree->Branch("zdc_ZdcModuleMinDeriv2nd", &t_ZdcModuleMinDeriv2nd, "zdc_ZdcModuleMinDeriv2nd[2][4]/F");
    m_outputTree->Branch("zdc_ZdcModulePresample", &t_ZdcModulePresample, "zdc_ZdcModulePresample[2][4]/F");
    m_outputTree->Branch("zdc_ZdcModulePreSampleAmp", &t_ZdcModulePreSampleAmp, "zdc_ZdcModulePreSamplemp[2][4]/F");
    m_outputTree->Branch("zdc_ZdcLucrodTriggerAmp",&t_ZdcLucrodTriggerAmp,"zdc_ZdcLucrodTriggerAmp[2][4]/S");
    m_outputTree->Branch("zdc_ZdcModuleMaxADC",&t_ZdcModuleMaxADC,"zdc_ZdcModuleMaxADC[2][4]/F");

    m_outputTree->Branch("zdc_RPDChannelAmplitude",&t_RPDChannelAmplitude,"zdc_RPDChannelAmplitude[2][16]/F");
    m_outputTree->Branch("zdc_RPDChannelMaxSample",&t_RPDChannelMaxSample,"zdc_RPDChannelMaxSample[2][16]/i");

    if (!(zdcCalib || zdcLaser || zdcOnly))
    {
      m_outputTree->Branch("mbts_in_e", &t_mbts_in_e, "mbts_in_e[2][8]/F");
      m_outputTree->Branch("mbts_in_t", &t_mbts_in_t, "mbts_in_t[2][8]/F");
      m_outputTree->Branch("mbts_out_e", &t_mbts_out_e, "mbts_out_e[2][4]/F");
      m_outputTree->Branch("mbts_out_t", &t_mbts_out_t, "mbts_out_t[2][4]/F");

      m_outputTree->Branch("T2mbts_in_e", &t_T2mbts_in_e, "T2mbts_in_e[2][8]/F");
      m_outputTree->Branch("T2mbts_in_t", &t_T2mbts_in_t, "T2mbts_in_t[2][8]/F");
      m_outputTree->Branch("T2mbts_out_e", &t_T2mbts_out_e, "T2mbts_out_e[2][4]/F");
      m_outputTree->Branch("T2mbts_out_t", &t_T2mbts_out_t, "T2mbts_out_t[2][4]/F");

      m_outputTree->Branch("L1ET", &t_L1ET, "L1ET/F");
      m_outputTree->Branch("L1ET24", &t_L1ET24, "L1ET24/F");

      m_outputTree->Branch("totalEt", &t_totalEt, "totalEt/F");
      m_outputTree->Branch("totalEt_TTsum", &t_totalEt_TTsum, "totalEt_TTsum/F");

      m_outputTree->Branch("totalEt24", &t_totalEt24, "totalEt24/F");
      m_outputTree->Branch("totalEt24_TTsum", &t_totalEt24_TTsum, "totalEt24_TTsum/F");

      m_outputTree->Branch("fcalEt", &t_fcalEt, "fcalEt/F");
      m_outputTree->Branch("fcalEtA", &t_fcalEtA, "fcalEtA/F");
      m_outputTree->Branch("fcalEtC", &t_fcalEtC, "fcalEtC/F");
      m_outputTree->Branch("fcalEtA_TT", &t_fcalEtA_TT, "fcalEtA_TT/F");
      m_outputTree->Branch("fcalEtC_TT", &t_fcalEtC_TT, "fcalEtC_TT/F");

      m_outputTree->Branch("nvx", &t_nvx, "nvx/I");
      m_outputTree->Branch("vx", &t_vx, "vx[3]/F");
      m_outputTree->Branch("pvindex", &t_pvindex, "pvindex/I");
      m_outputTree->Branch("vxntrk", &t_vxntrk, "vxntrk/I");
      m_outputTree->Branch("vx_trk_index", "vector<int>", &t_vx_trk_index);
      m_outputTree->Branch("vxtype", &t_vxtype, "vxtype/I");
      m_outputTree->Branch("vxcov", &t_vxcov, "vxcov[6]/F");
      m_outputTree->Branch("vxsumpt2", &t_vxsumpt2, "vxsumpt2/F");
      m_outputTree->Branch("nstrong", &t_nstrong, "nstrong/I");
      m_outputTree->Branch("puvxntrk", &t_puvxntrk, "puvxntrk/I");
      m_outputTree->Branch("puvxsumpt", &t_puvxsumpt, "puvxsumpt/F");
      m_outputTree->Branch("puvxz", &t_puvxz, "puvxz/F");
      m_outputTree->Branch("vxnlooseprimary", &t_vxnlooseprimary, "vxnlooseprimary/I");
      m_outputTree->Branch("vxnminbias", &t_vxnminbias, "vxnminbias/I");
      m_outputTree->Branch("vxngoodmuon", &t_vxngoodmuon, "vxngoodmuon/I");

      m_outputTree->Branch("t_nvtx", &t_nvtx, "nvtx/I");
      m_outputTree->Branch("vtx_type", "vector<int8_t>", &t_vtx_type);
      m_outputTree->Branch("vtx_x", "vector<float>", &t_vtx_x);
      m_outputTree->Branch("vtx_y", "vector<float>", &t_vtx_y);
      m_outputTree->Branch("vtx_z", "vector<float>", &t_vtx_z);
      m_outputTree->Branch("vtx_ntrk_all", "vector<int16_t>", &t_vtx_ntrk_all);
      m_outputTree->Branch("vtx_sumpt2_all", "vector<float>", &t_vtx_sumpt2_all);
      m_outputTree->Branch("vtx_ntrk", "vector<int16_t>", &t_vtx_ntrk);
      m_outputTree->Branch("vtx_sumpt2", "vector<float>", &t_vtx_sumpt2);
      m_outputTree->Branch("vtx_trk_index", "vector< vector<int16_t> >", &t_vtx_trk_index);

      m_outputTree->Branch("mbts_countA", &t_mbts_countA, "mbts_countA/s");
      m_outputTree->Branch("mbts_countC", &t_mbts_countC, "mbts_countC/s");
      m_outputTree->Branch("T2mbts_countAin", &t_T2mbts_countAin, "T2mbts_countAin/s");
      m_outputTree->Branch("T2mbts_countCin", &t_T2mbts_countCin, "T2mbts_countCin/s");
      m_outputTree->Branch("mbts_timeA", &t_mbts_timeA, "mbts_timeA/F");
      m_outputTree->Branch("mbts_timeC", &t_mbts_timeC, "mbts_timeC/F");
      m_outputTree->Branch("mbts_timeDiff", &t_mbts_timeDiff, "mbts_timeDiff/F");

      t_nclus = 0;
      m_outputTree->Branch("nclus", &t_nclus, "nclus/i");
      m_outputTree->Branch("clusEt", &t_clusEt, "clusEt/F");
      m_outputTree->Branch("clusEtMax", &t_clusEtMax, "clusEtMax/F");
      m_outputTree->Branch("clusetaMax", &t_clusetaMax, "clusetaMax/F");
      m_outputTree->Branch("clusphiMax", &t_clusphiMax, "clusphiMax/F");

      m_outputTree->Branch("cc_pt", "vector<float>", &t_cc_pt);
      m_outputTree->Branch("cc_eta", "vector<float>", &t_cc_eta);
      m_outputTree->Branch("cc_phi", "vector<float>", &t_cc_phi);
      m_outputTree->Branch("cc_e", "vector<float>", &t_cc_e);
      m_outputTree->Branch("cc_raw_m", "vector<float>", &t_cc_raw_m);
      m_outputTree->Branch("cc_raw_eta", "vector<float>", &t_cc_raw_eta);
      m_outputTree->Branch("cc_raw_phi", "vector<float>", &t_cc_raw_phi);
      m_outputTree->Branch("cc_raw_e", "vector<float>", &t_cc_raw_e);
      m_outputTree->Branch("cc_raw_samp", "vector<vector<float>>", &t_cc_raw_samp);
      m_outputTree->Branch("cc_sig", "vector<float>", &t_cc_sig);
      m_outputTree->Branch("cc_layer", "vector<int>", &t_cc_layer);

      m_outputTree->Branch("edgeGapA", &t_edgeGapA, "edgeGapA/F");
      m_outputTree->Branch("edgeGapC", &t_edgeGapC, "edgeGapC/F");

    }
  }
  
  ANA_MSG_DEBUG("Anti-howdy from Initialize!");
  
  
  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files.  You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed.  So any objects
  // you create here won't be available in the output if you have no
  // input events.

  ANA_MSG_INFO("enableOutputTree = " << enableOutputTree);
  ANA_MSG_INFO("writeOnlyTriggers = " << writeOnlyTriggers);
  ANA_MSG_INFO("enableOutputSamples = " << enableOutputSamples);
  ANA_MSG_INFO("enableTrigger = " << enableTrigger);
  ANA_MSG_INFO("zdcCalib = " << zdcCalib);
  ANA_MSG_INFO("zdcLaser = " << zdcLaser);
  ANA_MSG_INFO("zdcConfig = " << zdcConfig);
  ANA_MSG_INFO("reprocZdc = " << reprocZdc);
  ANA_MSG_INFO("auxSuffix = " << auxSuffix );
  ANA_MSG_INFO("zdcLowGainOnly = " << zdcLowGainOnly);
  ANA_MSG_INFO("enableClusters = " << enableClusters);
  ANA_MSG_INFO("trackLimit = " << trackLimit);
  ANA_MSG_INFO("trackLimitReject = " << trackLimitReject);

  ANA_MSG_DEBUG("initialize: Initialize!");


  // GRL

  if (useGRL)
  {
    const char* fullGRLFilePath = gSystem->ExpandPathName (grlFilename.c_str());
    ANA_MSG_INFO("GRL: " << fullGRLFilePath);
    std::vector<std::string> vecStringGRL;
    vecStringGRL.push_back(fullGRLFilePath);
    ANA_CHECK(m_grl.setProperty( "GoodRunsListVec", vecStringGRL));
    ANA_CHECK(m_grl.setProperty("PassThrough", false)); // if true (default) will ignore result of GRL and will just pass all events
    ANA_CHECK(m_grl.initialize());

  }

  if (enableTrigger) // HLT related
  {
    ANA_MSG_INFO("Trying to initialize TDT");
    ANA_CHECK(m_trigDecisionTool.initialize());
  }

  // ZDC re-reco tool
  if (reprocZdc)
  {
    ANA_MSG_INFO("Trying to configure ZDC Analysis Tool!");

    ANA_CHECK(m_zdcAnalysisTool.setProperty("FlipEMDelay", flipDelay));
    ANA_CHECK(m_zdcAnalysisTool.setProperty("LowGainOnly", zdcLowGainOnly));
    ANA_CHECK(m_zdcAnalysisTool.setProperty("DoCalib", doZdcCalib));
    ANA_CHECK(m_zdcAnalysisTool.setProperty("Configuration", zdcConfig));
    ANA_CHECK(m_zdcAnalysisTool.setProperty("AuxSuffix", auxSuffix));
    ANA_CHECK(m_zdcAnalysisTool.setProperty("ForceCalibRun", -1));
    
    ANA_MSG_INFO("Setting up zdcConfig=" << zdcConfig);
    if (zdcConfig=="LHCf2022")
      {
	ANA_CHECK(m_zdcAnalysisTool.setProperty("DoTrigEff", false)); // for now
	ANA_CHECK(m_zdcAnalysisTool.setProperty("DoTimeCalib", false)); // for now
	ANA_CHECK(m_zdcAnalysisTool.setProperty("Configuration", "LHCf2022"));
      }
    else if (zdcConfig == "PbPb2018")
      {
	ANA_CHECK(m_zdcAnalysisTool.setProperty("Configuration", "PbPb2018"));	
      }
    else if (zdcConfig == "pPb2016")
      {
	ANA_CHECK(m_zdcAnalysisTool.setProperty("Configuration", "pPb2016"));
      }
    else if (zdcConfig == "PbPb2015")
      {
	ANA_CHECK(m_zdcAnalysisTool.setProperty("Configuration", "PbPb2015"));
      }
    
    if (flipDelay)
      ANA_MSG_INFO("FLIP ZDC DELAY IN EM MODULES");
    else
      ANA_MSG_INFO("NO FLIP ZDC DELAY IN EM MODULES");

    ANA_MSG_INFO("Trying to initialize ZDC Analysis Tool!");

    ANA_CHECK(m_zdcAnalysisTool.initialize());
    
  }
  
  
  return StatusCode::SUCCESS;
}

  
  
StatusCode ZdcNtuple :: execute ()
{
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.


  // prefilter with a track limit

  if (!evtStore())
  {
    ANA_MSG_INFO("*** No event found! ***");
    return StatusCode::SUCCESS;
  }

  ANA_CHECK(evtStore()->retrieve( m_eventInfo, "EventInfo"));
  processEventInfo();

  //tracks used to go here

  m_trackParticles = 0;

  if (!(zdcCalib || zdcLaser || zdcOnly))
  {
    ANA_MSG_DEBUG("Trying to extract InDetTrackParticles from evtStore()=" << evtStore());
    ANA_CHECK(evtStore()->retrieve( m_trackParticles, "InDetTrackParticles") );
    size_t n = m_trackParticles->size();
    ANA_MSG_DEBUG("Done w/ extracting InDetTrackParticles with size = " << n);

    if (n > trackLimit && trackLimitReject)  return StatusCode::SUCCESS;
  }

  bool passTrigger = true;
  m_trigDecision = 0;
  if (enableTrigger)
  {
    ANA_CHECK(evtStore()->retrieve( m_trigDecision, "xTrigDecision"));
    if (!m_setupTrigHist) setupTriggerHistos();
    passTrigger = processTriggerDecision();
  }

  if (!m_isMC)
  {
    if (reprocZdc)
    {
      ANA_CHECK(m_zdcAnalysisTool->reprocessZdc());
    }
    else
      ANA_MSG_DEBUG ("No reprocessing");

    m_zdcSums = 0;
    ANA_CHECK( evtStore()->retrieve( m_zdcSums, "ZdcSums" ) );

    m_zdcModules = 0;
    ANA_CHECK(evtStore()->retrieve( m_zdcModules, "ZdcModules" ) ); // ZDC modules keep same name, but the aux data get different suffix during reprocessing

    processZdcNtupleFromModules(); // same model in both cases -- processZdcNtuple() goes straight to the anlaysis tool, which is good for debugging

 
  }

  if (!(zdcCalib || zdcLaser || zdcOnly))
  {

    // disable this for 7660
    ANA_CHECK(evtStore()->retrieve( m_caloSums, "CaloSums") );
    ANA_CHECK(evtStore()->retrieve( m_eventShapes, "HIEventShape") );

    m_lvl1EnergySumRoI = 0;
    ANA_CHECK(evtStore()->retrieve( m_lvl1EnergySumRoI, "LVL1EnergySumRoI") );
    processFCal();

    ANA_CHECK(evtStore()->retrieve( m_mbtsInfo, "MBTSForwardEventInfo") );
    ANA_CHECK(evtStore()->retrieve( m_mbtsModules, "MBTSModules") );

    m_trigT2MbtsBits = 0;
    ANA_CHECK(evtStore()->retrieve( m_trigT2MbtsBits, "HLT_xAOD__TrigT2MbtsBitsContainer_T2Mbts") );
    processMBTS();

    ANA_CHECK(evtStore()->retrieve( m_primaryVertices, "PrimaryVertices") );
    processInDet();


    ANA_CHECK(evtStore()->retrieve( m_caloClusters, "CaloCalTopoClusters"));
    processClusters();
    processGaps();

  }

  if (m_isMC)
  {
    ANA_CHECK(evtStore()->retrieve( m_truthParticleContainer, "TruthParticles"));
  }

  // if trigger enabled, only write out events which pass one of them, unless using MC

  if (enableTrigger && !passTrigger && !m_isMC && writeOnlyTriggers) return StatusCode::SUCCESS;


  if (enableOutputTree)
  {
    tree( "zdcTree" )->Fill();
  }

  return StatusCode::SUCCESS;
}

void ZdcNtuple::processZdcNtupleFromModules()
{

  ANA_MSG_DEBUG ("copying already processed info!");
  if (m_zdcSums)
  {

    ANA_MSG_DEBUG ("Sum 0 = " << m_zdcSums->at(0)->auxdataConst<float>("CalibEnergy"+auxSuffix)
                   << ", Sum 1 = " << m_zdcSums->at(1)->auxdataConst<float>("CalibEnergy"+auxSuffix));
    ANA_MSG_DEBUG ("Final 0 = " << m_zdcSums->at(0)->auxdataConst<float>("FinalEnergy"+auxSuffix)
                   << ", Final 1 = " << m_zdcSums->at(1)->auxdataConst<float>("FinalEnergy"+auxSuffix));

  }
  else
  {
    ANA_MSG_INFO( "No ZDC sums produced!");
  }

  for (size_t iside = 0; iside < 2; iside++)
  {
    t_ZdcAmp[iside] = 0; t_ZdcEnergy[iside] = 0; t_ZdcTime[iside] = 0; t_ZdcStatus[iside] = 0;
    t_ZdcTrigEff[iside] = 0;t_ZdcLucrodTriggerSideAmp[iside] = 0;
    for (int imod = 0; imod < 4; imod++)
    {
      t_ZdcModuleAmp[iside][imod] = 0; t_ZdcModuleTime[iside][imod] = 0; t_ZdcModuleStatus[iside][imod] = 0;

      t_ZdcModuleCalibAmp[iside][imod] = 0; t_ZdcModuleCalibTime[iside][imod] = 0; t_ZdcModuleChisq[iside][imod] = 0; t_ZdcModuleFitAmp[iside][imod] = 0;
      t_ZdcModuleFitT0[iside][imod] = 0; t_ZdcModuleBkgdMaxFraction[iside][imod] = 0; t_ZdcModuleAmpError[iside][imod] = 0;
      t_ZdcModuleMinDeriv2nd[iside][imod] = 0; t_ZdcModulePresample[iside][imod] = 0; t_ZdcModulePreSampleAmp[iside][imod] = 0;
      t_ZdcLucrodTriggerAmp[iside][imod] = 0;t_ZdcModuleMaxADC[iside][imod] = 0;
    }
  }

  t_ZdcModuleMask = 0;

  if (m_zdcSums)
  {
    ANA_MSG_DEBUG( "accessing ZdcSums" );
    for (const auto zdcSum : *m_zdcSums)
    {
      int iside = 0;
      if (zdcSum->zdcSide() > 0) iside = 1;

      //static SG::AuxElement::ConstAccessor< float > acc( "CalibEnergy" );
      //t_ZdcEnergy[iside] = acc(*zdcSum);

      t_ZdcEnergy[iside] = zdcSum->auxdataConst<float>("CalibEnergy"+auxSuffix);
      t_ZdcEnergyErr[iside] = zdcSum->auxdataConst<float>("CalibEnergyErr"+auxSuffix);

      t_ZdcAmp[iside] = zdcSum->auxdataConst<float>("UncalibSum"+auxSuffix);
      t_ZdcAmpErr[iside] = zdcSum->auxdataConst<float>("UncalibSumErr"+auxSuffix);
      t_ZdcLucrodTriggerSideAmp[iside] = zdcSum->auxdataConst<uint16_t>("LucrodTriggerSideAmp");
      ANA_MSG_VERBOSE("processZdcNtupleFromModules: ZdcSum energy = " << t_ZdcEnergy[iside]);

      t_ZdcTime[iside] = zdcSum->auxdataConst<float>("AverageTime"+auxSuffix);
      t_ZdcStatus[iside] = zdcSum->auxdataConst<unsigned int>("Status"+auxSuffix);
      t_ZdcModuleMask += (zdcSum->auxdataConst<unsigned int>("ModuleMask"+auxSuffix) << 4 * iside);
    }
  }

  ANA_MSG_DEBUG(  "accessing ZdcModules" );
  if (m_zdcModules)
  {
    for (const auto zdcMod : *m_zdcModules)
    {
      int iside = 0;
      if (zdcMod->zdcSide() > 0) iside = 1;
      int imod = zdcMod->zdcModule();

      ANA_MSG_VERBOSE ("Module " << zdcMod->zdcSide() << " " << zdcMod->zdcModule() << " amp:" << zdcMod->auxdataConst<float>("Amplitude"));

      if (zdcMod->zdcType() == 0) {
        // thi is the ZDC
        t_ZdcModuleCalibAmp[iside][imod] = zdcMod->auxdataConst<float>("CalibEnergy" + auxSuffix);
        t_ZdcModuleCalibTime[iside][imod] = zdcMod->auxdataConst<float>("CalibTime" + auxSuffix);
        t_ZdcModuleStatus[iside][imod] = zdcMod->auxdataConst<unsigned int>("Status" + auxSuffix);
        if (t_ZdcModuleAmp[iside][imod] != 0.)
          Warning("processZdcNtupleFromModules", "overwriting side %d module %d!", iside, imod);
        t_ZdcModuleAmp[iside][imod] = zdcMod->auxdataConst<float>("Amplitude" + auxSuffix);
        t_ZdcModuleTime[iside][imod] = zdcMod->auxdataConst<float>("Time" + auxSuffix);

        t_ZdcModuleChisq[iside][imod] = zdcMod->auxdataConst<float>("Chisq" + auxSuffix);
        t_ZdcModuleFitAmp[iside][imod] = zdcMod->auxdataConst<float>("FitAmp" + auxSuffix);
        t_ZdcModuleAmpError[iside][imod] = zdcMod->auxdataConst<float>("FitAmpError" + auxSuffix);
        t_ZdcModuleFitT0[iside][imod] = zdcMod->auxdataConst<float>("FitT0" + auxSuffix);
        t_ZdcModuleBkgdMaxFraction[iside][imod] = zdcMod->auxdataConst<float>("BkgdMaxFraction" + auxSuffix);
        t_ZdcModuleMinDeriv2nd[iside][imod] = zdcMod->auxdataConst<float>("MinDeriv2nd" + auxSuffix);
        t_ZdcModulePresample[iside][imod] = zdcMod->auxdataConst<float>("Presample" + auxSuffix);
        t_ZdcModulePreSampleAmp[iside][imod] = zdcMod->auxdataConst<float>("PreSampleAmp" + auxSuffix);
        t_ZdcLucrodTriggerAmp[iside][imod] = zdcMod->auxdataConst<uint16_t>("LucrodTriggerAmp");
        t_ZdcModuleMaxADC[iside][imod] = zdcMod->auxdataConst<float>("MaxADC");

        if (enableOutputSamples)
          {
            for (unsigned int isamp = 0; isamp < nsamplesZdc; isamp++) // 7 samples
        {
          if (nsamplesZdc == 7)
      {
        t_raw7[iside][imod][0][0][isamp] = (zdcMod->auxdataConst<std::vector<uint16_t>>("g0data")).at(isamp);
        t_raw7[iside][imod][1][0][isamp] = (zdcMod->auxdataConst<std::vector<uint16_t>>("g1data")).at(isamp);
      }
          
          if (nsamplesZdc == 15)
      {
        t_raw15[iside][imod][0][0][isamp] = (zdcMod->auxdataConst<std::vector<uint16_t>>("g0data")).at(isamp);
        t_raw15[iside][imod][1][0][isamp] = (zdcMod->auxdataConst<std::vector<uint16_t>>("g1data")).at(isamp);
      }

          if (nsamplesZdc == 24)
      {
        t_raw24[iside][imod][0][0][isamp] = (zdcMod->auxdataConst<std::vector<uint16_t>>("g0data")).at(isamp);
        t_raw24[iside][imod][1][0][isamp] = (zdcMod->auxdataConst<std::vector<uint16_t>>("g1data")).at(isamp);
      }
        }
          }

      } else if (zdcMod->zdcType() == 1) {
        // this is the RPD
        int rpdChannel = zdcMod->zdcChannel() - 1; // zero-based
        t_RPDChannelAmplitude[iside][rpdChannel] = zdcMod->auxdataConst<float>("RPDChannelAmplitude" + auxSuffix);
        t_RPDChannelMaxSample[iside][rpdChannel] = zdcMod->auxdataConst<unsigned int>("RPDChannelMaxSample" + auxSuffix);

      }

 
    }
  }
  else
  {
    ANA_MSG_INFO("No ZdcModules" << auxSuffix << " when expected!");
  }

  if (msgLvl (MSG::VERBOSE))
  {
    std::ostringstream message;
    message << "Dump zdc_ZdcModuleAmp: ";
    for (int iside = 0; iside < 2; iside++)
    {
      for (int imod = 0; imod < 4; imod++)
      {
        message << t_ZdcModuleAmp[iside][imod] << " ";
      }
    }
  }
}


bool ZdcNtuple::processTriggerDecision()
{
  ANA_MSG_DEBUG ("Processing trigger");

  bool passTrigger = false;

  t_trigger = 0;
  t_trigger_TBP = 0;
  for (int i = 0; i < 16; i++)
  {
    t_tav[i] = 0;
    t_tbp[i] = 0;
  }

  if (m_trigDecision)
    {
      for (int i = 0; i < 16; i++)
	{
	  t_tbp[i] = m_trigDecision->tbp().at(i);
	  t_tav[i] = m_trigDecision->tav().at(i);	  
	  ATH_MSG_DEBUG( "TD: " << i << " tbp: " << std::hex << t_tbp[i] << "\t" << t_tav[i] );
	}
    }

  if (enableTrigger)
  {

    int ic = 0;
    for (auto cg : m_chainGroups)
    {
      if (cg->isPassed())
      {
        t_trigger += (1 << ic);
        t_decisions[ic] = true;
        t_prescales[ic] = cg->getPrescale();
        passTrigger = true;
      }
      else
      {
        t_decisions[ic] = 0;
        t_prescales[ic] = 0;
      }


      if (cg->isPassedBits()&TrigDefs::EF_passedRaw)
      {
        t_trigger_TBP += (1 << ic);
      }


      ic++;
    }

    int irc = 0;
    for (auto cg : m_rerunChainGroups)
    {
      t_rerunDecisions[irc] = false;
      if (cg->isPassedBits()&TrigDefs::EF_passedRaw)
      {
        t_rerunDecisions[irc] = true;
      }
      irc++;
    }

  }

  return passTrigger;
}

void ZdcNtuple::processEventInfo()
{
  ANA_MSG_DEBUG( "processing event info");

  t_bcid = m_eventInfo->bcid();
  t_runNumber = m_eventInfo->runNumber();
  t_eventNumber = m_eventInfo->eventNumber();
  t_lumiBlock = m_eventInfo->lumiBlock();
  t_extendedLevel1ID = m_eventInfo->extendedLevel1ID();
  t_timeStamp = m_eventInfo->timeStamp();
  t_timeStampNSOffset = m_eventInfo->timeStampNSOffset();
  t_passBits = acceptEvent();
  t_avgIntPerCrossing = m_eventInfo->averageInteractionsPerCrossing();
  t_actIntPerCrossing = m_eventInfo->actualInteractionsPerCrossing();

  if ( !(m_eventCounter++ % 1000) || msgLvl(MSG::DEBUG))
  {
    ANA_MSG_INFO("Event# " << m_eventCounter << "Run " << m_eventInfo->runNumber() << " Event " << m_eventInfo->eventNumber() << " LB " << m_eventInfo->lumiBlock() );
  }

}

void ZdcNtuple::processInDet()
{
  ANA_MSG_DEBUG("processInDet(): processing tracks & vertices!");
  t_ntrk = 0;
  t_nvx = 0;
  t_vxntrk = 0;
  t_vx_trk_index.clear();
  t_vxsumpt2 = 0;
  t_vxtype = 0;
  t_pvindex = -1;
  t_puvxntrk = 0;
  t_puvxsumpt = 0;
  t_vxnlooseprimary = 0;
  t_vxnminbias = 0;

  int i;
  for (i = 0; i < 3; i++) t_vx[i] = 0;
  for (i = 0; i < 6; i++) t_vxcov[i] = 0;

  const xAOD::Vertex* primary_vertex = nullptr;
  size_t pv_index = -1;
  size_t vx_index = 0;
  float max_pileup_sumpT = 0.;
  int max_pileup_nTrack = 0;
  float max_pileup_z = 0;
  int nStrongPileup = 0;

  t_nvtx = 0;
  t_vtx_type.clear();
  t_vtx_x.clear();
  t_vtx_y.clear();
  t_vtx_z.clear();
  t_vtx_ntrk_all.clear();
  t_vtx_sumpt2_all.clear();
  t_vtx_ntrk.clear();
  t_vtx_sumpt2.clear();
  t_vtx_trk_index.clear();

  if (m_primaryVertices)
    {
      ANA_MSG_DEBUG("processInDet: processing vertices");

      t_nvx = m_primaryVertices->size();

      // start of new vertex representation
      t_nvtx = m_primaryVertices->size();
      for (const auto vertex : *m_primaryVertices)
	{
	  float vtx_sumpt2 = 0;
	  int vtx_ntrk = 0;

	  t_vtx_type.push_back(vertex->vertexType());
	  t_vtx_x.push_back(0);
	  t_vtx_y.push_back(0);
	  t_vtx_z.push_back(vertex->z());

	  t_vtx_ntrk.push_back(vtx_ntrk);
	  t_vtx_sumpt2.push_back(vtx_sumpt2 / 1e6);
	  t_vtx_ntrk_all.push_back(vertex->nTrackParticles());

	  if (vertex->isAvailable<float>("sumPt2"))
	    t_vtx_sumpt2_all.push_back(vertex->auxdataConst<float>("sumPt2"));
	  else
	    t_vtx_sumpt2_all.push_back(-1);

	  std::vector<int16_t> trk_index;
	  if ( m_trackParticles && vertex->nTrackParticles() <= trackLimit )
	    {
	      const std::vector< ElementLink< xAOD::TrackParticleContainer > >& vxTrackParticles = vertex->trackParticleLinks();
	      for (size_t itrk = 0; itrk < vxTrackParticles.size(); itrk++)
		{
		  ElementLink< xAOD::TrackParticleContainer > trkLink = vxTrackParticles.at(itrk);
		  trk_index.push_back(trkLink.index());
		}
	    }
	  t_vtx_trk_index.push_back(trk_index);

	  // end of new vertex representation

	  if (vertex->vertexType() == xAOD::VxType::PriVtx)
	    {
	      primary_vertex = vertex;
	      pv_index = vx_index;
	    }
	  if (vertex->vertexType() == xAOD::VxType::PileUp)
	    {
	      float pileup_sumpT = 0;
	      int pileup_nTrack = 0;
	      for (size_t itr = 0; itr < vertex->nTrackParticles(); itr++)
		{
		  int track_quality = trackQuality(vertex->trackParticle(itr), vertex);
		  if (track_quality != -1 && (track_quality & 128) != 0)
		    {
		      pileup_nTrack++;
		      pileup_sumpT += vertex->trackParticle(itr)->pt();
		    }
		}
	      if (pileup_sumpT > max_pileup_sumpT)
		{
		  max_pileup_sumpT = pileup_sumpT;
		  max_pileup_nTrack = pileup_nTrack;
		  max_pileup_z = vertex->z();
		}
	      if (pileup_sumpT > 5e3 || pileup_nTrack > 5) nStrongPileup++;
	    }
	  vx_index++;
	}
    }

  t_nstrong = nStrongPileup;

  if (primary_vertex != nullptr)
    {
      t_vx[0] = primary_vertex->x();
      t_vx[1] = primary_vertex->y();
      t_vx[2] = primary_vertex->z();
      /*
    const std::vector<float>& cov = primary_vertex->covariance();
      */
      //for (size_t i=0;i<cov.size();i++)
      for (size_t i = 0; i < 6; i++)
	{
	  //t_vxcov[i] = cov.at(i);
	  t_vxcov[i] = 0;
	}
      t_vxntrk = primary_vertex->nTrackParticles();
      if (primary_vertex->isAvailable<float>("sumPt2"))
	t_vxsumpt2 = primary_vertex->auxdata<float>("sumPt2");
      else
	t_vxsumpt2 = 0;
      t_vxtype = primary_vertex->vertexType();
      t_pvindex = pv_index;
      t_puvxz = max_pileup_z;
      t_puvxsumpt = max_pileup_sumpT;
      t_puvxntrk = max_pileup_nTrack;

      const std::vector< ElementLink< xAOD::TrackParticleContainer > >& vxTrackParticles = primary_vertex->trackParticleLinks();

      for (size_t itrk = 0; itrk < vxTrackParticles.size(); itrk++)
	{
	  ElementLink< xAOD::TrackParticleContainer > trkLink = vxTrackParticles.at(itrk);
	  if (!trkLink.isValid()) continue;
	  if (m_trackParticles)
	    {
	      if (m_trackParticles->size() <= trackLimit)
		t_vx_trk_index.push_back(trkLink.index());
	    }
	}

    }


  if (m_trackParticles)
    {
      ANA_MSG_DEBUG("processInDet: processing trackss");

      t_trk_pt.clear();
      t_trk_eta.clear();
      t_trk_phi.clear();
      t_trk_e.clear();
      t_trk_index.clear();
      t_trk_theta.clear();
      t_trk_charge.clear();
      t_trk_d0.clear();
      t_trk_z0.clear();
      t_trk_vz.clear();
      t_trk_vtxz.clear();
      t_trk_quality.clear();
      t_trk_nPixHits.clear();
      t_trk_nSctHits.clear();
      t_trk_nPixDead.clear();
      t_trk_nSctDead.clear();
      t_trk_nPixHoles.clear();
      t_trk_nSctHoles.clear();
      t_trk_nTrtHits.clear();
      t_trk_nTrtOutliers.clear();
      t_trk_inPixHits.clear();
      t_trk_exPixHits.clear();
      t_trk_ninPixHits.clear();
      t_trk_nexPixHits.clear();
      t_trk_pixeldEdx.clear();

      t_ntrk = m_trackParticles->size();

      if ( !enableTracks ) return;

      if ( t_ntrk <= trackLimit ) // dump all tracks
	{
	  int trk_index = 0;
	  for (const auto track : *m_trackParticles)
	    {
	      writeTrack(track, primary_vertex, trk_index++);
	    }
	}
      else // write small vertices
	{
	  for (const auto vertex : *m_primaryVertices)
	    {
	      if (vertex->nTrackParticles() <= trackLimit )
		{
		  const std::vector< ElementLink< xAOD::TrackParticleContainer > >& vxTrackParticles = vertex->trackParticleLinks();
		  for (size_t itrk = 0; itrk < vxTrackParticles.size(); itrk++)
		    {
		      ElementLink< xAOD::TrackParticleContainer > trkLink = vxTrackParticles.at(itrk);
		      writeTrack(*trkLink, vertex, trkLink.index());
		    }
		}
	    }
	}
    }

  return;
}


int ZdcNtuple::trackQuality(const xAOD::TrackParticle* track, const xAOD::Vertex* vertex)
{
  // Code from Soumya Mohapatra
  if (vertex) {}

  if (!track) return -1;

  float pt      = track->pt();
  float eta     = track->eta();
  float chi2    = track->chiSquared();
  float ndof    = track->numberDoF();
  float d0      = track->d0();
  float z0_wrtPV = track->z0();
  float theta   = track->theta();

  uint8_t   n_Ipix_hits     = track->auxdata<uint8_t>("numberOfInnermostPixelLayerHits");
  uint8_t   n_Ipix_expected = track->auxdata<uint8_t>("expectInnermostPixelLayerHit");
  uint8_t   n_NIpix_hits    = track->auxdata<uint8_t>("numberOfNextToInnermostPixelLayerHits");
  uint8_t   n_NIpix_expected = track->auxdata<uint8_t>("expectNextToInnermostPixelLayerHit");
  uint8_t   n_sct_hits      = track->auxdata<uint8_t>("numberOfSCTHits");
  uint8_t   n_pix_hits      = track->auxdata<uint8_t>("numberOfPixelHits");
  uint8_t   n_sct_holes     = track->auxdata<uint8_t>("numberOfSCTHoles");
  //uint8_t   n_pix_holes     =track->auxdata<uint8_t>("numberOfPixelHoles");
  uint8_t   n_sct_dead      = track->auxdata<uint8_t>("numberOfSCTDeadSensors");
  uint8_t   n_pix_dead      = track->auxdata<uint8_t>("numberOfPixelDeadSensors");

  if (std::abs(eta) > 2.5) return -1;

  bool pass_min_bias = true;
  {
    if (n_Ipix_expected > 0) {
      if (n_Ipix_hits == 0) pass_min_bias = false;
    }
    else {
      if (n_NIpix_expected > 0 && n_NIpix_hits == 0) pass_min_bias = false;
    }

    int n_sct = n_sct_hits + n_sct_dead;
    if     (pt <= 300) {if (n_sct < 2)  pass_min_bias = false;}
    else if (pt <= 400) {if (n_sct < 4)  pass_min_bias = false;}
    else if (pt > 400) {if (n_sct < 6)  pass_min_bias = false;}

    int n_pix = n_pix_hits + n_pix_dead;
    if (n_pix <= 0) pass_min_bias = false;
    if (std::abs(d0) > 1.5) pass_min_bias = false;
    if (std::abs(z0_wrtPV * sin(theta)) > 1.5) pass_min_bias = false;

    if (pt > 10000 && TMath::Prob(chi2, ndof) <= 0.01) pass_min_bias = false;
  }

  bool pass_hi_loose = true;
  {
    if (n_Ipix_expected > 0) {
      if (n_Ipix_hits == 0) pass_hi_loose = false;
    }
    else {
      if (n_NIpix_expected > 0 && n_NIpix_hits == 0) pass_hi_loose = false;
    }

    if (n_pix_hits == 0) pass_hi_loose = false;
    if (n_sct_hits < 6) pass_hi_loose = false;
    if (pt > 10000 && TMath::Prob(chi2, ndof) <= 0.01) pass_hi_loose = false;
    if (std::abs(d0) > 1.5) pass_hi_loose = false;
    if (std::abs(z0_wrtPV * sin(theta)) > 1.5) pass_hi_loose = false;
  }

  bool pass_hi_tight = true;
  if (!pass_hi_loose) pass_hi_tight = false;
  else {
    if (n_pix_hits < 2  ) pass_hi_tight = false;
    if (n_sct_hits < 8  ) pass_hi_tight = false;
    if (n_sct_holes > 1  ) pass_hi_tight = false;
    if (std::abs(d0)   > 1.0) pass_hi_tight = false;
    if (std::abs(z0_wrtPV * sin(theta)) > 1.0) pass_hi_tight = false;
    if (ndof == 0) pass_hi_tight = false;
    else if (chi2 / ndof > 6) pass_hi_tight = false;
  }

  int quality = 0;
  if (pass_min_bias) quality += 2;
  if (pass_hi_loose) quality += 4;
  if (pass_hi_tight) quality += 8;

  return quality;
}


void ZdcNtuple::writeTrack(const xAOD::TrackParticle* track, const xAOD::Vertex* vertex, int trk_index)
{
  t_trk_pt.push_back(track->pt());
  t_trk_eta.push_back(track->eta());
  t_trk_phi.push_back(track->phi());
  t_trk_e.push_back(track->e());
  t_trk_index.push_back(trk_index);
  t_trk_theta.push_back(track->theta());
  t_trk_charge.push_back(track->charge());
  t_trk_d0.push_back(track->d0());
  t_trk_z0.push_back(track->z0());
  t_trk_vz.push_back(track->vz());

  float vtxz = -999.;
  t_trk_vtxz.push_back(vtxz);

  t_trk_quality.push_back(trackQuality(track, vertex));
  t_trk_inPixHits.push_back(track->auxdata<uint8_t>("numberOfInnermostPixelLayerHits"));
  t_trk_exPixHits.push_back(track->auxdata<uint8_t>("expectInnermostPixelLayerHit"));
  t_trk_ninPixHits.push_back(track->auxdata<uint8_t>("numberOfNextToInnermostPixelLayerHits"));
  t_trk_nexPixHits.push_back(track->auxdata<uint8_t>("expectNextToInnermostPixelLayerHit"));
  t_trk_nSctHits.push_back(track->auxdata<uint8_t>("numberOfSCTHits"));
  t_trk_nPixHits.push_back(track->auxdata<uint8_t>("numberOfPixelHits"));
  t_trk_nSctDead.push_back(track->auxdata<uint8_t>("numberOfSCTDeadSensors"));
  t_trk_nPixDead.push_back(track->auxdata<uint8_t>("numberOfPixelDeadSensors"));
  t_trk_nSctHoles.push_back(track->auxdata<uint8_t>("numberOfSCTHoles"));
  t_trk_nPixHoles.push_back(track->auxdata<uint8_t>("numberOfPixelHoles"));
  t_trk_nTrtHits.push_back(track->auxdata<uint8_t>("numberOfTRTHits"));
  t_trk_nTrtOutliers.push_back(track->auxdata<uint8_t>("numberOfTRTOutliers"));
  float pixeldEdx = 0;
  track->summaryValue(pixeldEdx, xAOD::SummaryType::pixeldEdx);
  t_trk_pixeldEdx.push_back(pixeldEdx);
}


void ZdcNtuple::processFCal()
{
  ANA_MSG_DEBUG("processFCal: processing FCal");

  t_fcalEt = 0.;
  t_fcalEtA = 0.;
  t_fcalEtC = 0.;
  t_fcalEtA_TT = 0.;
  t_fcalEtC_TT = 0.;

  if (m_caloSums)
    {
      for (const auto calosum : *m_caloSums)
	{
	  const std::string name = calosum->auxdataConst<std::string>("Summary");
	  if (name == "FCal")
	    {
	      t_fcalEt = calosum->et();
	      ANA_MSG_DEBUG("processFCal: fcalEt = " << t_fcalEt);
	    }
	  
	  if (name == "All")
	    {
	      t_totalEt = calosum->et();
	      ANA_MSG_DEBUG("processFCal: totalEt = " << t_totalEt);
	    }
	}
    }

  t_fcalEtA = 0;
  t_fcalEtC = 0;
  t_totalEt24 = 0;

  if (m_eventShapes)
    {
      for (const auto eventShape : *m_eventShapes)
	{
	  int layer = eventShape->layer();
	  float eta = eventShape->etaMin();
	  float et = eventShape->et();
	  if (layer == 21 || layer == 22 || layer == 23)
	    {
	      if (eta > 0) t_fcalEtA += et;
	      if (eta < 0) t_fcalEtC += et;
	    }
	  
	  if (TMath::Abs(eta) < 2.4)
	    {
	      t_totalEt24 += et;
	    }
	}
    }

  t_L1ET = 0;
  t_L1ET24 = 0;

  if (m_lvl1EnergySumRoI)
  {
    t_L1ET = m_lvl1EnergySumRoI->energyT();
    //t_L1ET24 = m_lvl1EnergySumRoI->energyTRestricted(); // TBD when limited eta ET available
  }

  return;
}

void ZdcNtuple::processGaps()
{

  float eta_min = 5;
  float eta_max = -5;

  if (!m_caloClusters) return;
  for (const auto cl : *m_caloClusters)
  {

    if (cl->pt() < m_gapPtMin) continue;

    int etabin = h_TCSigCut->GetXaxis()->FindBin(cl->eta());
    if (etabin < 1 || etabin > h_TCSigCut->GetNbinsX()) continue;
    float sig_cut = h_TCSigCut->GetBinContent(etabin);
    float sig = cl->getMomentValue(xAOD::CaloCluster::CELL_SIGNIFICANCE);
    int cl_cell_sig_samp = static_cast<int>(cl->getMomentValue(xAOD::CaloCluster::CELL_SIG_SAMPLING));

    ANA_MSG_VERBOSE ("gapclus: etabin " << etabin << " sig_cut=" << sig_cut << " sig=" << sig << " samp=" << cl_cell_sig_samp);

    if (sig < sig_cut) continue;

    if (cl_cell_sig_samp >= CaloSampling::TileBar0 && cl_cell_sig_samp <= CaloSampling::TileExt2) continue;

    if (cl->eta() < eta_min) eta_min = cl->eta();
    if (cl->eta() > eta_max) eta_max = cl->eta();

  }

  t_edgeGapA = 4.9 - eta_max;
  t_edgeGapC = eta_min + 4.9;
  ANA_MSG_DEBUG("processGaps(): egA " << t_edgeGapA << " , egC " << t_edgeGapC);

}

void ZdcNtuple::processMBTS()
{
  ANA_MSG_DEBUG("processMBTS: trying to process!");
  t_mbts_countA = 0;
  t_mbts_countC = 0;
  t_T2mbts_countAin = 0;
  t_T2mbts_countCin = 0;
  t_mbts_timeA = 0.;
  t_mbts_timeC = 0.;
  t_mbts_timeDiff = 0.;

  if (m_mbtsInfo->size() > 0)
  {
    t_mbts_countA = m_mbtsInfo->at(0)->countA();
    t_mbts_countC = m_mbtsInfo->at(0)->countC();
    t_mbts_timeA = m_mbtsInfo->at(0)->timeA();
    t_mbts_timeC = m_mbtsInfo->at(0)->timeC();
    t_mbts_timeDiff = m_mbtsInfo->at(0)->timeDiff();
  }
  else
  {
    ANA_MSG_INFO("processMBTS: Warning: MBTS info empty!");
  }

  for (int iside = 0; iside < 2; iside++)
  {
    for (int iin = 0; iin < 8; iin++)
    {
      t_mbts_in_e[iside][iin] = 0.;
      t_mbts_in_t[iside][iin] = 0.;
      t_T2mbts_in_e[iside][iin] = 0.;
      t_T2mbts_in_t[iside][iin] = 0.;
    }
    for (int iout = 0; iout < 4; iout++)
    {
      t_mbts_out_e[iside][iout] = 0.;
      t_mbts_out_t[iside][iout] = 0.;
      t_T2mbts_out_e[iside][iout] = 0.;
      t_T2mbts_out_t[iside][iout] = 0.;
    }
  }

  ANA_MSG_DEBUG ("filling MBTS");

  if (m_mbtsModules == 0)
  {
    ANA_MSG_INFO("processMBTS: no MBTS container?");
    return;
  }

  for (const auto mbtsMod : *m_mbtsModules)
  {
    int iside = 1;
    if (mbtsMod->type() < 0) iside = 0.;
    float phibin = 0.;
    int iphibin = -1;
    if (mbtsMod->eta() > 3)
    {
      phibin = mbtsMod->phi() / (2 * TMath::Pi() / 8.) - 0.4;
      iphibin = static_cast<int>(phibin);
      if (iphibin < 0 || iphibin > 7)
      {
        ANA_MSG_INFO("processMBTS: MBTS has bad phi bin");
        continue;
      }
      t_mbts_in_e[iside][iphibin] = mbtsMod->e();
      t_mbts_in_t[iside][iphibin] = mbtsMod->time();
    }
    else
    {
      phibin = mbtsMod->phi() / (2 * TMath::Pi() / 4.) - 0.24;
      iphibin = static_cast<int>(phibin);
      if (iphibin < 0 || iphibin > 3)
      {
        ANA_MSG_INFO("processMBTS: MBTS has bad phi bin");
        continue;
      }
      t_mbts_out_e[iside][iphibin] = mbtsMod->e();
      t_mbts_out_t[iside][iphibin] = mbtsMod->time();
    }
  }

  if (!m_trigT2MbtsBits) return;

  for (const auto mbtsBits : *m_trigT2MbtsBits)
  {
    const std::vector<float>& energies = mbtsBits->triggerEnergies();
    const std::vector<float>& times = mbtsBits->triggerTimes();
    for (int imbts = 0; imbts < 32; imbts++)
    {
      int side = imbts / 16;
      int ring = (imbts - 16 * side) / 8;
      bool isInner = (ring == 0);
      int index = (imbts - 16 * side - ring * 8);
      if (!isInner)
      {
        if ((index % 2) != 0) continue; // skip odd out ring
        index /= 2;
      }
      int iside = (side == 0) ? 1 : 0; // code maps side 1 into first 16 bits and side -1 into second set

      ANA_MSG_VERBOSE ("imbts=" << imbts << " isInner=" << isInner << " iside=" << iside << " index=" << index << " e=" << energies.at(imbts) << " t=" << times.at(imbts));
      if (isInner)
      {
        t_T2mbts_in_e[iside][index] = energies.at(imbts);
        t_T2mbts_in_t[iside][index] = times.at(imbts);
        if (TMath::Abs(times.at(imbts)) < 12.0 && energies.at(imbts) > 40 / 222.)
        {
          if (iside == 0) t_T2mbts_countCin++;
          if (iside == 1) t_T2mbts_countAin++;
        }
      }
      else
      {
        t_T2mbts_out_e[iside][index] = energies.at(imbts);
        t_T2mbts_out_t[iside][index] = times.at(imbts);
      }
    }
  }

  return;
}

void ZdcNtuple::processClusters()
{
  //t_nclus = 0;

  t_cc_pt.clear();
  t_cc_eta.clear();
  t_cc_phi.clear();
  t_cc_e.clear();
  t_cc_raw_m.clear();
  t_cc_raw_eta.clear();
  t_cc_raw_phi.clear();
  t_cc_raw_e.clear();
  t_cc_raw_samp.clear();
  t_cc_layer.clear();
  t_cc_sig.clear();

  t_nclus = m_caloClusters->size();

  t_clusEt = 0;
  t_clusEtMax = -999;
  t_clusetaMax = 0;
  t_clusphiMax = 0;

  for (const auto cluster : *m_caloClusters)
  {
    t_cc_pt.push_back(cluster->pt());
    t_cc_eta.push_back(cluster->eta());
    t_cc_phi.push_back(cluster->phi());
    t_cc_e.push_back(cluster->e());
    t_cc_raw_m.push_back(cluster->rawM());
    t_cc_raw_eta.push_back(cluster->rawEta());
    t_cc_raw_phi.push_back(cluster->rawPhi());
    t_cc_raw_e.push_back(cluster->rawE());

    std::vector<float> energies;

    for (size_t s = CaloSampling::PreSamplerB; s < CaloSampling::Unknown; s++ )
    {
      bool hasSample = cluster->hasSampling( (xAOD::CaloCluster::CaloSample) s );
      float e = 0;
      if (hasSample)
      {
        e = cluster->eSample( (xAOD::CaloCluster::CaloSample) s);
      }
      energies.push_back(e);
    }
    t_cc_raw_samp.push_back(energies);

    float et = cluster->e() / TMath::CosH(cluster->eta());
    t_clusEt += et;
    if (et > t_clusEtMax)
    {
      t_clusEtMax = et;
      t_clusetaMax = cluster->eta();
      t_clusphiMax = cluster->phi();
    }

    double cell_sig = 0;
    if (!cluster->retrieveMoment(xAOD::CaloCluster::CELL_SIGNIFICANCE, cell_sig)) {ANA_MSG_DEBUG("processClusters() : No CELL_SIGNIFICANCE!");}
    t_cc_sig.push_back(cell_sig);
    double cell_layer = 0;
    if (!cluster->retrieveMoment(xAOD::CaloCluster::CELL_SIG_SAMPLING, cell_layer)) {ANA_MSG_DEBUG("processClusters() : No CELL_SIG_SAMPLING!");}
    t_cc_layer.push_back(static_cast<int>(cell_layer));
    //t_nclus++;
  }

  if ( (!enableClusters) || (t_ntrk >= trackLimit) ) // if disabled or if too many tracks
  {
    t_cc_pt.clear();
    t_cc_eta.clear();
    t_cc_phi.clear();
    t_cc_e.clear();
    t_cc_layer.clear();
    t_cc_sig.clear();
  }
  else
  {
    ANA_MSG_DEBUG("processClusters(): keeping clusters");
  }
  return;
}


uint32_t ZdcNtuple::acceptEvent()
{
  uint32_t passbits = 0;

  if (!m_isMC)
  {
    if (useGRL)
    {
      if (!m_grl->passRunLB(*m_eventInfo)) {
        passbits += 1; // UPC GRL
      }
    }

    /*
      if(!m_grl_mb->passRunLB(*m_eventInfo)){
      passbits += 4; // MB GRL
      }
    */

    if (   (m_eventInfo->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error )
           || (m_eventInfo->errorState(xAOD::EventInfo::Tile) == xAOD::EventInfo::Error )
           || (m_eventInfo->errorState(xAOD::EventInfo::SCT) == xAOD::EventInfo::Error )
           || (m_eventInfo->isEventFlagBitSet(xAOD::EventInfo::Core, 18) ) )
    {
      passbits += 2;
    } // end if event flags check
  } // end if the event is data

  return passbits;
}

void ZdcNtuple::setupTriggerHistos()
{
  if (!enableTrigger) return;

  m_outputTree = tree( "zdcTree" );
  ANA_MSG_INFO("setupTriggerHistos(): Setting up trigger histos and ntuple = " << m_outputTree);

  std::vector<std::string> triggers;
  std::vector<std::string> rerunTriggers;
  bool zdc_triggers = true;

  // ZDC triggers
  if (zdc_triggers)
  {
    if (zdcCalib)
      {
	if (lhcf2022)
	  {
	    triggers.push_back("HLT_noalg_ZDCPEB_L1LHCF");
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_OR");
	  }
	else
	  {
	    triggers.push_back("L1_ZDC_A");
	    triggers.push_back("L1_ZDC_C");
	    triggers.push_back("L1_ZDC_AND");
	    triggers.push_back("L1_ZDC_A_C");
	    triggers.push_back("L1_MBTS_2");
	  }
      }
    else
      {
	if (lhcf2022)
	  {
	    triggers.push_back("HLT_noalg_L1LHCF");
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_OR");	    
	    triggers.push_back("HLT_noalg_ZDCPEB_L1LHCF");
	    triggers.push_back("HLT_noalg_L1ZDC_OR");
	    triggers.push_back("HLT_noalg_L1ZDC_XOR_E2");	    
	    triggers.push_back("HLT_noalg_L1ZDC_XOR_E1_E3");	    
	    triggers.push_back("HLT_noalg_L1ZDC_A_AND_C");	    
	    triggers.push_back("HLT_mb_sptrk_L1ZDC_OR");
	    triggers.push_back("HLT_mb_sptrk_L1ZDC_XOR_E2");	    
	    triggers.push_back("HLT_mb_sptrk_L1ZDC_XOR_E1_E3");	    
	    triggers.push_back("HLT_mb_sptrk_L1ZDC_A_AND_C");	    
	    triggers.push_back("HLT_mb_sp100_trk30_hmt_L1ZDC_XOR_E2");
	    triggers.push_back("HLT_mb_sp100_trk30_hmt_L1ZDC_XOR_E1_E3");
	    triggers.push_back("HLT_mb_sp100_trk30_hmt_L1ZDC_A_AND_C");
	  }
      }
  }

  //char name[50];
  ANA_MSG_INFO("Adding trigger branches!");
  
  int ic = 0;
  for (auto &trig : triggers)
    {
      const Trig::ChainGroup* cg = m_trigDecisionTool->getChainGroup(trig);
      if (cg->getListOfTriggers().size())
	{
	  ANA_MSG_INFO("setupTriggerHistos(): Trigger found = " << trig.c_str() << " bit " << ic);
	}
      else
	{
	  ANA_MSG_INFO("setupTriggerHistos(): Trigger NOT found = " << trig.c_str()  << " bit " << ic);
	}
      m_chainGroups.push_back(cg);
      // force all triggers to show up in tree
      TString bname(trig.c_str());
      m_outputTree->Branch(bname, &(t_decisions[ic]), bname + "/O");
      m_outputTree->Branch("ps_" + bname, &(t_prescales[ic]), "ps_" + bname + "/F");
      ic++;
    }

  ATH_MSG_INFO( "triggers = " << triggers.size() << " chains = " << m_chainGroups.size() );

  int irc = 0;
  ANA_MSG_INFO("Adding rerun trigger branches!");
  for (auto &trig : rerunTriggers)
    {
      const Trig::ChainGroup* cg = m_trigDecisionTool->getChainGroup(trig);
      m_rerunChainGroups.push_back(cg);
      if (cg->getListOfTriggers().size())
	{
	  ANA_MSG_INFO("setupTriggerHistos(): Rerun trigger found = " << trig.c_str() << " bit " << irc);
	}
      else
	{
	  ANA_MSG_INFO("setupTriggerHistos(): Rerun trigger NOT found = " << trig.c_str()  << " bit " << irc);
	}
      // force all rerun triggers to show up in tree
      TString bname(trig.c_str());
      m_outputTree->Branch(bname, &(t_rerunDecisions[irc]), bname + "/O");
      irc++;
    }
  
  // trigger matching flags for electrons and muons
  
  m_setupTrigHist = true;

  ANA_MSG_INFO("setupTriggerHistos(): Finished setting up trigger");

}

// from TrigMuonMatching
double ZdcNtuple::dR(const double eta1, const double phi1, const double eta2, const double phi2)
{
  double deta = std::abs(eta1 - eta2);
  double dphi = std::abs(phi1 - phi2) < TMath::Pi() ? std::abs(phi1 - phi2) : 2 * TMath::Pi() - std::abs(phi1 - phi2);
  return std::sqrt(deta * deta + dphi * dphi);
}


StatusCode ZdcNtuple :: finalize ()
{
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.


  return StatusCode::SUCCESS;
}
