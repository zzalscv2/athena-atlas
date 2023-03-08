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

    # meed at least one histogram in the main group
    myGroup.defineHistogram('TOBTransverseEnergy;h_TOBTransverseEnergy', title='eFex TOB EM Transverse Energy [MeV]',
                            type='TH1F', path=trigPath+'eEM/', xbins=100,xmin=0,xmax=50000)


    #
    def bookSimCompHistos(simtype, leptype='eEM'):
        """
        simplify booking histograms for EM and Taus
        """
        group = myGroup
        group_title = ''
        if simtype == "simEqData":
            group = simEqDataGroup
            group_title = "(sim=data)"
        elif simtype == "simNeData":
            group = simNeDataGroup 
            group_title = "(sim!=data)"
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
                              xbins=50,xmin=-3.0,xmax=3.0,ybins=64,ymin=-math.pi,ymax=math.pi)

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

