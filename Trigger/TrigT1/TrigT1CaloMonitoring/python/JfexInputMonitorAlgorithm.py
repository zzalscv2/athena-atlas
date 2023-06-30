#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def JfexInputMonitoringConfig(inputFlags):
    '''Function to configure LVL1 JfexInput algorithm in the monitoring system.'''

    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # make the athena monitoring helper
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'JfexInputMonitoringCfg')

    # get any algorithms
    JfexInputMonAlg = helper.addAlgorithm(CompFactory.JfexInputMonitorAlgorithm,'JfexInputMonAlg')

    # add any steering
    groupName = 'JfexInputMonitor' # the monitoring group name is also used for the package name
    JfexInputMonAlg.Grouphist = groupName

    mainDir = 'L1Calo'
    trigPath = 'JfexInput/'
    Calosource_names = ["Barrel","Tile","EMEC","HEC","FCAL1","FCAL2","FCAL3"]
    FPGA_names = ["U1","U2","U4","U3"]
    
    from math import pi
    
    x_phi = []
    for i in range(67):
        phi = (-pi- pi/32) + pi/32*i 
        x_phi.append(phi)
    x_phi = sorted(x_phi)
    
    phi_bins = {
        'xbins': x_phi
    }
    
    eta_phi_bins = {
        'xbins': 100, 'xmin': -5, 'xmax': 5,
        'ybins': x_phi
    }
    
    # add monitoring algorithm to group, with group name and main directory 
    myGenericGroup = helper.addGroup(None, groupName+"Gen", mainDir)
    
    myGenericGroup.defineHistogram('genLocation,genType;jFEX_Errors', path=None, type='TH2I',
                            title='jFEX generic monitoring for shifters;Location;Type',
                            xbins=4, xmin=0, xmax=4, xlabels=["Sim_DataTowers","Sim_EmulatedTowers","Input_Mismatch","Input_Invalids"],
                            ybins=4, ymin=0, ymax=4, ylabels=["TOB", "global TOB", "EM layer", "HAD layer" ],
                            opt=['kCanRebin'])
                            
    JfexInputMonAlg.jFEXMonTool = myGenericGroup

    myGroup = helper.addGroup(JfexInputMonAlg, groupName , mainDir)
    DecorGroup    = helper.addGroup(JfexInputMonAlg, groupName+"_decorated"     , mainDir)
    DecorAllGroup = helper.addGroup(JfexInputMonAlg, groupName+"_decorated_all" , mainDir)
    EmulatedGroup = helper.addGroup(JfexInputMonAlg, groupName+"_emulated"   , mainDir)
    DetailsGroup  = []
    for i in range(7):
        DetailsGroup.append( helper.addGroup(JfexInputMonAlg, groupName+"_details_"+str(i)   , mainDir))
        
        if(i == 1):
            DetailsGroup[i].defineHistogram('DataEt,EmuSum;SumSCell_vs_Data_'+Calosource_names[i], title='Data Et vs Tile Et ('+ Calosource_names[i]+'); Data Et [GeV]; Tile Et [GeV]',
                            type='TH2F',path=trigPath+"expert/", xbins=200,xmin=0,xmax=100,ybins=200,ymin=0,ymax=100)
                            
        else:    
            DetailsGroup[i].defineHistogram('DataEt,EmuSum;SumSCell_vs_Data_'+Calosource_names[i], title='Data vs SCell Sum Et ('+ Calosource_names[i]+'); Data Et [MeV]; SCell Sum Et [MeV]',
                            type='TH2F',path=trigPath+"expert/", xbins=160,xmin=-2000,xmax=2000,ybins=160,ymin=-2000,ymax=2000)
                            
        

    myGroup.defineHistogram('region,type;DataErrors', title='jFEX Data mismatches per region; Region; Type',
                            type='TH2F',path=trigPath, xbins=7,xmin=0,xmax=7,ybins=2,ymin=0,ymax=2,xlabels=Calosource_names,ylabels=["Invalid codes","Data mismatch"])

    myGroup.defineHistogram('TowerEtaInvalid,TowerPhiInvalid;2Dmap_InvalidCodes', title='jFex DataTower Invalid Et codes; #eta; #phi',
                            type='TH2F',path=trigPath, **eta_phi_bins)
                            
    DecorGroup.defineHistogram('TowerEta,TowerPhi;2Dmap_MismatchedEts', title='jFex DataTower mismatches (no invalid codes); #eta; #phi',
                            type='TH2F',path=trigPath, **eta_phi_bins)
                            
    DecorGroup.defineHistogram('DataEt,SCellSum;SumSCell_vs_Data_Mismatched', title='Data vs SCell Sum Et (unmatching); Data Et [MeV]; SCell Sum Et [MeV]',
                            type='TH2F',path=trigPath, xbins=160,xmin=-2000,xmax=2000,ybins=160,ymin=-2000,ymax=2000)
                            
    DecorAllGroup.defineHistogram('DataEt,SCellSum;SumSCell_vs_Data_all', title='Data vs SCell Sum Et (all Towers); Data Et [MeV]; SCell Sum Et [MeV]',
                            type='TH2F',path=trigPath, xbins=160,xmin=-2000,xmax=2000,ybins=160,ymin=-2000,ymax=2000)
                            
    DecorGroup.defineHistogram('DataEt,frac_SCellSum;frac_SumSCell_vs_Data', title='Data vs (Et_{SCell}-Et_{Data})/Et_{Data} (no invalid codes); Data Et [MeV]; (Et_{SCell}-Et_{Data})/Et_{Data}',
                            type='TH2F',path=trigPath, xbins=160,xmin=-2000,xmax=2000,ybins=100,ymin=-20,ymax=20)
                            
    EmulatedGroup.defineHistogram('TowerEta,TowerPhi;2Dmap_DataVsEmulated', title='Input Data vs Emulated data; #eta; #phi',
                            type='TH2F',path=trigPath, **eta_phi_bins)
    EmulatedGroup.defineHistogram('TowerEtaDeco,TowerPhiDeco;2Dmap_EmulatedVsDecorated', title='Emulated vs Decorated data; #eta; #phi',
                            type='TH2F',path=trigPath, **eta_phi_bins)
                            
    # histograms of jFex tower variables (content of the container)
    myGroup.defineHistogram('NJfexTowers;h_nJfexTowers', title='Number of jFex towers',
                            type='TH1I', path=trigPath+"Content/", xbins=2000,xmin=0,xmax=20000)

    myGroup.defineHistogram('TowerEta;h_TowerEta', title='jFex Tower Eta',
                            type='TH1F', path=trigPath+"Content/", xbins=100,xmin=-5.0,xmax=5.0)

    myGroup.defineHistogram('TowerPhi;h_TowerPhi', title='jFex Tower Phi',
                            type='TH1F', path=trigPath+"Content/", **phi_bins)

    myGroup.defineHistogram('TowerEta,TowerPhi;h_TowerEtaPhiMap', title='jFex Tower Eta vs Phi',
                            type='TH2F',path=trigPath+"Content/", **eta_phi_bins)

    myGroup.defineHistogram('TowerGlobalEta;h_TowerGlobalEta', title='jFex Tower Global Eta',
                            type='TH1I', path=trigPath+"Content/", xbins=100,xmin=-50,xmax=50)

    myGroup.defineHistogram('TowerGlobalPhi;h_TowerGlobalPhi', title='jFex Tower Global Phi',
                            type='TH1F', path=trigPath+"Content/", xbins=67,xmin=-1,xmax=65)

    myGroup.defineHistogram('TowerGlobalEta,TowerGlobalPhi;h_TowerGlobalEtaPhiMap', title='jFex Tower Global Eta vs Phi',
                            type='TH2I',path=trigPath+"Content/", xbins=100,xmin=-50,xmax=50,ybins=67,ymin=-1,ymax=65)

    myGroup.defineHistogram('TowerModule;h_TowerModule', title='jFex Tower Module Number',
                            type='TH1I', path=trigPath+"Content/", xbins=6,xmin=0,xmax=6)
  
    myGroup.defineHistogram('TowerFpga;h_TowerFpga', title='jFex Tower FPGA Number',
                            type='TH1I', path=trigPath+"Content/", xbins=4,xmin=0,xmax=4,xlabels=FPGA_names)

    myGroup.defineHistogram('TowerChannel;h_TowerChannel', title='jFex Tower Channel Number',
                            type='TH1I', path=trigPath+"Content/", xbins=60,xmin=0,xmax=60)

    myGroup.defineHistogram('TowerDataID;h_TowerDataID', title='jFex Tower Data ID',
                            type='TH1I', path=trigPath+"Content/", xbins=16,xmin=0,xmax=16)

    myGroup.defineHistogram('TowerSimulationID;h_TowerSimulationID', title='jFex Tower Simulation ID',
                            type='TH1F', path=trigPath+"Content/", xbins=1000,xmin=0,xmax=1500000.0)

    myGroup.defineHistogram('TowerCalosource;h_TowerCalosource', title='jFex Tower Calo Source',
                            type='TH1I', path=trigPath+"Content/", xbins=7,xmin=0,xmax=7,xlabels=Calosource_names)

    myGroup.defineHistogram('TowerEtcount_barrel;h_TowerEtcount_barrel', title='jFex Tower Et Barrel',
                            type='TH1I', path=trigPath+"Content/", xbins=4096,xmin=0,xmax=4096)

    myGroup.defineHistogram('TowerEtcount_tile;h_TowerEtcount_tile', title='jFex Tower Et Tile',
                            type='TH1I', path=trigPath+"Content/", xbins=4096,xmin=0,xmax=4096)

    myGroup.defineHistogram('TowerEtcount_emec;h_TowerEtcount_emec', title='jFex Tower Et EMEC',
                            type='TH1I', path=trigPath+"Content/", xbins=4096,xmin=0,xmax=4096)

    myGroup.defineHistogram('TowerEtcount_hec;h_TowerEtcount_hec', title='jFex Tower Et HEC',
                            type='TH1I', path=trigPath+"Content/", xbins=4096,xmin=0,xmax=4096)

    myGroup.defineHistogram('TowerEtcount_fcal1;h_TowerEtcount_fcal1', title='jFex Tower Et FCAL1',
                            type='TH1I', path=trigPath+"Content/", xbins=4096,xmin=0,xmax=4096)

    myGroup.defineHistogram('TowerEtcount_fcal2;h_TowerEtcount_fcal2', title='jFex Tower Et FCAL2',
                            type='TH1I', path=trigPath+"Content/", xbins=4096,xmin=0,xmax=4096)

    myGroup.defineHistogram('TowerEtcount_fcal3;h_TowerEtcount_fcal3', title='jFex Tower Et FCAL3',
                            type='TH1I', path=trigPath+"Content/", xbins=4096,xmin=0,xmax=4096)

    myGroup.defineHistogram('TowerSaturationflag;h_TowerSaturationflag', title='jFex Tower Saturation FLag',
                            type='TH1I', path=trigPath+"Content/", xbins=2,xmin=0,xmax=2)

  
    acc = helper.result()
    result.merge(acc)
    return result    

if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import glob
    
    import argparse
    parser = argparse.ArgumentParser(prog='python -m TrigT1CaloMonitoring.JfexInputMonitoringAlgorithm',
                                   description="""Used to run jFEX Monitoring\n\n
                                   Example: python -m TrigT1CaloMonitoring.JfexInputMonitoringAlgorithm --filesInput file.root.\n
                                   Overwrite inputs using standard athena opts --filesInput, evtMax etc. see athena --help""")
    parser.add_argument('--evtMax',type=int,default=-1,help="number of events")
    parser.add_argument('--filesInput',nargs='+',help="input files",required=True)
    parser.add_argument('--skipEvents',type=int,default=0,help="number of events to skip")
    args = parser.parse_args()


    flags = initConfigFlags()
    flags.Input.Files = [file for x in args.filesInput for file in glob.glob(x)]
    flags.Output.HISTFileName = 'jFexInputData_Monitoring.root'
    
    flags.Exec.MaxEvents = args.evtMax
    flags.Exec.SkipEvents = args.skipEvents    

    flags.lock()
    flags.dump() # print all the configs

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    JfexInputMonitorCfg = JfexInputMonitoringConfig(flags)
    cfg.merge(JfexInputMonitorCfg)
    cfg.run()
