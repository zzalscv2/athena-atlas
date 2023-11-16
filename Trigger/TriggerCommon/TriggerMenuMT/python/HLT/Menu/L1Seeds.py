# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

#######################################
# trigger type definitions
######################################
from TriggerMenuMT.L1.Config.TriggerTypeDef import TT
rpcout_type = TT.muon | TT.phys
rpcin_type  = TT.muon | TT.phys
cl_type     = TT.calo | TT.phys


calo_exceptions = set([])


#######################################################
# obtain the l1 items according to the the trigger type
# function taken originally from TriggerPythonConfig
#######################################################
def Lvl1ItemByTriggerType(l1object, triggertype_pattern, triggertype_bitmask):
    """
    The function returns those items where the items triggertype, after applying the mask, matches the pattern.
    With this one can also select items where a certain bit is disabled
    """
    if triggertype_pattern<0 or triggertype_pattern>0xFF:
        raise RuntimeError('TriggerPythonConfig.Lvl1ItemByTriggerType(triggertype_pattern,triggertype_bitmask) needs to be called with 0<=triggertype_pattern<=0xFF, ' + \
                               + 'but is called with triggertype_pattern=%i' % triggertype_pattern)
    if triggertype_bitmask<0 or triggertype_bitmask>0xFF:
        raise RuntimeError('TriggerPythonConfig.Lvl1ItemByTriggerType(triggertype_pattern,triggertype_bitmask) needs to be called with 0<=triggertype_bitmask<=0xFF, ' + \
                               + 'but is called with triggertype_bitmask=%i' % triggertype_bitmask)
    itemsForMenu = [item for item in l1object if l1object[item]['ctpid'] != -1]
    if not itemsForMenu:
        log.error('No item defined for the L1 Menu, the TriggerConfL1 object does not contain items')
    res = [item for item in itemsForMenu if ( (triggertype_bitmask & int(l1object[item]['triggerType'],2)) == triggertype_pattern)]
    return res

##############################
# define the various seeds
##############################
def getL1BackgroundSeed():
    return [
        'L1_BCM_AC_CA_BGRP12', 'L1_BCM_AC_CA_UNPAIRED_ISO',
        'L1_BCM_Wide', 'L1_BCM_Wide_BGRP12', 'L1_BCM_Wide_EMPTY', 'L1_BCM_Wide_UNPAIREDB1', 'L1_BCM_Wide_UNPAIREDB2',
        'L1_BCM_2A_CALIB', 'L1_BCM_2C_CALIB',
        'L1_BCM_2A_EMPTY', 'L1_BCM_2C_EMPTY',
        'L1_BCM_2A_UNPAIREDB1', 'L1_BCM_2C_UNPAIREDB1', 'L1_BCM_2A_UNPAIREDB2', 'L1_BCM_2C_UNPAIREDB2',
        'L1_BCM_2A_FIRSTINTRAIN', 'L1_BCM_2C_FIRSTINTRAIN',
        'L1_J12_UNPAIREDB1', 'L1_J12_UNPAIREDB2',
        'L1_J12_UNPAIRED_ISO', 'L1_J12_UNPAIRED_NONISO',
        'L1_J50_UNPAIRED_ISO', 'L1_J50_UNPAIRED_NONISO',
        'L1_J12_EMPTY', 'L1_J12_FIRSTEMPTY', 'L1_J12_BGRP12',
        'L1_J12_UNPAIREDB1', 'L1_J12_UNPAIREDB2',
        ]

##############################
def getL1StandbySeed(l1items):
    standby_seeds = [ x for x in l1items if \
        "_EMPTY" not in x and "CALREQ" not in x and "ZB" not in x and \
        "-" not in x and "CMU" not in x and "RD" not in x and \
        "BCM" not in x and "BGRP12" not in x
    ]
    return standby_seeds

##############################
def getL1TopoSeed(l1items):
    return [ x for x in l1items if "-" in x or "CMU" in x ]

##############################
def getL1CaloSeed(l1seed, l1object):
    if ('EMPTY' in l1seed):
        #l1calo_seeds = ','.join([ x for x in Lvl1ItemByTriggerType(l1object, cl_type, cl_type) \
        #                              if (x not in calo_exceptions)  ])
        l1calo_seeds = [ x for x in Lvl1ItemByTriggerType(l1object, cl_type, cl_type) \
                                      if ('EMPTY' in x and 'FIRSTEMPTY' not in x)  ]
    else:
        l1calo_seeds = [ x for x in Lvl1ItemByTriggerType(l1object, cl_type, cl_type) if ("EMPTY" not in x or "FIRSTEMPTY" in x) \
                                      and (x not in calo_exceptions)  ]
    return l1calo_seeds

##############################
def getL1TauSeed(l1items):
    tau_seeds = [ x for x in l1items if "TAU" in x and "-" not in x and  "EMPTY" not in x  and "ISO" not in x]
    return tau_seeds

##############################
def getL1BSSeed():
    return ['L1_J15','L1_3J15','L1_3J10','L1_4J10']

def getL1JetBS():
    return ['L1_J15','L1_3J15','L1_3J10','L1_4J10']

##############################
def getL1MuonSeed(l1seed, l1object):
    if ('EMPTY' in l1seed): # to only get MU*_EMPTY items
        muon_seeds_list = [ x for x in Lvl1ItemByTriggerType(l1object, rpcout_type, rpcout_type) if ('MU' in x and '_EMPTY' in x )  ]
        muon_seeds_list = list(set(muon_seeds_list))
        muon_seeds      = muon_seeds_list
    else: #this one does NOT include the EMPTY items
        muon_seeds_list = [ x for x in Lvl1ItemByTriggerType(l1object, rpcout_type, rpcout_type) if ('MU' in x and 'FIRSTEMPTY' in x )  ]
        muon_seeds_list += [ x for x in Lvl1ItemByTriggerType(l1object, rpcin_type, rpcin_type)  ]
        muon_seeds_list = list(set(muon_seeds_list))
        muon_seeds      = muon_seeds_list

    return muon_seeds

##############################
def getEBnoL1PSSeed(l1items, l1seedname):

    ebitem = l1seedname.strip('L1_').rstrip('_noPS')
    # All of these L1 items must be PS=1 for an EB campaign
    l1EBitems = {
        'PhysicsHigh':
        [
            'L1_MU5VF_3MU3V',
            'L1_eEM18L_MU8F','L1_eEM26M', 'L1_eEM26T', 'L1_eEM28M','L1_2eEM10L_MU8F', 'L1_2eEM18M',
            'L1_eEM24L_3eEM12L',
            'L1_EM15VHI_2TAU12IM_J25_3J12', 'L1_EM15VHI_2TAU12IM_XE35', 'L1_EM15VHI_2TAU12IM',
            'L1_MU8F_TAU12IM_J25_2J12','L1_MU8F_TAU12IM_XE35','L1_MU8F_TAU20IM',
            'L1_4J15', 'L1_jJ160', 'L1_XE50','L1_2J15_XE55',
            'L1_TAU60', 'L1_eTAU80', 'L1_eTAU140', 'L1_TAU20IM_2TAU12IM_J25_2J20_3J12','L1_TAU20IM_2TAU12IM_XE35','L1_TAU20IM_2J20_XE45',
            'L1_MU14FCH', 'L1_MU18VFCH', 'L1_MU8F_3J20', 'L1_MU8F_2J20', 'L1_MU10BOM',
            'L1_J40p0ETA25_2J15p31ETA49', 'L1_J75p31ETA49',
            'L1_3MU5VF','L1_MU8F_2J15_J20',
            'L1_J40p0ETA25_2J25_J20p31ETA49',
            'L1_2MU5VF_3MU3V','L1_MU8VF_2MU5VF',
            'L1_MJJ-500-NFF', 'L1_J45p0ETA21_3J15p0ETA25', 'L1_SC111-CJ15', 'L1_BPH-7M11-25DR99-2MU3VF',
            'L1_HT190-J15s5pETA21', 'L1_TAU20IM_2TAU12IM_4J12p0ETA25', 'L1_DR-TAU20ITAU12I-J25'
        ],
        'PhysicsVeryHigh':
        [
            'L1_XE300', 'L1_J400', 'L1_6J15'
        ],
        'EMPTY': 
        [
            'L1_J12_EMPTY', 'L1_MU8VF_EMPTY', 'L1_TAU8_EMPTY', 'L1_TAU40_EMPTY', 'L1_eEM9_EMPTY'
        ],
        'FIRSTEMPTY':
        [
            'L1_J12_FIRSTEMPTY', 'L1_TAU8_FIRSTEMPTY', 'L1_EM7_FIRSTEMPTY'
        ],
        'UNPAIRED_ISO':
        [
            'L1_J12_UNPAIRED_ISO', 'L1_J15p31ETA49_UNPAIRED_ISO',
            'L1_MU3V_UNPAIRED_ISO', 'L1_eEM9_UNPAIRED_ISO', 'L1_TAU8_UNPAIRED_ISO', 'L1_TAU40_UNPAIRED_ISO'
        ],
        'UNPAIRED_NONISO':
        [
            'L1_J12_UNPAIRED_NONISO',
        ],
        'UNPAIREDB1':
        [
            'L1_BCM_Wide_UNPAIREDB1', 'L1_BCM_2A_UNPAIREDB1', 'L1_BCM_2C_UNPAIREDB1'
        ],
        'UNPAIREDB2':
        [
            'L1_BCM_Wide_UNPAIREDB2', 'L1_BCM_2A_UNPAIREDB2', 'L1_BCM_2C_UNPAIREDB2'
        ],
        'ABORTGAPNOTCALIB': [] # No more items defined in this historical bunchgroup
    }[ebitem]

    return l1EBitems

#####################################
def getL1LowLumi():
    return [
        'L1_eEM24L',
        'L1_2EM10VH',
        'L1_2MU5VF', 'L1_3MU3V',
        'L1_eEM18L_MU8F',
        'L1_2eEM10L_MU8F',
        'L1_TAU60', 'L1_TAU20IM_2TAU12IM_J25_2J20_3J12',
        'L1_EM15HI_2TAU12IM_XE35', 'L1_MU8F_TAU12IM_XE35',
        'L1_TAU20_2TAU12_XE35', 'L1_TAU20IM_2TAU12IM_XE35',
        'L1_EM15HI_2TAU12IM', 'L1_EM15HI_2TAU12IM_J25_3J12',
        'L1_EM15HI_TAU40_2TAU15',
        'L1_MU8F_TAU12IM_J25_2J12',
        'L1_MU8F_TAU12IM',
        'L1_J75', 'L1_4J15',
        'L1_XE50', 'L1_3J25p0ETA23',
        'L1_3J40', 'L1_2J15_XE55',
        'L1_MU5VF_J40', 'L1_J75p31ETA49'
    ]
        
#####################################
def getL1BKeePrimary():

    return [
        'L1_JPSI-1M5-eEM9', 'L1_JPSI-1M5-eEM15',
        'L1_BPH-0M9-eEM9-eEM7_MU5VF',
        'L1_eEM24L_3eEM12L',
        'L1_EM18VHI_MJJ-300',
        'L1_eEM18L_MU8F',
        'L1_BPH-0M9-EM7-EM5_2MU3V',
        'L1_MU14FCH',
        'L1_MU8F_2MU5VF',
        'L1_MU8F_TAU20IM',
        'L1_MU8F_TAU12IM_3J12',
        'L1_XE50',
        'L1_TAU60_2TAU40',
        'L1_TAU40_2TAU12IM_XE40',
        'L1_EM15VHI_2TAU12IM_XE35',
        'L1_TAU25IM_2TAU20IM_2J25_3J20',
        'L1_TAU20IM_2TAU12IM_4J12p0ETA25',
        'L1_EM15VHI_2TAU12IM_4J12',
        'L1_DR-TAU20ITAU12I-J25',
        'L1_MJJ-700',
        'L1_MJJ-500-NFF',
        'L1_J85_3J30',
        'L1_J40p0ETA25_2J25_J20p31ETA49',
        'L1_J25p0ETA23_2J15p31ETA49',
        'L1_J100',
        'L1_4J15', 
        'L1_3J35p0ETA23',
        'L1_3J15p0ETA25_XE40',
        'L1_2eEM24L',
        'L1_eEM18','L1_2eEM18', 'L1_2eEM18M', 'L1_2eEM18L',
        'L1_eEM26M', 'L1_eEM26L',
        'L1_eEM28M',
        'L1_eEM24L_3eEM12L',
        'L1_eEM22M_jMJJ-300',
        'L1_eEM18L_MU8F',
        'L1_2eEM10L_MU8F',
        'L1_BPH-0M9-eEM9-eEM7_2MU3V',
        'L1_MU18VFCH',
        'L1_eTAU80_2eTAU60',
        'L1_jJ160'
    ]

#####################################
def getL1BKeePrescaled():

    return [
        'L1_LFV-MU5VF',
        'L1_BPH-2M9-0DR15-MU5VFMU3V',
        'L1_BPH-2M9-0DR15-2MU3V', 
        'L1_BPH-2M9-0DR15-2MU3V',
        'L1_BPH-0M9-EM7-EM5_MU5VF', 
        'L1_BPH-0DR3-EM7J15_MU5VF', 
        'L1_BPH-0DR3-EM7J15_2MU3V', 
        'L1_JPSI-1M5-EM7',
        'L1_JPSI-1M5-EM12',
        'L1_TAU60', 
        'L1_J50',
        'L1_J50_DETA20-J50J',
        'L1_J40',
        'L1_3J25p0ETA23', # exist in menu, but currently not used at HLT. We may drop as CTP output
        'L1_EM20VH_3J20', # exist in menu, but currently not used at HLT. We may drop as CTP output
        'L1_EM18VHI_3J20', #  exist in menu, but currently not used at HLT. We may drop as CTP output
        'L1_eTAU80',
        'L1_eEM26L',
        'L1_eEM18',
        'L1_2eEM18L'
    ]

#####################################
# assigned the seeds to the L1 names
#####################################

L1_multiseed_simple_getters = {
    'L1_J': getL1JetBS,
    'L1_Bkg': getL1BackgroundSeed,
    'L1_BS': getL1BSSeed,
    'L1_LowLumi': getL1LowLumi,
    'L1_BKeePrimary': getL1BKeePrimary,
    'L1_BKeePrescaled': getL1BKeePrescaled,
}

valid_multiseeds = [
    'L1_All',
    # EnhancedBias
    'L1_PhysicsHigh_noPS', 'L1_PhysicsVeryHigh_noPS',
    'L1_EMPTY_noPS', 'L1_FIRSTEMPTY_noPS',
    'L1_UNPAIRED_ISO_noPS', 'L1_UNPAIRED_NONISO_noPS', 'L1_UNPAIREDB1_noPS', 'L1_UNPAIREB2_noPS', 'L1_ABORTGAPNOTCALIB_noPS',
    # Trigger types
    'L1_Calo', 'L1_Calo_EMPTY',
    'L1_Muon', 'L1_Muon_EMPTY',
    # Other groups defined by matching
    'L1_Standby', 'L1_Topo', 'L1_TAU'
] + list(L1_multiseed_simple_getters.keys())

def getSpecificL1Seeds(l1seedname, l1itemobject, menu_name):
    l1items = l1itemobject.keys()
    L1Seed = ''

    if l1seedname == '':
        log.error('No L1item name given!')
        raise RuntimeError('No name provided to multiseed getter')

    if l1seedname in L1_multiseed_simple_getters:
        L1Seed = L1_multiseed_simple_getters[l1seedname]()
    elif l1seedname == 'L1_Standby':
        L1Seed = getL1StandbySeed(l1items)
    elif l1seedname == 'L1_Topo':
        L1Seed = getL1TopoSeed(l1items)
    elif l1seedname == 'L1_TAU':
        L1Seed = getL1TauSeed(l1items)
    elif (l1seedname in ['L1_PhysicsHigh_noPS', 'L1_PhysicsVeryHigh_noPS', 'L1_EMPTY_noPS', 'L1_FIRSTEMPTY_noPS', 'L1_UNPAIRED_ISO_noPS', 'L1_UNPAIRED_NONISO_noPS', 'L1_UNPAIREDB1_noPS', 'L1_UNPAIREB2_noPS', 'L1_ABORTGAPNOTCALIB_noPS']):
        L1Seed =  getEBnoL1PSSeed(l1items, l1seedname)
    elif (l1seedname in ['L1_Calo', 'L1_Calo_EMPTY']):
        L1Seed = getL1CaloSeed(l1seedname, l1itemobject)
    elif (l1seedname in ['L1_Muon', 'L1_Muon_EMPTY']):
        L1Seed = getL1MuonSeed(l1seedname, l1itemobject)
    elif (l1seedname == 'L1_All'):
        return []
    else: 
        log.error('Given seed %s could not be found!', l1seedname)
        raise RuntimeError(f'Failed to retrieve L1 item list for {l1seedname}')

    # check if all the l1 background seeds given are in the current L1 menu
    for item in L1Seed:
        if item not in l1items:
            log.error('L1 item %s from %s seeds is not in current L1 menu', item, l1seedname)

    L1Seed.sort()
    L1Seed = ",".join(L1Seed)

    return L1Seed

#####################################
# map from l1item name to inputTE
#####################################
def getInputTEfromL1Item(l1item, menu_name):
    
    L1Map = {
        'L1_TAU8_EMPTY'          : ['HA8'],
        'L1_TAU8_FIRSTEMPTY'     : ['HA8'],
        'L1_TAU8_UNPAIRED_ISO'   : ['HA8'],
        'L1_TAU8_UNPAIRED_NONISO': ['HA8'],
        'L1_TAU12_EMPTY'         : ['HA12'],
        'L1_TAU12_FIRSTEMPTY'    : ['HA12'],
        'L1_TAU12_UNPAIRED_ISO'  : ['HA12'],
        'L1_RD0_FIRSTEMPTY'      : [''],
        'L1_TAU30'               : ['HA30'],
        'L1_TAU30_EMPTY'         : ['HA30'],
        'L1_TAU30_UNPAIRED_ISO'  : ['HA30'],
        'L1_TAU40'               : ['HA40'],
        'L1_TAU60'               : ['HA60'],
        'L1_TAU100'              : ['HA100'],
        }

    L1Map['L1_CALREQ2']=['CAL2']
        
    if l1item in L1Map:
        TE = L1Map[l1item]
        log.debug('Mapped L1 input TE from %s to %s.', l1item, TE)
        return TE
    else:
        TE = l1item.replace("L1_","").split("_")[0]
        TE = TE[1:] if TE[0].isdigit() else TE
        return TE
    
