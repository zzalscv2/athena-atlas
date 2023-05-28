#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def DQTGlobalWZFinderAlgConfig(flags):
	from AthenaMonitoring import AthMonitorCfgHelper
	from AthenaConfiguration.ComponentFactory import CompFactory
	from AthenaConfiguration.Enums import LHCPeriod
	helper = AthMonitorCfgHelper(flags, 'DQTGlobalWZFinderAlgCfg')

	algConfObj = CompFactory.DQTGlobalWZFinderAlg
	muonSelectionTool = CompFactory.CP.MuonSelectionTool("DQTMuonSelectionTool")
	r3MatchingTool = CompFactory.Trig.R3MatchingTool("R3MatchingTool")
	truthClassifier = CompFactory.MCTruthClassifier("MCTruthClassifier")

	# Configure MuonSelectionTool
	muonSelectionTool.MuQuality=1
	muonSelectionTool.MaxEta=2.4
	muonSelectionTool.IsRun3Geo=(flags.GeoModel.Run == LHCPeriod.Run3)
	muonSelectionTool.TurnOffMomCorr=True

	monAlg = helper.addAlgorithm(algConfObj, 'DQTGlobalWZFinderAlg',
					MuonSelectionTool = muonSelectionTool,
					R3MatchingTool = r3MatchingTool,
					MCTruthClassifier = truthClassifier)

	monAlg.doTrigger = flags.DQ.useTrigger

	monAlg.electronEtCut = 27
	monAlg.muonPtCut = 27
	monAlg.zCutLow = 66.0
	monAlg.zCutHigh = 116.0
	monAlg.muonMaxEta = muonSelectionTool.MaxEta
	monAlg.Z_ee_trigger = ["HLT_e26_lhtight_ivarloose_L1eEM26M", "HLT_e28_lhtight_ivarloose_L1eEM28M", "HLT_e60_lhmedium_L1eEM26M", "HLT_e140_lhloose_L1eEM26M", "HLT_e300_etcut_L1eEM26M", "HLT_e60_lhmedium_L1eEM28M", "HLT_e140_lhloose_L1eEM28M", "HLT_e300_etcut_L1eEM28M"]
	monAlg.Z_mm_trigger = ["HLT_mu24_ivarmedium_L1MU14FCH", "HLT_mu50_L1MU14FCH", "HLT_mu24_ivarmedium_L1MU18VFCH", "HLT_mu50_L1MU18VFCH"]



	# arguments are: algorithm, name of group used to access it from the alg,
	# the 'top level path' to put outputs in, and the default duration of
	# associated histograms

	maxLB = 3200

	optMC = ''
	optData = 'kAlwaysCreate'

	if flags.Input.isMC:
		optMC = 'kAlwaysCreate'
	
	group = helper.addGroup(monAlg, 'default', 'GLOBAL/DQTGlobalWZFinder', 'run')
	
	elegroup = helper.addGroup(monAlg, 'electron', 'GLOBAL/DQTGlobalWZFinder', 'run')
	muongroup = helper.addGroup(monAlg, 'muon', 'GLOBAL/DQTGlobalWZFinder', 'run')
	
	group_Zee = helper.addGroup(monAlg, 'Zee', 'GLOBAL/DQTGlobalWZFinder', 'run')
	group_Zmumu = helper.addGroup(monAlg, 'Zmumu', 'GLOBAL/DQTGlobalWZFinder', 'run')

	group_EleTP = helper.addGroup(monAlg, 'EleTP', 'GLOBAL/DQTGlobalWZFinder', 'run')
	group_EleContainerTP = helper.addGroup(monAlg, 'EleContainerTP', 'GLOBAL/DQTGlobalWZFinder', 'run')
	group_EleTrigTP = helper.addGroup(monAlg, 'EleTrigTP', 'GLOBAL/DQTGlobalWZFinder', 'run')

	group_MuonTruthEff = helper.addGroup(monAlg, 'MuonTruthEff', 'GLOBAL/DQTGlobalWZFinder', 'run')
	group_MuonTriggerTP = helper.addGroup(monAlg, 'MuonTriggerTP', 'GLOBAL/DQTGlobalWZFinder', 'run')
	group_MuonLooseTP = helper.addGroup(monAlg, 'MuonLooseTP', 'GLOBAL/DQTGlobalWZFinder', 'run')
	group_MuonInDetTP = helper.addGroup(monAlg, 'MuonInDetTP', 'GLOBAL/DQTGlobalWZFinder', 'run')

	group.defineHistogram('LB,avgLiveFrac',
				title='Livetime',
				type='TProfile',
				xbins=maxLB,
				xmin=0.5,
				xmax=maxLB+0.5,
				duration='lb',
				opt='kAlwaysCreate',
				merge='merge'
	)


	group.defineHistogram('LB,duration',
				title='LB length',
				type='TProfile',
				xbins=maxLB,
				xmin=0.5,
				xmax=maxLB+0.5,
				duration='lb',
				merge='merge'
	)

	
	group.defineHistogram('LB,avgIntPerXing',
				title='#mu',
				type='TProfile',
				xbins=maxLB,
				xmin=0.5,
				xmax=maxLB+0.5,
				duration='lb',
				merge='merge'
	)
	
	elegroup.defineHistogram('ele_Et',
				title='Selected Electron E_{T}',
				xbins=50,
				xmin=0,
				xmax=200,
				weight='evtWeight'
	)

	elegroup.defineHistogram('ele_Eta',
				title='Selected Electron #eta',
				xbins=50,
				xmin=-2.5,
				xmax=2.5,
				weight='evtWeight'
	)
	
	elegroup.defineHistogram('ele_Phi',
				title='Selected Electron #phi',
				xbins=34,
				xmin=-3.4,
				xmax=3.4,
				weight='evtWeight'
	)
	
	muongroup.defineHistogram('muon_Pt',
				title='Selected Muon p_{T}',
				xbins=200,
				xmin=0,
				xmax=200,
				weight='evtWeight'
	)

	muongroup.defineHistogram('muon_Eta',
				title='Selected Muon #eta',
				xbins=50,
				xmin=-2.5,
				xmax=2.5,
				weight='evtWeight'
	)

	muongroup.defineHistogram('muon_Phi',
				title='Selected Muon #phi',
				xbins=34,
				xmin=-3.4,
				xmax=3.4,
				weight='evtWeight'
	)
	
	m_zCutLow = 66000.0
	m_zCutHigh = 116000.0	
	nzbins = int( m_zCutHigh*0.001 - m_zCutLow*0.001 )

	m_zCutLow_elTP = 66000.0
	m_zCutHigh_elTP = 250000.0
	nzbins_elTP = int( m_zCutHigh_elTP*0.001 - m_zCutLow_elTP*0.001 )

	group_Zee.defineHistogram('Zeecharge;m_Z_Q_ele',
				title='Z#rightarrowee Charge',
				xbins=7,
				xmin=-3,
				xmax=3,
				weight='evtWeight',
				opt='kAlwaysCreate'
	)
	
	group_Zee.defineHistogram('mass;m_Z_mass_opsele',
				title='Z#rightarrowee (op. sign) Mass',
				xbins=nzbins,
				xmin=m_zCutLow,
				xmax=m_zCutHigh,
				weight='evtWeight',
				cutmask='osel',
				duration='lb',
				opt='kAlwaysCreate'
	)

	group_Zee.defineHistogram('LB;m_Z_Counter_el_os',
				title='Z#rightarrowee count per Lumi Block',
				xbins=maxLB,
				xmin=0.5,
				xmax=maxLB+0.5,
				weight='evtWeight',
				cutmask='osel',
				duration='lb',
				opt='kAlwaysCreate',
				merge='merge'
	)

	group_Zee.defineHistogram('LB;m_Z_Counter_el_ss',
				title='Z#rightarrowee count per Lumi Block',
				xbins=maxLB,
				xmin=0.5,
				xmax=maxLB+0.5,
				weight='evtWeight',
				cutmask='ssel',
				duration='lb',
				opt='kAlwaysCreate',
				merge='merge'
	)

	group_Zee.defineHistogram('mass;m_Z_mass_ssele',
				title='Z#rightarrowee (same sign) Mass',
				xbins=nzbins,
				xmin=m_zCutLow,
				xmax=m_zCutHigh,
				weight='evtWeight',
				cutmask='ssel',
				duration='lb',
				opt='kAlwaysCreate'
	)

	group_Zee.defineHistogram('pT1;m_leadingele_pt',
				title='Leading e p_{T}',
				xbins=200,
				xmin=0,
				xmax=200000,
				weight='evtWeight',
				cutmask='osel',
				opt=optData
	)
	
	group_Zee.defineHistogram('pT2;m_subleadingele_pt',
				title='Subleading e p_{T}',
				xbins=200,
				xmin=0,
				xmax=200000,
				weight='evtWeight',
				cutmask='osel',
				opt=optData
	)

	group_Zee.defineHistogram('eta1;m_leadingele_eta',
				title='Leading e #eta',
				xbins=50,
				xmin=-2.5,
				xmax=2.5,
				weight='evtWeight',
				cutmask='osel',
				opt=optData
	)

	group_Zee.defineHistogram('eta2;m_subleadingele_eta',
				title='Subleading e #eta',
				xbins=50,
				xmin=-2.5,
				xmax=2.5,
				weight='evtWeight',
				cutmask='osel',
				opt=optData
	)

	group_Zee.defineHistogram('phi1;m_leadingele_phi',
				title='Leading e #phi',
				xbins=34,
				xmin=-3.4,
				xmax=3.4,
				weight='evtWeight',
				cutmask='osel',
				opt=optData
	)

	group_Zee.defineHistogram('phi2;m_subleadingele_phi',
				title='Subleading e #phi',
				xbins=34,
				xmin=-3.4,
				xmax=3.4,
				weight='evtWeight',
				cutmask='osel',
				opt=optData
	)

	group_Zmumu.defineHistogram('Zmumucharge;m_Z_Q_mu',
				title='Z#rightarrow#mu#mu Charge',
				xbins=7,
				xmin=-3,
				xmax=3,
				weight='evtWeight',
				opt='kAlwaysCreate'
	)

	group_Zmumu.defineHistogram('mass;m_Z_mass_opsmu',
				title='Z#rightarrow#mu#mu (op. sign) Mass',
				xbins=nzbins,
				xmin=m_zCutLow,
				xmax=m_zCutHigh,
				weight='evtWeight',
				cutmask='osmu',
				duration='lb',
				opt='kAlwaysCreate'
	)

	group_Zmumu.defineHistogram('LB;m_Z_Counter_mu',
				title='Z#rightarrow#mu#mu count per Lumi Block',
				xbins=maxLB,
				xmin=0.5,
				xmax=maxLB+0.5,
				weight='evtWeight',
				cutmask='osmu',
				duration='lb',
				opt='kAlwaysCreate',
				merge='merge'
	)

	group_Zmumu.defineHistogram('mass;m_Z_mass_ssmu',
				title='Z#rightarrow#mu#mu (same sign) Mass',
				xbins=nzbins,
				xmin=m_zCutLow,
				xmax=m_zCutHigh,
				weight='evtWeight',
				cutmask='ssmu',
				duration='lb',
				opt='kAlwaysCreate'
	)

	group_Zmumu.defineHistogram('pT1;m_leadingmu_pt',
				title='Leading #mu p_{T}',
				xbins=200,
				xmin=0,
				xmax=200000,
				weight='evtWeight',
				cutmask='osmu',
				opt=optData
	)
	
	group_Zmumu.defineHistogram('pT2;m_subleadingmu_pt',
				title='Subleading #mu p_{T}',
				xbins=200,
				xmin=0,
				xmax=200000,
				weight='evtWeight',
				cutmask='osmu',
				opt=optData
	)

	group_Zmumu.defineHistogram('eta1;m_leadingmu_eta',
				title='Leading #mu #eta',
				xbins=50,
				xmin=-2.5,
				xmax=2.5,
				weight='evtWeight',
				cutmask='osmu',
				opt=optData
	)

	group_Zmumu.defineHistogram('eta2;m_subleadingmu_eta',
				title='Subleading #mu #eta',
				xbins=50,
				xmin=-2.5,
				xmax=2.5,
				weight='evtWeight',
				cutmask='osmu',
				opt=optData
	)

	group_Zmumu.defineHistogram('phi1;m_leadingmu_phi',
				title='Leading #mu #phi',
				xbins=34,
				xmin=-3.4,
				xmax=3.4,
				weight='evtWeight',
				cutmask='osmu',
				opt=optData
	)

	group_Zmumu.defineHistogram('phi2;m_subleadingmu_phi',
				title='Subleading #mu #phi',
				xbins=34,
				xmin=-3.4,
				xmax=3.4,
				weight='evtWeight',
				cutmask='osmu',
				opt=optData
	)

	group_Zmumu.defineTree('pT1, pT2, phi1, phi2, eta1, eta2, mass, evtWeight, isTruth, LB, eventNumber, runNumber;muontree',
				title='muontree',
				treedef='pT1/F:pT2/F:phi1/F:phi2/F:eta1/F:eta2/F:mass/F:evtWeight/F:isTruth/O:LB/I:eventNumber/l:runNumber/I',
				cutmask='writeTTrees',
				opt=optMC
	)

	group_EleTP.defineHistogram('mass;m_ele_tight_bad_os',
				title='',
				xbins=nzbins_elTP,
				xmin=m_zCutLow_elTP,
				xmax=m_zCutHigh_elTP,
				cutmask='bad_os',
				duration='lb',
				opt=optData
	)

	group_EleTP.defineHistogram('mass;m_ele_tight_bad_ss',
				title='',
				xbins=nzbins_elTP,
				xmin=m_zCutLow_elTP,
				xmax=m_zCutHigh_elTP,
				cutmask='bad_ss',
				duration='lb',
				opt=optData
	)

	group_EleTP.defineHistogram('mass;m_ele_tight_good_os',
				title='',
				xbins=nzbins_elTP,
				xmin=m_zCutLow_elTP,
				xmax=m_zCutHigh_elTP,
				cutmask='good_os',
				duration='lb',
				opt=optData
	)

	group_EleTP.defineHistogram('mass;m_ele_tight_good_ss',
				title='',
				xbins=nzbins_elTP,
				xmin=m_zCutLow_elTP,
				xmax=m_zCutHigh_elTP,
				cutmask='good_ss',
				duration='lb',
				opt=optData
	)

	group_EleTP.defineHistogram('mass;m_ele_template_os',
				title='',
				xbins=nzbins_elTP,
				xmin=m_zCutLow_elTP,
				xmax=m_zCutHigh_elTP,
				cutmask='template_os',
				duration='lb',
				opt=optData
	)

	group_EleTP.defineHistogram('mass;m_ele_template_ss',
				title='',
				xbins=nzbins_elTP,
				xmin=m_zCutLow_elTP,
				xmax=m_zCutHigh_elTP,
				cutmask='template_ss',
				duration='lb',
				opt=optData
	)
	
	group_EleContainerTP.defineHistogram('mass;m_elContainertp_nomatch',
				title='',
				xbins=nzbins_elTP,
				xmin=m_zCutLow_elTP,
				xmax=m_zCutHigh_elTP,
				cutmask='container_nomatch',
				duration='lb',
				opt=optData
	)

	group_EleContainerTP.defineHistogram('mass;m_ele_tight_passkine',
				title='',
				xbins=nzbins_elTP,
				xmin=m_zCutLow_elTP,
				xmax=m_zCutHigh_elTP,
				cutmask='pass_kine',
				duration='lb',
				opt=optData
	)

	group_EleTP.defineTree('pT, phi, eta, mass, isTruth, runNumber, LB, eventNumber, mtype, weight;electron_reco_tptree',
				title='electron_reco_tp_tree',
				treedef='pT/F:phi/F:eta/F:mass/F:runNumber/I:LB/I:eventNumber/l:mtype/I:weight/F',
				opt=optMC
	)

	group_EleTrigTP.defineHistogram('matched;m_eltrigtp_matches_os',
				title='Electron trigger TP stats',
				xbins=3,
				xmin=-0.5,
				xmax=2.5,
				weight='weight',
				cutmask='os',
				duration='lb',
				opt=optData
	)
				
	group_EleTrigTP.defineHistogram('matched;m_eltrigtp_matches_ss',
				title='Electron trigger TP stats',
				xbins=3,
				xmin=-0.5,
				xmax=2.5,
				weight='weight',
				cutmask='ss',
				duration='lb',
				opt=optData
	)

	group_EleTrigTP.defineTree('pT, phi, eta, mass, runNumber, LB, eventNumber, mtype, weight;electron_trig_tptree',
				title='electron_trig_tptree',
				treedef='pT/F:phi/F:eta/F:mass/F:runNumber/I:LB/I:eventNumber/l:mtype/I:weight/F',
				opt=optMC
	)

	group_MuonTruthEff.defineHistogram('match;mcmatch',
				title='Muon matching to truth in acceptance',
				xbins=2,
				xmin=-0.5,
				xmax=1.5,
				duration='lb',
				opt=optMC	
	)

	group_MuonTriggerTP.defineHistogram('matched;m_mutrigtp_matches',
				title='Muon Trigger TP stats',
				xbins=3,
				xmin=-0.5,
				xmax=2.5,
				weight='weight',
				duration='lb',
				opt=optData
	)

	group_MuonTriggerTP.defineTree('matched, isOS;m_bcid_mu_trigtp',
				title='m_bcid_mu_trigtp',
				treedef='nTrig_matches/I:isOS/O',
				cutmask='do_BCID',
				opt=optMC
	)

	group_MuonTriggerTP.defineTree('pT, eta, phi, mass, isTruth, runNumber, LB, eventNumber, mtype, weight;muon_trig_tptree',
				title='muon_trig_tptree',
				treedef='pT/F:phi/F:eta/F:mass/F:isTruth/O:runNumber/I:LB/I:eventNumber/l:mtype/I:weight/F',
				opt=optMC
	)

	group_MuonLooseTP.defineHistogram('mass;m_muloosetp_match_os',
				title='Muon loose TP match OS',
				xbins=nzbins,
				xmin=m_zCutLow,
				xmax=m_zCutHigh,
				cutmask='osmatch',
				duration='lb',
				opt=optData
	)

	group_MuonLooseTP.defineHistogram('mass;m_muloosetp_match_ss',
				title='Muon loose TP match SS',
				xbins=nzbins,
				xmin=m_zCutLow,
				xmax=m_zCutHigh,
				cutmask='ssmatch',
				duration='lb',
				opt=optData
	)

	group_MuonLooseTP.defineHistogram('mass;m_muloosetp_nomatch_os',
				title='Muon loose TP nomatch OS',
				xbins=nzbins,
				xmin=m_zCutLow,
				xmax=m_zCutHigh,
				cutmask='osnomatch',
				duration='lb',
				opt=optData
	)

	group_MuonLooseTP.defineHistogram('mass;m_muloosetp_nomatch_ss',
				title='Muon loose TP nomatch SS',
				xbins=nzbins,
				xmin=m_zCutLow,
				xmax=m_zCutHigh,
				cutmask='ssnomatch',
				duration='lb',
				opt=optData
	)

	group_MuonLooseTP.defineTree('pT, phi, eta, mass, isTruth, runNumber, LB, eventNumber, mtype, weight;muon_reco_tptree',
				title='muon_reco_tptree',
				treedef='pT/F:phi/F:eta/F:mass/F:isTruth/O:runNumber/I:LB/I:eventNumber/l:mtype/I:weight/F',
				opt=optMC
	)

	group_MuonInDetTP.defineHistogram('mass;m_mu_InDet_tp_match_os',
				title='Muon inner detector TP match OS',
				xbins=nzbins,
				xmin=m_zCutLow,
				xmax=m_zCutHigh,
				cutmask='osmatch',
				duration='lb',
				opt=optData
	)

	group_MuonInDetTP.defineHistogram('mass;m_mu_InDet_tp_match_ss',
				title='Muon inner detector TP match SS',
				xbins=nzbins,
				xmin=m_zCutLow,
				xmax=m_zCutHigh,
				cutmask='ssmatch',
				duration='lb',
				opt=optData
	)

	group_MuonInDetTP.defineHistogram('mass;m_mu_InDet_tp_nomatch_os',
				title='Muon inner detector TP nomatch OS',
				xbins=nzbins,
				xmin=m_zCutLow,
				xmax=m_zCutHigh,
				cutmask='osnomatch',
				duration='lb',
				opt=optData
	)

	group_MuonInDetTP.defineHistogram('mass;m_mu_InDet_tp_nomatch_ss',
				title='Muon inner detector TP nomatch SS',
				xbins=nzbins,
				xmin=m_zCutLow,
				xmax=m_zCutHigh,
				cutmask='ssnomatch',
				duration='lb',
				opt=optData
	)

	group_MuonInDetTP.defineTree('pT, eta, phi, mass, isTruth, runNumber, LB, eventNumber, mtype, weight;muon_indet_tptree',
				title='muon_indet_tptree',
				treedef='pT/F:phi/F:eta/F:mass/F:isTruth/O:runNumber/I:LB/I:eventNumber/l:mtype/I:weight/F',
				opt=optMC
	)

	result = helper.result()
	return result

