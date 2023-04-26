#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def EfexSimMonitoringConfig(flags):
    '''Function to configure LVL1 Efex simulation comparison algorithm in the monitoring system.'''

    import math

    # get the component factory - used for merging the algorithm results
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    
    # uncomment if you want to see all the flags
    #flags.dump() # print all the configs

    # make the athena monitoring helper
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'EfexSimMonitoringCfg')

    # get any algorithms
    EfexSimMonAlg = helper.addAlgorithm(CompFactory.EfexSimMonitorAlgorithm,'EfexSimMonAlg')

    # add any steering
    groupName = 'EfexSimMonitor' # the monitoring group name is also used for the package name
    EfexSimMonAlg.PackageName = groupName

    doXtobs = False
    if doXtobs:
        EfexSimMonAlg.eFexEMRoIContainer = "L1_eEMxRoI"
        EfexSimMonAlg.eFexTauRoIContainer = "L1_eTauxRoI"
        EfexSimMonAlg.eFexEMRoISimContainer = "L1_eEMxRoISim"
        EfexSimMonAlg.eFexTauSimRoIContainer = "L1_eTauxRoISim"


    mainDir = 'L1Calo'
    trigPath = 'EfexSim/'
    eventsPath = 'EfexSim/EventNumbers/'

    # add monitoring algorithm to group, with group name and main directory 
    myGroup = helper.addGroup(EfexSimMonAlg, groupName , mainDir)

    simEqDataGroup = helper.addGroup(EfexSimMonAlg, groupName+"_simEqData" , mainDir)
    simNeDataGroup = helper.addGroup(EfexSimMonAlg, groupName+"_simNeData" , mainDir)
    simNoDataGroup = helper.addGroup(EfexSimMonAlg, groupName+"_simNoData" , mainDir)
    dataNoSimGroup = helper.addGroup(EfexSimMonAlg, groupName+"_dataNoSim" , mainDir)

    # note: monitoring will require at least one histogram in every created group or will refuse to run

    for key in [EfexSimMonAlg.eFexEMRoIContainer,EfexSimMonAlg.eFexTauRoIContainer,EfexSimMonAlg.eFexEMRoISimContainer,EfexSimMonAlg.eFexTauSimRoIContainer]:
        sKey = str(key) if "+" not in str(key) else str(key)[str(key).index('+')+1:] # strip storegate name
        for suffix,inputType in ["","caloReadout"],["2","fexReadout"]:
            matchedFracGrp = helper.addGroup(EfexSimMonAlg, groupName+"_"+sKey+"_mismatchedFrac" + suffix, mainDir)
            matchedGrp = helper.addGroup(EfexSimMonAlg, groupName+"_"+sKey+"_matched" + suffix, mainDir)
            partmatchedGrp = helper.addGroup(EfexSimMonAlg, groupName+"_"+sKey+"_partmatched" + suffix, mainDir) # right location but wrong energy or other flags
            unmatchedGrp = helper.addGroup(EfexSimMonAlg, groupName+"_"+sKey+"_unmatched" + suffix, mainDir)
            matchedFracGrp.defineHistogram("tobEta,tobPhi,tobMismatched;mismatchedFrac", title=f"Mismatched Fraction {sKey} ({inputType} simput);#eta;#phi", type='TProfile2D', path=trigPath+inputType+"/" + sKey + "/",
                                           xbins=50,xmin=-2.5,xmax=2.5,ybins=64,ymin=-math.pi,ymax=math.pi)
            matchedGrp.defineHistogram("tobEta,tobPhi;matched", title=f"Matched {sKey} ({inputType} simput);#eta;#phi;matched "+sKey, type='TH2F', path=trigPath+inputType+"/" + sKey + "/",
                                   xbins=50,xmin=-2.5,xmax=2.5,ybins=64,ymin=-math.pi,ymax=math.pi)
            partmatchedGrp.defineHistogram("tobEta,tobPhi;locationOnly_matched", title=f"LocationOnly-matched {sKey} ({inputType} simput);#eta;#phi", type='TH2F', path=trigPath+inputType+"/" + sKey + "/",
                                     xbins=50,xmin=-2.5,xmax=2.5,ybins=64,ymin=-math.pi,ymax=math.pi)
            unmatchedGrp.defineHistogram("tobEta,tobPhi;unmatched", title=f"Unmatched {sKey} ({inputType} simput);#eta;#phi", type='TH2F', path=trigPath+inputType+"/" + sKey + "/",
                                     xbins=50,xmin=-2.5,xmax=2.5,ybins=64,ymin=-math.pi,ymax=math.pi)
    myGroup.defineTree('LBN,EventNumber,fexReadout,tobType,dataEtas,dataPhis,dataWord0s,simEtas,simPhis,simWord0s;mismatched',
                                "lbn/l:eventNumber/l:fexReadout/i:tobType/i:dataEtas/vector<float>:dataPhis/vector<float>:dataWord0s/vector<unsigned int>:simEtas/vector<float>:simPhis/vector<float>:simWord0s/vector<unsigned int>",title="mismatched",path=trigPath)
    myGroup.defineHistogram('LBNString,tobAndReadoutType;mismatchedTobTypes_vs_lbn', path=trigPath, type='TH2I', weight='nTOBs',
                            title='TOBs;LB;Tob Type (simput Type);Events',
                            xbins=1, xmin=0, xmax=1, xlabels=[""],
                            ybins=4, ymin=-0.5, ymax=3.5, ylabels=["em (calo)","tau (calo)","em (fex)","tau (fex)"],
                            opt=['kCanRebin'])
    #
    def bookSimCompHistos(simtype, leptype='eEM'):
        """
        simplify booking histograms for EM and Taus
        """
        group = myGroup
        group_title = ''
        if simtype == "simEqData":
            group = simEqDataGroup
            group_title = "(events where sim=data)"
        elif simtype == "simNeData":
            group = simNeDataGroup 
            group_title = "(Mismatched where sim!=data but nSim=nData)"
        elif simtype == "simNoData":
            group = simNoDataGroup 
            group_title = "(sim no data)"
        elif simtype == "dataNoSim":
            group = dataNoSimGroup 
            group_title = "(data no sim)"

        lepPrefix='em'
        if 'eTau' in leptype:
            lepPrefix='tau'

        # Eta vs phi
        varname = lepPrefix+'TOBEta,'+lepPrefix+'TOBPhi;h_'+lepPrefix+'TOBEtaPhiMap_'+simtype 
        title = "eFex TOB EM Eta vs Phi "+group_title+";TOB EM Eta;TOB EM Phi"

        if leptype == 'eTau':
            title = title.replace('EM','Tau')
        path=trigPath+simtype+'/'+leptype+'/' 
        group.defineHistogram(varname, title=title, type='TH2F', path=path,
                              xbins=50,xmin=-2.5,xmax=2.5,ybins=64,ymin=-math.pi,ymax=math.pi)

        # Shelf vs FPGA
        varname = lepPrefix+'TOBshelfNumber,'+lepPrefix+'TOBeFEXNumber;h_'+lepPrefix+'TOBShelfVsModule_'+simtype 
        title = "eFex TOB EM Shelf Number vs Module Number "+group_title+";TOB EM Shelf Number;TOB EM Module"
        if leptype == 'eTau':
            title = title.replace('EM','Tau')
        group.defineHistogram(varname, title=title, type='TH2F', path=path,
                              xbins=2,xmin=0.0,xmax=2.0,ybins=12,ymin=0,ymax=12)

        # Mismatch Event Number Samples        
        if "simEqData" not in simtype:
            varname = lepPrefix+'EventNumberSh0,'+lepPrefix+'ModuleSh0;h_'+lepPrefix+'MismatchEventsSh0_'+simtype 
            title = 'eFex TOB EM Mismatch Event Numbers Shelf0;Events with Error/Mismatch;Shelf0 Module Number'
            if leptype == 'eTau':
                title = title.replace('EM','Tau')
            group.defineHistogram(varname, title=title, type='TH2I',
                            path=eventsPath, merge='merge',
                            xbins=1,ybins=12,ymin=0,ymax=12.0)
            #
            varname = lepPrefix+'EventNumberSh1,'+lepPrefix+'ModuleSh1;h_'+lepPrefix+'MismatchEventsSh1_'+simtype 
            title = 'eFex TOB EM Mismatch Event Numbers Shelf1;Events with Error/Mismatch;Shelf1 Module Number'
            if leptype == 'eTau':
                title = title.replace('EM','Tau')
            group.defineHistogram(varname, title=title, type='TH2I',
                            path=eventsPath, merge='merge',
                            xbins=1,ybins=12,ymin=0,ymax=12.0)


    # EM
    bookSimCompHistos('simEqData')
    bookSimCompHistos('simNeData')
    bookSimCompHistos('simNoData')
    bookSimCompHistos('dataNoSim')
    # Tau
    bookSimCompHistos('simEqData','eTau')
    bookSimCompHistos('simNeData','eTau')
    bookSimCompHistos('simNoData','eTau')
    bookSimCompHistos('dataNoSim','eTau')

    acc = helper.result()
    result.merge(acc)
    return result


if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    import glob

    # MCs processed adding L1_eEMRoI
    inputs = glob.glob('/eos/user/t/thompson/ATLAS/LVL1_mon/MC_ESD/l1calo.361024.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ4W.eFex_gFex_2022-01-13T2101.root')
    
    flags.Input.Files = inputs
    flags.Output.HISTFileName = 'ExampleMonitorOutput_LVL1_MC.root'

    flags.Exec.MaxEvents=10

    flags.lock()
    flags.dump() # print all the configs

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg  
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    EfexSimMonitorCfg = EfexSimMonitoringConfig(flags)
    cfg.merge(EfexSimMonitorCfg)

    cfg.run()

