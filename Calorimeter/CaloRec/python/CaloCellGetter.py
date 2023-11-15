# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# specifies Calo cell making
# so far only handle the RawChannel->CaloCell step
# not all possibility of CaloCellMaker_jobOptions.py integrated yet
from RecExConfig.Configured import Configured
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
from AthenaCommon.Resilience import treatException
from RecExConfig.RecFlags import rec
from AthenaCommon.AppMgr import ServiceMgr as svcMgr
import traceback

class CaloCellGetter (Configured)  :
    _outputType = "CaloCellContainer"
    _output = { _outputType : "AllCalo" }

        
    def configure(self):
        from AthenaCommon.Logging import logging
        mlog = logging.getLogger('CaloCellGetter::configure:')
        mlog.info ('entering')

        doStandardCellReconstruction = True
        from CaloRec.CaloCellFlags import jobproperties
        #from AthenaCommon.AppMgr import ToolSvc

        if not jobproperties.CaloCellFlags.doFastCaloSim.statusOn:
            doFastCaloSim = False
            mlog.info("doFastCaloSim not set, so not using it")
        else:
            doFastCaloSim = jobproperties.CaloCellFlags.doFastCaloSim()
            if doFastCaloSim:
                mlog.info("doFastCaloSim requested")
                doStandardCellReconstruction = False
                if jobproperties.CaloCellFlags.doFastCaloSimAddCells():
                    doStandardCellReconstruction = True
                    mlog.info("doFastCaloSimAddCells requested: FastCaloSim is added to fullsim calorimeter")
                else:    
                    mlog.info("doFastCaloSimAddCells not requested: Stand alone FastCaloSim is running")
            else:    
                mlog.info("doFastCaloSim explicitly not requested")



        # get handle to upstream object
        # handle tile

        if doStandardCellReconstruction:
            from LArROD.LArRODFlags import larRODFlags
            from AthenaCommon.GlobalFlags import globalflags
            if larRODFlags.readDigits() and globalflags.DataSource() == 'data':
                from LArROD.LArRawChannelBuilderDefault import LArRawChannelBuilderDefault
                LArRawChannelBuilderDefault()
          
        
        # writing of thinned digits
        if jobproperties.CaloCellFlags.doLArThinnedDigits.statusOn and jobproperties.CaloCellFlags.doLArThinnedDigits():
            from AthenaCommon.GlobalFlags import globalflags
            if globalflags.DataSource() == 'data':
                try:
                    from LArROD.LArDigits import DefaultLArDigitThinner
                    LArDigitThinner = DefaultLArDigitThinner('LArDigitThinner')  # automatically added to topSequence
                    LArDigitThinner.InputContainerName = "FREE"
                    LArDigitThinner.OutputContainerName = "LArDigitContainer_Thinned"
                except Exception:
                    treatException("Problem with LArDigitThinner ")

        # now configure the algorithm, part of this could be done in a separate class
        # cannot have same name
        try:        
            from CaloRec.CaloRecConf import CaloCellMaker                
        except Exception:
            mlog.error("could not import CaloRec.CaloCellMaker")
            print(traceback.format_exc())
            return False

        theCaloCellMaker = CaloCellMaker()
        self._CaloCellMakerHandle = theCaloCellMaker



        if doStandardCellReconstruction:
            # configure CaloCellMaker here
            # check LArCellMakerTool_jobOptions.py for full configurability 
            # FIXME 

            if rec.doLArg():
                from LArCabling.LArCablingAccess import LArOnOffIdMapping
                LArOnOffIdMapping()

                try:
                    from LArCellRec.LArCellRecConf import LArCellBuilderFromLArRawChannelTool
                    theLArCellBuilder = LArCellBuilderFromLArRawChannelTool()
                except Exception:
                    mlog.error("could not get handle to LArCellBuilderFromLArRawChannel Quit")
                    print(traceback.format_exc())
                    return False

                if jobproperties.CaloCellFlags.doLArCreateMissingCells():
                    theLArCellBuilder.addDeadOTX = True

                # add the tool to list of tool ( should use ToolHandle eventually) 
                theCaloCellMaker += theLArCellBuilder
                theCaloCellMaker.CaloCellMakerToolNames += [theLArCellBuilder]
            

            if rec.doTile():

                from AthenaCommon.GlobalFlags import globalflags
                if globalflags.DataSource() == 'data' and globalflags.InputFormat() == 'bytestream':
                    try:
                        svcMgr.ByteStreamCnvSvc.ROD2ROBmap = [ "-1" ]
                        if "TileDigitsContainer/TileDigitsCnt" not in svcMgr.ByteStreamAddressProviderSvc.TypeNames:
                            svcMgr.ByteStreamAddressProviderSvc.TypeNames+=["TileBeamElemContainer/TileBeamElemCnt",
                                                                            "TileDigitsContainer/TileDigitsCnt",
                                                                            "TileL2Container/TileL2Cnt",
                                                                            "TileLaserObject/TileLaserObj",
                                                                            "TileMuonReceiverContainer/TileMuRcvCnt" ]
                    except Exception:
                        mlog.warning("Cannot add TileDigitsContainer/TileDigitsCnt et al. to bytestream list")

                    # set options for TileRawChannelMaker
                    from TileRecUtils.TileRecFlags import jobproperties
                    jobproperties.TileRecFlags.TileRunType = 1  # physics run type

                    # reading of digits can be disabled before calling CaloCellGetter
                    # if this is not done, but digits are not available in BS file
                    # reading of digits is automatically disabled at start of run
                    if jobproperties.TileRecFlags.readDigits()                \
                        and not (jobproperties.TileRecFlags.doTileFlat        \
                                 or jobproperties.TileRecFlags.doTileFit      \
                                 or jobproperties.TileRecFlags.doTileFitCool  \
                                 or jobproperties.TileRecFlags.doTileOF1      \
                                 or jobproperties.TileRecFlags.doTileOpt2     \
                                 or jobproperties.TileRecFlags.doTileOptATLAS \
                                 or jobproperties.TileRecFlags.doTileMF):
                        
                        from AthenaCommon.BeamFlags import jobproperties
                        # run Opt filter with iterations by default, both for cosmics and collisions before 2011
                        # run Opt filter without iterations for collisions in 2011 and later 
                        if 'doTileOpt2' not in dir():
                            from RecExConfig.AutoConfiguration import GetRunNumber
                            rn = GetRunNumber()
                            if not athenaCommonFlags.isOnline() and rn > 0 and rn < 171194:
                                doTileOpt2 = True
                            elif jobproperties.Beam.beamType()=='collisions':
                                doTileOpt2 = False # use OF without iterations for collisions
                            else:
                                doTileOpt2 = True # always run OF with iterations for cosmics
                                    
                        # jobproperties.TileRecFlags.calibrateEnergy=True # use pCb for RawChannels
                        # please, note that time correction and best phase are used only for collisions
                        if doTileOpt2:
                            jobproperties.TileRecFlags.doTileOpt2 = True  # run optimal filter with iterations
                            jobproperties.TileRecFlags.doTileOptATLAS = False  # disable optimal filter without iterations
                            jobproperties.TileRecFlags.correctAmplitude = False  # don't do parabolic correction
                            if jobproperties.Beam.beamType() == 'collisions':
                                jobproperties.TileRecFlags.correctTime = True  # apply time correction in physics runs
                                jobproperties.TileRecFlags.BestPhaseFromCOOL = False  # best phase is not needed for iterations
                        else:
                            jobproperties.TileRecFlags.doTileOpt2 = False  # disable optimal filter with iterations
                            jobproperties.TileRecFlags.doTileOptATLAS = True  # run optimal filter without iterations
                            jobproperties.TileRecFlags.correctAmplitude = True  # apply parabolic correction
                            if jobproperties.Beam.beamType() == 'collisions':
                                jobproperties.TileRecFlags.correctTime = False  # don't need time correction if best phase is used
                                jobproperties.TileRecFlags.BestPhaseFromCOOL = True  # use best phase stored in DB

                    try:
                        from TileRecUtils.TileRawChannelGetter import TileRawChannelGetter
                        theTileRawChannelGetter = TileRawChannelGetter() # noqa: F841
                    except Exception:
                        mlog.error("could not load TileRawChannelGetter Quit")
                        print(traceback.format_exc())
                        return False

                    try:
                        from TileRecAlgs.TileRecAlgsConf import TileDigitsFilter
                        from AthenaCommon.AlgSequence import AlgSequence
                        topSequence = AlgSequence()
                        topSequence += TileDigitsFilter()
                    except Exception:
                        mlog.error("Could not configure TileDigitsFilter")

                from AthenaCommon.AlgSequence import AthSequencer
                condSequence = AthSequencer("AthCondSeq")
                checkDCS = hasattr(condSequence, 'TileDCSCondAlg')
                try:
                    from TileRecUtils.TileRecUtilsConf import TileCellBuilder
                    theTileCellBuilder = TileCellBuilder(CheckDCS = checkDCS)
                    from TileRecUtils.TileRecFlags import jobproperties
                    theTileCellBuilder.TileRawChannelContainer = jobproperties.TileRecFlags.TileRawChannelContainer()

                    if (jobproperties.TileRecFlags.noiseFilter() == 1 and jobproperties.TileRecFlags.readDigits()
                        and globalflags.DataSource() == 'data' and not globalflags.isOverlay()):
                        theTileCellBuilder.TileDSPRawChannelContainer = 'TileRawChannelCntCorrected'

                    rawChannelContainer = ''
                    if globalflags.DataSource() == 'data' and globalflags.InputFormat() == 'bytestream':
                        if jobproperties.TileRecFlags.readDigits():
                            # everything is already corrected at RawChannel level
                            theTileCellBuilder.correctTime = False
                            theTileCellBuilder.correctAmplitude = False
                        else:
                            rawChannelContainer = 'TileRawChannelCnt'
                            # by default parameters are tuned for opt.filter without iterations
                            theTileCellBuilder.correctTime = jobproperties.TileRecFlags.correctTime()
                            theTileCellBuilder.correctAmplitude = jobproperties.TileRecFlags.correctAmplitude()
                            theTileCellBuilder.AmpMinForAmpCorrection = jobproperties.TileRecFlags.AmpMinForAmpCorrection()
                            if jobproperties.TileRecFlags.TimeMaxForAmpCorrection() <= jobproperties.TileRecFlags.TimeMinForAmpCorrection() :
                                from AthenaCommon.BeamFlags import jobproperties
                                mlog.info("adjusting min/max time of parabolic correction for %s", jobproperties.Beam.bunchSpacing)
                                halfBS = jobproperties.Beam.bunchSpacing.get_Value()/2.
                                jobproperties.TileRecFlags.TimeMinForAmpCorrection = -halfBS
                                jobproperties.TileRecFlags.TimeMaxForAmpCorrection = halfBS
                            if jobproperties.TileRecFlags.TimeMaxForAmpCorrection() > jobproperties.TileRecFlags.TimeMinForAmpCorrection():
                                theTileCellBuilder.TimeMinForAmpCorrection = jobproperties.TileRecFlags.TimeMinForAmpCorrection()
                                theTileCellBuilder.TimeMaxForAmpCorrection = jobproperties.TileRecFlags.TimeMaxForAmpCorrection()

                    theCaloCellMaker += theTileCellBuilder
                    theCaloCellMaker.CaloCellMakerToolNames += [theTileCellBuilder]
                except Exception:
                    mlog.error("could not get handle to TileCellBuilder Quit")
                    print(traceback.format_exc())
                    return False
             

        if doFastCaloSim:
            mlog.info ('configuring FastCaloSim here')        
            
            try:
                from FastCaloSim.FastCaloSimConf import EmptyCellBuilderTool
                theEmptyCellBuilderTool=EmptyCellBuilderTool()
                theCaloCellMaker += theEmptyCellBuilderTool
                theCaloCellMaker.CaloCellMakerToolNames += [ theEmptyCellBuilderTool ]

                print(theEmptyCellBuilderTool)
                mlog.info("configure EmptyCellBuilderTool worked")
            except Exception:
                mlog.error("could not get handle to EmptyCellBuilderTool Quit")
                print(traceback.format_exc())
                return False
            

            try:
                from FastCaloSim.FastCaloSimFactory import FastCaloSimFactory
                theFastShowerCellBuilderTool=FastCaloSimFactory()

                theCaloCellMaker += theFastShowerCellBuilderTool
                theCaloCellMaker.CaloCellMakerToolNames += [ theFastShowerCellBuilderTool ]
                mlog.info("configure FastShowerCellBuilderTool worked")
            except Exception:
                mlog.error("could not get handle to FastShowerCellBuilderTool Quit")
                print(traceback.format_exc())
                return False


        #
        # CaloCellContainerFinalizerTool : closing container and setting up iterators
        #
    
        from CaloRec.CaloRecConf import CaloCellContainerFinalizerTool     
        theCaloCellContainerFinalizerTool=CaloCellContainerFinalizerTool()
        theCaloCellMaker += theCaloCellContainerFinalizerTool
        theCaloCellMaker.CaloCellMakerToolNames += [theCaloCellContainerFinalizerTool ]

        #
        # Mergeing of calo cellcontainer with sparse raw channel container with improved energies
        #

        doLArMerge = False
        if  globalflags.DataSource() == 'data'  and jobproperties.CaloCellFlags.doLArRawChannelMerge.statusOn and jobproperties.CaloCellFlags.doLArRawChannelMerge():
            from LArROD.LArRODFlags import larRODFlags
            if larRODFlags.readDigits() and larRODFlags.keepDSPRaw():
                doLArMerge = True
        if doLArMerge:
            try:
                from LArCellRec.LArCellRecConf import LArCellMerger
                theLArCellMerger = LArCellMerger()
            except Exception:
                mlog.error("could not get handle to LArCellMerge Quit")
                print(traceback.format_exc())
                return False
            theLArCellMerger.RawChannelsName = larRODFlags.RawChannelFromDigitsContainerName()
            theCaloCellMaker += theLArCellMerger
            theCaloCellMaker.CaloCellMakerToolNames += [theLArCellMerger]


        #
        # masking of noisy and sporadic noisy cells in LAr
        #

        doNoiseMask = False
        if jobproperties.CaloCellFlags.doLArNoiseMasking.statusOn and jobproperties.CaloCellFlags.doLArNoiseMasking():
            doNoiseMask = True
        doSporadicMask = False
        if jobproperties.CaloCellFlags.doLArSporadicMasking.statusOn and jobproperties.CaloCellFlags.doLArSporadicMasking():
            doSporadicMask = True
   
        if doNoiseMask or doSporadicMask :
            try:
                from LArCellRec.LArCellRecConf import LArCellNoiseMaskingTool
                theLArCellNoiseMaskingTool = LArCellNoiseMaskingTool()
            except Exception:
                mlog.error("could not get handle to LArCellNoiseMaskingTool Quit")
                print(traceback.format_exc())
                return False

            if doSporadicMask:
                theLArCellNoiseMaskingTool.SporadicProblemsToMask=["sporadicBurstNoise",]

            if doNoiseMask:
                theLArCellNoiseMaskingTool.ProblemsToMask=["highNoiseHG","highNoiseMG","highNoiseLG","deadReadout","deadPhys"]

            # quality cut for sporadic noise masking
            theLArCellNoiseMaskingTool.qualityCut=4000
            theCaloCellMaker += theLArCellNoiseMaskingTool
            theCaloCellMaker.CaloCellMakerToolNames += [theLArCellNoiseMaskingTool]

        # 
        #  masking of Feb problems
        #
        doBadFebMasking = False
        if jobproperties.CaloCellFlags.doLArBadFebMasking.statusOn and jobproperties.CaloCellFlags.doLArBadFebMasking():
            from AthenaCommon.GlobalFlags import globalflags
            if globalflags.DataSource() == 'data':
                doBadFebMasking = True

        if doBadFebMasking:
            try:
                from LArCellRec.LArCellRecConf import LArBadFebMaskingTool
                theLArBadFebMaskingTool = LArBadFebMaskingTool()
            except Exception:
                mlog.error("could not get handle to LArBadFebMaskingTool Quit")
                print(traceback.format_exc())
                return False
            theCaloCellMaker += theLArBadFebMaskingTool

            theCaloCellMaker.CaloCellMakerToolNames += [theLArBadFebMaskingTool]

        #
        #  emulate gain pathologies on MC
        #
        doGainPathology=False
        if jobproperties.CaloCellFlags.doLArCellGainPathology.statusOn and jobproperties.CaloCellFlags.doLArCellGainPathology():
            from AthenaCommon.GlobalFlags import globalflags
            if globalflags.DataSource() == 'geant4': 
                doGainPathology=True

        if doGainPathology:
            try:
                from LArCellRec.LArCellRecConf import LArCellGainPathology
                theLArCellGainPathology = LArCellGainPathology()
            except Exception:
                mlog.error("could not get handle to LArCellGainPatholog< Quit")
                print(traceback.format_exc())
                return False
            theCaloCellMaker += theLArCellGainPathology

            theCaloCellMaker.CaloCellMakerToolNames += [theLArCellGainPathology]




        # lar miscalibration if MC only  (should be done after finalisation)  

        if not jobproperties.CaloCellFlags.doLArCellEmMisCalib.statusOn:
            # the flag has not been set, so decide a reasonable default
            # this is the old global flags should use the new one as
            # soon as monitoring does 
            from AthenaCommon.GlobalFlags import globalflags
            if globalflags.DataSource() == 'data':
                doLArCellEmMisCalib = False
                mlog.info("jobproperties.CaloCellFlags.doLArMisCalib not set and real data: do not apply LArCellEmMisCalibTool")
            else:
                doLArCellEmMisCalib = True
                mlog.info("jobproperties.CaloCellFlags.doLArMisCalib not set and Monte Carlo: apply LArCellEmMisCalibTool")          
        else:
            doLArCellEmMisCalib=jobproperties.CaloCellFlags.doLArCellEmMisCalib()
            if doLArCellEmMisCalib:
                mlog.info("LArCellEmMisCalibTool requested")
            else:    
                mlog.info("LArCellEmMisCalibTool explicitly not requested")
                
        if doLArCellEmMisCalib:        
            try:
                from LArCellRec.LArCellRecConf import LArCellEmMiscalib
                theLArCellEmMiscalib = LArCellEmMiscalib("LArCellEmMiscalib")
            except Exception:
                mlog.error("could not get handle to LArCellEmMisCalib Quit")
                print(traceback.format_exc())
                return False

            # examples on how to change miscalibration. Default values are 0.005 and 0.007 
            #        theLArCellEmMiscalib.SigmaPerRegion = 0.005
            #        theLArCellEmMiscalib.SigmaPerCell = 0.005

            try:
                from CaloRec.CaloRecConf import CaloCellContainerCorrectorTool
                from CaloIdentifier import SUBCALO
                theMisCalibTool = CaloCellContainerCorrectorTool("MisCalibTool",
                        CaloNums=[ SUBCALO.LAREM ],
                        CellCorrectionToolNames=[ theLArCellEmMiscalib])
            except Exception:
                mlog.error("could not get handle to MisCalibTool Quit")
                print(traceback.format_exc())
                return False

            theCaloCellMaker+=theMisCalibTool
            theCaloCellMaker.CaloCellMakerToolNames += [theMisCalibTool]

        #
        # Pedestal shift correction
        #
        doPedestalCorr = False
        if jobproperties.CaloCellFlags.doPedestalCorr.statusOn:
           from AthenaCommon.GlobalFlags import globalflags
           if jobproperties.CaloCellFlags.doPedestalCorr() and (globalflags.DataSource() == 'data' or jobproperties.CaloCellFlags.doPileupOffsetBCIDCorr) : 
               doPedestalCorr = True
               mlog.info("Apply cell level pedestal shift correction")

        if doPedestalCorr:
            try:
                from CaloCellCorrection.CaloCellPedestalCorrDefault import CaloCellPedestalCorrDefault
                theCaloCellPedestalCorr = CaloCellPedestalCorrDefault()
                theCaloCellMaker += theCaloCellPedestalCorr
                theCaloCellMaker.CaloCellMakerToolNames += [theCaloCellPedestalCorr]
            except Exception:
                mlog.error("could not get handle to CaloCellPedestalCorr")
                print(traceback.format_exc())


        # 
        # HV correction for offline reprocessing, reading HV from Cool-DCS database
        #
        doHVCorr = False
        from AthenaCommon.DetFlags import DetFlags
        if DetFlags.dcs.LAr_on():
            if jobproperties.CaloCellFlags.doLArHVCorr.statusOn:
               from AthenaCommon.GlobalFlags import globalflags
               if jobproperties.CaloCellFlags.doLArHVCorr() and globalflags.DataSource() == 'data': 
                   doHVCorr = True
                   mlog.info("Redoing HV correction at cell level from COOL/DCS database")
       

        if doHVCorr:
            from LArCellRec.LArCellRecConf import LArCellContHVCorrTool
            theLArCellHVCorrTool = LArCellContHVCorrTool()
    
            #theCaloCellMaker += theHVCorrTool
            theCaloCellMaker.CaloCellMakerToolNames += [theLArCellHVCorrTool]

        #
        # Correction for dead cells, where we average the energy density of neighbor cells                     
        #
        doNeighborsAverage = False
        if jobproperties.CaloCellFlags.doDeadCellCorr.statusOn:
            if jobproperties.CaloCellFlags.doDeadCellCorr():
                doNeighborsAverage = True

        if doNeighborsAverage :
           try:
               from CaloCellCorrection.CaloCellCorrectionConf import CaloCellNeighborsAverageCorr
               theCaloCellNeighborsAverageCorr = CaloCellNeighborsAverageCorr("CaloCellNeighborsAverageCorr")
               theCaloCellNeighborsAverageCorr.testMode = False
           except Exception:
               mlog.error("could not get handle to  CaloCellNeighborsAverageCorr  Quit")
               print(traceback.format_exc())
               return False
           theCaloCellMaker +=  theCaloCellNeighborsAverageCorr
           theCaloCellMaker.CaloCellMakerToolNames += [theCaloCellNeighborsAverageCorr]


        # 
        # correction for missing Febs based on L1 readout
        doLArDeadOTXCorr = False
        if jobproperties.CaloCellFlags.doLArDeadOTXCorr.statusOn and jobproperties.CaloCellFlags.doLArCreateMissingCells.statusOn :
           if jobproperties.CaloCellFlags.doLArDeadOTXCorr() and jobproperties.CaloCellFlags.doLArCreateMissingCells() and doStandardCellReconstruction:
               if rec.doTrigger():
                   doLArDeadOTXCorr=True
               else:
                   if globalflags.DataSource.get_Value() != 'geant4': #warning only if not MC
                      mlog.warning("Trigger is switched off. Can't run deadOTX correction.")

        if doLArDeadOTXCorr:

            try:
                from LArCellRec.LArCellDeadOTXCorrToolDefault import LArCellDeadOTXCorrToolDefault
                theLArCellDeadOTXCorr = LArCellDeadOTXCorrToolDefault()
            except Exception:
                mlog.error("could not get handle to LArCellDeadOTXCorr Quit")
                print(traceback.format_exc())

            theCaloCellMaker += theLArCellDeadOTXCorr
            theCaloCellMaker.CaloCellMakerToolNames += [theLArCellDeadOTXCorr]

        if jobproperties.CaloCellFlags.doCaloCellEnergyCorr() and globalflags.DataSource() == 'data' and not athenaCommonFlags.isOnline():
            
            try:
                from CaloCellCorrection.CaloCellCorrectionConf import CaloCellEnergyRescaler
                theCCERescalerTool = CaloCellEnergyRescaler()
                theCCERescalerTool.Folder = "/LAR/CellCorrOfl/EnergyCorr"
                from IOVDbSvc.CondDB import conddb
                # conddb.addFolder("","/LAR/CellCorrOfl/EnergyCorr<tag>EnergyScale-00</tag><db>sqlite://;schema=escale.db;dbname=COMP200</db>")
                conddb.addFolder("LAR_OFL", "/LAR/CellCorrOfl/EnergyCorr",className="AthenaAttributeList")
                theCaloCellMaker += theCCERescalerTool
                theCaloCellMaker.CaloCellMakerToolNames += [theCCERescalerTool]
            except Exception:
                mlog.error("could not get handle to CaloCellEnergyRescaler Quit")
                print(traceback.format_exc())
                return False
            pass


        if jobproperties.CaloCellFlags.doCaloCellTimeCorr() and globalflags.DataSource() == 'data' and not athenaCommonFlags.isOnline():
            try:
                from CaloCellCorrection.CaloCellCorrectionConf import CaloCellTimeCorrTool
                theLArTimeCorr = CaloCellTimeCorrTool()
                theLArTimeCorr.Folder = "/LAR/TimeCorrectionOfl/CellTimeOffset"
                from IOVDbSvc.CondDB import conddb
                # conddb.addFolder("","/LAR/TimeCorrectionOfl/CellTimeOffset<tag>LARTimeCorrectionOflCellTimeOffset-empty</tag><db>sqlite://;schema=timecorr.db;dbname=COMP200</db>")
                conddb.addFolder("LAR_OFL", "/LAR/TimeCorrectionOfl/CellTimeOffset",className="AthenaAttributeList")
                theCaloCellMaker.CaloCellMakerToolNames += [theLArTimeCorr]
                
            except Exception:
                mlog.error("could not get handle to CaloCellTimeCorrTool Quit")
                print(traceback.format_exc())
                return False
  
            pass
    
        
        # make lots of checks (should not be necessary eventually)
        # to print the check add:

        from CaloRec.CaloRecConf import CaloCellContainerCheckerTool   
        theCaloCellContainerCheckerTool = CaloCellContainerCheckerTool()     
        # FIXME
        # theCaloCellContainerCheckerTool.OutputLevel=DEBUG

        theCaloCellMaker += theCaloCellContainerCheckerTool
        theCaloCellMaker.CaloCellMakerToolNames += [theCaloCellContainerCheckerTool] 


        #

        # sets output key  
        theCaloCellMaker.CaloCellsOutputName=self.outputKey()        


        # register output in objKeyStore
        from RecExConfig.ObjKeyStore import objKeyStore
        objKeyStore.addStreamESD(self.outputType(),self.outputKey())

        # Also note that we produce it as a transient output.
        objKeyStore.addTransient (self.outputType(),self.outputKey())

        from TileRecUtils.TileDQstatusAlgDefault import TileDQstatusAlgDefault
        TileDQstatusAlgDefault (TileRawChannelContainer = rawChannelContainer)
        
        # now add algorithm to topSequence
        # this should always come at the end

        mlog.info(" now adding CaloCellMaker to topSequence")        

        from AthenaCommon.AlgSequence import AlgSequence
        topSequence = AlgSequence()

        topSequence += theCaloCellMaker
        
        return True

    def CaloCellMakerHandle(self):
        return self._CaloCellMakerHandle

   
# would work only if one output object type
    def outputKey(self):
        return self._output[self._outputType]

    def outputType(self):
        return self._outputType



