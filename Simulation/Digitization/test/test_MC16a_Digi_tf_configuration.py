#!/usr/bin/env python

from __future__ import print_function
import pickle
import subprocess
import unittest
import os
import six

def HepMCVersion():
    try:
        from AthenaPython.PyAthena import HepMC3 # noqa: F401
        HepMCVersion=3
    except ImportError:
        HepMCVersion=2
    return HepMCVersion

class TestDigitizationMC16a(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        os.environ["TRF_ECHO"] = "1"
        config_picklefilename = 'DigitizationMC16a_config.pkl'
        command = [
            'Digi_tf.py',
            '--athenaopts', '"--config-only={}"'.format(config_picklefilename),
            '--inputHITSFile', '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.simul.HITS.e4993_s3091/HITS.10504490._000425.pool.root.1',
            '--conditionsTag', 'default:OFLCOND-MC16-SDR-25-02',
            '--geometryVersion', 'default:ATLAS-R2-2016-01-00-01',
            '--inputHighPtMinbiasHitsFile', '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361239.Pythia8EvtGen_A3NNPDF23LO_minbias_inelastic_high.merge.HITS.e4981_s3087_s3089/HITS.10501933._000005.pool.root.1',
            '--inputLowPtMinbiasHitsFile', '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.361238.Pythia8EvtGen_A3NNPDF23LO_minbias_inelastic_low.merge.HITS.e4981_s3087_s3089/HITS.10501925._000003.pool.root.1',
            '--jobNumber', '1',
            '--maxEvents', '25',
            '--numberOfCavernBkg', '0',
            '--numberOfHighPtMinBias', '0.116075313',
            '--numberOfLowPtMinBias', '44.3839246425',
            '--outputRDOFile', 'mc16a_ttbar.RDO.pool.root',
            '--digiSteeringConf', 'StandardSignalOnlyTruth',
            '--postExec', 'all:CfgMgr.MessageSvc().setError+=["HepMcParticleLink"]', 'condSeq.LArAutoCorrTotalCondAlg.deltaBunch=1',
            '--postInclude', 'default:PyJobTransforms/UseFrontier.py',
            '--pileupFinalBunch', '6',
            '--preExec', 'all:from AthenaCommon.BeamFlags import jobproperties;jobproperties.Beam.numberOfCollisions.set_Value_and_Lock(20.0);from LArROD.LArRODFlags import larRODFlags;larRODFlags.NumberOfCollisions.set_Value_and_Lock(20);larRODFlags.nSamples.set_Value_and_Lock(4);larRODFlags.doOFCPileupOptimization.set_Value_and_Lock(True);larRODFlags.firstSample.set_Value_and_Lock(0);larRODFlags.useHighestGainAutoCorr.set_Value_and_Lock(True)',
            '--preInclude', 'HITtoRDO:Digitization/ForceUseOfPileUpTools.py,SimulationJobOptions/preInclude.PileUpBunchTrainsMC15_2015_25ns_Config1.py,RunDependentSimData/configLumi_run284500_mc16a.py',
            '--skipEvents', '0',
            # would otherwise fail due to missing RDO file:
            '--outputFileValidation', 'False',
        ]
        subprocess.check_call(command)

        with open(config_picklefilename, 'rb') as picklefile:
            job_config_dict = pickle.load(picklefile)

        cls._job_config_dict = job_config_dict

    def _assert_Algorithm_property_unordered_equal(self,
                                                   algorithm_name,
                                                   property_name,
                                                   expected_property_value):
        algconfigdict = self._job_config_dict[algorithm_name]
        actual_property_value_as_str = algconfigdict[property_name]
        # need to evaluate to obtain actual Python object
        actual_property_value = eval(actual_property_value_as_str)
        expected_property_value_sorted = sorted(expected_property_value)
        actual_property_value_sorted = sorted(actual_property_value)

        failure_message = "{algorithm}.{property} has a different " \
                          "value than expected!\n" \
                          "expected (sorted):\n" \
                          " {expected}\n" \
                          "actual (sorted):\n" \
                          " {actual}".format(
                              algorithm=algorithm_name,
                              property=property_name,
                              expected=expected_property_value_sorted,
                              actual=actual_property_value_sorted)
        self.assertEqual(expected_property_value_sorted,
                         actual_property_value_sorted,
                         failure_message)


    def _assert_Algorithm_property_ordered_equal(self,
                                                 algorithm_name,
                                                 property_name,
                                                 expected_property_value):
        algconfigdict = self._job_config_dict[algorithm_name]
        actual_property_value_as_str = algconfigdict[property_name]
        # need to evaluate to obtain actual Python object
        actual_property_value = eval(actual_property_value_as_str)

        failure_message = "{algorithm}.{property} has a different " \
                          "value than expected!\n" \
                          "expected:\n" \
                          " {expected}\n" \
                          "actual:\n" \
                          " {actual}".format(
                              algorithm=algorithm_name,
                              property=property_name,
                              expected=str(expected_property_value),
                              actual=str(actual_property_value))
        self.assertEqual(expected_property_value,
                         actual_property_value,
                         failure_message)


    def _assert_Algorithm_property_equal(self,
                                   algorithm_name,
                                   property_name,
                                   expected_property_value):
        algconfigdict = self._job_config_dict[algorithm_name]
        actual_property_value = algconfigdict[property_name]

        failure_message = "{algorithm}.{property} has a different " \
                          "value than expected!\n" \
                          "expected:\n" \
                          " {expected}\n" \
                          "actual:\n" \
                          " {actual}".format(
                              algorithm=algorithm_name,
                              property=property_name,
                              expected=str(expected_property_value),
                              actual=str(actual_property_value))
        self.assertEqual(str(expected_property_value),
                         str(actual_property_value),
                         failure_message)


    def _detailed_ConfigurablePropertiesCheck(self,
                                              tested_configurable_name,
                                              expected_property_list,
                                              expected_nonstring_properties,
                                              expected_string_properties):
        testedConfigurableDict = self._job_config_dict[tested_configurable_name]
        actual_list = testedConfigurableDict.keys()
        expected_property_value_sorted = sorted(expected_property_list)
        actual_property_value_sorted = sorted(actual_list)

        failure_message = "{configurable}.keys() has a different " \
                          "value than expected!\n" \
                          "expected (sorted):\n" \
                          " {expected}\n" \
                          "actual (sorted):\n" \
                          " {actual}".format(
                              configurable=tested_configurable_name,
                              expected=expected_property_value_sorted,
                              actual=actual_property_value_sorted)

        self.assertEqual(expected_property_value_sorted,
                         actual_property_value_sorted,
                         failure_message)

        for key, value in six.iteritems (expected_nonstring_properties):
            expected_Property = eval(value)
            self._assert_Algorithm_property_equal(
                tested_configurable_name,
                key,
                expected_Property)

        for key, value in six.iteritems (expected_string_properties):
            expected_Property = value
            self._assert_Algorithm_property_equal(
                tested_configurable_name,
                key,
                expected_Property)


    def test___PileUpToolsAlg_is_second_in_AthAlgSeq(self):
        expected_AlgSequence = ['Simulation::BeamSpotFixerAlg/BeamSpotFixerAlg', 'PileUpToolsAlg/StandardSignalOnlyTruthPileUpToolsAlg', 'LArHitEMapToDigitAlg/LArHitEMapToDigitAlg', 'LArRawChannelBuilderAlg/LArRawChannelBuilder', 'TileDigitsMaker/TileDigitsMaker', 'TileDQstatusAlg/TileDQstatusAlg', 'TileRawChannelMaker/TileRChMaker', 'TileRawChannelToL2/TileRawChannelToL2', 'CscDigitToCscRDO/CscDigitToCscRDO', 'MdtDigitToMdtRDO/MdtDigitToMdtRDO', 'RpcDigitToRpcRDO/RpcDigitToRpcRDO', 'TgcDigitToTgcRDO/TgcDigitToTgcRDO', 'LArTTL1Maker/LArTTL1Maker', 'TileHitToTTL1/TileHitToTTL1', 'TilePulseForTileMuonReceiver/TilePulseForTileMuonReceiver', 'TileMuonReceiverDecision/TileMuonReceiverDecision']
        ignore_Algs = ['EventInfoTagBuilder/EventInfoTagBuilder']
        ath_alg_seqence_as_str = self._job_config_dict['AthAlgSeq']['Members']
        # need to evaluate to obtain actual Python object
        ath_alg_seqence_list = [ alg for alg in eval(ath_alg_seqence_as_str) if alg not in ignore_Algs ]

        actual_2nd_ath_alg_sequence_entry = ath_alg_seqence_list[1]
        print(ath_alg_seqence_list)
        expected_2nd_ath_alg_sequence_entry = "PileUpToolsAlg/StandardSignalOnlyTruthPileUpToolsAlg"
        self.assertEqual(expected_2nd_ath_alg_sequence_entry,
                         actual_2nd_ath_alg_sequence_entry)
        self.assertEqual(expected_AlgSequence, ath_alg_seqence_list)


    def test___StandardSignalOnlyTruthPileUpToolsAlg_PileUpTools(self):
        expected_PileUpTools = ['SimpleMergeMcEventCollTool/SignalOnlyMcEventCollTool','MergeTruthJetsTool/MergeAntiKt4TruthJetsTool','MergeTruthJetsTool/MergeAntiKt6TruthJetsTool','MergeTrackRecordCollTool/MergeMuonEntryLayerTool','MergeCalibHitsTool/MergeCalibHitsTool','BCM_DigitizationTool/BCM_DigitizationTool','PixelDigitizationTool/PixelDigitizationTool','SCT_DigitizationTool/SCT_DigitizationTool','TRTDigitizationTool/TRTDigitizationTool','LArPileUpTool/LArPileUpTool','TileHitVecToCntTool/TileHitVecToCntTool','CscDigitizationTool/CscDigitizationTool','MdtDigitizationTool/MdtDigitizationTool','RpcDigitizationTool/RpcDigitizationTool','TgcDigitizationTool/TgcDigitizationTool']
        if HepMCVersion() == 2:
            expected_PileUpTools = ['MergeMcEventCollTool/SignalOnlyMcEventCollTool','MergeTruthJetsTool/MergeAntiKt4TruthJetsTool','MergeTruthJetsTool/MergeAntiKt6TruthJetsTool','MergeTrackRecordCollTool/MergeMuonEntryLayerTool','MergeCalibHitsTool/MergeCalibHitsTool','BCM_DigitizationTool/BCM_DigitizationTool','PixelDigitizationTool/PixelDigitizationTool','SCT_DigitizationTool/SCT_DigitizationTool','TRTDigitizationTool/TRTDigitizationTool','LArPileUpTool/LArPileUpTool','TileHitVecToCntTool/TileHitVecToCntTool','CscDigitizationTool/CscDigitizationTool','MdtDigitizationTool/MdtDigitizationTool','RpcDigitizationTool/RpcDigitizationTool','TgcDigitizationTool/TgcDigitizationTool']
        self._assert_Algorithm_property_unordered_equal(
            'StandardSignalOnlyTruthPileUpToolsAlg',
            'PileUpTools',
            expected_PileUpTools)


    def test___MergeAntiKt4TruthJetsTool_properties(self):
        tested_configurable_name = 'StandardSignalOnlyTruthPileUpToolsAlg.MergeAntiKt4TruthJetsTool'
        expected_property_list = ['DetStore', 'EvtStore', 'ExtraInputs', 'ExtraOutputs', 'FirstXing', 'InTimeOutputTruthJetCollKey', 'LastXing', 'OutOfTimeTruthJetCollKey', 'PileUpMergeSvc']
        expected_nonstring_properties = {'LastXing': '75', 'FirstXing': '-125'}
        expected_string_properties = {} # Not checking any specific property values
        self._detailed_ConfigurablePropertiesCheck(
            tested_configurable_name,
            expected_property_list,
            expected_nonstring_properties,
            expected_string_properties)


    def test___MergeMuonEntryLayerTool_properties(self):
        tested_configurable_name = 'StandardSignalOnlyTruthPileUpToolsAlg.MergeMuonEntryLayerTool'
        expected_property_list = ['DetStore', 'EvtStore', 'ExtraInputs', 'ExtraOutputs', 'FirstXing', 'LastXing', 'PileUpMergeSvc', 'TrackRecordCollKey', 'TrackRecordCollOutputKey']
        expected_nonstring_properties = {'LastXing': '1', 'FirstXing': '-1'}
        expected_string_properties = {} # Not checking any specific property values
        self._detailed_ConfigurablePropertiesCheck(
            tested_configurable_name,
            expected_property_list,
            expected_nonstring_properties,
            expected_string_properties)


    def test___MergeCalibHitsTool_properties(self):
        tested_configurable_name = 'StandardSignalOnlyTruthPileUpToolsAlg.MergeCalibHitsTool'
        expected_property_list = ['DetStore', 'EvtStore', 'ExtraInputs', 'ExtraOutputs', 'FirstXing', 'LastXing', 'PileUpMergeSvc']
        expected_nonstring_properties = {'LastXing': '1', 'FirstXing': '-1'}
        expected_string_properties = {} # Not checking any specific property values
        self._detailed_ConfigurablePropertiesCheck(
            tested_configurable_name,
            expected_property_list,
            expected_nonstring_properties,
            expected_string_properties)


    def test___BCM_DigitizationTool_properties(self):
        tested_configurable_name = 'StandardSignalOnlyTruthPileUpToolsAlg.BCM_DigitizationTool'
        expected_property_list = ['DetStore', 'EffDistanceParam', 'EffSharpnessParam', 'EvtStore', 'ExtraInputs', 'ExtraOutputs', 'FirstXing', 'HitCollName', 'LastXing', 'MergeSvc', 'MIPDeposit', 'ModNoise', 'ModSignal', 'NinoThr', 'OutputRDOKey', 'OutputSDOKey', 'RndmSvc', 'TimeDelay']
        expected_nonstring_properties = {'LastXing': '0', 'FirstXing': '-25'}
        expected_string_properties = {'HitCollName': 'BCMHits'}
        self._detailed_ConfigurablePropertiesCheck(
            tested_configurable_name,
            expected_property_list,
            expected_nonstring_properties,
            expected_string_properties)


    def test___SignalOnlyMcEventCollTool_properties(self):
        tested_configurable_name = 'StandardSignalOnlyTruthPileUpToolsAlg.SignalOnlyMcEventCollTool'
        expected_property_list = ['TruthCollInputKey', 'TruthCollOutputKey', 'OnlySaveSignalTruth', 'OverrideEventNumbers', 'PileUpMergeSvc', 'ExtraOutputs', 'LastXing', 'ExtraInputs', 'DetStore', 'FirstXing', 'EvtStore']
        expected_nonstring_properties = {'ExtraOutputs': '[]', 'LastXing': '0', 'OverrideEventNumbers': 'True', 'ExtraInputs': '[]', 'FirstXing': '0'}
        expected_string_properties = {'TruthCollInputKey': 'TruthEvent', 'TruthCollOutputKey': 'TruthEvent'}
        if HepMCVersion() == 2:
            expected_property_list = ['KeepUnstable', 'SaveInTimeMinBias', 'AddBackgroundCollisionVertices', 'zRange', 'TruthCollInputKey', 'TruthCollOutputKey', 'OnlySaveSignalTruth', 'OutOfTimeAbsEtaMax', 'PileUpMergeSvc', 'rRange', 'CompressOutputCollection', 'ExtraOutputs', 'AbsEtaMax', 'LastXing', 'SaveOutOfTimeMinBias', 'ExtraInputs', 'DetStore', 'SaveCavernBackground', 'FirstXing', 'SaveRestOfMinBias', 'EvtStore', 'HighTimeToKeep', 'LowTimeToKeep']
            expected_nonstring_properties = {'KeepUnstable': 'False', 'SaveInTimeMinBias': 'True', 'AddBackgroundCollisionVertices': 'True', 'zRange': '200.0', 'OnlySaveSignalTruth': 'True', 'OutOfTimeAbsEtaMax': '3.0', 'rRange': '20.0', 'CompressOutputCollection': 'False', 'ExtraOutputs': '[]', 'AbsEtaMax': '5.0', 'LastXing': '0', 'SaveOutOfTimeMinBias': 'True', 'ExtraInputs': '[]', 'SaveCavernBackground': 'True', 'FirstXing': '0', 'SaveRestOfMinBias': 'False', 'HighTimeToKeep': '50.5', 'LowTimeToKeep': '-50.5'}
        self._detailed_ConfigurablePropertiesCheck(
            tested_configurable_name,
            expected_property_list,
            expected_nonstring_properties,
            expected_string_properties)


    def test___PixelDigitizationTool_properties(self):
        tested_configurable_name = 'StandardSignalOnlyTruthPileUpToolsAlg.PixelDigitizationTool'
        expected_property_list = ['ChargeTools', 'DetStore', 'EnergyDepositionTool', 'EvtStore', 'ExtraInputs', 'ExtraOutputs', 'FirstXing', 'FrontEndSimTools', 'HardScatterSplittingMode', 'InputObjectName', 'LastXing', 'PileUpMergeSvc', 'PixelDetEleCollKey', 'RDOCollName', 'RndmSvc', 'SDOCollName']
        expected_nonstring_properties = {'LastXing': '25', 'FirstXing': '-25'}
        expected_string_properties = {'InputObjectName': 'PixelHits'}
        self._detailed_ConfigurablePropertiesCheck(
            tested_configurable_name,
            expected_property_list,
            expected_nonstring_properties,
            expected_string_properties)


    def test___SCT_DigitizationTool_properties(self):
        tested_configurable_name = 'StandardSignalOnlyTruthPileUpToolsAlg.SCT_DigitizationTool'
        expected_property_list = ['BarrelOnly', 'DetStore', 'EnableHits', 'EvtStore', 'ExtraInputs', 'ExtraOutputs', 'FirstXing', 'FrontEnd', 'HardScatterSplittingMode', 'InputObjectName', 'LastXing', 'MergeSvc', 'OutputObjectName', 'OutputSDOName', 'RandomDisabledCellGenerator', 'RndmSvc', 'SCTDetEleCollKey', 'SurfaceChargesGenerator']
        expected_nonstring_properties = {'LastXing': '25', 'FirstXing': '-50'}
        expected_string_properties = {'InputObjectName': 'SCT_Hits', 'OutputObjectName': 'SCT_RDOs', 'OutputSDOName': 'SCT_SDO_Map'}
        self._detailed_ConfigurablePropertiesCheck(
            tested_configurable_name,
            expected_property_list,
            expected_nonstring_properties,
            expected_string_properties)


    def test___TRTDigitizationTool_properties(self):
        tested_configurable_name = 'StandardSignalOnlyTruthPileUpToolsAlg.TRTDigitizationTool'
        expected_property_list = ['AtlasFieldCacheCondObj', 'DataObjectName', 'DetStore', 'DigVersContainerKey', 'EvtStore', 'ExtraInputs', 'ExtraOutputs', 'FirstXing', 'HardScatterSplittingMode', 'InDetTRTCalDbTool', 'InDetTRTStrawStatusSummaryTool', 'LastXing', 'MergeSvc', 'OutputObjectName', 'OutputSDOName', 'Override_TrtRangeCutProperty', 'PAI_Tool_Ar', 'PAI_Tool_Kr', 'PAI_Tool_Xe', 'RandomSeedOffset', 'RndmSvc', 'SimDriftTimeTool', 'TRT_StrawNeighbourSvc']
        expected_nonstring_properties = {'LastXing': '50', 'FirstXing': '-50'}
        expected_string_properties = {'OutputObjectName': 'TRT_RDOs', 'OutputSDOName': 'TRT_SDO_Map'} # No Input name property
        self._detailed_ConfigurablePropertiesCheck(
            tested_configurable_name,
            expected_property_list,
            expected_nonstring_properties,
            expected_string_properties)


    def test___LArPileUpTool_properties(self):
        tested_configurable_name = 'StandardSignalOnlyTruthPileUpToolsAlg.LArPileUpTool'
        expected_property_list = ['CablingKey', 'DetStore', 'CaloDetDescrManager', 'DoDigiTruthReconstruction', 'EvtStore', 'ExtraInputs', 'ExtraOutputs', 'FirstXing', 'InputDigitContainer', 'InputHitContainers', 'LArHitEMapKey', 'LArHitEMap_DigiHSTruthKey', 'LArXTalkWeightGlobal', 'LastXing', 'NoiseOnOff', 'PileUp', 'PileUpMergeSvc', 'RandomSeedOffset', 'RndmEvtOverlay', 'RndmSvc', 'TriggerTimeToolName', 'useLArFloat']
        expected_string_properties = {}
        expected_nonstring_properties = {'LastXing': '101', 'FirstXing': '-751',
                                         'InputHitContainers': '["LArHitEMB","LArHitEMEC","LArHitHEC","LArHitFCAL"]'}
        self._detailed_ConfigurablePropertiesCheck(
            tested_configurable_name,
            expected_property_list,
            expected_nonstring_properties,
            expected_string_properties)


    def test___TileHitVecToCntTool_properties(self):
        tested_configurable_name = 'StandardSignalOnlyTruthPileUpToolsAlg.TileHitVecToCntTool'
        expected_property_list = ['DetStore', 'DoHSTruthReconstruction', 'EvtStore', 'ExtraInputs', 'ExtraOutputs', 'FirstXing', 'LastXing', 'PileUp', 'PileUpMergeSvc', 'RndmSvc', 'TileCablingSvc', 'TileHitContainer', 'TileHitContainer_DigiHSTruth', 'TileHitVectors', 'TileSamplingFraction', 'TriggerTimeTool']
        expected_nonstring_properties = {'LastXing': '150', 'FirstXing': '-200', 'TileHitVectors': '["TileHitVec", "MBTSHits"]'}
        expected_string_properties = {'TileHitContainer': 'TileHitCnt'}
        self._detailed_ConfigurablePropertiesCheck(
            tested_configurable_name,
            expected_property_list,
            expected_nonstring_properties,
            expected_string_properties)


    def test___CscDigitizationTool_properties(self):
        tested_configurable_name = 'StandardSignalOnlyTruthPileUpToolsAlg.CscDigitizationTool'
        expected_property_list = ['CSCSimDataCollectionOutputName', 'DetStore', 'DriftVelocity', 'ElectronEnergy', 'EvtStore', 'ExtraInputs', 'ExtraOutputs', 'FirstXing', 'InputObjectName', 'LastXing', 'MuonIdHelperSvc', 'NewDigitEDM', 'OutputObjectName', 'PileUpMergeSvc', 'RndmSvc', 'WindowLowerOffset', 'WindowUpperOffset', 'amplification', 'cscCalibTool', 'isPileUp', 'pedestal']
        expected_nonstring_properties = {'LastXing': '175', 'FirstXing': '-375'}
        expected_string_properties = {'InputObjectName': 'CSC_Hits', 'OutputObjectName': 'CSC_DIGITS'}
        self._detailed_ConfigurablePropertiesCheck(
            tested_configurable_name,
            expected_property_list,
            expected_nonstring_properties,
            expected_string_properties)


    def test___MdtDigitizationTool_properties(self):
        tested_configurable_name = 'StandardSignalOnlyTruthPileUpToolsAlg.MdtDigitizationTool'
        expected_property_list = ['CalibrationDbTool', 'DetStore', 'DigitizationTool', 'DiscardEarlyHits', 'DoQballCharge', 'EvtStore', 'ExtraInputs', 'ExtraOutputs', 'FirstXing', 'InputObjectName', 'LastXing', 'MuonIdHelperSvc', 'OutputObjectName', 'OutputSDOName', 'PileUpMergeSvc', 'ReadKey', 'RndmSvc', 'UseTof']
        expected_nonstring_properties = {'LastXing': '150', 'FirstXing': '-800'}
        expected_string_properties = {} # Not checking any specific property values
        self._detailed_ConfigurablePropertiesCheck(
            tested_configurable_name,
            expected_property_list,
            expected_nonstring_properties,
            expected_string_properties)


    def test___RpcDigitizationTool_properties(self):
        tested_configurable_name = 'StandardSignalOnlyTruthPileUpToolsAlg.RpcDigitizationTool'
        expected_property_list = ['ClusterSize1_2uncorr', 'ClusterSize_fromCOOL', 'CutProjectedTracks', 'DeadTime', 'DetStore', 'DumpFromDbFirst', 'EfficiencyPatchForBMShighEta', 'Efficiency_fromCOOL', 'EvtStore', 'ExtraInputs', 'ExtraOutputs', 'FirstXing', 'FracClusterSize1_A', 'FracClusterSize1_C', 'FracClusterSize2_A', 'FracClusterSize2_C', 'FracClusterSizeTail_A', 'FracClusterSizeTail_C', 'IgnoreRunDependentConfig', 'InputObjectName', 'LastXing', 'MeanClusterSizeTail_A', 'MeanClusterSizeTail_C', 'OnlyEtaEff_A', 'OnlyEtaEff_C', 'OnlyPhiEff_A', 'OnlyPhiEff_C', 'OutputObjectName', 'OutputSDOName', 'PanelId_OFF_fromlist', 'PanelId_OK_fromlist', 'PatchForRpcTime', 'PatchForRpcTimeShift', 'PhiAndEtaEff_A', 'PhiAndEtaEff_C', 'PileUpMergeSvc', 'PrintCalibrationVector', 'RPCInfoFromDb', 'ReadKey', 'RndmSvc', 'testbeam_clustersize', 'turnON_clustersize', 'turnON_efficiency']
        expected_nonstring_properties = {'LastXing': '125', 'FirstXing': '-150'}
        expected_string_properties = {} # Not checking any specific property values
        self._detailed_ConfigurablePropertiesCheck(
            tested_configurable_name,
            expected_property_list,
            expected_nonstring_properties,
            expected_string_properties)


    def test___TgcDigitizationTool_properties(self):
        tested_configurable_name = 'StandardSignalOnlyTruthPileUpToolsAlg.TgcDigitizationTool'
        expected_property_list = ['DetStore', 'EvtStore', 'ExtraInputs', 'ExtraOutputs', 'FirstXing', 'FourBunchDigitization', 'InputObjectName', 'LastXing', 'OutputObjectName', 'OutputSDOName', 'PileUpMergeSvc', 'RndmSvc', 'TGCDigitASDposKey', 'TGCDigitCrosstalkKey', 'TGCDigitTimeOffsetKey']
        expected_nonstring_properties = {'LastXing': '75', 'FirstXing': '-50'}
        expected_string_properties = {} # Not checking any specific property values
        self._detailed_ConfigurablePropertiesCheck(
            tested_configurable_name,
            expected_property_list,
            expected_nonstring_properties,
            expected_string_properties)


if __name__ == '__main__':
    unittest.main()
