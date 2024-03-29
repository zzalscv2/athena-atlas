#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

def LArSuperCellMonConfig(flags, **kwargs):
    from AthenaCommon.Logging import logging
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    from AthenaConfiguration.ComponentFactory import CompFactory
    mlog = logging.getLogger( 'LArSuperCellMonConfig' )
    mask=True

    from AthenaMonitoring.AthMonitorCfgHelper import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'LArSuperCellMonAlgCfg')

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    cfg=ComponentAccumulator()

    if not flags.DQ.enableLumiAccess and not flags.DQ.Environment == 'online':
       mlog.warning('This algo needs Lumi access, returning empty config')
       return cfg

    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    cfg.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    cfg.merge(TileGMCfg(flags))

    from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg
    cfg.merge(DetDescrCnvSvcCfg(flags))

    if flags.Common.isOnline:
       cfg.addCondAlgo(CompFactory.CaloSuperCellAlignCondAlg('CaloSuperCellAlignCondAlg'))       

    from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
    cfg.merge(BunchCrossingCondAlgCfg(flags))

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    cfg.merge(CaloNoiseCondAlgCfg(flags))
    cfg.merge(CaloNoiseCondAlgCfg(flags,noisetype="electronicNoise"))

    cfg.merge(ByteStreamReadCfg(flags))
    from LArByteStream.LArRawSCDataReadingConfig import LArRawSCDataReadingCfg
    cfg.merge(LArRawSCDataReadingCfg(flags))

    from TrigT1CaloFexPerf.EmulationConfig import emulateSC_Cfg
    cfg.merge(emulateSC_Cfg(flags))

    from LArCellRec.LArRAWtoSuperCellConfig import LArRAWtoSuperCellCfg
    cfg.merge(LArRAWtoSuperCellCfg(flags,mask=mask) )

    # Reco SC:
    #get SC onl-offl mapping from DB    
    from LArCabling.LArCablingConfig import LArOnOffIdMappingSCCfg
    cfg.merge(LArOnOffIdMappingSCCfg(flags))
    
    # and elec. calib. coeffs
    from LArConfiguration.LArElecCalibDBConfig import LArElecCalibDBSCCfg

    larLATOMEBuilderAlg=CompFactory.LArLATOMEBuilderAlg("LArLATOMEBuilderAlg")
    from LArConditionsCommon.LArRunFormat import getLArDTInfoForRun
    try:
        runinfo=getLArDTInfoForRun(flags.Input.RunNumbers[0], connstring="COOLONL_LAR/CONDBR2")
        streamTypes=runinfo.streamTypes()
    except Exception as e:
        mlog.warning("Could not get DT run info, using defaults !")
        mlog.warning(e)
        streamTypes=["RawADC"]
    
    for i in range(0,len(streamTypes)):
        mlog.info("runinfo.streamTypes()[i]: "+str(streamTypes[i]))
        if streamTypes[i] ==  "RawADC":
            larLATOMEBuilderAlg.LArDigitKey = "SC"
            larLATOMEBuilderAlg.isADCBas = False
        if streamTypes[i] ==  "ADC":
            larLATOMEBuilderAlg.isADCBas = True
            larLATOMEBuilderAlg.LArDigitKey = "SC_ADC_BAS"

    cfg.addEventAlgo(larLATOMEBuilderAlg)
    cfg.merge(LArRAWtoSuperCellCfg(flags,name="LArRAWRecotoSuperCell",mask=mask,doReco=True,SCIn="SC_ET_RECO",SCellContainerOut="SCell_ET_RECO") )


    cfg.merge(LArElecCalibDBSCCfg(flags, condObjs=["Ramp","DAC2uA", "Pedestal", "uA2MeV", "MphysOverMcal", "OFC", "Shape", "HVScaleCorr"]))

    
    #return cfg
    algname='LArSuperCellMonAlg'
    lArCellMonAlg=CompFactory.LArSuperCellMonAlg(algname,CaloCellContainerReco="SCell_ET_RECO",doSCReco=True)


    if flags.Input.isMC is False and not flags.Common.isOnline:
       from LumiBlockComps.LuminosityCondAlgConfig import  LuminosityCondAlgCfg
       cfg.merge(LuminosityCondAlgCfg(flags))
       from LumiBlockComps.LBDurationCondAlgConfig import  LBDurationCondAlgCfg
       cfg.merge(LBDurationCondAlgCfg(flags))


    from AthenaConfiguration.Enums import BeamType
    if flags.Beam.Type is BeamType.Cosmics:
        algname=algname+'Cosmics'

    LArSuperCellMonConfigCore(helper, lArCellMonAlg, flags,
                                     flags.Beam.Type is BeamType.Cosmics,
                                     flags.Input.isMC, algname, RemoveMasked=mask)

    cfg.merge(helper.result())

    return cfg


def LArSuperCellMonConfigCore(helper, algclass, flags, isCosmics=False, isMC=False, algname='LArSuperCellMonAlg', RemoveMasked=True):


    LArSuperCellMonAlg = helper.addAlgorithm(algclass, algname)


    GroupName="LArSuperCellMon"
    LArSuperCellMonAlg.MonGroupName = GroupName

    LArSuperCellMonAlg.EnableLumi = False
    LArSuperCellMonAlg.CaloCellContainer = flags.LAr.DT.ET_IDKey
    LArSuperCellMonAlg.CaloCellContainerRef = flags.Trigger.L1.L1CaloSuperCellContainerName
    LArSuperCellMonAlg.RemoveMasked = RemoveMasked
    

    do2DOcc = True #TMP
    print(do2DOcc)



   #---single Group for non threshold histograms
    cellMonGroup = helper.addGroup(
        LArSuperCellMonAlg,
        GroupName,
        '/LAr/LArSuperCellMon_NoTrigSel/'

    )


    #--define histograms
    sc_hist_path='SC/'


    cellMonGroup.defineHistogram('superCellEt;h_SuperCellEt',
                                 title='Super Cell E_T [MeV]; MeV; # entries',
                                 type='TH1F', path=sc_hist_path,
                                 xbins =  100,xmin=0,xmax=50000)
    cellMonGroup.defineHistogram('superCellEta;h_SuperCellEta',
                                 title='Super Cell eta; #eta; # entries',
                                 type='TH1F', path=sc_hist_path,
                                 xbins =  100,xmin=-5,xmax=5)
    cellMonGroup.defineHistogram('superCelltime;h_SuperCelltime',
                                 title='Super Cell time [ns]; ns; # entries',
                                 type='TH1F', path=sc_hist_path,
                                 xbins = 100, xmin=-400,xmax=400)
    cellMonGroup.defineHistogram('superCelltimeReco;h_SuperCelltimeReco',
                                 title='Reco Super Cell time [ns]; ns; # entries',
                                 type='TH1F', path=sc_hist_path,
                                 xbins = 100, xmin=-400,xmax=400)
    cellMonGroup.defineHistogram('superCellprovenance;h_SuperCellprovenance',
                                 title='Super Cell provenance; bitmask ; # entries',
                                 type='TH1F', path=sc_hist_path,
                                 xbins = 700, xmin=0,xmax=700)
    cellMonGroup.defineHistogram('BCID,superCellEt;h_SuperCellEt_vs_BCID',
                                 title='Super Cell ET [MeV] vs BCID ; BCID from train front; %',
                                 type='TH2F', path=sc_hist_path,
                                 xbins = 50, xmin=0,xmax=50,
                                 ybins = 80, ymin=-1000,ymax=1000)
    cellMonGroup.defineHistogram('BCID,superCellEtRef;h_SuperCellEtRef_vs_BCID',
                                 title='Super Cell ET [MeV] vs BCID ; BCID from train front; %',
                                 type='TH2F', path=sc_hist_path,
                                 xbins = 50, xmin=0,xmax=50,
                                 ybins = 80, ymin=-1000,ymax=1000)

    cellMonGroup.defineHistogram('resolution;h_SuperCellResolution',
                                 title='Super Cell reconstruction resolution ; %; # entries',
                                 type='TH1F', path=sc_hist_path,
                                 xbins = 70, xmin=-20,xmax=120)
    cellMonGroup.defineHistogram('resolutionPass;h_SuperCellResolutionPass',
                                 title='Super Cell reconstruction resolution for BCIDed ; %; # entries',
                                 type='TH1F', path=sc_hist_path,
                                 xbins = 70, xmin=-20,xmax=120)
    cellMonGroup.defineHistogram('superCellEt,resolution;h_SuperCellResolution_vs_ET',
                                 title='Super Cell reconstruction resolution vs ET ; [MeV]; %',
                                 type='TH2F', path=sc_hist_path,
                                 xbins = 100, xmin=0,xmax=50000,
                                 ybins = 70, ymin=-20,ymax=120)
    cellMonGroup.defineHistogram('superCellEta,resolutionHET;h_SuperCellResolution_vs_eta',
                                 title='Super Cell reconstruction resolution vs #eta ; #eta; %',
                                 type='TH2F', path=sc_hist_path,
                                 xbins = 100, xmin=-5,xmax=5,
                                 ybins = 40, ymin=-20,ymax=20)
    cellMonGroup.defineHistogram('BCID,resolution;h_SuperCellResolution_vs_BCID',
                                 title='Super Cell reconstruction resolution vs BCID ; BCID from train front; %',
                                 type='TH2F', path=sc_hist_path,
                                 xbins = 50, xmin=0,xmax=50,
                                 ybins = 80, ymin=-120,ymax=120)

    cellMonGroup.defineHistogram('superCellEtRef,superCellEt;h_SuperCellEtLin',
                                 title='Super Cell E_T Linearity; Ref SC E_T [MeV]; SC E_T [MeV]',
                                 type='TH2F', path=sc_hist_path,
                                 xbins =  100,xmin=0,xmax=50000,
                                 ybins =  100,ymin=0,ymax=50000)
    cellMonGroup.defineHistogram('superCelltimeRef,superCelltimeReco;h_SuperCelltimeLin',
                                 title='Super Cell time Linearity; Ref SC time [ns]; Reco SC time [ns]',
                                 type='TH2F', path=sc_hist_path,
                                 xbins = 100, xmin=-200,xmax=200,
                                 ybins = 100, ymin=-200,ymax=200)
    cellMonGroup.defineHistogram('superCellprovenanceRef,superCellprovenance;h_SuperCellprovenanceLin',
                                 title='Super Cell provenance Linearity; Ref SC bitmask ; SC bitmask',
                                 type='TH2F', path=sc_hist_path,
                                 xbins = 17, xmin=0,xmax=680,
                                 ybins = 17, ymin=0,ymax=680)
    cellMonGroup.defineHistogram('BCID;h_BCID',
                                 title='BCID from the front of the train; BCID ; # entries',
                                 type='TH1F', path=sc_hist_path,
                                 xbins = 120, xmin=0,xmax=120)

    sc_hist_path='SC_Layer/'
    for part in LArSuperCellMonAlg.LayerNames:
           partp='('+part+')'
           cellMonGroup.defineHistogram('superCellEt_'+part+';h_SuperCellEt'+part,
                                        title='Super Cell E_T [MeV] '+partp+'; MeV; # entries',
                                        type='TH1F', path=sc_hist_path,
                                        xbins =  100,xmin=0,xmax=50000)
           cellMonGroup.defineHistogram('superCellEta_'+part+';h_SuperCellEta'+part,
                                        title='Super Cell eta '+partp+'; #eta; # entries',
                                        type='TH1F', path=sc_hist_path,
                                        xbins =  100,xmin=-5,xmax=5)
           cellMonGroup.defineHistogram('superCelltime_'+part+';h_SuperCelltime'+part,
                                        title='Super Cell time [ns] '+partp+'; ns; # entries',
                                        type='TH1F', path=sc_hist_path,
                                        xbins = 100, xmin=-400,xmax=400)
           cellMonGroup.defineHistogram('superCelltimeReco_'+part+';h_SuperCelltimeReco'+part,
                                        title='Reco Super Cell time [ns] '+partp+'; ns; # entries',
                                        type='TH1F', path=sc_hist_path,
                                        xbins = 100, xmin=-400,xmax=400)
           cellMonGroup.defineHistogram('superCellprovenance_'+part+';h_SuperCellprovenance'+part,
                                        title='Super Cell provenance '+partp+'; bitmask ; # entries',
                                        type='TH1F', path=sc_hist_path,
                                        xbins = 700, xmin=0,xmax=700)
           cellMonGroup.defineHistogram('BCID,superCellEt_'+part+';h_SuperCellET_vs_BCID'+part,
                                        title='Super Cell ET [MeV] vs BCID '+partp+'; BCID from train front; %',
                                        type='TH2F', path=sc_hist_path,
                                        xbins = 50, xmin=0,xmax=50,
                                        ybins = 100, ymin=-1000,ymax=1000)
           cellMonGroup.defineHistogram('BCID,superCellEtRef_'+part+';h_SuperCellRefET_vs_BCID'+part,
                                        title='Super Cell ET [MeV] vs BCID '+partp+'; BCID from train front; %',
                                        type='TH2F', path=sc_hist_path,
                                        xbins = 50, xmin=0,xmax=50,
                                        ybins = 100, ymin=-1000,ymax=1000)
        
           cellMonGroup.defineHistogram('resolution_'+part+';h_SuperCellResolution'+part,
                                        title='Super Cell reconstruction resolution '+partp+'; %; # entries',
                                        type='TH1F', path=sc_hist_path,
                                        xbins = 70, xmin=-20,xmax=120)
           cellMonGroup.defineHistogram('resolutionPass_'+part+';h_SuperCellResolutionPass'+part,
                                        title='Super Cell reconstruction resolution for BCIDed '+partp+'; %; # entries',
                                        type='TH1F', path=sc_hist_path,
                                        xbins = 70, xmin=-20,xmax=120)
           cellMonGroup.defineHistogram('superCellEt_'+part+',resolution_'+part+';h_SuperCellResolution_vs_ET'+part,
                                        title='Super Cell reconstruction resolution vs ET '+partp+'; [MeV]; %',
                                        type='TH2F', path=sc_hist_path,
                                        xbins = 100, xmin=0,xmax=50000,
                                        ybins = 70, ymin=-20,ymax=120)
           cellMonGroup.defineHistogram('superCellEta_'+part+',resolutionHET_'+part+';h_SuperCellResolution_vs_eta'+part,
                                        title='Super Cell reconstruction resolution vs #eta  '+partp+'; #eta; %',
                                        type='TH2F', path=sc_hist_path,
                                        xbins = 100, xmin=-5,xmax=5,
                                        ybins = 40, ymin=-20,ymax=20)
           cellMonGroup.defineHistogram('BCID,resolution_'+part+';h_SuperCellResolution_vs_BCID'+part,
                                        title='Super Cell reconstruction resolution vs BCID '+partp+'; BCID from train front; %',
                                        type='TH2F', path=sc_hist_path,
                                        xbins = 50, xmin=0,xmax=50,
                                        ybins = 80, ymin=-120,ymax=120)
        
           cellMonGroup.defineHistogram('superCellEtRef_'+part+',superCellEt_'+part+';h_SuperCellEtLin'+part,
                                        title='Super Cell E_T Linearity '+partp+'; Ref SC E_T [MeV]; SC E_T [MeV]',
                                        type='TH2F', path=sc_hist_path,
                                        xbins =  100,xmin=0,xmax=50000,
                                        ybins =  100,ymin=0,ymax=50000)
           cellMonGroup.defineHistogram('superCelltimeRef_'+part+',superCelltimeReco_'+part+';h_SuperCelltimeLin'+part,
                                        title='Super Cell time Linearity '+partp+'; Ref SC time [ns]; Reco SC time [ns]',
                                        type='TH2F', path=sc_hist_path,
                                        xbins = 100, xmin=-200,xmax=200,
                                        ybins = 100, ymin=-200,ymax=200)
           cellMonGroup.defineHistogram('superCellprovenanceRef_'+part+',superCellprovenance_'+part+';h_SuperCellprovenanceLin'+part,
                                        title='Super Cell provenance Linearity '+partp+'; Ref SC bitmask ; SC bitmask',
                                        type='TH2F', path=sc_hist_path,
                                        xbins = 17, xmin=0,xmax=680,
                                        ybins = 17, ymin=0,ymax=680)


           cellMonGroup.defineHistogram('cellEnergy_'+part+';CellEnergy_'+part,
                                        title='Cell Energy in ' +part+';Cell Energy [MeV];Cell Events',
                                        type='TH1F', path=sc_hist_path,
                                        xbins =  100,xmin=0,xmax=50000
                                        )
    LArSuperCellMonAlg.doDatabaseNoiseVsEtaPhi = True

    for part in LArSuperCellMonAlg.LayerNames:        
        
        cellMonGroup.defineHistogram('celleta_'+part+';NCellsActiveVsEta_'+part,
                                           title="No. of Active Cells in #eta for "+part+";cell #eta",
                                           type='TH1F', path=sc_hist_path,
                                           xbins = 100,xmin=-5,xmax=5
                                           )
        
        cellMonGroup.defineHistogram('cellphi_'+part+';NCellsActiveVsPhi_'+part,
                                           title="No. of Active Cells in #phi for "+part+";cell #phi",
                                           type='TH1F', path=sc_hist_path,
                                           xbins =  100,xmin=-5,xmax=5
                                           )

        cellMonGroup.defineHistogram('celleta_'+part+',cellphi_'+part+';DatabaseNoiseVsEtaPhi_'+part,
                                           title="Map of Noise Values from the Database vs (#eta,#phi) for "+part+";cell #eta;cell #phi",
                                           weight='cellnoisedb_'+part,
                                           cutmask='doDatabaseNoisePlot',
                                           type='TH2F', path="DatabaseNoise/", 
                                           xbins =  100,xmin=-5,xmax=5,
                                           ybins =  100,ymin=-5,ymax=5,
                                           merge='weightedAverage')
        


    return LArSuperCellMonAlg


if __name__=='__main__':

    # Setup logs
    from AthenaCommon.Constants import DEBUG
    from AthenaCommon.Constants import WARNING
    from AthenaConfiguration.Enums import LHCPeriod, BunchStructureSource
    from AthenaCommon.Logging import log
    log.setLevel(DEBUG)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    #from AthenaConfiguration.TestDefaults import defaultTestFiles
    #flags.Input.Files = defaultTestFiles.ESD
    # to test tier0 workflow:
    #flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/data15_13TeV.00278748.physics_ZeroBias.merge.RAW._lb0384._SFO-ALL._0001.1']
    #flags.Input.Files = ['../data22_13p6TeV/data22_13p6TeV.00432180.physics_Main.daq.RAW._lb0335._SFO-16._0001.data']
    #flags.Input.Files = ['/eos/atlas/atlastier0/daq/data22_13p6TeV/express_express/00432180/data22_13p6TeV.00432180.express_express.daq.RAW/data22_13p6TeV.00432180.express_express.daq.RAW._lb0374._SFO-12._0001.data']
    flags.Input.Files = ['/eos/atlas/atlastier0/daq/data22_13p6TeV/express_express/00439798/data22_13p6TeV.00439798.express_express.daq.RAW/data22_13p6TeV.00439798.express_express.daq.RAW._lb1085._SFO-16._0001.data']

    #flags.Calo.Cell.doPileupOffsetBCIDCorr=True
    flags.Output.HISTFileName = 'LArSuperCellMonOutput.root'
    flags.DQ.enableLumiAccess = True
    flags.DQ.useTrigger = False
    flags.DQ.Environment = 'tier0'
    #flags.DQ.Environment = 'online'
    flags.IOVDb.GlobalTag = "CONDBR2-ES1PA-2022-07"
    flags.Common.isOnline = True
    flags.GeoModel.Run=LHCPeriod.Run3
    flags.Exec.OutputLevel=WARNING
    flags.Beam.BunchStructureSource=BunchStructureSource.FILLPARAMS
    #flags.Beam.BunchStructureSource=BunchStructureSource.Lumi
    import sys
    flags.fillFromArgs(sys.argv[1:])
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    cfg = MainServicesCfg(flags)
    storeGateSvc = cfg.getService("StoreGateSvc")
    storeGateSvc.Dump=True

    # in case of tier0 workflow:
    #from CaloRec.CaloRecoConfig import CaloRecoCfg
    #cfg.merge(CaloRecoCfg(flags))

    #from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    #cfg.merge(PoolReadCfg(flags))

    if not flags.DQ.Environment == 'online':
       from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
       # FillParamsFolderKey = /TDAQ/OLC/LHC/FILLPARAMS
       cfg.merge(BunchCrossingCondAlgCfg(flags))

    cfg.merge(LArSuperCellMonConfig(flags)) 

    f=open("LArSuperCellMon.pkl","wb")
    cfg.store(f)
    f.close()

    cfg.run()
