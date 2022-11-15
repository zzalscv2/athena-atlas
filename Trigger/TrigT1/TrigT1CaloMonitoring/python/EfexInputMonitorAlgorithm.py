#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
def EfexInputMonitoringConfig(inputFlags):
    '''Function to configure LVL1 EfexInput algorithm in the monitoring system.'''

    import math 
    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # any things that need setting up for job e.g.
    #from AtlasGeoModel.AtlasGeoModelConfig import AtlasGeometryCfg
    #result.merge(AtlasGeometryCfg(inputFlags))

    # make the athena monitoring helper
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'EfexInputMonitoringCfg')

    # get any algorithms
    EfexInputMonAlg = helper.addAlgorithm(CompFactory.EfexInputMonitorAlgorithm,'EfexInputMonAlg')

    # add any steering
    groupName = 'EfexInputMonitor' # the monitoring group name is also used for the package name
    EfexInputMonAlg.PackageName = groupName

    mainDir = 'L1Calo'
    trigPath = 'EfexInput/'

    # See if the file contains xTOBs else use TOBs
    #hasXtobs = True if "L1_eFexTower" in inputFlags.Input.Collections else False
    #if not hasXtobs:
    #    EfexInputMonAlg.eFexTowerContainer = "L1_eEMRoI"

    #tobStr = "TOB"

    # add monitoring algorithm to group, with group name and main directory 
    myGroup = helper.addGroup(EfexInputMonAlg, groupName , mainDir)
    EmErrorGroup = helper.addGroup(EfexInputMonAlg, groupName+"_EmError", mainDir)
    HadErrorGroup = helper.addGroup(EfexInputMonAlg, groupName+"_HadError", mainDir)

    # histograms of eFex tower variables
    myGroup.defineHistogram('NEfexTowers;h_nEfexTowers', title='Number of eFex towers',
                            type='TH1I', path=trigPath, xbins=500,xmin=0,xmax=5000)

    myGroup.defineHistogram('TowerEta;h_TowerEta', title='eFex Tower Eta',
                            type='TH1F', path=trigPath, xbins=100,xmin=-3.0,xmax=3.0)

    myGroup.defineHistogram('TowerPhi;h_TowerPhi', title='eFex Tower Phi',
                            type='TH1F', path=trigPath, xbins=64,xmin=-math.pi,xmax=math.pi)

    myGroup.defineHistogram('TowerEta,TowerPhi;h_TowerEtaPhiMap', title='eFex Tower Eta vs Phi',
                            type='TH2F',path=trigPath, xbins=50,xmin=-3.0,xmax=3.0,ybins=64,ymin=-math.pi,ymax=math.pi)

    myGroup.defineHistogram('TowerEtcount1;h_TowerEtcount1', title='eFex Tower Et Count1 (pre-sampler)',
                            type='TH1F', path=trigPath, xbins=100,xmin=0,xmax=100.0)

    EmErrorGroup.defineHistogram('TowerEtcount1;h_TowerEtcount1_Error', title='eFex Tower Et Count1 (pre-sampler) Error',
                                 type='TH1F', path=trigPath+'Error', xbins=100,xmin=0,xmax=100.0)

    myGroup.defineHistogram('TowerEtcount2;h_TowerEtcount2', title='eFex Tower Et Count2 (Front layer)',
                            type='TH1F', path=trigPath, xbins=100,xmin=0,xmax=100.0)

    EmErrorGroup.defineHistogram('TowerEtcount2;h_TowerEtcount2_Error', title='eFex Tower Et Count2 (Front layer) Error',
                            type='TH1F', path=trigPath+'Error', xbins=100,xmin=0,xmax=100.0)

    myGroup.defineHistogram('TowerEtcount3;h_TowerEtcount3', title='eFex Tower Et Count3 (Front layer)',
                            type='TH1F', path=trigPath, xbins=100,xmin=0,xmax=100.0)

    EmErrorGroup.defineHistogram('TowerEtcount3;h_TowerEtcount3_Error', title='eFex Tower Et Count3 (Front layer) Error',
                            type='TH1F', path=trigPath+'Error', xbins=100,xmin=0,xmax=100.0)

    myGroup.defineHistogram('TowerEtcount4;h_TowerEtcount4', title='eFex Tower Et Count4 (Front layer)',
                            type='TH1F', path=trigPath, xbins=100,xmin=0,xmax=100.0)

    EmErrorGroup.defineHistogram('TowerEtcount4;h_TowerEtcount4_Error', title='eFex Tower Et Count4 (Front layer) Error',
                            type='TH1F', path=trigPath+'Error', xbins=100,xmin=0,xmax=100.0)

    myGroup.defineHistogram('TowerEtcount5;h_TowerEtcount5', title='eFex Tower Et Count5 (Front layer)',
                            type='TH1F', path=trigPath, xbins=100,xmin=0,xmax=100.0)

    EmErrorGroup.defineHistogram('TowerEtcount5;h_TowerEtcount5_Error', title='eFex Tower Et Count5 (Front layer) Error',
                            type='TH1F', path=trigPath+'Error', xbins=100,xmin=0,xmax=100.0)

    myGroup.defineHistogram('TowerEtcount6;h_TowerEtcount6', title='eFex Tower Et Count6 (Middle layer)',
                            type='TH1F', path=trigPath, xbins=100,xmin=0,xmax=100.0)

    EmErrorGroup.defineHistogram('TowerEtcount6;h_TowerEtcount6_Error', title='eFex Tower Et Count6 (Middle layer) Error',
                            type='TH1F', path=trigPath+'Error', xbins=100,xmin=0,xmax=100.0)

    myGroup.defineHistogram('TowerEtcount7;h_TowerEtcount7', title='eFex Tower Et Count7 (Middle layer)',
                            type='TH1F', path=trigPath, xbins=100,xmin=0,xmax=100.0)

    EmErrorGroup.defineHistogram('TowerEtcount7;h_TowerEtcount7_Error', title='eFex Tower Et Count7 (Middle layer) Error',
                            type='TH1F', path=trigPath+'Error', xbins=100,xmin=0,xmax=100.0)

    myGroup.defineHistogram('TowerEtcount8;h_TowerEtcount8', title='eFex Tower Et Count8 (Middle layer)',
                            type='TH1F', path=trigPath, xbins=100,xmin=0,xmax=100.0)

    EmErrorGroup.defineHistogram('TowerEtcount8;h_TowerEtcount8_Error', title='eFex Tower Et Count8 (Middle layer) Error',
                            type='TH1F', path=trigPath+'Error', xbins=100,xmin=0,xmax=100.0)

    myGroup.defineHistogram('TowerEtcount9;h_TowerEtcount9', title='eFex Tower Et Count9 (Middle layer)',
                            type='TH1F', path=trigPath, xbins=100,xmin=0,xmax=100.0)

    EmErrorGroup.defineHistogram('TowerEtcount9;h_TowerEtcount9_Error', title='eFex Tower Et Count9 (Middle layer) Error',
                            type='TH1F', path=trigPath+'Error', xbins=100,xmin=0,xmax=100.0)

    myGroup.defineHistogram('TowerEtcount10;h_TowerEtcount10', title='eFex Tower Et Count10 (Back layer)',
                            type='TH1F', path=trigPath, xbins=100,xmin=0,xmax=100.0)

    EmErrorGroup.defineHistogram('TowerEtcount10;h_TowerEtcount10_Error', title='eFex Tower Et Count10 (Back layer) Error',
                            type='TH1F', path=trigPath+'Error', xbins=100,xmin=0,xmax=100.0)

    myGroup.defineHistogram('TowerEtcount11;h_TowerEtcount11', title='eFex Tower Et Count11 (Hadronic layer)',
                            type='TH1F', path=trigPath, xbins=100,xmin=0,xmax=100.0)

    HadErrorGroup.defineHistogram('TowerEtcount11;h_TowerEtcount11_Error', title='eFex Tower Et Count11 (Hadronic layer) Error',
                            type='TH1F', path=trigPath+'Error', xbins=100,xmin=0,xmax=100.0)

    myGroup.defineHistogram('TowerModule;h_TowerModule', title='eFex Tower Module Number',
                            type='TH1F', path=trigPath, xbins=24,xmin=0,xmax=24.0)

    myGroup.defineHistogram('TowerFpga;h_TowerFpga', title='eFex Tower FPGA Number',
                            type='TH1F', path=trigPath, xbins=4,xmin=0,xmax=4.0)

    myGroup.defineHistogram('TowerEmstatus;h_TowerEmstatus', title='eFex tower EM status bit',
                            type='TH1F', path=trigPath, xbins=10,xmin=0,xmax=2400.0)

    myGroup.defineHistogram('TowerHadstatus;h_TowerHadstatus', title='eFex Tower Hadronic status bit',
                            type='TH1F', path=trigPath, xbins=10,xmin=0,xmax=2400.0)
    
    acc = helper.result()
    result.merge(acc)
    return result


if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    import glob

    inputs = glob.glob('/eos/atlas/atlastier0/rucio/data18_13TeV/physics_Main/00354311/data18_13TeV.00354311.physics_Main.recon.ESD.f1129/data18_13TeV.00354311.physics_Main.recon.ESD.f1129._lb0013._SFO-8._0001.1')

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

    EfexInputMonitorCfg = EfexInputMonitoringConfig(ConfigFlags)
    cfg.merge(EfexInputMonitorCfg)

    # options - print all details of algorithms, very short summary 
    cfg.printConfig(withDetails=False, summariseProps = True)

    nevents=10
    cfg.run(nevents)
