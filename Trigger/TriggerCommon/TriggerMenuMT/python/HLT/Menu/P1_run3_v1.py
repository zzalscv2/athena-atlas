# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# P1_run3_v1.py menu containing monitoring chains used only at P1
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],
#from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp

from ..Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

from .Physics_pp_run3_v1 import (
    SingleMuonGroup,
    SinglePhotonGroup,
    SingleJetGroup,
    JetStreamersGroup,
    MinBiasGroup,
    ZeroBiasGroup,
    SupportGroup,
    SupportLegGroup,
    SupportPhIGroup,
    Topo2Group,
    LegacyTopoGroup,
)

from AthenaCommon.Logging import logging
log = logging.getLogger( __name__ )

def addCommonP1Signatures(chains):

    log.info('[setupMenu] Adding common P1 menu chains now')
    
    chainsP1 = ChainStore()

    chainsP1['Muon'] = [    
        # ATR-20650
        ChainProp(name='HLT_mu0_muoncalib_L1MU3V_EMPTY', stream=['Muon_Calibration'], groups=['PS:Online','RATE:Muon_Calibration','BW:Muon']),
        ChainProp(name='HLT_mu0_muoncalib_L1MU14FCH', stream=['Muon_Calibration'], groups=['PS:Online','RATE:Muon_Calibration','BW:Muon']),
    ]
    
    chainsP1['Egamma'] = [
        # ATR-21355 - cannot be moved to the calibSlice because they need to configure the photon/ sequence
        ChainProp(name='HLT_g12_loose_LArPEBHLT_L1EM10VH', stream=['LArCells'], groups=['PS:Online']+SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g20_loose_LArPEBHLT_L1EM15', stream=['LArCells'], groups=['PS:Online']+SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g40_loose_LArPEBHLT_L1EM22VHI', stream=['LArCells'], groups=['PS:Online']+SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g60_loose_LArPEBHLT_L1EM22VHI', stream=['LArCells'], groups=['PS:Online']+SinglePhotonGroup+SupportLegGroup),
        ChainProp(name='HLT_g80_loose_LArPEBHLT_L1EM22VHI', stream=['LArCells'], groups=['PS:Online']+SinglePhotonGroup+SupportLegGroup),
    ]

    chainsP1['Jet'] = [

        # ATR-21355 - cannot be moved to the calibSlice because they need to configure the photon/ sequence
        ChainProp(name='HLT_j25_LArPEBHLT_L1J15', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online']+SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j25f_LArPEBHLT_L1J15p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online']+SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j40_LArPEBHLT_L1J20', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online']+SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j75f_LArPEBHLT_L1J30p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online']+SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j140f_LArPEBHLT_L1J75p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online']+SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j165_LArPEBHLT_L1J100', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online']+SingleJetGroup+SupportLegGroup),
    ]

    chainsP1['Calib'] = [
        ChainProp(name='HLT_noalg_LArPEBCalib_L1RD0_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['LArPEB'], groups=['PS:Online','RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1RD0_BGRP11', l1SeedThresholds=['FSNOSEED'], stream=['LArPEB'], groups=['PS:Online','RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1RD2_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['LArPEB'], groups=['PS:Online','RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_Lvl1CaloPEB_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['L1CaloCalib'], groups=['PS:Online','RATE:Calibration','BW:Detector']),

        ChainProp(name='HLT_noalg_LArPEBCalib_L1J400_LAR', l1SeedThresholds=['FSNOSEED'], stream=['LArPEBDigitalTrigger'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1jJ500_LAR', l1SeedThresholds=['FSNOSEED'], stream=['LArPEBDigitalTrigger'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1LAR-ZEE', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup+LegacyTopoGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1LAR-ZEE-eEM', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportPhIGroup+Topo2Group),

        ChainProp(name='HLT_noalg_LATOMEPEB_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['LArPEBDigitalTrigger'], groups=['PS:Online','RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_LATOMEPEB_L1RD0_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['LArPEBDigitalTrigger'], groups=['PS:Online','RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_LATOMEPEB_L1RD0_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED'], stream=['LArPEBDigitalTrigger'], groups=['PS:Online','RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_LATOMEPEB_L1RD0_BGRP7', l1SeedThresholds=['FSNOSEED'], stream=['LArPEBDigitalTrigger'], groups=['PS:Online','RATE:Calibration','BW:Detector']),

        ChainProp(name='HLT_noalg_LArPEBCalib_L1EM10VH', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1EM15', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1EM22VHI', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1J15', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1J15p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1J20', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1J30p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1J75p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1J100', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),

        # LAr noise burst chains
        ChainProp(name='HLT_larnoiseburst_L1XE60', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_larnoiseburst_L1J75', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_larnoiseburst_L1J100', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_larnoiseburst_L1J40_XE50', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_larnoiseburst_L1J40_XE60', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_larnoiseburst_L1All', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['PS:Online','PS:NoHLTRepro','RATE:Calibration','BW:Detector']), # Temporary for testing, high CPU cost
        ChainProp(name='HLT_acceptedevts_larnoiseburst_L1All', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['PS:Online','RATE:DISCARD','BW:DISCARD']),

        # End of event chains for MET
        ChainProp(name='HLT_acceptedevts_metcalo_L1All', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['PS:Online','RATE:DISCARD', 'BW:DISCARD']),
        ChainProp(name='HLT_acceptedevts_mettrk_L1All', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['PS:Online','RATE:DISCARD', 'BW:DISCARD']),
        
        ## larpsall/em*FIRSTEMPTY
        ChainProp(name='HLT_larpsall_L1J12_FIRSTEMPTY', l1SeedThresholds=['J12'], stream=['LArNoiseBurst'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_larpsall_L1J30_FIRSTEMPTY', l1SeedThresholds=['J30'], stream=['LArNoiseBurst'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        
        ## larpsall/em*EMPTY
        ChainProp(name='HLT_larpsall_L1J12_EMPTY', l1SeedThresholds=['J12'], stream=['CosmicCalo'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_larpsall_L1J30_EMPTY', l1SeedThresholds=['J30'], stream=['CosmicCalo'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_larpsall_L1TAU8_EMPTY', l1SeedThresholds=['TAU8'], stream=['CosmicCalo'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_larpsall_L1J30p31ETA49_EMPTY', l1SeedThresholds=['J30p31ETA49'], stream=['CosmicCalo'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),

        # ATR-25019 Test the definition for the 'AFPCalib' stream
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online','RATE:Calibration','BW:Detector']),
        # ATR-25019 AFPCalib streamers
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_NSA_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_NSC_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSA_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSC_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSA_TOF_T0_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSA_TOF_T1_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSA_TOF_T2_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSA_TOF_T3_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSC_TOF_T0_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSC_TOF_T1_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSC_TOF_T2_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSC_TOF_T3_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_C', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A_AND_C_TOF_T0T1', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A_OR_C_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A_OR_C_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A_OR_C_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A_OR_C_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['PS:Online']+SupportGroup),

        #ATR-26256 PixelNoise Stream
        ChainProp(name='HLT_noalg_LumiPEB_L1RD0_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['PixelNoise'], groups=['PS:Online']+SupportGroup),

        #ATR-25327 Test the definition for the 'PixelBeam' and 'VdM' streams
        ChainProp(name='HLT_noalg_LumiPEB_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1RD0_BGRP11', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=['PS:Online']+SupportGroup),
        #ATR-25327 PixelBeam streamers
        ChainProp(name='HLT_noalg_LumiPEB_L1RD0_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_LumiPEB_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_LumiPEB_L1MBTS_1_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_LumiPEB_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_LumiPEB_L1MBTS_2_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=['PS:Online']+SupportGroup),
        #ATR-25327 VdM streamers
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1MBTS_1_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1MBTS_2_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1RD0_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=['PS:Online']+SupportGroup),
    ]

    chainsP1['Cosmic'] = [
        ChainProp(name='HLT_noalg_SCTPEB_L1RD0_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['SCTNoise'], groups=['PS:Online','RATE:SCTCalibration','BW:Detector']), # HLT_sct_noise
        ChainProp(name='HLT_noalg_laser_TilePEB_L1CALREQ2', l1SeedThresholds=['FSNOSEED'], stream=['Tile'], groups=['PS:Online','RATE:TileCalibration','BW:Detector']), # HLT_tilecalib_laser 
        ChainProp(name='HLT_noalg_CIS_TilePEB_L1CALREQ1', l1SeedThresholds=['FSNOSEED'], stream=['Tile'], groups=['PS:Online','RATE:TileCalibration','BW:Detector']), # HLT_tilecalib_CIS 
        ChainProp(name='HLT_cosmic_id_L1MU3V_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['IDCosmic'], groups=['PS:Online']+SupportGroup+SingleMuonGroup),
        ChainProp(name='HLT_cosmic_id_L1MU8VF_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['IDCosmic'], groups=['PS:Online']+SupportGroup+SingleMuonGroup),
    ]

    chainsP1['Streaming'] = [

        ChainProp(name='HLT_noalg_L1RD0_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['Background'], groups=['PS:Online','BW:Other']),  
        
        ChainProp(name='HLT_noalg_mb_L1RD0_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup),
        ChainProp(name='HLT_noalg_mb_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup),  

        # ID monitoring
        ChainProp(name='HLT_noalg_idmon_L1RD0_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['IDMonitoring','express'],groups=['PS:Online','RATE:Monitoring','BW:Detector']),
        ChainProp(name='HLT_noalg_idmon_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['IDMonitoring'],groups=['PS:Online','RATE:Monitoring','BW:Detector']),
        ChainProp(name='HLT_noalg_idmon_L1RD0_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['IDMonitoring'],groups=['PS:Online','RATE:Monitoring','BW:Detector']),

        # L1 combined streamers
        ChainProp(name='HLT_noalg_L1Bkg',      l1SeedThresholds=['FSNOSEED'], stream=['Background'], groups=['PS:Online','RATE:SeededStreamers', 'BW:Other']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1Standby',      l1SeedThresholds=['FSNOSEED'], stream=['Standby'], groups=['PS:Online','RATE:SeededStreamers', 'BW:Other']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1Calo',      l1SeedThresholds=['FSNOSEED'], stream=['L1Calo'], groups=['PS:Online','RATE:SeededStreamers', 'BW:Other']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1Calo_EMPTY',      l1SeedThresholds=['FSNOSEED'], stream=['L1Calo'], groups=['PS:Online','RATE:SeededStreamers', 'BW:Other']+SupportLegGroup),

        # muon streamers
        ChainProp(name='HLT_noalg_L1MU3V_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['Background'], groups=['PS:Online']+SingleMuonGroup),
        ChainProp(name='HLT_noalg_cosmicmuons_L1MU3V_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicMuons','express'],groups=['PS:Online','RATE:Cosmic_Muon','BW:Muon']),
        ChainProp(name='HLT_noalg_cosmicmuons_L1MU8VF_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicMuons','express'],groups=['PS:Online','RATE:Cosmic_Muon','BW:Muon']),
        ChainProp(name='HLT_noalg_bkg_L1MU3V_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['Background'],groups=['PS:Online','BW:Other']),
        ChainProp(name='HLT_noalg_bkg_L1MU8VF_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['Background'],groups=['PS:Online','BW:Other']),

        # L1 calo streamers
        ChainProp(name='HLT_noalg_l1calo_L1J400', l1SeedThresholds=['FSNOSEED'], stream=['L1Calo'], groups=['PS:Online']+JetStreamersGroup+['BW:Other']+SupportLegGroup),
        
        # Cosmic calo stream
        ChainProp(name='HLT_noalg_L1RD1_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['PS:Online','RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_L1J30p31ETA49_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J12_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J30_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J12_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1J30_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1RD0_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo'], groups=['PS:Online','RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_L1TAU8_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1RD0_BGRP7', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo'], groups=['PS:Online','RATE:Calibration','BW:Detector']),

        ChainProp(name='HLT_noalg_LArPEBNoise_L1J12_EMPTY',  l1SeedThresholds=['J12'], stream=['LArCellsEmpty'],groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1TAU8_EMPTY', l1SeedThresholds=['TAU8'], stream=['LArCellsEmpty'],groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1J30p31ETA49_EMPTY', l1SeedThresholds=['J30p31ETA49'], stream=['LArCellsEmpty'],groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        #                         
        ChainProp(name='HLT_noalg_LArPEBNoise_L1J12_FIRSTEMPTY',  l1SeedThresholds=['J12'],  stream=['LArCellsEmpty'],groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1J30_FIRSTEMPTY',  l1SeedThresholds=['J30'],  stream=['LArCellsEmpty'],groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        
        # TGC
        ChainProp(name='HLT_noalg_L1TGC_BURST',  l1SeedThresholds=['FSNOSEED'], stream=['TgcNoiseBurst'],groups=['PS:Online','RATE:Calibration','BW:Detector']),

#        ChainProp(name='HLT_noalg_L1RD1_BGRP10', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['PS:Online','RATE:Calibration','BW:Detector']),

        #ZeroBias
        ChainProp(name='HLT_noalg_L1ZB',        l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'],groups=['PS:Online', 'RATE:CPS_ZB']+ZeroBiasGroup+SupportLegGroup),# ATR-21367
        ChainProp(name='HLT_noalg_zb_L1RD1_EMPTY',        l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'],groups=['PS:Online']+ZeroBiasGroup+SupportGroup),# ATR-25032


        # MBTS
        ChainProp(name='HLT_noalg_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), 
        ChainProp(name='HLT_noalg_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), 
        ChainProp(name='HLT_noalg_L1MBTS_1_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), 
        ChainProp(name='HLT_noalg_L1MBTS_A', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1MBTS_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1MBTS_1_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-21740
        ChainProp(name='HLT_noalg_L1RD2_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), # ATR-21367
        ChainProp(name='HLT_noalg_L1MBTS_2_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-21999
        ChainProp(name='HLT_noalg_L1MBTS_1_1_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-21999
    ]

    chainsP1['MinBias'] =[
        # MBTS
        ChainProp(name='HLT_mb_mbts_all_L1MBTS_A', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], monGroups=['mbMon:online'], groups=['PS:Online']+MinBiasGroup),
        ChainProp(name='HLT_mb_mbts_all_L1MBTS_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], monGroups=['mbMon:online'], groups=['PS:Online']+MinBiasGroup),
        ChainProp(name='HLT_mb_mbts_all_L1MBTS_1_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], monGroups=['mbMon:online'], groups=['PS:Online']+MinBiasGroup), #ATR-21740
        ChainProp(name='HLT_mb_mbts_all_L1RD2_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], monGroups=['mbMon:online'], groups=['PS:Online']+MinBiasGroup), # ATR-21367
        ChainProp(name='HLT_mb_mbts_all_L1ZB',        l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'],monGroups=['mbMon:online'], groups=['PS:Online', 'RATE:CPS_ZB']+ZeroBiasGroup+SupportLegGroup),# ATR-21367
        ChainProp(name='HLT_mb_mbts_all_L1MBTS_2_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], monGroups=['mbMon:online'], groups=['PS:Online']+MinBiasGroup), #ATR-21999
        ChainProp(name='HLT_mb_mbts_all_L1MBTS_1_1_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], monGroups=['mbMon:online'], groups=['PS:Online']+MinBiasGroup), #ATR-21999

    ]

    chainsP1['Monitor'] = [
        ChainProp(name='HLT_timeburner_L1All', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['PS:Online','PS:NoHLTRepro','RATE:DISCARD','BW:DISCARD']),
        ChainProp(name='HLT_mistimemonj400_L1All', l1SeedThresholds=['FSNOSEED'], stream=['Mistimed'], groups=['PS:Online','RATE:Monitoring','BW:Other']),
    ]

    chainsP1['Beamspot'] = [
        ChainProp(name='HLT_beamspot_trkFS_trkfast_BeamSpotPEB_L1J15',  l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['PS:Online', 'RATE:BeamSpot',  'BW:BeamSpot']+SupportLegGroup),
    ]

    for sig in chainsP1:
        for chain in chainsP1[sig]:
            if 'Main' in chain.stream:
                log.error("chain %s in common P1 list with Main stream. Please move this to Physics menu file", chain.name)
                raise RuntimeError("Move %s chain to Physics menu file",chain.name)

    addP1Signatures(chains,chainsP1)

def addHighMuP1Signatures(chains):
    
    log.info('[setupMenu] Adding high-mu P1 menu chains now')
    
    chainsP1 = ChainStore()

    chainsP1['Egamma'] = [
        # ATR-21355 - cannot be moved to the calibSlice because they need to configure the photon/ sequence
        ChainProp(name='HLT_g3_loose_LArPEBHLT_L1EM3', stream=['LArCells'], groups=['PS:Online']+SinglePhotonGroup+SupportLegGroup),
    ]

    chainsP1['Calib'] = [
        # Phase I jet inputs ATR-24411, seed needs to be checked
        #ChainProp(name='HLT_larpsall_L1jJ40', l1SeedThresholds=['jJ40'], stream=['CosmicCalo'],groups=['PS:Online','Support:PhaseI','RATE:Calibration','BW:Detector']),

        # IDCalib Chains
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1J100', stream=['IDCalib'], groups=['PS:Online']+SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1XE50', stream=['IDCalib'], groups=['PS:Online']+SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']),
        #IDcalib for lower lumi
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1J30', stream=['IDCalib'], groups=['PS:Online']+SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1XE35', stream=['IDCalib'], groups=['PS:Online']+SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 

        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L14J15', stream=['IDCalib'], groups=['PS:Online']+SupportLegGroup+['RATE:Calibration','BW:Detector'],  l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L14J15', stream=['IDCalib'], groups=['PS:Online']+SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        #
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1jJ160',  stream=['IDCalib'], groups=['PS:Online']+SupportPhIGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1jXE100', stream=['IDCalib'], groups=['PS:Online']+SupportPhIGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L14jJ40',  stream=['IDCalib'], groups=['PS:Online']+SupportPhIGroup+['RATE:Calibration','BW:Detector'],  l1SeedThresholds=['FSNOSEED']),

        ChainProp(name='HLT_noalg_L1NSW_MONITOR', l1SeedThresholds=['FSNOSEED'], stream=['NSWTriggerMonitor'], groups=['PS:Online']+SupportGroup),

        # Lumi items for vdM programme
        ChainProp(name='HLT_noalg_LumiPEB_L1RD0_BGRP15', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1RD0_BGRP10', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1MBTS_2_BGRP11', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1MBTS_1_1_BGRP11', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1LUCID_A_BGRP11', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=['PS:Online']+SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1LUCID_C_BGRP11', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=['PS:Online']+SupportGroup),

        ChainProp(name='HLT_noalg_LArPEBCalib_L1EM3', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_larpsallem_L1EM3_EMPTY', l1SeedThresholds=['EM3'], stream=['CosmicCalo'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),

        ChainProp(name='HLT_larpsallem_L1EM7_FIRSTEMPTY', l1SeedThresholds=['EM7'], stream=['LArNoiseBurst'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_larpsallem_L1EM7_EMPTY', l1SeedThresholds=['EM7'], stream=['CosmicCalo'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1EM3_EMPTY',  l1SeedThresholds=['EM3'],  stream=['LArCellsEmpty'],groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1EM7_EMPTY',  l1SeedThresholds=['EM7'], stream=['LArCellsEmpty'],groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1EM7_FIRSTEMPTY',  l1SeedThresholds=['EM7'],  stream=['LArCellsEmpty'],groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
    ]

    chainsP1['Streaming'] = [
        ChainProp(name='HLT_noalg_L1EM3_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo'],groups=['PS:Online','RATE:Cosmic_Calo','BW:MinBias','RATE:Calibration']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1EM7_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
    ]

    # Random Seeded EB chains which select at the HLT based on L1 TBP bits
    chainsP1['EnhancedBias'] = [
        ChainProp(name='HLT_eb_low_L1RD2_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:Online']+ ["RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_eb_medium_L1RD2_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:Online']+ ["RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_L1PhysicsHigh_noPS', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:Online']+ ["RATE:EnhancedBias", "BW:Detector"]+SupportLegGroup ),
        ChainProp(name='HLT_noalg_L1PhysicsVeryHigh_noPS', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:Online']+ ["RATE:EnhancedBias", "BW:Detector"]+SupportLegGroup ),

        ChainProp(name='HLT_noalg_L1RD3_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:Online']+ ["RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_L1RD3_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:Online']+ ["RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_L1EMPTY_noPS', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:Online']+ ["RATE:EnhancedBias", "BW:Detector"]+SupportLegGroup ),
        ChainProp(name='HLT_noalg_L1FIRSTEMPTY_noPS', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:Online']+ ["RATE:EnhancedBias", "BW:Detector"]+SupportLegGroup ),
        ChainProp(name='HLT_noalg_L1UNPAIRED_ISO_noPS', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:Online']+ ["RATE:EnhancedBias", "BW:Detector"]+SupportLegGroup ),
        ChainProp(name='HLT_noalg_L1UNPAIRED_NONISO_noPS', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=['PS:Online']+ ["RATE:EnhancedBias", "BW:Detector"]+SupportLegGroup),
    ]

    chainsP1['Beamspot'] = [
        ChainProp(name='HLT_beamspot_trkFS_trkfast_BeamSpotPEB_L14J20',  l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['PS:Online','RATE:BeamSpot',  'BW:BeamSpot']+SupportLegGroup),

        ChainProp(name='HLT_j0_pf_ftf_preselj20_beamspotVtx_BeamSpotPEB_L1J15' , l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['PS:Online', 'RATE:BeamSpot',  'BW:BeamSpot', 'RATE:CPS_J15']+SupportLegGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselcHT450_beamspotVtx_BeamSpotPEB_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['PS:Online','RATE:BeamSpot',  'BW:BeamSpot']+SupportLegGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_pf_ftf_presel6c25_beamspotVtx_BeamSpotPEB_L14J15', l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['PS:Online','RATE:BeamSpot',  'BW:BeamSpot', 'RATE:CPS_4J15']+SupportLegGroup),
        ChainProp(name='HLT_j0_pf_ftf_presel2c20b85_beamspotVtx_BeamSpotPEB_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['PS:Online','PS:Online', 'RATE:BeamSpot',  'BW:BeamSpot']+SupportLegGroup),
    ]

    addP1Signatures(chains,chainsP1)

def addLowMuP1Signatures(chains):
    log.info('[setupMenu] Adding low-mu P1 menu chains now')
    
    chainsP1 = ChainStore()

    chainsP1['Calib'] = [
        # IDCalib Chains
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1J100', stream=['IDCalib'], groups=['PS:Online']+SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1XE50', stream=['IDCalib'], groups=['PS:Online']+SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']),
        #IDcalib for lower lumi
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1J30', stream=['IDCalib'], groups=['PS:Online']+SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1XE35', stream=['IDCalib'], groups=['PS:Online']+SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
    ]

    # Intentionally commented -- may be used specifically for low-mu MBTS validation
    # Probably only relevant in lowMu
    # chainsP1['Streaming'] = [
    #    ChainProp(name='HLT_noalg_L1MBTSA0', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA4', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA5', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA6', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA7', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA8', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA9', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA10', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA11', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA12', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA13', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA14', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA15', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
       
    #    ChainProp(name='HLT_noalg_L1MBTSC0', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC4', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC5', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC6', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC7', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC8', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC9', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC10', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC11', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC12', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC13', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC14', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC15', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['PS:Online']+MinBiasGroup), #ATR-23216
    # ]

    addP1Signatures(chains,chainsP1)

def addHeavyIonP1Signatures(chains):
    log.info('[setupMenu] Adding heavy-ion P1 menu chains now')    

    chainsP1 = ChainStore()

    chainsP1['Calib'] = [
        # Disabled for now, ZDC not available
        # # 'ZDCCalib' stream
        # ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_OR_LHCF', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['PS:Online','RATE:Calibration','BW:Detector']),

        # ALFA streamers for alignment (ATR-23602)
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_ANY',     l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_ELAST15', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_ELAST18', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_SYST17',  l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_SYST18',  l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        # ALFA single counters
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_B7L1U_OD_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_B7L1L_OD_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_A7L1U_OD_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_A7L1L_OD_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_A7R1U_OD_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_A7R1L_OD_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_B7R1U_OD_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_B7R1L_OD_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        # Upper/lower coincidence
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_B7L1_OD_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_A7L1_OD_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_B7R1_OD_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
        ChainProp(name='HLT_noalg_AlfaPEB_L1ALFA_A7R1_OD_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['ALFACalib'], groups=MinBiasGroup+SupportGroup),
    ]

    # ALFA Diffractive triggers
    chainsP1['MinBias'] = [
        ChainProp(name='HLT_noalg_L1ALFA_Diff_Phys', l1SeedThresholds=['FSNOSEED'], stream=["MinBias"], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1ALFA_CDiff_Phys', l1SeedThresholds=['FSNOSEED'], stream=["MinBias"], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_noalg_L1J12_ALFA_EINE', l1SeedThresholds=['FSNOSEED'], stream=["MinBias"], groups=['PS:NoHLTReprocessing']+MinBiasGroup),

        ChainProp(name='HLT_mb_sptrk_vetombts2in_L1ALFA_EINE', l1SeedThresholds=['FSNOSEED'], stream=["MinBias"], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_mb_sptrk_vetombts2in_L1ALFA_ANY', l1SeedThresholds=['FSNOSEED'], stream=["MinBias"], groups=['PS:NoHLTReprocessing']+MinBiasGroup),

        ChainProp(name='HLT_mb_sptrk_vetombts2in_L1TRT_ALFA_EINE', l1SeedThresholds=['FSNOSEED'], stream=["MinBias"], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
        ChainProp(name='HLT_mb_sptrk_vetombts2in_L1TRT_ALFA_ANY', l1SeedThresholds=['FSNOSEED'], stream=["MinBias"], groups=['PS:NoHLTReprocessing']+MinBiasGroup),
    ]

    addP1Signatures(chains,chainsP1)


def addCosmicP1Signatures(chains):
    log.info('[setupMenu] Adding cosmic P1 menu chains now')

    chainsP1 = ChainStore()

    chainsP1['Egamma'] = [
        # ATR-21355 - cannot be moved to the calibSlice because they need to configure the photon/ sequence
        ChainProp(name='HLT_g3_loose_LArPEBHLT_L1EM3', stream=['LArCells'], groups=['PS:Online']+SinglePhotonGroup+SupportLegGroup),
    ]

    chainsP1['Calib'] = [
        # IDCalib Chains
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1J100', stream=['IDCalib'], groups=['PS:Online']+SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1XE50', stream=['IDCalib'], groups=['PS:Online']+SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']),
        #IDcalib for lower lumi
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1J30', stream=['IDCalib'], groups=['PS:Online']+SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1XE35', stream=['IDCalib'], groups=['PS:Online']+SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 

        ChainProp(name='HLT_noalg_LArPEBCalib_L1EM3', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_larpsallem_L1EM3_EMPTY', l1SeedThresholds=['EM3'], stream=['CosmicCalo'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),

        ChainProp(name='HLT_larpsallem_L1EM7_FIRSTEMPTY', l1SeedThresholds=['EM7'], stream=['LArNoiseBurst'], groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_larpsallem_L1EM7_EMPTY', l1SeedThresholds=['EM7'], stream=['CosmicCalo'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1EM3_EMPTY',  l1SeedThresholds=['EM3'],  stream=['LArCellsEmpty'],groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1EM7_EMPTY',  l1SeedThresholds=['EM7'], stream=['LArCellsEmpty'],groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1EM7_FIRSTEMPTY',  l1SeedThresholds=['EM7'],  stream=['LArCellsEmpty'],groups=['PS:Online','RATE:Calibration','BW:Detector']+SupportLegGroup),
    ]

    chainsP1['Streaming'] = [
        ChainProp(name='HLT_noalg_L1EM3_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo'],groups=['PS:Online','RATE:Cosmic_Calo','BW:MinBias','RATE:Calibration']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1EM7_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['PS:Online','RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportLegGroup),
    ]

    addP1Signatures(chains,chainsP1)


def addP1Signatures(chains, chainsP1):

    for sig,chainsInSig in chainsP1.items():
        for chain in chainsInSig:
            if 'Main' in chain.stream:
                log.error("chain %s in P1 menu [%s] with Main stream. Please move this to PhysicsP1 menu file", chain.name, sig)
                raise RuntimeError("Move %s chain to PhysicsP1 menu file",chain.name)
            for group in chain.groups:
                if 'Primary' in group:
                    log.error("chain %s in P1 menu [%s] with Primary tag. Please move this to Physics menu file", chain.name, sig)
                    raise RuntimeError("Move %s chain to Physics menu file",chain.name)

    for sig,chainsInSig in chainsP1.items():
        chains[sig] += chainsInSig
