# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from TrigEFMissingET.TrigEFMissingETConf import EFMissingET

from TrigEFMissingET.TrigEFMissingETConf import EFMissingETFromFEBHeader
from TrigEFMissingET.TrigEFMissingETConf import EFMissingETFromCells
from TrigEFMissingET.TrigEFMissingETConf import EFMissingETFromClusters
from TrigEFMissingET.TrigEFMissingETConf import EFMissingETFromClustersPS
from TrigEFMissingET.TrigEFMissingETConf import EFMissingETFlags
from TrigEFMissingET.TrigEFMissingETConf import EFMissingETFromHelper

# Import CaloNoiseToolDefault for EFMissingETFromCells
from CaloTools.CaloNoiseToolDefault import CaloNoiseToolDefault
theCaloNoiseTool=CaloNoiseToolDefault()
from AthenaCommon.AppMgr import ToolSvc
ToolSvc+=theCaloNoiseTool

from AthenaCommon.Constants import VERBOSE,DEBUG,INFO
from AthenaCommon.SystemOfUnits import GeV
from AthenaCommon.SystemOfUnits import nanosecond


class EFMissingETBase (EFMissingET):
    __slots__ = []
    def __init__(self, name):
        super( EFMissingETBase, self ).__init__(name)

        from TrigEFMissingET.TrigEFMissingETMonitoring import TrigEFMissingETValidationMonitoring, TrigEFMissingETOnlineMonitoring, TrigEFMissingETCosmicMonitoring
        validation = TrigEFMissingETValidationMonitoring()
        online = TrigEFMissingETOnlineMonitoring()
        cosmic = TrigEFMissingETCosmicMonitoring()

        from TrigTimeMonitor.TrigTimeHistToolConfig import TrigTimeHistToolConfig
        time = TrigTimeHistToolConfig("EFMissingET_Time")
        time.TimerHistLimits = [0, 250]

        self.AthenaMonTools = [ validation, online, cosmic, time]


##### loop over cells, no noise suppression #####
class EFMissingET_Fex_allCells (EFMissingETBase):
    __slots__ = []
    def __init__ (self, name="EFMissingET_Fex_allCells"):
        super(EFMissingET_Fex_allCells, self).__init__(name)

        # name of TrigMissingET object
        self.MissingETOutputKey = "TrigEFMissingET"

        # tools
        febTool =    EFMissingETFromFEBHeader("TheFEBTool")
        cellTool =   EFMissingETFromCells("TheCellTool")
        flagTool =   EFMissingETFlags("TheFlagsTool")
        helperTool = EFMissingETFromHelper("TheHelperTool")
        #
        febTool.ParentFexName = name
        cellTool.ParentFexName = name
        flagTool.ParentFexName = name
        helperTool.ParentFexName = name
        #
        cellTool.CaloNoiseTool=theCaloNoiseTool
        cellTool.DoCellNoiseSuppression = False

        #### settings for robustness checks:
        # |energy| thresholds to make cell-level robustness checks = 2 rms
        cellTool.MinCellSNratio = []
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0,5.0 ] # PreSamplB, EMB1, EMB2, EMB3
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0,5.0 ] # PreSamplE, EME1, EME2, EME3
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0,5.0 ] # HEC0, HEC1, HEC2, HEC3
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # TileBar0, TileBar1, TileBar2
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # TileGap0, TileGap1, TileGap2
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # TileExt0, TileExt1, TileExt2
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # FCalEM, FCalHad1, FCalHad2
        # fraction of energy deposited in EM samplings
        flagTool.MaxEMfraction = 1.0
        flagTool.MinEMfraction = 0.0
        flagTool.MaxTileGapEratio = 1.0
        # max/min energy ratios from each subdet.
        flagTool.MaxSumEratioInEMB = 1.0
        flagTool.MaxSumEratioInEME = 1.0
        flagTool.MaxSumEratioInHEC = 1.0
        flagTool.MaxSumEratioInTileBar = 1.0
        flagTool.MaxSumEratioInTileGap = 1.0
        flagTool.MaxSumEratioInTileExt = 1.0
        flagTool.MaxSumEratioInFCal = 1.0
        flagTool.MinSumEratioInEMB = 0.0
        flagTool.MinSumEratioInEME = 0.0
        flagTool.MinSumEratioInHEC = 0.0
        flagTool.MinSumEratioInTileBar = 0.0
        flagTool.MinSumEratioInTileGap = 0.0
        flagTool.MinSumEratioInTileExt = 0.0
        flagTool.MinSumEratioInTileExt = 0.0
        # max/min comp energies
        flagTool.MaxCompE = []
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCompE = []
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # FCalEM, FCalHad1, FCalHad2
        # max/min cell energies
        flagTool.MaxCellE = []
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCellE = []
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # FCalEM, FCalHad1, FCalHad2
        # max/min cell times
        flagTool.MaxCellTime = []
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCellTime = []
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # FCalEM, FCalHad1, FCalHad2
        # max cell chi-square
        flagTool.WorstCellQuality = []
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # PreSamplE, EME1, EME2, EME3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # HEC0, HEC1, HEC2, HEC3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileBar0, TileBar1, TileBar2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileGap0, TileGap1, TileGap2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileExt0, TileExt1, TileExt2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # FCalEM, FCalHad1, FCalHad2

        ## chain of tools
        self.Tools = []
#        self.Tools += [ febTool ]
        self.Tools += [ cellTool ]
        self.Tools += [ flagTool ]
        self.Tools += [ helperTool ]

        # component flags (-1 means skip)
        self.ComponentFlags = []
        self.ComponentFlags += [ 0,0,0,0 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentFlags += [ 0,0,0,0 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentFlags += [ 0,0,0,0 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentFlags += [ 0,0,0 ]   # TileBar0, TileBar1, TileBar2
        self.ComponentFlags += [ 0,0,0 ]   # TileGap0, TileGap1, TileGap2
        self.ComponentFlags += [ 0,0,0 ]   # TileExt0, TileExt1, TileExt2
        self.ComponentFlags += [ 0,0,0 ]   # FCalEM, FCalHad1, FCalHad2
        self.ComponentFlags += [ 0 ]       # EM Topo
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0 ]       # Had Topo
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0 ]       # Muons
        # calibration: constant term (MeV)
        self.ComponentCalib0 = []
        self.ComponentCalib0 += [ 0,0,0,0 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentCalib0 += [ 0,0,0,0 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentCalib0 += [ 0,0,0,0 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentCalib0 += [ 0,0,0 ]   # TileBar0, TileBar1, TileBar2
        self.ComponentCalib0 += [ 0,0,0 ]   # TileGap0, TileGap1, TileGap2
        self.ComponentCalib0 += [ 0,0,0 ]   # TileExt0, TileExt1, TileExt2
        self.ComponentCalib0 += [ 0,0,0 ]   # FCalEM, FCalHad1, FCalHad2
        self.ComponentCalib0 += [ 0 ]       # EM Topo
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0 ]       # Had Topo
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0 ]       # Muons
        # calibration: multiplicative constant
        self.ComponentCalib1 = []
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileBar0, TileBar1, TileBar2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileGap0, TileGap1, TileGap2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileExt0, TileExt1, TileExt2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # FCalEM, FCalHad1, FCalHad2
        self.ComponentCalib1 += [ 1.00 ]                # EM Topo
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00 ]                # Had Topo
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00 ]                # Muons


##### loop over cells with noise suppression #####
class EFMissingET_Fex_noiseSupp (EFMissingETBase):
    __slots__ = []
    def __init__ (self, name="EFMissingET_Fex_noiseSupp"):
        super(EFMissingET_Fex_noiseSupp, self).__init__(name)

        # name of TrigMissingET object
        #self.MissingETOutputKey = "TrigEFMissingET_noiseSupp"
        # Use the following if not run together with 2-sided
        self.MissingETOutputKey = "TrigEFMissingET"

        # tools
        febTool =    EFMissingETFromFEBHeader("TheFEBTool")
        cellTool =   EFMissingETFromCells("TheCellTool")
        flagTool =   EFMissingETFlags("TheFlagsTool")
        helperTool = EFMissingETFromHelper("TheHelperTool")
        #
        febTool.ParentFexName = name
        cellTool.ParentFexName = name
        flagTool.ParentFexName = name
        helperTool.ParentFexName = name
        #
        cellTool.CaloNoiseTool=theCaloNoiseTool
        cellTool.DoCellNoiseSuppression = True

        #### settings for robustness checks:
        # |energy| thresholds to make cell-level robustness checks = 2 rms
        cellTool.MinCellSNratio = []
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0,5.0 ] # PreSamplB, EMB1, EMB2, EMB3
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0,5.0 ] # PreSamplE, EME1, EME2, EME3
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0,5.0 ] # HEC0, HEC1, HEC2, HEC3
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # TileBar0, TileBar1, TileBar2
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # TileGap0, TileGap1, TileGap2
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # TileExt0, TileExt1, TileExt2
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # FCalEM, FCalHad1, FCalHad2
        # fraction of energy deposited in EM samplings
        flagTool.MaxEMfraction = 1.0
        flagTool.MinEMfraction = 0.0
        flagTool.MaxTileGapEratio = 1.0
        # max/min energy ratios from each subdet.
        flagTool.MaxSumEratioInEMB = 1.0
        flagTool.MaxSumEratioInEME = 1.0
        flagTool.MaxSumEratioInHEC = 1.0
        flagTool.MaxSumEratioInTileBar = 1.0
        flagTool.MaxSumEratioInTileGap = 1.0
        flagTool.MaxSumEratioInTileExt = 1.0
        flagTool.MaxSumEratioInFCal = 1.0
        flagTool.MinSumEratioInEMB = 0.0
        flagTool.MinSumEratioInEME = 0.0
        flagTool.MinSumEratioInHEC = 0.0
        flagTool.MinSumEratioInTileBar = 0.0
        flagTool.MinSumEratioInTileGap = 0.0
        flagTool.MinSumEratioInTileExt = 0.0
        flagTool.MinSumEratioInTileExt = 0.0
        # max/min comp energies
        flagTool.MaxCompE = []
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCompE = []
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # FCalEM, FCalHad1, FCalHad2
        # max/min cell energies
        flagTool.MaxCellE = []
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCellE = []
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # FCalEM, FCalHad1, FCalHad2
        # max/min cell times
        flagTool.MaxCellTime = []
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCellTime = []
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # FCalEM, FCalHad1, FCalHad2
        # max cell chi-square
        flagTool.WorstCellQuality = []
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # PreSamplE, EME1, EME2, EME3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # HEC0, HEC1, HEC2, HEC3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileBar0, TileBar1, TileBar2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileGap0, TileGap1, TileGap2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileExt0, TileExt1, TileExt2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # FCalEM, FCalHad1, FCalHad2

        ## chain of tools
        self.Tools = []
#        self.Tools += [ febTool ]
        self.Tools += [ cellTool ]
        self.Tools += [ flagTool ]
        self.Tools += [ helperTool ]

        # component flags (-1 means skip)
        self.ComponentFlags = []
        self.ComponentFlags += [ 0,0,0,0 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentFlags += [ 0,0,0,0 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentFlags += [ 0,0,0,0 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentFlags += [ 0,0,0 ]   # TileBar0, TileBar1, TileBar2
        self.ComponentFlags += [ 0,0,0 ]   # TileGap0, TileGap1, TileGap2
        self.ComponentFlags += [ 0,0,0 ]   # TileExt0, TileExt1, TileExt2
        self.ComponentFlags += [ 0,0,0 ]   # FCalEM, FCalHad1, FCalHad2
        self.ComponentFlags += [ 0 ]       # EM Topo
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0 ]       # Had Topo
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0 ]       # Muons
        # calibration: constant term (MeV)
        self.ComponentCalib0 = []
        self.ComponentCalib0 += [ 0,0,0,0 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentCalib0 += [ 0,0,0,0 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentCalib0 += [ 0,0,0,0 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentCalib0 += [ 0,0,0 ]   # TileBar0, TileBar1, TileBar2
        self.ComponentCalib0 += [ 0,0,0 ]   # TileGap0, TileGap1, TileGap2
        self.ComponentCalib0 += [ 0,0,0 ]   # TileExt0, TileExt1, TileExt2
        self.ComponentCalib0 += [ 0,0,0 ]   # FCalEM, FCalHad1, FCalHad2
        self.ComponentCalib0 += [ 0 ]       # EM Topo
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0 ]       # Had Topo
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0 ]       # Muons
        # calibration: multiplicative constant
        self.ComponentCalib1 = []
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileBar0, TileBar1, TileBar2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileGap0, TileGap1, TileGap2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileExt0, TileExt1, TileExt2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # FCalEM, FCalHad1, FCalHad2
        self.ComponentCalib1 += [ 1.00 ]                # EM Topo
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00 ]                # Had Topo
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00 ]                # Muons

##### loop over cells with noise suppression #####
class EFMissingET_Fex_2sidednoiseSupp (EFMissingETBase):
    __slots__ = []
    def __init__ (self, name="EFMissingET_Fex_2sidednoiseSupp"):
        super(EFMissingET_Fex_2sidednoiseSupp, self).__init__(name)

        # name of TrigMissingET object
        self.MissingETOutputKey = "TrigEFMissingET"

        # tools
        febTool =    EFMissingETFromFEBHeader("TheFEBTool")
        cellTool =   EFMissingETFromCells("TheCellTool")
        flagTool =   EFMissingETFlags("TheFlagsTool")
        helperTool = EFMissingETFromHelper("TheHelperTool")
        #
        febTool.ParentFexName = name
        cellTool.ParentFexName = name
        flagTool.ParentFexName = name
        helperTool.ParentFexName = name
        #
        cellTool.CaloNoiseTool=theCaloNoiseTool
        cellTool.DoCellNoiseSuppression = True
        cellTool.CaloNoiseOneSidedCut = -5.0

        #### settings for robustness checks:
        # |energy| thresholds to make cell-level robustness checks = 2 rms
        cellTool.MinCellSNratio = []
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0,5.0 ] # PreSamplB, EMB1, EMB2, EMB3
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0,5.0 ] # PreSamplE, EME1, EME2, EME3
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0,5.0 ] # HEC0, HEC1, HEC2, HEC3
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # TileBar0, TileBar1, TileBar2
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # TileGap0, TileGap1, TileGap2
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # TileExt0, TileExt1, TileExt2
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # FCalEM, FCalHad1, FCalHad2
        # fraction of energy deposited in EM samplings
        flagTool.MaxEMfraction = 1.0
        flagTool.MinEMfraction = 0.0
        flagTool.MaxTileGapEratio = 1.0
        # max/min energy ratios from each subdet.
        flagTool.MaxSumEratioInEMB = 1.0
        flagTool.MaxSumEratioInEME = 1.0
        flagTool.MaxSumEratioInHEC = 1.0
        flagTool.MaxSumEratioInTileBar = 1.0
        flagTool.MaxSumEratioInTileGap = 1.0
        flagTool.MaxSumEratioInTileExt = 1.0
        flagTool.MaxSumEratioInFCal = 1.0
        flagTool.MinSumEratioInEMB = 0.0
        flagTool.MinSumEratioInEME = 0.0
        flagTool.MinSumEratioInHEC = 0.0
        flagTool.MinSumEratioInTileBar = 0.0
        flagTool.MinSumEratioInTileGap = 0.0
        flagTool.MinSumEratioInTileExt = 0.0
        flagTool.MinSumEratioInTileExt = 0.0
        # max/min comp energies
        flagTool.MaxCompE = []
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCompE = []
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # FCalEM, FCalHad1, FCalHad2
        # max/min cell energies
        flagTool.MaxCellE = []
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCellE = []
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # FCalEM, FCalHad1, FCalHad2
        # max/min cell times
        flagTool.MaxCellTime = []
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCellTime = []
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # FCalEM, FCalHad1, FCalHad2
        # max cell chi-square
        flagTool.WorstCellQuality = []
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # PreSamplE, EME1, EME2, EME3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # HEC0, HEC1, HEC2, HEC3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileBar0, TileBar1, TileBar2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileGap0, TileGap1, TileGap2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileExt0, TileExt1, TileExt2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # FCalEM, FCalHad1, FCalHad2

        ## chain of tools
        self.Tools = []
#        self.Tools += [ febTool ]
        self.Tools += [ cellTool ]
        self.Tools += [ flagTool ]
        self.Tools += [ helperTool ]

        # component flags (-1 means skip)
        self.ComponentFlags = []
        self.ComponentFlags += [ 0,0,0,0 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentFlags += [ 0,0,0,0 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentFlags += [ 0,0,0,0 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentFlags += [ 0,0,0 ]   # TileBar0, TileBar1, TileBar2
        self.ComponentFlags += [ 0,0,0 ]   # TileGap0, TileGap1, TileGap2
        self.ComponentFlags += [ 0,0,0 ]   # TileExt0, TileExt1, TileExt2
        self.ComponentFlags += [ 0,0,0 ]   # FCalEM, FCalHad1, FCalHad2
        self.ComponentFlags += [ 0 ]       # EM Topo
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0 ]       # Had Topo
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0 ]       # Muons
        # calibration: constant term (MeV)
        self.ComponentCalib0 = []
        self.ComponentCalib0 += [ 0,0,0,0 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentCalib0 += [ 0,0,0,0 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentCalib0 += [ 0,0,0,0 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentCalib0 += [ 0,0,0 ]   # TileBar0, TileBar1, TileBar2
        self.ComponentCalib0 += [ 0,0,0 ]   # TileGap0, TileGap1, TileGap2
        self.ComponentCalib0 += [ 0,0,0 ]   # TileExt0, TileExt1, TileExt2
        self.ComponentCalib0 += [ 0,0,0 ]   # FCalEM, FCalHad1, FCalHad2
        self.ComponentCalib0 += [ 0 ]       # EM Topo
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0 ]       # Had Topo
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0 ]       # Muons
        # calibration: multiplicative constant
        self.ComponentCalib1 = []
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileBar0, TileBar1, TileBar2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileGap0, TileGap1, TileGap2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileExt0, TileExt1, TileExt2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # FCalEM, FCalHad1, FCalHad2
        self.ComponentCalib1 += [ 1.00 ]                # EM Topo
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00 ]                # Had Topo
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00 ]                # Muons

##### loop over LAR FEBs and Tile cells (with noise suppression) #####
class EFMissingET_Fex_FEB (EFMissingETBase):
    __slots__ = []
    def __init__ (self, name="EFMissingET_Fex_FEB"):
        super(EFMissingET_Fex_FEB, self).__init__(name)

        # name of TrigMissingET object
        self.MissingETOutputKey = "TrigEFMissingET_FEB"

        # tools
        febTool =    EFMissingETFromFEBHeader("TheFEBTool")
        cellTool =   EFMissingETFromCells("TheCellTool")
        flagTool =   EFMissingETFlags("TheFlagsTool")
        helperTool = EFMissingETFromHelper("TheHelperTool")
        #
        febTool.ParentFexName = name
        cellTool.ParentFexName = name
        flagTool.ParentFexName = name
        helperTool.ParentFexName = name
        #
        cellTool.CaloNoiseTool=theCaloNoiseTool
        cellTool.DoCellNoiseSuppression = True

        #### settings for robustness checks:
        # |energy| thresholds to make cell-level robustness checks = 2 rms
        cellTool.MinCellSNratio = []
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0,5.0 ] # PreSamplB, EMB1, EMB2, EMB3
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0,5.0 ] # PreSamplE, EME1, EME2, EME3
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0,5.0 ] # HEC0, HEC1, HEC2, HEC3
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # TileBar0, TileBar1, TileBar2
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # TileGap0, TileGap1, TileGap2
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # TileExt0, TileExt1, TileExt2
        cellTool.MinCellSNratio += [ 5.0,5.0,5.0 ] # FCalEM, FCalHad1, FCalHad2
        # fraction of energy deposited in EM samplings
        flagTool.MaxEMfraction = 1.0
        flagTool.MinEMfraction = 0.0
        flagTool.MaxTileGapEratio = 1.0
        # max/min energy ratios from each subdet.
        flagTool.MaxSumEratioInEMB = 1.0
        flagTool.MaxSumEratioInEME = 1.0
        flagTool.MaxSumEratioInHEC = 1.0
        flagTool.MaxSumEratioInTileBar = 1.0
        flagTool.MaxSumEratioInTileGap = 1.0
        flagTool.MaxSumEratioInTileExt = 1.0
        flagTool.MaxSumEratioInFCal = 1.0
        flagTool.MinSumEratioInEMB = 0.0
        flagTool.MinSumEratioInEME = 0.0
        flagTool.MinSumEratioInHEC = 0.0
        flagTool.MinSumEratioInTileBar = 0.0
        flagTool.MinSumEratioInTileGap = 0.0
        flagTool.MinSumEratioInTileExt = 0.0
        flagTool.MinSumEratioInTileExt = 0.0
        # max/min comp energies
        flagTool.MaxCompE = []
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCompE = []
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # FCalEM, FCalHad1, FCalHad2
        # max/min cell energies
        flagTool.MaxCellE = []
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCellE = []
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # FCalEM, FCalHad1, FCalHad2
        # max/min cell times
        flagTool.MaxCellTime = []
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCellTime = []
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # FCalEM, FCalHad1, FCalHad2
        # max cell chi-square
        flagTool.WorstCellQuality = []
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # PreSamplE, EME1, EME2, EME3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # HEC0, HEC1, HEC2, HEC3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileBar0, TileBar1, TileBar2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileGap0, TileGap1, TileGap2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileExt0, TileExt1, TileExt2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # FCalEM, FCalHad1, FCalHad2

        ## chain of tools
        self.Tools = []
        self.Tools += [ febTool ]
        #self.Tools += [ cellTool ]
        self.Tools += [ flagTool ]
        self.Tools += [ helperTool ]

        # component flags (-1 means skip)
        self.ComponentFlags = []
        self.ComponentFlags += [ 0,0,0,0 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentFlags += [ 0,0,0,0 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentFlags += [ 0,0,0,0 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentFlags += [ 0,0,0 ]   # TileBar0, TileBar1, TileBar2
        self.ComponentFlags += [ 0,0,0 ]   # TileGap0, TileGap1, TileGap2
        self.ComponentFlags += [ 0,0,0 ]   # TileExt0, TileExt1, TileExt2
        self.ComponentFlags += [ 0,0,0 ]   # FCalEM, FCalHad1, FCalHad2
        self.ComponentFlags += [ 0 ]       # EM Topo
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0 ]       # Had Topo
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0 ]       # Muons
        # calibration: constant term (MeV)
        self.ComponentCalib0 = []
        self.ComponentCalib0 += [ 0,0,0,0 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentCalib0 += [ 0,0,0,0 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentCalib0 += [ 0,0,0,0 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentCalib0 += [ 0,0,0 ]   # TileBar0, TileBar1, TileBar2
        self.ComponentCalib0 += [ 0,0,0 ]   # TileGap0, TileGap1, TileGap2
        self.ComponentCalib0 += [ 0,0,0 ]   # TileExt0, TileExt1, TileExt2
        self.ComponentCalib0 += [ 0,0,0 ]   # FCalEM, FCalHad1, FCalHad2
        self.ComponentCalib0 += [ 0 ]       # EM Topo
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0 ]       # Had Topo
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0 ]       # Muons
        # calibration: multiplicative constant
        self.ComponentCalib1 = []
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileBar0, TileBar1, TileBar2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileGap0, TileGap1, TileGap2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileExt0, TileExt1, TileExt2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # FCalEM, FCalHad1, FCalHad2
        self.ComponentCalib1 += [ 1.00 ]                # EM Topo
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00 ]                # Had Topo
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00 ]                # Muons

##### Use topo. clusters for noise suppression #####
class EFMissingET_Fex_topoClusters (EFMissingETBase):
    __slots__ = []
    def __init__ (self, name="EFMissingET_Fex_topoClusters"):
        super(EFMissingET_Fex_topoClusters, self).__init__(name)

        # name of TrigMissingET object
        self.MissingETOutputKey = "TrigEFMissingET_topocl"
        self.doTopoClusters = True

        # tools
        clusterTool = EFMissingETFromClusters("TheClusterTool")
        clusterTool2 = EFMissingETFromClusters("TheClusterTool2")
        flagTool =   EFMissingETFlags("TheFlagsTool")
        helperTool = EFMissingETFromHelper("TheHelperTool")
        #
        clusterTool.ParentFexName = name
        clusterTool2.ParentFexName = name
        flagTool.ParentFexName = name
        helperTool.ParentFexName = name

        # Add uncalibrated clusters into permanent object?
        clusterTool2.SaveUncalibrated = True

        # fraction of energy deposited in EM samplings
        flagTool.MaxEMfraction = 1.0
        flagTool.MinEMfraction = 0.0
        flagTool.MaxTileGapEratio = 1.0
        # max/min energy ratios from each subdet.
        flagTool.MaxSumEratioInEMB = 1.0
        flagTool.MaxSumEratioInEME = 1.0
        flagTool.MaxSumEratioInHEC = 1.0
        flagTool.MaxSumEratioInTileBar = 1.0
        flagTool.MaxSumEratioInTileGap = 1.0
        flagTool.MaxSumEratioInTileExt = 1.0
        flagTool.MaxSumEratioInFCal = 1.0
        flagTool.MinSumEratioInEMB = 0.0
        flagTool.MinSumEratioInEME = 0.0
        flagTool.MinSumEratioInHEC = 0.0
        flagTool.MinSumEratioInTileBar = 0.0
        flagTool.MinSumEratioInTileGap = 0.0
        flagTool.MinSumEratioInTileExt = 0.0
        flagTool.MinSumEratioInTileExt = 0.0
        # max/min comp energies
        flagTool.MaxCompE = []
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCompE = []
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # FCalEM, FCalHad1, FCalHad2
        # max/min cell energies
        flagTool.MaxCellE = []
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCellE = []
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # FCalEM, FCalHad1, FCalHad2
        # max/min cell times
        flagTool.MaxCellTime = []
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCellTime = []
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # FCalEM, FCalHad1, FCalHad2
        # max cell chi-square
        flagTool.WorstCellQuality = []
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # PreSamplE, EME1, EME2, EME3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # HEC0, HEC1, HEC2, HEC3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileBar0, TileBar1, TileBar2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileGap0, TileGap1, TileGap2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileExt0, TileExt1, TileExt2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # FCalEM, FCalHad1, FCalHad2

        ## chain of tools
        self.Tools = []
        self.Tools += [ clusterTool ]
        self.Tools += [ clusterTool2 ]
        self.Tools += [ flagTool ]
        self.Tools += [ helperTool ]

        # component flags (-1 means skip)
        self.ComponentFlags = []
        self.ComponentFlags += [ 0,0,0,0 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentFlags += [ 0,0,0,0 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentFlags += [ 0,0,0,0 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentFlags += [ 0,0,0 ]   # TileBar0, TileBar1, TileBar2
        self.ComponentFlags += [ 0,0,0 ]   # TileGap0, TileGap1, TileGap2
        self.ComponentFlags += [ 0,0,0 ]   # TileExt0, TileExt1, TileExt2
        self.ComponentFlags += [ 0,0,0 ]   # FCalEM, FCalHad1, FCalHad2
        self.ComponentFlags += [ 0 ]       # EM Topo
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0 ]       # Had Topo
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0 ]       # Muons
        # calibration: constant term (MeV)
        self.ComponentCalib0 = []
        self.ComponentCalib0 += [ 0,0,0,0 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentCalib0 += [ 0,0,0,0 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentCalib0 += [ 0,0,0,0 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentCalib0 += [ 0,0,0 ]   # TileBar0, TileBar1, TileBar2
        self.ComponentCalib0 += [ 0,0,0 ]   # TileGap0, TileGap1, TileGap2
        self.ComponentCalib0 += [ 0,0,0 ]   # TileExt0, TileExt1, TileExt2
        self.ComponentCalib0 += [ 0,0,0 ]   # FCalEM, FCalHad1, FCalHad2
        self.ComponentCalib0 += [ 0 ]       # EM Topo
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0 ]       # Had Topo
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0 ]       # Muons
        # calibration: multiplicative constant
        self.ComponentCalib1 = []
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileBar0, TileBar1, TileBar2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileGap0, TileGap1, TileGap2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileExt0, TileExt1, TileExt2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # FCalEM, FCalHad1, FCalHad2
        self.ComponentCalib1 += [ 1.00 ]                # EM Topo
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00 ]                # Had Topo
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00 ]                # Muons


##### Use topo. clusters for noise suppression -- w/ pileup subtraction #####
class EFMissingET_Fex_topoClustersPS (EFMissingETBase):
    __slots__ = []
    def __init__ (self, name="EFMissingET_Fex_topoClustersPS"):
        super(EFMissingET_Fex_topoClustersPS, self).__init__(name)

        # name of TrigMissingET object
        self.MissingETOutputKey = "TrigEFMissingET_topocl_PS"
        self.doTopoClusters = True

        # tools
        clusterTool = EFMissingETFromClustersPS("TheClusterToolPS")
        clusterTool2 = EFMissingETFromClustersPS("TheClusterTool2PS")
        flagTool =   EFMissingETFlags("TheFlagsTool")
        helperTool = EFMissingETFromHelper("TheHelperTool")
        #
        clusterTool.ParentFexName = name
        clusterTool2.ParentFexName = name
        flagTool.ParentFexName = name
        helperTool.ParentFexName = name

        # ClusterTool options
        clusterTool.SubtractPileup = True
        clusterTool.PileupDebug = False
        clusterTool.PileupNumRings = 10
        clusterTool.PileupNumStdDev = 2.0

        # ClusterTool2 options
        clusterTool2.SubtractPileup = True
        clusterTool2.PileupDebug = False
        clusterTool2.PileupNumRings = 10
        clusterTool2.PileupNumStdDev = 2.0

        # Add uncalibrated clusters into permanent object?
        clusterTool2.SaveUncalibrated = True

        # fraction of energy deposited in EM samplings
        flagTool.MaxEMfraction = 1.0
        flagTool.MinEMfraction = 0.0
        flagTool.MaxTileGapEratio = 1.0
        # max/min energy ratios from each subdet.
        flagTool.MaxSumEratioInEMB = 1.0
        flagTool.MaxSumEratioInEME = 1.0
        flagTool.MaxSumEratioInHEC = 1.0
        flagTool.MaxSumEratioInTileBar = 1.0
        flagTool.MaxSumEratioInTileGap = 1.0
        flagTool.MaxSumEratioInTileExt = 1.0
        flagTool.MaxSumEratioInFCal = 1.0
        flagTool.MinSumEratioInEMB = 0.0
        flagTool.MinSumEratioInEME = 0.0
        flagTool.MinSumEratioInHEC = 0.0
        flagTool.MinSumEratioInTileBar = 0.0
        flagTool.MinSumEratioInTileGap = 0.0
        flagTool.MinSumEratioInTileExt = 0.0
        flagTool.MinSumEratioInTileExt = 0.0
        # max/min comp energies
        flagTool.MaxCompE = []
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCompE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCompE = []
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCompE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # FCalEM, FCalHad1, FCalHad2
        # max/min cell energies
        flagTool.MaxCellE = []
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV,1e4*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCellE += [ 1e4*GeV,1e4*GeV,1e4*GeV ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCellE = []
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV,0.0*GeV ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCellE += [ 0.0*GeV,0.0*GeV,0.0*GeV ] # FCalEM, FCalHad1, FCalHad2
        # max/min cell times
        flagTool.MaxCellTime = []
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # PreSamplE, EME1, EME2, EME3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond,+10*nanosecond ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileBar0, TileBar1, TileBar2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileGap0, TileGap1, TileGap2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # TileExt0, TileExt1, TileExt2
        flagTool.MaxCellTime += [ +10*nanosecond,+10*nanosecond,+10*nanosecond ] # FCalEM, FCalHad1, FCalHad2
        flagTool.MinCellTime = []
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # PreSamplE, EME1, EME2, EME3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond,-10*nanosecond ] # HEC0, HEC1, HEC2, HEC3
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileBar0, TileBar1, TileBar2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileGap0, TileGap1, TileGap2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # TileExt0, TileExt1, TileExt2
        flagTool.MinCellTime += [ -10*nanosecond,-10*nanosecond,-10*nanosecond ] # FCalEM, FCalHad1, FCalHad2
        # max cell chi-square
        flagTool.WorstCellQuality = []
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # PreSamplB, EMB1, EMB2, EMB3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # PreSamplE, EME1, EME2, EME3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3,4e3 ] # HEC0, HEC1, HEC2, HEC3
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileBar0, TileBar1, TileBar2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileGap0, TileGap1, TileGap2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # TileExt0, TileExt1, TileExt2
        flagTool.WorstCellQuality += [ 4e3,4e3,4e3 ] # FCalEM, FCalHad1, FCalHad2

        ## chain of tools
        self.Tools = []
        self.Tools += [ clusterTool ]
        self.Tools += [ clusterTool2 ]
        self.Tools += [ flagTool ]
        self.Tools += [ helperTool ]

        # component flags (-1 means skip)
        self.ComponentFlags = []
        self.ComponentFlags += [ 0,0,0,0 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentFlags += [ 0,0,0,0 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentFlags += [ 0,0,0,0 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentFlags += [ 0,0,0 ]   # TileBar0, TileBar1, TileBar2
        self.ComponentFlags += [ 0,0,0 ]   # TileGap0, TileGap1, TileGap2
        self.ComponentFlags += [ 0,0,0 ]   # TileExt0, TileExt1, TileExt2
        self.ComponentFlags += [ 0,0,0 ]   # FCalEM, FCalHad1, FCalHad2
        self.ComponentFlags += [ 0 ]       # EM Topo
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0 ]       # Had Topo
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0,0 ]
        self.ComponentFlags += [ 0 ]       # Muons
        # calibration: constant term (MeV)
        self.ComponentCalib0 = []
        self.ComponentCalib0 += [ 0,0,0,0 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentCalib0 += [ 0,0,0,0 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentCalib0 += [ 0,0,0,0 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentCalib0 += [ 0,0,0 ]   # TileBar0, TileBar1, TileBar2
        self.ComponentCalib0 += [ 0,0,0 ]   # TileGap0, TileGap1, TileGap2
        self.ComponentCalib0 += [ 0,0,0 ]   # TileExt0, TileExt1, TileExt2
        self.ComponentCalib0 += [ 0,0,0 ]   # FCalEM, FCalHad1, FCalHad2
        self.ComponentCalib0 += [ 0 ]       # EM Topo
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0 ]       # Had Topo
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0, 0 ]
        self.ComponentCalib0 += [ 0 ]       # Muons
        # calibration: multiplicative constant
        self.ComponentCalib1 = []
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # PreSamplB, EMB1, EMB2, EMB3
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # PreSamplE, EME1, EME2, EME3
        self.ComponentCalib1 += [ 1.00,1.00,1.00,1.00 ] # HEC0, HEC1, HEC2, HEC3
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileBar0, TileBar1, TileBar2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileGap0, TileGap1, TileGap2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # TileExt0, TileExt1, TileExt2
        self.ComponentCalib1 += [ 1.00,1.00,1.00 ]      # FCalEM, FCalHad1, FCalHad2
        self.ComponentCalib1 += [ 1.00 ]                # EM Topo
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00 ]                # Had Topo
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00,1.00 ]
        self.ComponentCalib1 += [ 1.00 ]                # Muons

##### THE DEFAULT FEX #####
class EFMissingET_Fex (EFMissingET_Fex_2sidednoiseSupp):
    __slots__ = []
    def __init__ (self, name="EFMissingET_Fex"):
        super(EFMissingET_Fex, self).__init__(name)

        # name of TrigMissingET object
        self.MissingETOutputKey = "TrigEFMissingET"
