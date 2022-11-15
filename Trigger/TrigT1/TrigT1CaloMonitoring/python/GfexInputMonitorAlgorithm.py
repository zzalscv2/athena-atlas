#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
def GfexInputMonitoringConfig(inputFlags):
    '''Function to configure LVL1 GfexInput algorithm in the monitoring system.'''

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
    helper = AthMonitorCfgHelper(inputFlags,'GfexInputMonitoringCfg')

    # get any algorithms
    GfexInputMonAlg = helper.addAlgorithm(CompFactory.GfexInputMonitorAlgorithm,'GfexInputMonAlg')

    # add any steering
    groupName = 'GfexInputMonitor' # the monitoring group name is also used for the package name
    GfexInputMonAlg.PackageName = groupName

    mainDir = 'L1Calo'
    trigPath = 'GfexInput/'

    # See if the file contains xTOBs else use TOBs
    #hasXtobs = True if "L1_eFexTower" in inputFlags.Input.Collections else False
    #if not hasXtobs:
    #    EfexInputMonAlg.eFexTowerContainer = "L1_eEMRoI"

    #tobStr = "TOB"

    # add monitoring algorithm to group, with group name and main directory 
    myGroup = helper.addGroup(GfexInputMonAlg, groupName , mainDir)

    # histograms of gFex tower variables
    myGroup.defineHistogram('NGfexTowers;h_nGfexTowers', title='Number of gFex towers',
                            type='TH1I', path=trigPath, xbins=500,xmin=0,xmax=5000)

    myGroup.defineHistogram('TowerEta;h_TowerEta', title='gFex Tower Eta',
                            type='TH1F', path=trigPath, xbins=100,xmin=-5.0,xmax=5.0)

    myGroup.defineHistogram('TowerPhi;h_TowerPhi', title='gFex Tower Phi',
                            type='TH1F', path=trigPath, xbins=66,xmin=-math.pi,xmax=math.pi)

    myGroup.defineHistogram('TowerEta,TowerPhi;h_TowerEtaPhiMap', title='gFex Tower Eta vs Phi',
                            type='TH2F',path=trigPath, xbins=100,xmin=-5.0,xmax=5.0,ybins=66,ymin=-math.pi,ymax=math.pi)

    myGroup.defineHistogram('TowerEtaindex;h_TowerEtaindex', title='gFex Tower Eta Index',
                            type='TH1F', path=trigPath, xbins=50,xmin=0.0,xmax=35.0)

    myGroup.defineHistogram('TowerPhiindex;h_TowerPhiindex', title='gFex Tower Phi Index',
                            type='TH1F', path=trigPath, xbins=64,xmin=0.0,xmax=32.0)

    myGroup.defineHistogram('TowerEtaindex,TowerPhiindex;h_TowerEtaPhiMapindex', title='gFex Tower Eta vs Phi index',
                            type='TH2F',path=trigPath, xbins=50,xmin=0.0,xmax=35.0,ybins=64,ymin=0,ymax=32.0)

    myGroup.defineHistogram('TowerFpga;h_TowerFpga', title='gFex Tower FPGA Number',
                            type='TH1F', path=trigPath, xbins=4,xmin=0,xmax=4.0)

    myGroup.defineHistogram('TowerEt;h_TowerEt', title='gFex Tower Et',
                            type='TH1F', path=trigPath, xbins=1000,xmin=0,xmax=1000.0)

    myGroup.defineHistogram('TowerSaturationflag;h_TowerSaturationflag', title='gFex Tower Saturation FLag',
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

    GfexInputMonitorCfg = GfexInputMonitoringConfig(ConfigFlags)
    cfg.merge(GfexInputMonitorCfg)

    # options - print all details of algorithms, very short summary 
    cfg.printConfig(withDetails=False, summariseProps = True)

    nevents=10
    cfg.run(nevents)
