# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

###########################################################################
# Configure L2 and EF monitoring chains
###########################################################################
from AthenaCommon.Logging import logging
logging.getLogger().info("Importing %s",__name__)
log = logging.getLogger( 'TriggerMenu.calibcosmon.MonitorDef' )

from TriggerMenu.menu.HltConfig import L2EFChainDef


###########################################################################
###########################################################################

class L2EFChain_Monitoring(L2EFChainDef):
    def __init__(self, chainDict):
        self.L2sequenceList   = []
        self.EFsequenceList   = []
        self.L2signatureList  = []
        self.EFsignatureList  = []
        self.TErenamingDict   = []
        
        self.chainName = chainDict['chainName']
        self.chainPart = chainDict['chainParts']  
        self.chainL1Item = chainDict['L1item']       
        self.chainCounter = chainDict['chainCounter']       
        self.L2Name = 'L2_'+self.chainPart['chainPartName']
        self.EFName = 'EF_'+self.chainPart['chainPartName']
        self.monType = self.chainPart['monType']

        self.L2InputTE = self.chainL1Item 

        if ('robrequest' in self.monType):
            self.setupROBRequestMonChains()
        elif ('mistimemonl1bccorr' in self.monType or 'mistimemonl1bccorrnomu' in self.monType):
            self.setupL1BCCorrMonChains(self.chainName)
        elif ('timeburner' in self.monType):
            self.setupTimeBurnerChain()
        elif ('idmon' in self.monType):
            self.setupIdmonTrkFS()
        elif ('costmonitor' in self.monType):
            self.setupCostMonChain()
        elif ('cscmon' in self.monType):
            self.setupCSCMonChain()
        elif ('l1calooverflow' in self.monType):
            self.setupL1CaloOverflow()
        else:
            log.error("No suitable configuration for chain %s found!" % self.chainName)

        L2EFChainDef.__init__(self, self.chainName, self.L2Name, self.chainCounter,
                              self.L2InputTE, self.EFName, self.chainCounter, self.L2InputTE)

            
    def defineSequences(self):   
        for sequence in self.L2sequenceList:
            self.addL2Sequence(*sequence)            
        for sequence in self.EFsequenceList:
            self.addEFSequence(*sequence)
          
    def defineSignatures(self):
        for signature in self.L2signatureList:
            self.addL2Signature(*signature)
        for signature in self.EFsignatureList:
            self.addEFSignature(*signature)          
           
    def defineTErenaming(self):
        self.TErenamingMap=self.TErenamingDict


    ####################################
    ####################################
    def setupROBRequestMonChains(self):
        from TrigGenericAlgs.TrigGenericAlgsConf import ROBRequestAlgo
        ROBRequester = ROBRequestAlgo("DummyROBRequest")
        self.L2sequenceList += [[ '' , [ROBRequester],  'L2_DummyROBRequest']]
        self.L2signatureList += [ [['L2_DummyROBRequest']] ]

    ####################################
    ####################################
    def setupL1BCCorrMonChains(self,chainname):
        from TrigGenericAlgs.TrigGenericAlgsConfig import L1CorrelationAlgoConfig

        
        
        if "nomu" in chainname:
            L1CorrAlgo = L1CorrelationAlgoConfig("L1CorrAlgoNoMuon")
            L1CorrAlgo.noMuon = True
            L1CorrAlgo.m_l1itemlist = ["L1_EM22VHI","L1_J120","L1_J400"]
            self.EFsequenceList += [[ '' , [L1CorrAlgo],  'EF_DummyL1CorrAlgoNoMuon']]
            self.EFsignatureList += [ [['EF_DummyL1CorrAlgoNoMuon']] ]
        else:
            L1CorrAlgo = L1CorrelationAlgoConfig("L1CorrAlgo")
            L1CorrAlgo.noMuon = False
            L1CorrAlgo.m_l1itemlist = ["L1_EM22VHI","L1_MU20","L1_J120","L1_J400"]
            self.EFsequenceList += [[ '' , [L1CorrAlgo],  'EF_DummyL1CorrAlgo']]
            self.EFsignatureList += [ [['EF_DummyL1CorrAlgo']] ]
            
        ## Afterwards set up caloclustering to access timing information
        ###  Stole that one here from MissingETDef.py it prepares the caloclustering

        # chain = ['j0', '',  [], ["Main"], ['RATE:SingleJet', 'BW:Jet'], -1]
        
        # from TriggerMenu.menu import DictFromChainName
        # theDictFromChainName = DictFromChainName.DictFromChainName()
        # jetChainDict = theDictFromChainName.getChainDict(chain)
        
        # from TriggerMenu.jet.JetDef import generateHLTChainDef
        # jetChainDict['chainCounter'] = 9151
        # jetChainDef = generateHLTChainDef(jetChainDict)

        # #obtaining DummyUnseededAllTEAlgo/RoiCreator
        # input0=jetChainDef.sequenceList[0]['input']
        # output0 =jetChainDef.sequenceList[0]['output']
        # algo0 =jetChainDef.sequenceList[0]['algorithm']

        # #obtaining TrigCaloCellMaker/FS, TrigCaloClusterMaker, TrigHLTEnergyDensity
        # input1=jetChainDef.sequenceList[1]['input']
        # output1 =jetChainDef.sequenceList[1]['output']
        # algo1 =jetChainDef.sequenceList[1]['algorithm']
        # print 
        # self.EFsequenceList +=[[ input0,algo0,  output0 ]]      ## Why two times??
        # self.EFsequenceList +=[[ input0,algo0,  output0 ]]            
        # self.EFsequenceList +=[[ input1,algo1,  output1 ]]            
        
        # self.EFsignatureList += [ [[output1]] ]

    ####################################
    ####################################
    def setupCSCMonChain(self):
        from TrigDetCalib.TrigDetCalibConfig import CSCSubDetListWriter        
        CSC_PEB = CSCSubDetListWriter('CSCSubDetListWriter_' + self.chainName)

        outputTE='L2_CSCSubDetListWriter_'+self.chainName

        self.L2sequenceList +=  [ [ '', [CSC_PEB], outputTE ] ]        

        self.L2signatureList += [ [[outputTE]] ]
        

    ####################################
    ####################################
    def setupTimeBurnerChain(self):
        from TrigGenericAlgs.TrigGenericAlgsConf import TimeBurner
        TimeBurner= TimeBurner("DummyTimeBurner")
        TimeBurner.TimeDelay = 15
        self.L2sequenceList += [[ self.L2InputTE,     
                                  [TimeBurner], 
                                  'L2_DummyTimeBurner']]
        self.L2signatureList += [ [['L2_DummyTimeBurner']] ]
        

    ####################################
    ####################################
    def setupCostMonChain(self):
        #from TrigDetCalib.TrigDetCalibConfig import TDAQ_L2SubDetListWriter
        #l2_seq_peb = 'HLT_l2_'+self.chainName+'_peb'
        #self.L2sequenceList += [[  '',[TDAQ_L2SubDetListWriter('TDAQ_L2SubDetListWriter')],l2_seq_peb]]
        #self.L2signatureList += [ [[l2_seq_peb]] ]

        from TrigDetCalib.TrigDetCalibConfig import TDAQ_HLTSubDetListWriter        
        ef_seq_peb = 'HLT_'+self.chainName+'_peb'
        self.L2sequenceList += [['',[TDAQ_HLTSubDetListWriter('TDAQ_HLTSubDetListWriter')],ef_seq_peb]]                               
        self.L2signatureList += [ [[ef_seq_peb]] ]

    ####################################
    ####################################
    def setupL1CaloOverflow(self):
        from TrigDetCalib.TrigDetCalibConf import TrigL1CaloOverflow
        CaloOverflowMonitor = TrigL1CaloOverflow("TrigL1CaloOverflow")
        self.L2sequenceList += [[ '' , [CaloOverflowMonitor],  'L2_l1calooverflow']]
        self.L2signatureList += [ [['L2_l1calooverflow']] ]
        
    ####################################
    ####################################
    def setupIdmonTrkFS(self):

        ## necessary alg imports
        from TrigGenericAlgs.TrigGenericAlgsConf import PESA__DummyUnseededAllTEAlgo as DummyRoI
        theL2dummy = DummyRoI("DummyRoI_L2InDetMon")        
        theEFdummy = DummyRoI("DummyRoI_EFInDetMon")

        try:
            from TrigL2SiTrackFinder.TrigL2SiTrackFinder_Config import TrigL2SiTrackFinder_FullScanA, TrigL2SiTrackFinder_FullScanB
            theTrigL2SiTrackFinder_FullScanA = TrigL2SiTrackFinder_FullScanA()
            theTrigL2SiTrackFinder_FullScanB = TrigL2SiTrackFinder_FullScanB()
        except:
            theTrigL2SiTrackFinder_FullScanA = None
            theTrigL2SiTrackFinder_FullScanB = None


        try:
            from TrigL2SiTrackFinder.TrigL2SiTrackFinder_Config import TrigL2SiTrackFinder_FullScanBC
            theTrigL2SiTrackFInder_FullScanBC = TrigL2SiTrackFinder_FullScanBC()
        except:
            theTrigL2SiTrackFInder_FullScanBC = None


            
        from InDetTrigRecExample import TrigEFIDSequence, TrigEFIDInsideOut_FullScan
        theTrigEFIDSequence = TrigEFIDSequence("FullScan","fullScan","TRTOnly")
        theTrigEFIDInsideOut_FullScan = TrigEFIDInsideOut_FullScan()
        
        from TrigGenericAlgs.TrigGenericAlgsConf import  PESA__DummyUnseededAllTEAlgo

        
        self.L2SequenceList += [[self.L2InputTE,
                           [theL2dummy,
                            theTrigL2SiTrackFinder_FullScanA,
                            theTrigL2SiTrackFinder_FullScanB,
                            theTrigL2SiTrackFinder_FullScanBC],
                           'L2_FStracks_L2StarAB']]
        
        
        
        self.EFsequenceList += [[["",
                           [PESA__DummyUnseededAllTEAlgo("EFDummyAlgo")]+
                           theTrigEFIDInsideOut_FullScan,
                           'EF_FStracks']]]
        
        self.EFsequenceList += [[[ "", [theEFdummy ]+ theTrigEFIDSequence, 'EF_FStracks_TRTOnly']]]
        

        self.L2signatureList += [ [['L2_FStracks_L2StarAB']] ]
        self.EFsignatureList += [ [['EF_FStracks', 'EF_FStracks_TRTOnly']] ]
  	       
	

		#<SIGNATURE logic="1" signature_counter="1">
                #	<TRIGGERELEMENT te_name="EF_FStracks"/>
                #	<TRIGGERELEMENT te_name="EF_FStracks_TRTOnly"/>
	

        #<SEQUENCE algorithm="PESA::DummyUnseededAllTEAlgo/EFDummyAlgo InDet::Pixel_TrgClusterization/PixelClustering_FullScan_EFID InDet::SCT_TrgClusterization/SCTClustering_FullScan_EFID InDet::TRT_TrgRIO_Maker/TRTDriftCircleMaker_FullScan_EFID InDet::SiTrigSpacePointFinder/SiTrigSpacePointFinder_FullScan_EFID InDet::SiTrigSPSeededTrackFinder/SiTrigTrackFinder_FullScan_EFID InDet::InDetTrigAmbiguitySolver/TrigAmbiguitySolver_FullScan_EFID InDet::TRT_TrigTrackExtensionAlg/TRTTrackExtAlg_FullScan_EFID InDet::InDetTrigExtensProcessor/TrigExtProcessor_FullScan_EFID InDet::TrigTrackSlimmer/InDetTrigTrackSlimmer_FullScan_EFID InDet::TrigVxPrimary/TrigVxPrimary_FullScan_EFID InDet::TrigParticleCreator/InDetTrigParticleCreation_FullScan_EFID" input="" output="EF_FStracks"/>

