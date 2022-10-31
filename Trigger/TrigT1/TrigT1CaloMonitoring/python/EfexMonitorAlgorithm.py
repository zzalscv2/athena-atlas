#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
def EfexMonitoringConfig(inputFlags):
    '''Function to configure LVL1 Efex algorithm in the monitoring system.'''

    import math

    # get the component factory - used for merging the algorithm results
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    
    # uncomment if you want to see all the flags
    #inputFlags.dump() # print all the configs

    # make the athena monitoring helper
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'EfexMonitoringCfg')

    # get any algorithms
    EfexMonAlg = helper.addAlgorithm(CompFactory.EfexMonitorAlgorithm,'EfexMonAlg')

    # add any steering
    groupName = 'EfexMonitor' # the monitoring group name is also used for the package name
    EfexMonAlg.PackageName = groupName
    EfexMonAlg.LowPtCut = 0.0
    EfexMonAlg.HiPtCut = 15000.0
    cut_names = ["LowPtCut", "HiPtCut"]
    cut_vals = [EfexMonAlg.LowPtCut, EfexMonAlg.HiPtCut]

    mainDir = 'L1Calo'
    trigPath = 'Efex/'

    # See if the file contains xTOBs else use TOBs
    hasXtobs = True if "L1_eEMxRoI" in inputFlags.Input.Collections else False
    if not hasXtobs:
        EfexMonAlg.eFexEMRoIContainer = "L1_eEMRoI"
        EfexMonAlg.eFexTauRoIContainer = "L1_eTauRoI"

    tobStr = "xTOB" if hasXtobs else "TOB"

    # add monitoring algorithm to group, with group name and main directory 
    lowPtCutGroup = helper.addGroup(EfexMonAlg, groupName+'_LowPtCut' , mainDir)
    hiPtCutGroup = helper.addGroup(EfexMonAlg, groupName+'_HiPtCut' , mainDir)
    groups = [lowPtCutGroup, hiPtCutGroup]

    for myGroup, cut_name, cut_val in zip(groups, cut_names, cut_vals):
        cut_title_addition = '' if (cut_val == 0.0) else ' (Et>' + '%.1f'%(cut_val/1000) + 'GeV cut)'

        # histograms of eEM variables
        myGroup.defineHistogram('nEMTOBs_nocut;h_nEmTOBs_nocut', title='Number of eFex EM '+tobStr+'s'+cut_title_addition+';EM '+tobStr+'s;Number of EM '+tobStr+'s',
                                type='TH1I', path=trigPath+'eEM'+cut_name+'/', xbins=10,xmin=0,xmax=10)

        myGroup.defineHistogram('nEMTOBs;h_nEmTOBs', title='Number of eFex EM '+tobStr+'s'+cut_title_addition+';EM '+tobStr+'s;Number of EM '+tobStr+'s',
                                type='TH1I', path=trigPath+'eEM'+cut_name+'/', xbins=10,xmin=0,xmax=10)

        myGroup.defineHistogram('TOBTransverseEnergy;h_TOBTransverseEnergy', title='eFex '+tobStr+' EM Transverse Energy [MeV]'+cut_title_addition,
                                type='TH1F', path=trigPath+'eEM'+cut_name+'/', xbins=100,xmin=0,xmax=50000)

        myGroup.defineHistogram('TOBEta;h_TOBEta', title='eFex '+tobStr+' EM Eta'+cut_title_addition,
                                type='TH1F', path=trigPath+'eEM'+cut_name+'/', xbins=50,xmin=-3.0,xmax=3.0)

        myGroup.defineHistogram('TOBPhi;h_TOBPhi', title='eFex '+tobStr+' EM Phi'+cut_title_addition,
                                type='TH1F', path=trigPath+'eEM'+cut_name+'/', xbins=64,xmin=-math.pi,xmax=math.pi)

        myGroup.defineHistogram('TOBEta,TOBPhi;h_TOBEtaPhiMap', title='eFex '+tobStr+' EM Eta vs Phi'+cut_title_addition+';'+tobStr+' EM Eta;'+tobStr+' EM Phi',
                                type='TH2F',path=trigPath+'eEM'+cut_name+'/', xbins=50,xmin=-3.0,xmax=3.0,ybins=64,ymin=-math.pi,ymax=math.pi)

        myGroup.defineHistogram('TOBshelfNumber;h_TOBshelfNumber', title='eFex '+tobStr+' EM Shelf Number'+cut_title_addition,
                                type='TH1F', path=trigPath+'eEM'+cut_name+'/', xbins=2,xmin=0,xmax=2)

        myGroup.defineHistogram('TOBeFEXNumberSh0;h_TOBeFEXNumberShelf0', title='eFex '+tobStr+' EM Module Number Shelf 0'+cut_title_addition,
                                type='TH1F', path=trigPath+'eEM'+cut_name+'/', xbins=12,xmin=0,xmax=12)

        myGroup.defineHistogram('TOBeFEXNumberSh1;h_TOBeFEXNumberShelf1', title='eFex '+tobStr+' EM Module Number Shelf 1'+cut_title_addition,
                                type='TH1F', path=trigPath+'eEM'+cut_name+'/', xbins=12,xmin=0,xmax=12)

        myGroup.defineHistogram('TOBfpga;h_TOBfpga', title='eFex '+tobStr+' EM FPGA'+cut_title_addition,
                                type='TH1F', path=trigPath+'eEM'+cut_name+'/', xbins=4,xmin=0,xmax=4)

        myGroup.defineHistogram('TOBReta;h_TOBReta', title='eFex '+tobStr+' EM Reta'+cut_title_addition,
                                type='TH1F', path=trigPath+'eEM'+cut_name+'/',xbins=250,xmin=0,xmax=1)

        myGroup.defineHistogram('TOBRhad;h_TOBRhad', title='eFex '+tobStr+' EM Rhad'+cut_title_addition,
                                type='TH1F', path=trigPath+'eEM'+cut_name+'/', xbins=250,xmin=0,xmax=1) 

        myGroup.defineHistogram('TOBWstot;h_TOBWstot', title='eFex '+tobStr+' EM Wstot'+cut_title_addition,
                                type='TH1F', path=trigPath+'eEM'+cut_name+'/', xbins=250,xmin=0,xmax=1) 

        threshold_labels = ['fail','loose','medium','tight']
        myGroup.defineHistogram('TOBReta_threshold;h_TOBReta_threshold', title='eFex '+tobStr+' EM Reta threshold'+cut_title_addition,
                                type='TH1F', path=trigPath+'eEM'+cut_name+'/',xbins=4,xmin=0,xmax=4.0,xlabels=threshold_labels)

        myGroup.defineHistogram('TOBRhad_threshold;h_TOBRhad_threshold', title='eFex '+tobStr+' EM Rhad threshold'+cut_title_addition,
                                type='TH1F', path=trigPath+'eEM'+cut_name+'/', xbins=4,xmin=0,xmax=4.0,xlabels=threshold_labels)

        myGroup.defineHistogram('TOBWstot_threshold;h_TOBWstot_threshold', title='eFex '+tobStr+' EM Wstot threshold'+cut_title_addition,
                                type='TH1F', path=trigPath+'eEM'+cut_name+'/', xbins=4,xmin=0,xmax=4.0,xlabels=threshold_labels)

        # plotting of eTau variables
        myGroup.defineHistogram('nTauTOBs_nocut;h_nTauTOBs_nocut', title='Number of eFex Tau '+tobStr+'s'+cut_title_addition+';Tau '+tobStr+'s;Number of Tau '+tobStr+'s',
                                type='TH1I', path=trigPath+'eTau'+cut_name+'/', xbins=10,xmin=0,xmax=10)
        
        myGroup.defineHistogram('nTauTOBs;h_nTauTOBs', title='Number of eFex Tau '+tobStr+'s'+cut_title_addition+';Tau '+tobStr+'s;Number of Tau '+tobStr+'s',
                                type='TH1I', path=trigPath+'eTau'+cut_name+'/', xbins=10,xmin=0,xmax=10)

        myGroup.defineHistogram('tauTOBTransverseEnergy;h_tauTOBTransverseEnergy', title='eFex '+tobStr+' Tau Transverse Energy [MeV]'+cut_title_addition,
                                type='TH1F', path=trigPath+'eTau'+cut_name+'/', xbins=100,xmin=0,xmax=50000)

        myGroup.defineHistogram('tauTOBEta;h_tauTOBEta', title='eFex '+tobStr+' Tau Eta'+cut_title_addition,
                                type='TH1F', path=trigPath+'eTau'+cut_name+'/', xbins=60,xmin=-3.0,xmax=3.0)

        myGroup.defineHistogram('tauTOBPhi;h_tauTOBPhi', title='eFex '+tobStr+' Tau Phi'+cut_title_addition,
                                type='TH1F', path=trigPath+'eTau'+cut_name+'/', xbins=100,xmin=-math.pi,xmax=math.pi)

        myGroup.defineHistogram('tauTOBEta,tauTOBPhi;h_tauTOBEtaPhiMap', title='eFex '+tobStr+' Tau Eta vs Phi'+cut_title_addition+';'+tobStr+' Tau Eta;'+tobStr+' Tau Phi',
                                type='TH2F',path=trigPath+'eTau'+cut_name+'/', xbins=50,xmin=-3.0,xmax=3.0,ybins=64,ymin=-math.pi,ymax=math.pi)

        myGroup.defineHistogram('tauTOBshelfNumber;h_tauTOBshelfNumber', title='eFex '+tobStr+' Tau Shelf Number'+cut_title_addition,
                                type='TH1F', path=trigPath+'eTau'+cut_name+'/', xbins=2,xmin=0,xmax=2)

        myGroup.defineHistogram('tauTOBeFEXNumberSh0;h_tauTOBeFEXNumberShelf0', title='eFex '+tobStr+' Tau Module Number Shelf 0'+cut_title_addition,
                                type='TH1F', path=trigPath+'eTau'+cut_name+'/', xbins=12,xmin=0,xmax=12)

        myGroup.defineHistogram('tauTOBeFEXNumberSh1;h_tauTOBeFEXNumberShelf1', title='eFex '+tobStr+' Tau Module Number Shelf 1'+cut_title_addition,
                                type='TH1F', path=trigPath+'eTau'+cut_name+'/', xbins=12,xmin=0,xmax=12)


        myGroup.defineHistogram('tauTOBfpga;h_tauTOBfpga', title='eFex '+tobStr+' Tau FPGA'+cut_title_addition,
                                type='TH1F', path=trigPath+'eTau'+cut_name+'/', xbins=4,xmin=0,xmax=4)

        myGroup.defineHistogram('tauTOBRcore;h_tauTOBRcore', title='eFex '+tobStr+' Tau rCore'+cut_title_addition,
                                type='TH1F', path=trigPath+'eTau'+cut_name+'/', xbins=250,xmin=0,xmax=1) 

        myGroup.defineHistogram('tauTOBRhad;h_tauTOBRhad', title='eFex '+tobStr+' Tau rHad'+cut_title_addition,
                                type='TH1F', path=trigPath+'eTau'+cut_name+'/', xbins=250,xmin=0,xmax=1) 

        myGroup.defineHistogram('tauTOBRcore_threshold;h_tauTOBRcore_threshold', title='eFex '+tobStr+' Tau rCore threshold'+cut_title_addition,
                                type='TH1F', path=trigPath+'eTau'+cut_name+'/', xbins=4,xmin=0,xmax=4.0, xlabels=threshold_labels)

        myGroup.defineHistogram('tauTOBRhad_threshold;h_tauTOBRhad_threshold', title='eFex '+tobStr+' Tau rHad threshold'+cut_title_addition,
                                type='TH1F', path=trigPath+'eTau'+cut_name+'/', xbins=4,xmin=0,xmax=4.0, xlabels=threshold_labels)

        myGroup.defineHistogram('tauTOBthree_threshold;h_tauTOBthree_threshold', title='eFex '+tobStr+' Tau 3 taus threshold'+cut_title_addition,
                                type='TH1F', path=trigPath+'eTau'+cut_name+'/', xbins=4,xmin=0,xmax=4.0, xlabels=threshold_labels)

    acc = helper.result()
    result.merge(acc)
    return result


if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    import glob

    # MCs processed adding L1_eEMRoI
    inputs = glob.glob('/eos/user/t/thompson/ATLAS/LVL1_mon/MC_ESD/l1calo.361024.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ4W.eFex_gFex_2022-01-13T2101.root')
    
    ConfigFlags.Input.Files = inputs
    ConfigFlags.Output.HISTFileName = 'ExampleMonitorOutput_LVL1_MC.root'

    ConfigFlags.lock()
    ConfigFlags.dump() # print all the configs

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg  
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(ConfigFlags)
    cfg.merge(PoolReadCfg(ConfigFlags))

    EfexMonitorCfg = EfexMonitoringConfig(ConfigFlags)
    cfg.merge(EfexMonitorCfg)

    # options - print all details of algorithms, very short summary 
    cfg.printConfig(withDetails=False, summariseProps = True)

    nevents=10
    cfg.run(nevents)

