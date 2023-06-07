#!/usr/bin/env python

# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# @brief: This is the dbgEventInfo class for the Debug Stream event analysis

from __future__ import print_function

import eformat
from ROOT import gStyle, gROOT, addressof, TTree, vector, string
from TrigByteStreamTools.hltResultMT import get_collections

import logging
msg = logging.getLogger('PyJobTransforms.' + __name__)

class dbgEventInfo:

    def __init__(self, dbgStep='_Default', inputFile=''):
        # Event Info Tree values
        self.Run_Number                            = 0
        self.Stream_Tag_Name                       = 'None'
        self.Stream_Tag_Type                       = 'None'
        self.Lvl1_ID                               = 0
        self.Global_ID                             = 0
        self.Lumiblock                             = 0
        self.Node_ID                               = 0
        self.HLT_Result                            = 'None'
        self.SuperMasterKey                        = 0
        self.HLTPrescaleKey                        = 0
        self.HLT_Decision                          = 0
        self.L1_Chain_Names                        = []
        self.HLT_Chain_Names                       = []
        self.EventStatusNames                      = 'None'

        #define the string length 
        self.strlength = 100

        self.eventCounter = 0
        self.rootDefinitions(dbgStep, inputFile)
        

        # Full event Specific Status - based on ATL-DAQ-98-129 
        self.EventSpecificStatus = ['Reserved',
                                    'Reserved',
                                    'Reserved',
                                    'HLTSV_DUPLICATION_WARN',
                                    'Reserved',
                                    'Reserved',
                                    'Reserved',
                                    'Reserved',
                                    'DCM_PROCESSING_TIMEOUT',
                                    'HLTPU_PROCESSING_TIMEOUT',
                                    'DCM_DUPLICATION_WARN',
                                    'DCM_RECOVERED_EVENT',
                                    'PSC_PROBLEM',
                                    'DCM_FORCED_ACCEPT',
                                    'Reserved',
                                    'PARTIAL_EVENT']

        # Copied from TrigSteeringEvent/OnlineErrorCode.h 
        self.onlineErrorCode = ['UNCLASSIFIED',
                                'BEFORE_NEXT_EVENT',
                                'CANNOT_RETRIEVE_EVENT',
                                'NO_EVENT_INFO',
                                'SCHEDULING_FAILURE',
                                'CANNOT_ACCESS_SLOT',
                                'PROCESSING_FAILURE',
                                'NO_HLT_RESULT',
                                'OUTPUT_BUILD_FAILURE',
                                'OUTPUT_SEND_FAILURE',
                                'AFTER_RESULT_SENT',
                                'COOL_UPDATE',
                                'TIMEOUT',
                                'RESULT_TRUNCATION',
                                'MISSING_CTP_FRAGMENT',
                                'BAD_CTP_FRAGMENT']


    def eventCount(self, event):
        self.eventCounter += 1
        msg.info('Running debug_stream analysis on event :{0}'.format(self.eventCounter))


    def eventInfo(self, event, L1ChainNames, HLTChainNames):
        # Fill all necessary information based on bs event info
        self.Run_Number = event.run_no()
        self.Global_ID = event.global_id()
        self.Lumiblock = event.lumi_block()
        self.Lvl1_ID = event.lvl1_id()

        msg.debug("Event Counter Lvl1 ID               = %s", self.Lvl1_ID & 0xffffff)
        msg.debug("Event Counter Reset Counter Lvl1 ID = %s", (self.Lvl1_ID & 0xff000000) >> 24)

        self.L1_Chain_Names = L1ChainNames
        self.HLT_Chain_Names = HLTChainNames

        streamtagNames = ','.join([tag.name for tag in event.stream_tag()])
        self.Stream_Tag_Name = streamtagNames

        streamtagTypes = ','.join([tag.type for tag in event.stream_tag()])
        self.Stream_Tag_Type = streamtagTypes
        
        
        # Check if the length of the Stream_Tag_Name & Stream_Tag_Type
        # if too long then cut off the end so that the Root file can fill properly
        if (len(self.Stream_Tag_Name) > self.strlength):
            # print warning and cut out the string characters > strlength char 
            msg.warn("Stream_Tag_Name has length %s reducing to length %s",len(self.Stream_Tag_Name), self.strlength)
            self.Stream_Tag_Name = self.Stream_Tag_Name[0:self.strlength]
        
        if (len(self.Stream_Tag_Type) > self.strlength):
            # print warning and cut out the string characters > strlength char 
            msg.warn("Stream_Tag_Type has length %s reducing to length %s",len(self.Stream_Tag_Type), self.strlength)
            self.Stream_Tag_Type = self.Stream_Tag_Type[0:self.strlength]

        self.eventStatus(event)
        self.lvl1Info(event, L1ChainNames)
        self.hltInfo(event, HLTChainNames)
        self.hltResult(event)


    def eventConfig(self, configKeys=None, event=None):
        # Set configuration data: PSK and SMK
        if configKeys:
            for key in configKeys['HLTPSK']:
                if event.lumi_block() >= key[1] and event.lumi_block() <= key[2]:
                    self.HLTPrescaleKey = key[0]
                    break

            self.SuperMasterKey = configKeys['SMK']
        elif event:
            # Find TrigConfKeys EDM collection and extract data
            for rob in event.children():
                if rob.source_id().subdetector_id() != eformat.helper.SubDetector.TDAQ_HLT:
                    continue
                collections = get_collections(rob, skip_payload=False)
                configList = [c for c in collections if 'xAOD::TrigConfKeys_v' in c.name_persistent]

                for conf in configList:
                    configKeys = conf.deserialise()
                    if configKeys:
                        self.HLTPrescaleKey = configKeys.hltpsk()
                        self.SuperMasterKey = configKeys.smk()
                        return

            msg.info("Smk and hltpskey unavailable in this event")
        else:
            msg.info("Cannot retrieve smk and hltpskey")

        msg.info('HLT_Configuration  smk:{0}   hltpskey :{1}'.format(self.SuperMasterKey, self.HLTPrescaleKey))


    def eventStatus(self, event):
        # Get full event specific status
        # Based on ATL-DAQ-98-129 section 5.8.3

        # Helper function to check if bit was on.
        def getBit(word, index):
            return (word >> index) & 0x1

        # Get list of statuses based on passed bits
        def checkBits(word, firstBit, lastBit, status):
            statusList = []
            for i in range(firstBit, lastBit):
                if getBit(word, i):
                    # Get and index in status list
                    statusIdx = i - firstBit
                    statusList.append(status[statusIdx])

            return statusList

        # Check second word of first status element with Full Event specific status
        statusList = checkBits(event.status()[0], 16, 32, self.EventSpecificStatus)
        
        # Check if PSC_PROBLEM bit was on and retrieve Online Error Codes
        # stored in following event status words
        if getBit(event.status()[0], 28):
            statusLen = len(event.status())
            if statusLen > 1:
                # Skip first event - already analyzed
                for i in range(1, statusLen):
                    #To protect against the repetition of the first event status element
                    #If the integer value of the current event status element is greater than the length of the onlineErrorCode list
                    #then skip over this element to the next
                    if int(event.status()[i]) >= len(self.onlineErrorCode):
                         continue 
                    statusList.append(self.onlineErrorCode[int(event.status()[i])])
            else:
                msg.warn("Cannot find additional words for PSC_PROBLEM")
        
        
        #ensure EventStatusNames are None if statusList is empty
        if len(statusList) != 0:  
           self.EventStatusNames = ','.join(str(name) for name in statusList)

        msg.info('Event Status :%s', self.EventStatusNames)
        
        # Check if the length of the EventStatusNames
        # if too long then cut off the end so that the Root file can fill properly
        if (len(self.EventStatusNames) > self.strlength):
            # print warning and cut out the string characters > strlength char 
            msg.warn("EventStatusNames has length %s reducing to length %s",len(self.EventStatusNames), self.strlength)
            self.EventStatusNames = self.EventStatusNames[0:self.strlength]

 
    def lvl1Info(self, event, L1ItemNames):
        # Get LVL1 info for BP AV and AV-TrigIDs and store them in ROOT vectors
        self.L1_Triggered_BP.clear()
        self.L1_Triggered_AV.clear()
        self.L1_Triggered_IDs.clear()

        # Map not available
        if not L1ItemNames:
            return

        # Decode Lvl1 trigger info
        info = event.lvl1_trigger_info()
        l1Bits = self.decodeTriggerBits(info, 3)  # TBP, TAP, TAV

        for id in l1Bits[0]:
            if id in L1ItemNames:
                self.L1_Triggered_BP.push_back(L1ItemNames[id])
            else:
                msg.debug('Item %s not found in the menu - probably is disabled', id)

        for id in l1Bits[2]:
            self.L1_Triggered_IDs.push_back(id)
            try:
                self.L1_Triggered_AV.push_back(L1ItemNames[id])
            except TypeError:
                msg.warn("Item name for ctpid %s not found!", id)

        msg.info('LVL1 Triggered ID Chains AV : %s', l1Bits[2])


    def hltInfo(self, event, HLTChainNames):
        # Get HLT info and store it in ROOT vectors
        self.HLT_Triggered_Names.clear()
        self.HLT_Triggered_IDs.clear()

        # Map not available
        if not HLTChainNames:
            return

        # Find ROD minor version
        version = (1,1)
        for rob in event.children():
            if rob.source_id().subdetector_id() == eformat.helper.SubDetector.TDAQ_HLT:
                rod_version = rob.rod_version()
                minor_version = rod_version.minor_version()
                minor_version_M = (minor_version & 0xff00) >> 8
                minor_version_L = minor_version & 0xff
                version = (minor_version_M, minor_version_L)
                break

        # Version 1.0 has {passed, prescaled, rerun}, 1.1 and later only {passed, prescaled}
        num_sets = 3 if version[0] < 1 or version==(1,0) else 2

        # Decode HLT trigger info
        info = event.event_filter_info()
        chainList = self.decodeTriggerBits(info, num_sets)

        for id in chainList[0]:
            self.HLT_Triggered_IDs.push_back(id)
            try:
                self.HLT_Triggered_Names.push_back(HLTChainNames[id])
            except TypeError:
                msg.warn("Chain name for counter %s not found!", id)

        msg.info('HLT Triggered ID Chains  : %s', chainList[0])


    # Copied from ​TrigTools/TrigByteStreamTools/trigbs_dumpHLTContentInBS_run3.py
    def decodeTriggerBits(self, words, num_sets, base=32):
        assert len(words) % num_sets == 0
        n_words_per_set = len(words) // num_sets
        result = []
        for iset in range(num_sets):
            words_in_set = words[iset*n_words_per_set:(iset+1)*n_words_per_set]
            bit_indices = []
            for iw in range(len(words_in_set)):
                bit_indices.extend([base*iw+i for i in range(base) if words_in_set[iw] & (1 << i)])
            result.append(bit_indices)
        return result


    # If line 246 not possible - to delete
    def getChain(self, counter, s):
        # Prints chains and their information
        from cppyy.gbl import HLT
        ch = HLT.Chain(s)
        ch.deserialize(s)
        try:
            msg.info('.... chain {0:-3d}: {1} Counter: {2:-4d} Passed: {3} (Raw: {4} Prescaled: {5} PassThrough: {6}) Rerun: {7} LastStep: {8} Err: {9}'
                    .format(counter, self.HLT_Chain_Names[ch.getChainCounter()], ch.getChainCounter(), ch.chainPassed(), ch.chainPassedRaw(),
                    ch.isPrescaled(), ch.isPassedThrough(), ch.isResurrected(), ch.getChainStep(), ch.getErrorCode().str()))
        except IndexError:
            msg.warn("Chain with counter %s not found", ch.getChainCounter())


    # If line 246 not possible - to delete
    def getAllChains(self, blob):
        msg.info('... chains: %s', len(blob))
        for i in range(len(blob)):
            self.getChain(i, blob[i])


    def hltResult(self, event):
        # Get the hlt result, status, decision etc if hlt_result exist
        hltResultFound = False
        for rob in event.children():
            if rob.source_id().subdetector_id() == eformat.helper.SubDetector.TDAQ_HLT:
                msg.info('.. {0} source: {1} source ID: 0x{2:x} size: {3}'
                        .format(rob.__class__.__name__, rob.source_id(), rob.source_id().code(), rob.fragment_size_word()*4))
                try:
                    msg.info('version   :%s', rob.rod_version())
                    msg.info('l1id      :%s', event.lvl1_id())

                    self.Node_ID = rob.source_id().module_id()

                    # TODO chains and navigation printouts - needs ATR-22653

                    # ROBs are created only for accepted events
                    self.HLT_Decision = 1
                    hltResultFound = True

                except Exception as ex:
                    msg.info('... **** problems in analyzing payload %s', ex)
                    msg.info('... **** raw data[:10] %s', list(rob.rod_data())[:10])
                    msg.info('.. EOF HLTResult for HLT')

        if not hltResultFound:
            msg.info('No HLT Result for HLT')

        msg.info('HLT Decision :%s', self.HLT_Decision)


    def fillTree(self):
        # Fill Event_Info Tree
        self.Event_Info.Run_Number             = self.Run_Number
        self.Event_Info.Stream_Tag_Name        = self.Stream_Tag_Name
        self.Event_Info.Stream_Tag_Type        = self.Stream_Tag_Type
        self.Event_Info.Lvl1_ID                = self.Lvl1_ID
        self.Event_Info.Global_ID              = self.Global_ID
        self.Event_Info.Node_ID                = self.Node_ID
        self.Event_Info.Lumiblock              = self.Lumiblock
        self.Event_Info.SuperMasterKey         = self.SuperMasterKey
        self.Event_Info.HLTPrescaleKey         = self.HLTPrescaleKey
        self.Event_Info.HLT_Decision           = self.HLT_Decision
        self.Event_Info.EventStatusNames       = self.EventStatusNames

        self.event_info_tree.Fill()


    def rootDefinitions(self,dbgStep,inputFile):
        gStyle.SetCanvasColor(0)
        gStyle.SetOptStat(000000)
        gROOT.SetStyle("Plain")
  
      
        # Create new ROOT Tree to store debug info
        if dbgStep == "_Pre" :
            gROOT.ProcessLine("#define STRINGLENGTH "+str(self.strlength))
            gROOT.ProcessLine(
            "struct EventInfoTree {\
            Int_t   Run_Number;\
            Char_t  Stream_Tag_Name[STRINGLENGTH];\
            Char_t  Stream_Tag_Type[STRINGLENGTH];\
            UInt_t  Lvl1_ID;\
            ULong_t Global_ID;\
            Int_t   Lumiblock;\
            Int_t   Node_ID;\
            ULong_t   SuperMasterKey;\
            ULong_t   HLTPrescaleKey;\
            Int_t  HLT_Decision;\
            Char_t  EventStatusNames[STRINGLENGTH];\
            };" )
  
        # Bind the ROOT tree with EventInfo class members
        from ROOT import EventInfoTree
        self.Event_Info = EventInfoTree()
        self.event_info_tree = TTree('Event_Info' + dbgStep, inputFile)
  
        self.event_info_tree.Branch('Run_Number',       addressof(self.Event_Info, 'Run_Number'),       'run_Number/I')
        self.event_info_tree.Branch('Stream_Tag_Name',  addressof(self.Event_Info, 'Stream_Tag_Name'),  'stream_Tag_Name/C')
        self.event_info_tree.Branch('Stream_Tag_Type',  addressof(self.Event_Info, 'Stream_Tag_Type'),  'stream_Tag_Type/C')
        self.event_info_tree.Branch('Lvl1_ID',          addressof(self.Event_Info, 'Lvl1_ID'),          'lvl1_ID/I')
        self.event_info_tree.Branch('Global_ID',        addressof(self.Event_Info, 'Global_ID'),        'global_ID/I')
        self.event_info_tree.Branch('Lumiblock',        addressof(self.Event_Info, 'Lumiblock'),        'lumiblock/I')
        self.event_info_tree.Branch('Node_ID',          addressof(self.Event_Info, 'Node_ID'),          'node_ID/I')
        self.event_info_tree.Branch('SuperMasterKey',   addressof(self.Event_Info, 'SuperMasterKey'),   'superMasterKey/I')
        self.event_info_tree.Branch('HLTPrescaleKey',   addressof(self.Event_Info, 'HLTPrescaleKey'),   'hltPrescaleKey/I')
        self.event_info_tree.Branch('HLT_Decision',     addressof(self.Event_Info, 'HLT_Decision'),     'hLT_Decision/I')
        self.event_info_tree.Branch('EventStatusNames', addressof(self.Event_Info, 'EventStatusNames'), 'eventStatusNames/C')

        # Setup vector data
        self.L1_Triggered_AV  = vector( string )()
        self.L1_Triggered_BP  = vector( string )()
        self.L1_Triggered_IDs = vector( int )()
        self.HLT_Triggered_Names     = vector( string )()
        self.HLT_Triggered_IDs = vector( int )()

        self.event_info_tree._L1_Triggered_BP     = self.L1_Triggered_BP
        self.event_info_tree._L1_Triggered_AV     = self.L1_Triggered_AV
        self.event_info_tree._L1_Triggered_IDs    = self.L1_Triggered_IDs
        self.event_info_tree._HLT_Triggered_Names = self.HLT_Triggered_Names
        self.event_info_tree._HLT_Triggered_IDs   = self.HLT_Triggered_IDs

        self.event_info_tree.Branch('L1_Triggered_BP',  self.L1_Triggered_BP)
        self.event_info_tree.Branch('L1_Triggered_AV',  self.L1_Triggered_AV)
        self.event_info_tree.Branch('L1_Triggered_IDs', self.L1_Triggered_IDs)
        self.event_info_tree.Branch('HLT_Triggered_Names', self.HLT_Triggered_Names)
        self.event_info_tree.Branch('HLT_Triggered_IDs', self.HLT_Triggered_IDs)
        
