# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import libpbeastpy; pbeast = libpbeastpy.ServerProxy('https://atlasop.cern.ch')
import logging; log = logging.getLogger("DCSCalculator2.variable")

from DQUtils.general import timer
from DQUtils.sugar import IOVSet, RANGEIOV_VAL, RunLumi, TimestampType, define_iov_type, make_iov_type

from DCSCalculator2 import config
from DCSCalculator2.consts import RED, YELLOW
from DCSCalculator2.libcore import map_channels
from DCSCalculator2.subdetector import DCSC_DefectTranslate_Subdetector
from DCSCalculator2.variable import DefectIOV, GoodIOV, DCSC_Variable, DCSC_Variable_With_Mapping

class DCSC_Merged_Variable(DCSC_Variable):
    def __init__(self, evaluator, folders, **kwargs):
        mapping = kwargs.pop('mapping', None)
        folder_merge = ','.join(folders)
            
        super().__init__(folder_merge, evaluator, **kwargs)
        self.folder_names = folders
        self.variable_channels_map = mapping
    
    def get_variable(self, name):
        for var in self.variables:
            if var.folder_name == name:
                return var
        raise RuntimeError("Folder '%s' not found" % name)

    def read(self, query_range, folder_base, folder_name):
        var_iovs = []
        folders = folder_name.split(',')

        for folder in folders:
            iovs = DCSC_Variable.read(self, query_range, folder_base, folder)
            if folder in self.variable_channels_map:
                iovs = map_channels(iovs, self.variable_channels_map[folder], folder)
            var_iovs.append(iovs)

        iovs = self.merge_input_variables(*var_iovs)

        if config.opts.check_input_time:
            self.print_time_info(iovs)
            
        if log.isEnabledFor(logging.INFO):
            input_hash = hash(iovs)
            self.input_hashes.append(input_hash)
            #log.info("  -> Input hash: % 09x (len=%i)", input_hash, len(iovs))
            
        return iovs

    def merge_input_variables(self, *inputs):
        type_bases = [iovs.iov_type for iovs in inputs]
        base_names = [base.__name__[:-4] for base in type_bases]
        attributes = [base.lower() for base in base_names]
        clazz = make_iov_type('MERGER_OF_' + '_AND_'.join(base_names), tuple(['channel'] + attributes))

        inputs_by_channel = [iovs.by_channel for iovs in inputs]
        all_channels = sorted(set(y for x in inputs_by_channel for y in x.keys()))

        result = []
        for channel in all_channels:
            c_inputs = [x[channel] for x in inputs_by_channel]
            result.extend(iov for iov in self.merge_inputs(clazz, channel, *c_inputs))

        return IOVSet(result, iov_type=clazz, origin=self.folder_names)

    def merge_inputs(self, clazz, channel, *inputs):
        result = []
        lengths = [len(iovs) for iovs in inputs]
        indices = [0 for x in inputs]
        while sum([1 for index,length in zip(indices,lengths) if index >= length]) < 1:
            iovs = [iovs[index] for iovs,index in zip(inputs,indices)]
            since = max(iov.since for iov in iovs)
            until = min(iov.until for iov in iovs)
            result.append(clazz(since, until, channel, *iovs))
            indices = [index+1 if until==iov.until else index for iov,index in zip(iovs,indices)]
        return IOVSet(result, iov_type=clazz, origin=self.folder_names)

@define_iov_type
def PBeastIOV(channel, value):
    "Stores the value of an object's attribute queried from pBeast"

class TDAQC_Variable(DCSC_Variable):
    """
    A variable which reads data from pBeast.
    """
    TIME_RATIO = 1e3

    @staticmethod
    def timeCOOL2PBeast(timestamp):
        return int(timestamp/TDAQC_Variable.TIME_RATIO)
    
    @staticmethod
    def timePBeast2COOL(timestamp):
        return TimestampType(timestamp*TDAQC_Variable.TIME_RATIO)

    def __init__(self, query, evaluator, *, regex=False):
        self.regex = regex
        self.query = query
        self.partition, self.className, self.attribute, self.path = query.split('.', 3)
        self.evaluator = evaluator
        self.input_hashes = []
    
    def __repr__(self):
        return f"<TDAQCVariable {self.query}>"
    
    def read(self, query_range, query, *, regex=False):
        """
        Read the relevant data from pBeast for this variable, and convert them to COOL-like format
        """
        partition, className, attribute, path = query.split('.', 3)
        
        log.info(f"Querying pBeast object{'s using regex' if regex else ''} {query}")

        since, until = query_range
        since, until = TDAQC_Variable.timeCOOL2PBeast(since), TDAQC_Variable.timeCOOL2PBeast(until)
        
        data = pbeast.get_data(partition, className, attribute, path, regex, since, until)[0].data
        def instantiate(since, until, channel, value):
            return PBeastIOV(TDAQC_Variable.timePBeast2COOL(since), TDAQC_Variable.timePBeast2COOL(until), channel, value)

        iovs = []
        for channel, entries in data.items():
            if not entries:
                continue
            first = entries[0]
            since = first.ts
            value = first.value
            for point in entries:
                if point.value == value:
                    continue
                iovs.append(instantiate(since, point.ts, channel, value))
                since = point.ts
                value = point.value
            last = entries[-1]
            iovs.append(instantiate(since, last.ts, channel, value))
        iovs = IOVSet(iovs, iov_type=PBeastIOV, origin=query)
            
        if log.isEnabledFor(logging.INFO):
            input_hash = hash(iovs)
            self.input_hashes.append(input_hash)
            log.info("  -> Input hash: % 09x (len=%i)", input_hash, len(iovs))
            
        return iovs

    def calculate_good_iovs(self, lbtime, subdetector):
        """
        Calculate LB-wise "good" states
        """

        self.subdetector = subdetector
        
        since, until = lbtime.first, lbtime.last
        if self.timewise_folder:
            query_range = RANGEIOV_VAL(since.since, until.until)
        else:
            query_range = RANGEIOV_VAL(RunLumi(since.Run, since.LumiBlock), 
                                       RunLumi(until.Run, until.LumiBlock))
        
        # Read the database
        iovs = self.read(query_range, self.query, regex=self.regex)
        # Decide the states of the input iovs
        iovs = self.make_good_iovs(iovs)
        # Apply a mapping for input channels if necessary
        iovs = self.map_input_channels(iovs)

        if self.timewise_folder and not config.opts.timewise:
            # we might already know the defect mapping
            with timer("Quantize %s (%i iovs over %i lbs)" % 
                       (self.query, len(iovs), len(lbtime))):
                # Quantize to luminosity block
                iovs = self.quantize(lbtime, iovs)

        self.iovs = iovs
        return self

class TDAQC_Multi_Channel_Variable(TDAQC_Variable):
    def make_good_iov(self, iov):
        """
        Determine if channels in one input iov are good.
        """
        giov = []
        for channel, goodness in self.evaluator(iov):
            current = GoodIOV(iov.since, iov.until, channel, goodness)
            current._orig_iov = iov
            giov.append(current)
        return giov
    
    def make_good_iovs(self, iovs):
        """
        Determine whether each iov signifies a good or bad state.
        """
        results = []
        for iov in iovs:
            results.append(self.make_good_iov(iov))
        return IOVSet(sum(zip(*results), ())) # Sort by channel first

class TDAQC_Bit_Flag_Variable(TDAQC_Multi_Channel_Variable):
    def __init__(self, query, evaluator, *, regex=False, mapping=dict()):
        super().__init__(query, evaluator, regex=regex)
        self.mapping = mapping

    def make_good_iov(self, iov):
        """
        Determine if channels in one input iov are good.
        """
        giov = []
        for bit, channel in self.mapping.get(iov.channel, dict()).items():
            test_iov = PBeastIOV(iov.since, iov.until, channel, ((iov.value >> bit) & 1) == 1)
            current = GoodIOV(iov.since, iov.until, channel, self.evaluator(test_iov))
            current._orig_iov = iov
            giov.append(current)
        return giov
        
# DCS channels
A_FAR_SIT_HV, A_NEAR_SIT_HV, C_FAR_SIT_HV, C_NEAR_SIT_HV =   1,   5,   9,  13
A_FAR_SIT_LV, A_NEAR_SIT_LV, C_FAR_SIT_LV, C_NEAR_SIT_LV =  21,  25,  29,  33
A_FAR_TOF_HV,                C_FAR_TOF_HV                =  51,       59
A_FAR_TOF_LV,                C_FAR_TOF_LV                =  71,       79
A_FAR_GARAGE, A_NEAR_GARAGE, C_FAR_GARAGE, C_NEAR_GARAGE = 101, 105, 109, 113

# TDAQ channels
A_FAR_SIT_DISABLED, A_NEAR_SIT_DISABLED, C_FAR_SIT_DISABLED, C_NEAR_SIT_DISABLED = 1001, 1005, 1009, 1013
A_FAR_TOF_DISABLED,                      C_FAR_TOF_DISABLED                      = 1051,       1059

# Thresholds
SIT_HV_DEAD_BAND = 0.05
TOF_HV_DEAD_BAND = 0.90

SIT_LV_CURRENT_LOW, SIT_LV_CURRENT_HIGH = 0.4, 0.6

def remove_None(value, default):
    return value if value is not None else default

def disabledEvaluator(iov):
    diff = 8 if '4' == iov.channel[-1] else 0
    outlinks = [8, 10, 12, 14, 0, 2, 4, 6, 9, 11]
    mapping = list(range(1001, 1009)) + [1051, 1052]
    mapping = list(map(lambda x: x+diff, mapping))
    return [(c, (iov.value >> o) & 1 == 0) for o, c in zip(outlinks, mapping)]

class AFP(DCSC_DefectTranslate_Subdetector):
    folder_base = '/AFP/DCS'
    variables = [
        DCSC_Merged_Variable(
            lambda iov: -remove_None(iov.hv.voltage, 0) > iov.hv_voltage_set.voltageSet - SIT_HV_DEAD_BAND,
            ['SIT/HV', 'SIT/HV_VOLTAGE_SET'],
            mapping={
                'SIT/HV': dict(zip([6,7,1,2,8,3,4,9,10,11,12,5,13,14,15,16], range(1,17)))
            }
        ),
        DCSC_Variable_With_Mapping('SIT/LV', lambda iov: SIT_LV_CURRENT_LOW <= remove_None(iov.current, 0) <= SIT_LV_CURRENT_HIGH),
        DCSC_Merged_Variable(
            lambda iov: -remove_None(iov.tof.pmt_voltage, 0) > iov.tof_pmt_voltage_set.pmt_voltageSet - TOF_HV_DEAD_BAND,
            ['TOF', 'TOF_PMT_VOLTAGE_SET'],
            mapping={
                'TOF': {1: A_FAR_TOF_HV, 2: C_FAR_TOF_HV},
                'TOF_PMT_VOLTAGE_SET': {1: A_FAR_TOF_HV, 2: C_FAR_TOF_HV},
            }
        ),
        DCSC_Variable_With_Mapping('STATION', lambda iov: iov.inphysics is True),
        #TDAQC_Multi_Channel_Variable('ATLAS.RceMonitoring.Disabled.Monitoring.RceMonitoring_RCE[34]', disabledEvaluator, regex=True),
        TDAQC_Bit_Flag_Variable(
            'ATLAS.RceMonitoring.Disabled.Monitoring.RceMonitoring_RCE[34]',
            lambda iov: iov.value is False,
            regex=True,
            mapping={
                'Monitoring.RceMonitoring_RCE3': dict(zip([8, 10, 12, 14, 0, 2, 4, 6, 9, 11],list(range(1001, 1009)) + [1051, 1053])),
                'Monitoring.RceMonitoring_RCE4': dict(zip([8, 10, 12, 14, 0, 2, 4, 6, 9, 11],list(range(1009, 1014)) + [1059, 1061])),
            }
        )
    ]

    equality_breaker = 0.0001

    dead_fraction_caution = 0 + equality_breaker
    dead_fraction_bad = 0.25 + equality_breaker

    mapping = {
        A_FAR_SIT_HV:  range( 1,  5), A_NEAR_SIT_HV: range( 5,  9),
        C_FAR_SIT_HV:  range( 9, 13), C_NEAR_SIT_HV: range(13, 17),
        
        A_FAR_SIT_LV:  range(21, 25), A_NEAR_SIT_LV: range(25, 29),
        C_FAR_SIT_LV:  range(29, 33), C_NEAR_SIT_LV: range(33, 37),

        A_FAR_TOF_HV: [A_FAR_TOF_HV], C_FAR_TOF_HV: [C_FAR_TOF_HV],

        A_FAR_GARAGE: [A_FAR_GARAGE], A_NEAR_GARAGE: [A_NEAR_GARAGE],
        C_FAR_GARAGE: [C_FAR_GARAGE], C_NEAR_GARAGE: [C_NEAR_GARAGE],

        A_FAR_SIT_DISABLED: range(1001, 1005), A_NEAR_SIT_DISABLED: range(1005, 1009),
        C_FAR_SIT_DISABLED: range(1009, 1013), C_NEAR_SIT_DISABLED: range(1013, 1017),

        A_FAR_TOF_DISABLED: [1051, 1053], C_FAR_TOF_DISABLED: [1059, 1061]
    }

    @staticmethod
    def color_to_defect_translator(channel, defect, color, comment):
        def translator_core(iovs):
            return [DefectIOV(iov.since, iov.until, defect, True,
                            comment=comment(iov))
                    for iov in iovs if iov.channel == channel
                    and iov.Code == color]
        return translator_core

    def __init__(self, *args, **kwargs):
        super(AFP, self).__init__(*args, **kwargs)
        self.set_input_mapping('SIT/LV', dict(zip(list(range(9, 17)) + list(range(1, 9)), range(21, 37))))
        self.set_input_mapping('STATION', dict(zip([1, 2, 3, 4], [C_FAR_GARAGE, C_NEAR_GARAGE, A_FAR_GARAGE, A_NEAR_GARAGE])))
        self.translators = [AFP.color_to_defect_translator(*cdcc)
                            for cdcc in [
                                (C_FAR_SIT_HV,  'AFP_C_FAR_SIT_PARTIALLY_OPERATIONAL_HV',  YELLOW, AFP.comment_SIT_HV),
                                (C_FAR_SIT_HV,  'AFP_C_FAR_SIT_NOT_OPERATIONAL_HV',        RED,    AFP.comment_SIT_HV),
                                (C_NEAR_SIT_HV, 'AFP_C_NEAR_SIT_PARTIALLY_OPERATIONAL_HV', YELLOW, AFP.comment_SIT_HV),
                                (C_NEAR_SIT_HV, 'AFP_C_NEAR_SIT_NOT_OPERATIONAL_HV',       RED,    AFP.comment_SIT_HV),
                                (A_FAR_SIT_HV,  'AFP_A_FAR_SIT_PARTIALLY_OPERATIONAL_HV',  YELLOW, AFP.comment_SIT_HV),
                                (A_FAR_SIT_HV,  'AFP_A_FAR_SIT_NOT_OPERATIONAL_HV',        RED,    AFP.comment_SIT_HV),
                                (A_NEAR_SIT_HV, 'AFP_A_NEAR_SIT_PARTIALLY_OPERATIONAL_HV', YELLOW, AFP.comment_SIT_HV),
                                (A_NEAR_SIT_HV, 'AFP_A_NEAR_SIT_NOT_OPERATIONAL_HV',       RED,    AFP.comment_SIT_HV),

                                (C_FAR_SIT_LV,  'AFP_C_FAR_SIT_PARTIALLY_OPERATIONAL_LV',  YELLOW, AFP.comment_SIT_LV),
                                (C_FAR_SIT_LV,  'AFP_C_FAR_SIT_NOT_OPERATIONAL_LV',        RED,    AFP.comment_SIT_LV),
                                (C_NEAR_SIT_LV, 'AFP_C_NEAR_SIT_PARTIALLY_OPERATIONAL_LV', YELLOW, AFP.comment_SIT_LV),
                                (C_NEAR_SIT_LV, 'AFP_C_NEAR_SIT_NOT_OPERATIONAL_LV',       RED,    AFP.comment_SIT_LV),
                                (A_FAR_SIT_LV,  'AFP_A_FAR_SIT_PARTIALLY_OPERATIONAL_LV',  YELLOW, AFP.comment_SIT_LV),
                                (A_FAR_SIT_LV,  'AFP_A_FAR_SIT_NOT_OPERATIONAL_LV',        RED,    AFP.comment_SIT_LV),
                                (A_NEAR_SIT_LV, 'AFP_A_NEAR_SIT_PARTIALLY_OPERATIONAL_LV', YELLOW, AFP.comment_SIT_LV),
                                (A_NEAR_SIT_LV, 'AFP_A_NEAR_SIT_NOT_OPERATIONAL_LV',       RED,    AFP.comment_SIT_LV),

                                (A_FAR_TOF_HV, 'AFP_A_FAR_TOF_NOT_OPERATIONAL_HV', RED, AFP.comment_TOF_HV),
                                (C_FAR_TOF_HV, 'AFP_C_FAR_TOF_NOT_OPERATIONAL_HV', RED, AFP.comment_TOF_HV),

                                (C_FAR_GARAGE,  'AFP_C_FAR_IN_GARAGE',  RED, AFP.comment_GARAGE),
                                (C_NEAR_GARAGE, 'AFP_C_NEAR_IN_GARAGE', RED, AFP.comment_GARAGE),
                                (A_FAR_GARAGE,  'AFP_A_FAR_IN_GARAGE',  RED, AFP.comment_GARAGE),
                                (A_NEAR_GARAGE, 'AFP_A_NEAR_IN_GARAGE', RED, AFP.comment_GARAGE),

                                (C_FAR_SIT_DISABLED,  'AFP_C_FAR_SIT_PARTIALLY_OPERATIONAL_TDAQ',  YELLOW, AFP.comment_SIT_DISABLED),
                                (C_FAR_SIT_DISABLED,  'AFP_C_FAR_SIT_NOT_OPERATIONAL_TDAQ',        RED,    AFP.comment_SIT_DISABLED),
                                (C_NEAR_SIT_DISABLED, 'AFP_C_NEAR_SIT_PARTIALLY_OPERATIONAL_TDAQ', YELLOW, AFP.comment_SIT_DISABLED),
                                (C_NEAR_SIT_DISABLED, 'AFP_C_NEAR_SIT_NOT_OPERATIONAL_TDAQ',       RED,    AFP.comment_SIT_DISABLED),
                                (A_FAR_SIT_DISABLED,  'AFP_A_FAR_SIT_PARTIALLY_OPERATIONAL_TDAQ',  YELLOW, AFP.comment_SIT_DISABLED),
                                (A_FAR_SIT_DISABLED,  'AFP_A_FAR_SIT_NOT_OPERATIONAL_TDAQ',        RED,    AFP.comment_SIT_DISABLED),
                                (A_NEAR_SIT_DISABLED, 'AFP_A_NEAR_SIT_PARTIALLY_OPERATIONAL_TDAQ', YELLOW, AFP.comment_SIT_DISABLED),
                                (A_NEAR_SIT_DISABLED, 'AFP_A_NEAR_SIT_NOT_OPERATIONAL_TDAQ',       RED,    AFP.comment_SIT_DISABLED),

                                (A_FAR_TOF_DISABLED, 'AFP_A_FAR_TOF_NOT_OPERATIONAL_TDAQ', RED, AFP.comment_TOF_DISABLED),
                                (C_FAR_TOF_DISABLED, 'AFP_C_FAR_TOF_NOT_OPERATIONAL_TDAQ', RED, AFP.comment_TOF_DISABLED),
                            ]]

    @staticmethod
    def comment_SIT_HV(iov):
        return AFP.comment_planes(iov, 'out of nominal voltage')

    @staticmethod
    def comment_SIT_LV(iov):
        return AFP.comment_planes(iov, 'not configured for data-taking')

    @staticmethod
    def comment_TOF_HV(iov):
        return 'ToF out of nominal voltage'

    @staticmethod
    def comment_GARAGE(iov):
        return 'Station not in physics'
    
    @staticmethod
    def comment_SIT_DISABLED(iov):
        return AFP.comment_planes(iov, 'removed from readout')
    
    @staticmethod
    def comment_TOF_DISABLED(iov):
        return AFP.comment_device(iov, 'ToF TDC', 'removed from readout')
    
    @staticmethod
    def comment_planes(iov, message):
        return AFP.comment_device(iov, 'SiT plane', message)

    @staticmethod
    def comment_device(iov, device, message):
        count = iov.NConfig - iov.NWorking
        return '%d %s%s %s' % (count, device, '' if count == 1 else 's', message)
        
    def merge_variable_states(self, states):
        """
        Merge input channel states across variables, taking the worst.
        
        Ignore configuration variables and variables without channel.
        """

        # Remove variables without channel
        states = [state for state in states if state and state.channel is not None]
                
        # More simplistic way of doing the above, but cannot handle config vars:
        return min(state.good for state in states) if len(states) > 0 else None
