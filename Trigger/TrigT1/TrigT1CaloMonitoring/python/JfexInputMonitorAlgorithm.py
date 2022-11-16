#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
def JfexInputMonitoringConfig(inputFlags):
    '''Function to configure LVL1 JfexInput algorithm in the monitoring system.'''

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
    helper = AthMonitorCfgHelper(inputFlags,'JfexInputMonitoringCfg')

    # get any algorithms
    JfexInputMonAlg = helper.addAlgorithm(CompFactory.JfexInputMonitorAlgorithm,'JfexInputMonAlg')

    # add any steering
    groupName = 'JfexInputMonitor' # the monitoring group name is also used for the package name
    JfexInputMonAlg.PackageName = groupName

    mainDir = 'L1Calo'
    trigPath = 'JfexInput/'
    Calosource_names = ["Barrel","Tile","EMEC","HEC","FCAL1","FCAL2","FCAL3"]
    # See if the file contains xTOBs else use TOBs
    #hasXtobs = True if "L1_eFexTower" in inputFlags.Input.Collections else False
    #if not hasXtobs:
    #    EfexInputMonAlg.eFexTowerContainer = "L1_eEMRoI"

    #tobStr = "TOB"

    # add monitoring algorithm to group, with group name and main directory 
    myGroup = helper.addGroup(JfexInputMonAlg, groupName , mainDir)

    # histograms of gFex tower variables
    myGroup.defineHistogram('NJfexTowers;h_nJfexTowers', title='Number of jFex towers',
                            type='TH1I', path=trigPath, xbins=2000,xmin=0,xmax=20000)

    myGroup.defineHistogram('TowerEta;h_TowerEta', title='jFex Tower Eta',
                            type='TH1F', path=trigPath, xbins=100,xmin=-5.0,xmax=5.0)

    myGroup.defineHistogram('TowerPhi;h_TowerPhi', title='jFex Tower Phi',
                            type='TH1F', path=trigPath, xbins=66,xmin=-math.pi,xmax=math.pi)

    myGroup.defineHistogram('TowerEta,TowerPhi;h_TowerEtaPhiMap', title='jFex Tower Eta vs Phi',
                            type='TH2F',path=trigPath, xbins=100,xmin=-5.0,xmax=5.0,ybins=66,ymin=-math.pi,ymax=math.pi)

    myGroup.defineHistogram('TowerGlobalEta;h_TowerGlobalEta', title='jFex Tower Global Eta',
                            type='TH1F', path=trigPath, xbins=100,xmin=-50,xmax=50)

    myGroup.defineHistogram('TowerGlobalPhi;h_TowerGlobalPhi', title='jFex Tower Global Phi',
                            type='TH1F', path=trigPath, xbins=67,xmin=-1,xmax=65)

    myGroup.defineHistogram('TowerGlobalEta,TowerGlobalPhi;h_TowerGlobalEtaPhiMap', title='jFex Tower Global Eta vs Phi',
                            type='TH2F',path=trigPath, xbins=100,xmin=-50,xmax=50,ybins=67,ymin=-1,ymax=65)

    myGroup.defineHistogram('TowerModule;h_TowerModule', title='jFex Tower Module Number',
                            type='TH1F', path=trigPath, xbins=6,xmin=0,xmax=6.0)
  
    myGroup.defineHistogram('TowerFpga;h_TowerFpga', title='jFex Tower FPGA Number',
                            type='TH1F', path=trigPath, xbins=4,xmin=0,xmax=4.0)

    myGroup.defineHistogram('TowerChannel;h_TowerChannel', title='jFex Tower Channel Number',
                            type='TH1F', path=trigPath, xbins=62,xmin=0,xmax=62.0)

    myGroup.defineHistogram('TowerDataID;h_TowerDataID', title='jFex Tower Data ID',
                            type='TH1F', path=trigPath, xbins=16,xmin=0,xmax=16.0)

    myGroup.defineHistogram('TowerSimulationID;h_TowerSimulationID', title='jFex Tower Simulation ID',
                            type='TH1F', path=trigPath, xbins=1000,xmin=0,xmax=1500000.0)

    myGroup.defineHistogram('TowerCalosource;h_TowerCalosource', title='jFex Tower Calo Source',
                            type='TH1F', path=trigPath, xbins=7,xmin=0,xmax=7.0,xlabels=Calosource_names)

    myGroup.defineHistogram('TowerEtcount_barrel;h_TowerEtcount_barrel', title='jFex Tower Et Barrel',
                            type='TH1F', path=trigPath, xbins=100,xmin=-10,xmax=500.0)

    myGroup.defineHistogram('TowerEtcount_tile;h_TowerEtcount_tile', title='jFex Tower Et Tile',
                            type='TH1F', path=trigPath, xbins=100,xmin=-10,xmax=500.0)

    myGroup.defineHistogram('TowerEtcount_emec;h_TowerEtcount_emec', title='jFex Tower Et EMEC',
                            type='TH1F', path=trigPath, xbins=100,xmin=-10,xmax=500.0)

    myGroup.defineHistogram('TowerEtcount_hec;h_TowerEtcount_hec', title='jFex Tower Et HEC',
                            type='TH1F', path=trigPath, xbins=100,xmin=-10,xmax=500.0)

    myGroup.defineHistogram('TowerEtcount_fcal1;h_TowerEtcount_fcal1', title='jFex Tower Et FCAL1',
                            type='TH1F', path=trigPath, xbins=100,xmin=-10,xmax=500.0)

    myGroup.defineHistogram('TowerEtcount_fcal2;h_TowerEtcount_fcal2', title='jFex Tower Et FCAL2',
                            type='TH1F', path=trigPath, xbins=100,xmin=-10,xmax=500.0)

    myGroup.defineHistogram('TowerEtcount_fcal3;h_TowerEtcount_fcal3', title='jFex Tower Et FCAL3',
                            type='TH1F', path=trigPath, xbins=100,xmin=-10,xmax=500.0)

    myGroup.defineHistogram('TowerSaturationflag;h_TowerSaturationflag', title='jFex Tower Saturation FLag',
                            type='TH1F', path=trigPath, xbins=2,xmin=0,xmax=2.0)

  
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

    JfexInputMonitorCfg = JfexInputMonitoringConfig(ConfigFlags)
    cfg.merge(JfexInputMonitorCfg)

    # options - print all details of algorithms, very short summary 
    cfg.printConfig(withDetails=False, summariseProps = True)

    nevents=10
    cfg.run(nevents)
