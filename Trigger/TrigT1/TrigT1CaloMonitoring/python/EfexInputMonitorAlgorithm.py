#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def EfexInputMonitoringConfig(inputFlags):
    '''Function to configure LVL1 EfexInput algorithm in the monitoring system.'''

    import math 
    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

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


    # add monitoring algorithm to group, with group name and main directory 
    eventsGroup = helper.addGroup(EfexInputMonAlg, groupName , mainDir)
    eventsGroup.defineHistogram('NEfexTowers;fexTowers', title='Number of eFex towers;nEfexTowers;Events',
                            type='TH1I', path=trigPath+"events/",xbins=1, xmin=0, xmax=1, xlabels=["0"],opt=['kCanRebin'])

    fexTowerGroup = helper.addGroup(EfexInputMonAlg, groupName+"_fexTowers", mainDir)
    slotLabels = ["PS","L1_1","L1_2","L1_3","L1_4","L2_1","L2_2","L2_3","L2_4","L3","Tile","HEC"]
    for i in range(0,12): # one extra "slot" to separate hec and tile
        fexTowerGroup.defineHistogram(f'TowerEtcount{i+1};slot-{slotLabels[i]}',title=f'{slotLabels[i]} fexCount;fexCount;fexTowers',
                                        type='TH1F', path=trigPath+"fexTowers/fexCount", xbins=100,xmin=0,xmax=100.0)
        caloCountGroup = helper.addGroup(EfexInputMonAlg, groupName+"_slot" + str(i) , mainDir)
        caloCountGroup.defineHistogram('TowerEta,TowerPhi,RefTowerCount;slot-' + slotLabels[i], title=f'Average caloReadout Count (slot={slotLabels[i]});#eta;#phi;Average caloCount',
                                  type='TProfile2D',path=trigPath+"caloCounts_avg/phi_vs_eta", xbins=50,xmin=-2.5,xmax=2.5,ybins=64,ymin=-math.pi,ymax=math.pi)
        fexCountGroup = helper.addGroup(EfexInputMonAlg, groupName+"_fex_slot" + str(i) , mainDir)
        fexCountGroup.defineHistogram('TowerEta,TowerCount;slot-' + slotLabels[i], title=f'fexReadout Count (slot={slotLabels[i]});#eta;fexCount;fexCounts',
                                     type='TH2I',path=trigPath+"fexCounts/fexCount_vs_eta", xbins=50,xmin=-2.5,xmax=2.5,ybins=1024,ymin=-0.5,ymax=1023.5)
        fexCountGroup.defineHistogram('TowerEta,TowerPhi;slot-' + slotLabels[i], title=f'fexReadout Sum of Counts ({slotLabels[i]});#eta;#phi;#Sum of counts',
                                     weight = 'TowerCount',
                                     type='TH2F',path=trigPath+"fexCounts_sum/phi_vs_eta/", xbins=50,xmin=-2.5,xmax=2.5,ybins=64,ymin=-math.pi,ymax=math.pi)
    fexTowerGroup.defineHistogram('TowerEmstatus;em_status',title='em status bit;em_status;fexTowers',
                                  type='TH1F', path=trigPath+"fexTowers/", xbins=10,xmin=0,xmax=2400.0)
    fexTowerGroup.defineHistogram('TowerHadstatus;had_status',title='hadronic status bit;em_status;fexTowers',
                                  type='TH1F', path=trigPath+"fexTowers/", xbins=10,xmin=0,xmax=2400.0)
    fexTowerGroup.defineHistogram('TowerEta;eta', title='eFex Tower Eta;#eta;fexTowers',
                            type='TH1F', path=trigPath+"fexTowers/", xbins=100,xmin=-3.0,xmax=3.0)
    fexTowerGroup.defineHistogram('TowerPhi;phi', title='eFex Tower Phi;#phi;fexTowers',
                            type='TH1F', path=trigPath+"fexTowers/", xbins=64,xmin=-math.pi,xmax=math.pi)
    fexTowerGroup.defineHistogram('TowerEta,TowerPhi;phi_vs_eta', title='eFex Tower Eta vs Phi;#eta;#phi;fexTowers',
                            type='TH2F',path=trigPath+"fexTowers/", xbins=50,xmin=-2.5,xmax=2.5,ybins=64,ymin=-math.pi,ymax=math.pi)

    refCompareFracGroup = helper.addGroup(EfexInputMonAlg,groupName+"_RefCompareFrac", mainDir)
    refCompareTreeGroup = helper.addGroup(EfexInputMonAlg,groupName+"_RefCompareTree", mainDir)
    refCompareTreeHistGroup = helper.addGroup(EfexInputMonAlg,groupName+"_RefCompareTreeHist", mainDir)

    refCompareTreeHistGroup.defineHistogram('TowerEta,TowerPhi;phi_vs_eta',title="location of mismatches;#eta;#phi;mismatches",type='TH2I',
                             path=trigPath+"mismatches/",xbins=50,xmin=-2.5,xmax=2.5,ybins=64,ymin=-math.pi,ymax=math.pi)
    refCompareFracGroup.defineHistogram('TowerEta,TowerPhi,Weight;phi_vs_eta',title="fraction of matches;#eta;#phi;Fraction of matches",type='TProfile2D',
                                    path=trigPath+"fexTowers_matchedFrac/",xbins=50,xmin=-2.5,xmax=2.5,ybins=64,ymin=-math.pi,ymax=math.pi)
    refCompareTreeGroup.defineTree('EventNumber,TowerId,TowerEta,TowerPhi,TowerEmstatus,TowerHadstatus,TowerSlot,TowerCount,RefTowerCount,SlotSCID;mismatched',
                                   "eventNumber/l:id/I:eta/F:phi/F:em_status/i:had_status/i:slot/I:count/I:ref_count/I:scid/string",
                                   title="mismatched",path=trigPath+"mismatches/")
    refCompareTreeHistGroup.defineHistogram('LBNString,TowerSlotSplitHad;slot_vs_lbn', path=trigPath+"mismatches/", type='TH2I',
                            title='Mismatched counts;LB;Slot;Number of mismatches',
                            xbins=1, xmin=0, xmax=1, xlabels=[""],
                            ybins=12, ymin=-0.5, ymax=11.5, ylabels=slotLabels,
                            opt=['kCanRebin'])
    refCompareTreeHistGroup.defineHistogram('LBNString,SlotSCID;scid_vs_lbn', path=trigPath+"mismatches/", type='TH2I',
                                        title='Mismatched counts;LB;SCID;mismatches',
                                        xbins=1, xmin=0, xmax=1, xlabels=[""],
                                        ybins=1, ymin=0, ymax=1, ylabels=[""],
                                        opt=['kCanRebin'])
    refCompareTreeHistGroup.defineHistogram('TowerCount,RefTowerCount;caloCount_vs_fexCount', path=trigPath+"mismatches/", type='TH2I',
                                        title='Mismatched counts;Fex Readout;Calo Readout;mismatches',
                                        xbins=60, xmin=-0.5, xmax=59.5,
                                        ybins=60, ymin=-0.5, ymax=59.5)

    acc = helper.result()
    result.merge(acc)
    return result


if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import glob

    inputs = glob.glob('/eos/atlas/atlastier0/rucio/data18_13TeV/physics_Main/00354311/data18_13TeV.00354311.physics_Main.recon.ESD.f1129/data18_13TeV.00354311.physics_Main.recon.ESD.f1129._lb0013._SFO-8._0001.1')

    flags = initConfigFlags()
    flags.Input.Files = inputs
    flags.Output.HISTFileName = 'ExampleMonitorOutput_LVL1_MC.root'

    flags.lock()
    flags.dump() # print all the configs

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    EfexInputMonitorCfg = EfexInputMonitoringConfig(flags)
    cfg.merge(EfexInputMonitorCfg)

    # options - print all details of algorithms, very short summary 
    cfg.printConfig(withDetails=False, summariseProps = True)

    nevents=10
    cfg.run(nevents)
