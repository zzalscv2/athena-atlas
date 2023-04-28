# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#
# Run this file in order to print out the empty slots

from TriggerMenuMT.L1.Base.L1MenuFlags import L1MenuFlags
from TriggerMenuMT.L1.Menu.MenuCommon import print_available, RequiredL1Items, FixedIDMap, defineCommonL1Flags

def defineMenu():

    defineCommonL1Flags(L1MenuFlags)

    L1MenuFlags.items = RequiredL1Items + [

        ##
        # single EM
        ##
        'L1_EM8VH', 'L1_EM10', 'L1_EM10VH', 'L1_EM12', 'L1_EM14', 'L1_EM15', 'L1_EM16', 'L1_EM18VH', 'L1_EM20VH', 'L1_EM20VHI', 'L1_EM22',
        'L1_EM22VHI',
        'L1_EM20VH_FIRSTEMPTY',
        # new calo
        'L1_eEM1', 'L1_eEM2',
        'L1_eEM5', 'L1_eEM9', 'L1_eEM18', 'L1_eEM15',
        'L1_eEM12L', 'L1_eEM18L', 'L1_eEM26', 'L1_eEM26M',

        ## 
        # MU
        ##
        'L1_MU3V', 'L1_MU5VF', 'L1_MU8F', 'L1_MU8VF', 'L1_MU14FCH', 'L1_MU14FCHR',
        'L1_MU3VF', 'L1_MU8FC', 'L1_MU15VFCH', 'L1_MU10BOM',
        'L1_2MU3V', 'L1_2MU5VF', 'L1_2MU8F', 'L1_MU8VF_2MU5VF', 'L1_MU5VF_2MU3V',
        'L1_3MU3V', 'L1_3MU5VF', 'L1_MU5VF_3MU3V', 'L1_4MU3V',
        'L1_2MU5VF_3MU3V', 'L1_2MU8VF',

        'L1_2MU14FCH_OVERLAY',
        'L1_MU3V_EMPTY', 'L1_MU5VF_EMPTY', 'L1_MU3V_FIRSTEMPTY', 'L1_MU8VF_EMPTY',
        'L1_MU3V_UNPAIRED_ISO',

        ##
        # combined lepton (e and mu)
        ##
        'L1_2EM10', 'L1_2EM15', 'L1_2EM16',
        'L1_2EM20VH',
        # new calo
        #'L1_2eEM7', 'L1_2eEM9', 'L1_2eEM15',
        'L1_2eEM18L',

        
        # combined mu - jet
        'L1_MU3V_J12','L1_MU3V_J15', 
        'L1_MU3V_jJ30', 'L1_MU3V_jJ40',

        'L1_TAU8', 'L1_TAU60', 'L1_TAU12IM', 'L1_TAU20IM',
        'L1_TAU8_EMPTY',
        # new calo
        'L1_eTAU12',


        # single jet
        'L1_J12','L1_J15','L1_J20','L1_J25', 'L1_J30', 'L1_J40', 'L1_J50' ,'L1_J75','L1_J85', 'L1_J100',
        'L1_J20p31ETA49', 'L1_J30p31ETA49', 'L1_J50p31ETA49', 'L1_J75p31ETA49', 'L1_J15p31ETA49',
        'L1_J12_EMPTY','L1_J12_FIRSTEMPTY', 'L1_J12_UNPAIRED_ISO', 'L1_J12_UNPAIRED_NONISO',
        'L1_J15p31ETA49_UNPAIRED_ISO',
        'L1_J30_EMPTY', 'L1_J30_FIRSTEMPTY', 'L1_J30p31ETA49_EMPTY', 'L1_J30p31ETA49_UNPAIRED_ISO', 'L1_J30p31ETA49_UNPAIRED_NONISO',
        'L1_J50_UNPAIRED_ISO', 'L1_J50_UNPAIRED_NONISO',
        'L1_J100_FIRSTEMPTY',
        'L1_J12_BGRP12',
        'L1_J400', 'L1_J400_LAR',
        # di-jet
        'L1_2J15',
        # new calo
        'L1_jJ500', 'L1_jJ500_LAR',
        'L1_jJ20', 'L1_jJ30',
        'L1_jJ40', 'L1_jJ50', 'L1_jJ55', 'L1_jJ60', 'L1_jJ80', 'L1_jJ90',
        'L1_jJ40p31ETA49', 'L1_jJ50p31ETA49', 'L1_jJ60p31ETA49', 'L1_jJ90p31ETA49', 'L1_jJ125p31ETA49',


        # XE
        'L1_XE35', 'L1_XE40', 'L1_XE45', 'L1_XE50', 
        'L1_XE55', 'L1_XE60', 'L1_XE30', 'L1_XE300',
       
        'L1_J40_XE50', 'L1_J40_XE60', 
 
         # calo
        'L1_TE3', 'L1_TE5', # also for HMT triggers
        'L1_TE20', 'L1_TE50',
        'L1_TE100', 'L1_TE200',
        'L1_TE10000', 'L1_TE12000',
        'L1_TE3p0ETA49', 'L1_TE7p0ETA49',
        'L1_TE600p0ETA49', 'L1_TE1500p0ETA49', 'L1_TE3000p0ETA49', 'L1_TE3500p0ETA49', 'L1_TE6500p0ETA49', 'L1_TE8000p0ETA49',
        'L1_TE50_VTE600p0ETA49',
        # calo overlay
        'L1_MBTS_1_VTE50_OVERLAY',
        'L1_TE50_OVERLAY', 'L1_TE600p0ETA49_OVERLAY', 'L1_TE1500p0ETA49_OVERLAY', 'L1_TE3000p0ETA49_OVERLAY',
        'L1_TE3500p0ETA49_OVERLAY', 'L1_TE6500p0ETA49_OVERLAY', 'L1_TE8000p0ETA49_OVERLAY',
        
        # new calo
        'L1_gTE200',
        'L1_jTE200',
        # additional jTE items for 2023 heavy ion runs
        'L1_jTE600',
        'L1_jTE1500',
        'L1_jTE3000',
        'L1_VjTE200',
        'L1_VjTE600',

        #L1 forward GAP
        'L1_GAP_A', 'L1_GAP_C', 'L1_GAP_AANDC',

        #UPC - MU
        'L1_MU3V_VTE50', 'L1_MU5VF_VTE50', 'L1_2MU3V_VTE50',
        
        #UPC - EM
        'L1_TAU1_TE3_VTE200', 'L1_TAU1_TE5_VTE200',
        'L1_TAU1_VTE200', 'L1_TAU1_VTE200_EMPTY',
        'L1_2TAU1_VTE200', 'L1_2TAU1_VTE50',
        'L1_EM7_VTE200',
        
        #UPC - new EM
        #'L1_eEM1_TE3_VgTE200', 'L1_2eEM1_VgTE50'
        
        #UPC - calo, MBTS, calo  
        'L1_ZDC_XOR_VTE200', 'L1_VZDC_A_VZDC_C_TE5_VTE200',
        'L1_ZDC_A_VZDC_C_VTE200', 'L1_ZDC_C_VZDC_A_VTE200',
        'L1_MBTS_1_ZDC_A_VZDC_C_VTE200', 'L1_MBTS_1_ZDC_C_VZDC_A_VTE200',
        'L1_TE3p0ETA49_ZDC_A_VZDC_C_VTE200', 'L1_TE3p0ETA49_ZDC_C_VZDC_A_VTE200', 'L1_TE5_ZDC_A_VZDC_C_VTE200','L1_TE5_ZDC_C_VZDC_A_VTE200','L1_TE20_ZDC_A_VZDC_C_VTE200', 'L1_TE20_ZDC_C_VZDC_A_VTE200', 
        #UPC - calo, MBTS - legacy
        'L1_MBTS_1_VTE50',
        'L1_MBTS_1_1_VTE50',
        'L1_MBTS_1_VTE200',
        #UPC - calo, TRT - legacy
        'L1_TRT_VTE50',
        'L1_TRT_VTE200',
        'L1_TRT_VTE20',
        #UPC - calo only - legacy
        'L1_VTE20',
        'L1_VTE50', 'L1_TE3_VTE50', 'L1_TE5_VTE50',
        'L1_TE5_VTE20',
        'L1_VTE200', 'L1_TE3_VTE200', 'L1_TE5_VTE200', 'L1_TE20_VTE200', 'L1_TE50_VTE200',
        'L1_J12_VTE200',
        
        
        #LUCID
        'L1_LUCID_A', 'L1_LUCID_C',

        # ZDC
        'L1_ZDC_A','L1_ZDC_C','L1_ZDC_A_C',
        'L1_ZDC_AND', 'L1_ZDC_XOR',
        'L1_ZDC_C_VZDC_A', 'L1_ZDC_A_VZDC_C',
        # ZDC and calo
        'L1_ZDC_A_C_VTE50',

        # Run3 ZDC items for heavy ion runs 
        # Commented out for more CTP space for 2022 Nov heavy ion test run (ATR-26405) 
        # They are needed for scheduled 2023 5 TeV pp and Pb+Pb runs, so not removed from the menu
        #'L1_VZDC_A_VZDC_C',
        #'L1_1TO4ZDC_A_VZDC_C',
        #'L1_VZDC_A_1TO4ZDC_C',
        #'L1_1TO4ZDC_A_1TO4ZDC_C',
        #'L1_5ZDC_A_VZDC_C',
        #'L1_VZDC_A_5ZDC_C',
        #'L1_ZDC_1TO4XOR5',
        #'L1_5ZDC_A_5ZDC_C',
        
        
        # VDM

        # ZDC bits and comb for debugging
        # Commented out for more CTP space for 2022 Nov heavy ion test run (ATR-26405) 
        # They are needed for scheduled 2023 5 TeV pp and Pb+Pb runs, so not removed from the menu
        #'L1_ZDC_BIT2',
        #'L1_ZDC_BIT1',
        #'L1_ZDC_BIT0',
        #'L1_ZDC_COMB0',
        #'L1_ZDC_COMB1',
        #'L1_ZDC_COMB2',
        #'L1_ZDC_COMB3',
        #'L1_ZDC_COMB4',
        #'L1_ZDC_COMB5',
        #'L1_ZDC_COMB6',
        #'L1_ZDC_COMB7',

        # ZDC items for LHCf+ZDC special run ATR-26051
        # Commented out for more CTP space for 2022 Nov heavy ion test run (ATR-26405) 
        # They are needed for scheduled 2023 5 TeV pp runs, so not removed from the menu
        #'L1_ZDC_OR'           ,
        #'L1_ZDC_XOR_E2'       ,
        #'L1_ZDC_XOR_E1_E3'    ,
        #'L1_ZDC_E1_AND_E1'    ,
        #'L1_ZDC_E1_AND_E2ORE3',
        #'L1_ZDC_E2_AND_E2'    ,
        #'L1_ZDC_E2_AND_E3'    ,
        #'L1_ZDC_E3_AND_E3'    ,
        #'L1_ZDC_A_AND_C'      ,
        #'L1_ZDC_OR_EMPTY', 'L1_ZDC_OR_UNPAIRED_ISO', 'L1_ZDC_OR_UNPAIRED_NONISO',
        #'L1_ZDC_OR_LHCF',

        # LHCF
        'L1_LHCF', 'L1_LHCF_UNPAIRED_ISO', 'L1_LHCF_EMPTY',

        # AFP
        #'L1_EM7_AFP_A_OR_C', 'L1_EM7_AFP_A_AND_C',
        'L1_MU5VF_AFP_A_OR_C', 'L1_MU5VF_AFP_A_AND_C',
        'L1_eEM9_AFP_A_OR_C','L1_eEM9_AFP_A_AND_C',

        'L1_AFP_A_OR_C_J12', 'L1_AFP_A_AND_C_J12',
        'L1_AFP_A_OR_C_jJ20', 'L1_AFP_A_AND_C_jJ20',
        'L1_AFP_A_OR_C_jJ30', 'L1_AFP_A_AND_C_jJ30',
     
        'L1_AFP_A_AND_C_TOF_J20', 'L1_AFP_A_AND_C_TOF_T0T1_J20', 
        'L1_AFP_A_AND_C_TOF_J30', 'L1_AFP_A_AND_C_TOF_T0T1_J30',
        'L1_AFP_A_AND_C_TOF_J50', 'L1_AFP_A_AND_C_TOF_T0T1_J50',
        'L1_AFP_A_AND_C_TOF_J75', 'L1_AFP_A_AND_C_TOF_T0T1_J75',

        'L1_AFP_A_AND_C_TOF_jJ50', 'L1_AFP_A_AND_C_TOF_T0T1_jJ50', 
        'L1_AFP_A_AND_C_TOF_jJ60', 'L1_AFP_A_AND_C_TOF_T0T1_jJ60',
        'L1_AFP_A_AND_C_TOF_jJ90', 'L1_AFP_A_AND_C_TOF_T0T1_jJ90', 
        'L1_AFP_A_AND_C_TOF_jJ125', 'L1_AFP_A_AND_C_TOF_T0T1_jJ125',

        'L1_AFP_A_OR_C', 'L1_AFP_A_AND_C', 'L1_AFP_A', 'L1_AFP_C', 'L1_AFP_A_AND_C_TOF_T0T1',
        'L1_AFP_FSA_BGRP12', 'L1_AFP_FSC_BGRP12', 'L1_AFP_NSA_BGRP12', 'L1_AFP_NSC_BGRP12',
        'L1_AFP_FSA_TOF_T0_BGRP12', 'L1_AFP_FSA_TOF_T1_BGRP12', 'L1_AFP_FSA_TOF_T2_BGRP12', 'L1_AFP_FSA_TOF_T3_BGRP12',
        'L1_AFP_FSC_TOF_T0_BGRP12', 'L1_AFP_FSC_TOF_T1_BGRP12', 'L1_AFP_FSC_TOF_T2_BGRP12', 'L1_AFP_FSC_TOF_T3_BGRP12',
        'L1_AFP_A_OR_C_UNPAIRED_ISO', 'L1_AFP_A_OR_C_UNPAIRED_NONISO',
        'L1_AFP_A_OR_C_EMPTY', 'L1_AFP_A_OR_C_FIRSTEMPTY',
        'L1_AFP_A_OR_C_MBTS_2', 'L1_AFP_A_AND_C_MBTS_2',
       

        # MBTS (ATR-24701)
        'L1_MBTS_1', 'L1_MBTS_1_1',  'L1_MBTS_2',
        'L1_MBTS_2_2', 'L1_MBTS_3_3',  'L1_MBTS_4_4',
        'L1_MBTS_1_EMPTY', 'L1_MBTS_1_1_EMPTY', 'L1_MBTS_2_EMPTY',
        #'L1_MBTS_1_UNPAIRED', 'L1_MBTS_2_UNPAIRED',
        'L1_MBTS_1_UNPAIRED_ISO', 'L1_MBTS_1_1_UNPAIRED_ISO', 'L1_MBTS_2_UNPAIRED_ISO',
        'L1_MBTS_A', 'L1_MBTS_C',
        # extra MBTS
        #'L1_MBTSA0', 'L1_MBTSA1', 'L1_MBTSA2', 'L1_MBTSA3', 'L1_MBTSA4', 'L1_MBTSA5', 'L1_MBTSA6', 'L1_MBTSA7', 'L1_MBTSA8', 'L1_MBTSA9', 'L1_MBTSA10', 'L1_MBTSA11', 'L1_MBTSA12', 'L1_MBTSA13', 'L1_MBTSA14', 'L1_MBTSA15',
        #'L1_MBTSC0', 'L1_MBTSC1', 'L1_MBTSC2', 'L1_MBTSC3', 'L1_MBTSC4', 'L1_MBTSC5', 'L1_MBTSC6', 'L1_MBTSC7', 'L1_MBTSC8', 'L1_MBTSC9', 'L1_MBTSC10', 'L1_MBTSC11', 'L1_MBTSC12', 'L1_MBTSC13', 'L1_MBTSC14', 'L1_MBTSC15',


        # L1 items for 2022 Nov. heavy ion test run, ATR-26405
        # Additionla peripheral physics L1 items
        'L1_VTE5', 
        'L1_MBTS_1_VTE5', 
        # Additioanl supporting itesm for BeamSpot, IDCalib
        'L1_J12_VTE100',
        'L1_J30_VTE200',
        'L1_J100_VTE200', # to be checked if J100 is too high
        'L1_XE35_VTE200',
        'L1_XE50_VTE200',
 

        #--------------------------------
        # TOPO items
        #--------------------------------

        'L1_LAR-ZEE', 'L1_LAR-ZEE-eEM',

        #ATR-17320
        'L1_CEP-CjJ100',
        'L1_CEP-CjJ90' ,

        #ATR-21371
        'L1_ALFA_ANY',
        'L1_ALFA_ELAST15', 'L1_ALFA_ELAST18',
        'L1_ALFA_B7L1U','L1_ALFA_B7L1L','L1_ALFA_A7L1U','L1_ALFA_A7L1L','L1_ALFA_A7R1U','L1_ALFA_A7R1L','L1_ALFA_B7R1U','L1_ALFA_B7R1L',
        'L1_ALFA_SYST9', 'L1_ALFA_SYST10', 'L1_ALFA_SYST11', 'L1_ALFA_SYST12', 'L1_ALFA_SYST17', 'L1_ALFA_SYST18',

        # For alignment (ATR-23602)
        # ALFA single counters
        'L1_ALFA_B7L1U_OD_BGRP12', 'L1_ALFA_B7L1L_OD_BGRP12', 'L1_ALFA_A7L1U_OD_BGRP12', 'L1_ALFA_A7L1L_OD_BGRP12',
        'L1_ALFA_A7R1U_OD_BGRP12', 'L1_ALFA_A7R1L_OD_BGRP12', 'L1_ALFA_B7R1U_OD_BGRP12', 'L1_ALFA_B7R1L_OD_BGRP12',
        # Upper/lower coincidence
        'L1_ALFA_B7L1_OD_BGRP12', 'L1_ALFA_A7L1_OD_BGRP12', 'L1_ALFA_B7R1_OD_BGRP12', 'L1_ALFA_A7R1_OD_BGRP12',

        # ATR-23602
        'L1_MBTS_1_A_ALFA_C', 'L1_MBTS_1_C_ALFA_A', 'L1_J12_ALFA_ANY', 'L1_MU3V_ALFA_ANY', 'L1_TE5_ALFA_ANY',  
        'L1_MU3V_ALFA_EINE', 'L1_J12_ALFA_EINE', 'L1_TE5_ALFA_EINE',
        'L1_EM3_ALFA_ANY', 'L1_EM3_ALFA_EINE', 'L1_2EM3_ALFA_EINE',
        # 'L1_DPHI-2eEM5_VTE5p24ETA49_ALFA_EINE',
        'L1_TRT_ALFA_ANY', 'L1_TRT_ALFA_EINE',
        'L1_J12_ALFA_ANY_UNPAIRED_ISO',
        ]



# Run this file as python python/L1/Menu_MC_HI_run3_v1.py to print out available IDs
    
    L1MenuFlags.CtpIdMap = FixedIDMap

if __name__ == "__main__":
    defineMenu()
    print_available(L1MenuFlags)
