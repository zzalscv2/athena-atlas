#!/usr/bin/env python

import sys
import unittest

from ISF_Algorithms.CollectionMergerConfig import CollectionMergerCfg
from AthenaConfiguration.ComponentAccumulator import ConfigurationError
class Test_generate_mergeable_collection_name(unittest.TestCase):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_isISFRunAndHITSMergingRequired_expectBareNameWithSuffixReturned(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Sim.ISFRun = True
        flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-03-02-00'
        mergeDict = {'ID':True, 'CALO':True, 'MUON':True}
        flags.Sim.ISF.HITSMergingRequired = mergeDict
        flags.Detector.EnableBCM = False
        flags.lock()

        bare_collection_name = 'aTestCollection'
        mergeable_collection_suffix = '_TESTSUFFIX'
        merger_input_property = 'PixelHits'
        region = 'ID'
        result, actual_collection_name = CollectionMergerCfg(flags,
                                                             bare_collection_name,
                                                             mergeable_collection_suffix,
                                                             merger_input_property,
                                                             region)
        print('***************** TEST 1 ***********************')
        result.printConfig(withDetails=True, summariseProps=True, prefix='CATest')
        expected_collection_name = 'aTestCollection_TESTSUFFIX'
        self.assertEqual(expected_collection_name, actual_collection_name)

    def test_isISFRunAndHITSMergingRequiredWithEmptyCollectionMergerAlgorithm_expectCollectionAddedToCollectionMergerAlgorithm(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Sim.ISFRun = True
        flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-03-02-00'
        mergeDict = {'ID':True, 'CALO':True, 'MUON':True}
        flags.Sim.ISF.HITSMergingRequired = mergeDict
        flags.Detector.EnableBCM = False
        flags.lock()

        bare_collection_name = 'aTestCollection'
        mergeable_collection_suffix = '_TESTSUFFIX'
        merger_input_property = 'PixelHits'
        region = 'ID'
        result, actual_collection_name = CollectionMergerCfg(flags,
                                                             bare_collection_name,
                                                             mergeable_collection_suffix,
                                                             merger_input_property,
                                                             region)
        print('***************** TEST 2 ***********************')
        result.printConfig(withDetails=True, summariseProps=True, prefix='CATest')
        actual_collection_merger_input = None
        try:
            actual_collection_merger_input = result.getEventAlgo("ISF_CollectionMerger").InputPixelHits
        except ConfigurationError:
            pass
        expected_collection_merger_input = ['aTestCollection_TESTSUFFIX']
        self.assertEqual(expected_collection_merger_input,
                         actual_collection_merger_input)

    def test_isISFRunAndNoHITSMergingRequired_expectBareCollectionNameReturned(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Sim.ISFRun = True
        flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-03-02-00'
        mergeDict = {'ID':False, 'CALO':True, 'MUON':False}
        flags.Sim.ISF.HITSMergingRequired = mergeDict
        flags.Detector.EnableBCM = False
        flags.lock()

        bare_collection_name = 'aTestCollection'
        mergeable_collection_suffix = '_TESTSUFFIX'
        merger_input_property = 'PixelHits'
        region = 'ID'
        result, actual_collection_name = CollectionMergerCfg(flags,
                                                             bare_collection_name,
                                                             mergeable_collection_suffix,
                                                             merger_input_property,
                                                             region)
        print('***************** TEST 3 ***********************')
        result.printConfig(withDetails=True, summariseProps=True, prefix='CATest')
        expected_collection_name = 'aTestCollection'
        self.assertEqual(expected_collection_name, actual_collection_name)

    def test_isISFRunAndNoHITSMergingRequiredWithEmptyCollectionMergerAlgorithm_expectCollectionMergerAlgorithmUntouched(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Sim.ISFRun = True
        flags.GeoModel.AtlasVersion = 'ATLAS-R3S-2021-03-02-00'
        mergeDict = {'ID':False, 'CALO':True, 'MUON':False}
        flags.Sim.ISF.HITSMergingRequired = mergeDict
        flags.Detector.EnableBCM = False
        flags.lock()

        bare_collection_name = 'aTestCollection'
        mergeable_collection_suffix = '_TESTSUFFIX'
        merger_input_property = 'PixelHits'
        region = 'ID'
        result, actual_collection_name = CollectionMergerCfg(flags,
                                                             bare_collection_name,
                                                             mergeable_collection_suffix,
                                                             merger_input_property,
                                                             region)
        print('***************** TEST 4 ***********************')
        result.printConfig(withDetails=True, summariseProps=True, prefix='CATest')
        actual_collection_merger_input = None
        try:
            actual_collection_merger_input = result.getEventAlgo("ISF_CollectionMerger").InputPixelHits
        except ConfigurationError:
            pass
        expected_collection_merger_input = None # we don't expect ISF_CollectionMerger to have been created
        self.assertEqual(expected_collection_merger_input,
                         actual_collection_merger_input)

    def test_isNotISFRunAndNoHITSMergingRequired_expectBareCollectioNameReturned(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Sim.ISFRun = False
        mergeDict = {'ID':False, 'CALO':True, 'MUON':False}
        flags.Sim.ISF.HITSMergingRequired = mergeDict
        flags.Detector.EnableBCM = False
        flags.lock()

        bare_collection_name = 'aTestCollection'
        mergeable_collection_suffix = '_TESTSUFFIX'
        merger_input_property = 'PixelHits'
        region = 'ID'
        result, actual_collection_name = CollectionMergerCfg(flags,
                                                             bare_collection_name,
                                                             mergeable_collection_suffix,
                                                             merger_input_property,
                                                             region)
        print('***************** TEST 5 ***********************')
        result.printConfig(withDetails=True, summariseProps=True, prefix='CATest')
        expected_collection_name = 'aTestCollection'
        self.assertEqual(expected_collection_name, actual_collection_name)

    def test_isNotISFRunAndHITSMergingRequired_expectBareCollectioNameReturned(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Sim.ISFRun = False
        mergeDict = {'ID':True, 'CALO':True, 'MUON':True}
        flags.Sim.ISF.HITSMergingRequired = mergeDict
        flags.Detector.EnableBCM = False
        flags.lock()

        bare_collection_name = 'aTestCollection'
        mergeable_collection_suffix = '_TESTSUFFIX'
        merger_input_property = 'PixelHits'
        region = 'ID'
        result, actual_collection_name = CollectionMergerCfg(flags,
                                                             bare_collection_name,
                                                             mergeable_collection_suffix,
                                                             merger_input_property,
                                                             region)
        print('***************** TEST 6 ***********************')
        result.printConfig(withDetails=True, summariseProps=True, prefix='CATest')
        expected_collection_name = 'aTestCollection'
        self.assertEqual(expected_collection_name, actual_collection_name)

    def test_isNotISFRunAndNoHITSMergingRequired_expectCollectionMergerAlgorithmUntouched(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Sim.ISFRun = False
        mergeDict = {'ID':True, 'CALO':True, 'MUON':True}
        flags.Sim.ISF.HITSMergingRequired = mergeDict
        flags.Detector.EnableBCM = False
        flags.lock()

        bare_collection_name = 'aTestCollection'
        mergeable_collection_suffix = '_TESTSUFFIX'
        merger_input_property = 'PixelHits'
        region = 'ID'
        result, actual_collection_name = CollectionMergerCfg(flags,
                                                             bare_collection_name,
                                                             mergeable_collection_suffix,
                                                             merger_input_property,
                                                             region)
        print('***************** TEST 7 ***********************')
        result.printConfig(withDetails=True, summariseProps=True, prefix='CATest')
        actual_collection_merger_input = None
        try:
            actual_collection_merger_input = result.getEventAlgo("ISF_CollectionMerger").InputPixelHits
        except ConfigurationError:
            pass
        expected_collection_merger_input = None # we don't expect ISF_CollectionMerger to have been created
        self.assertEqual(expected_collection_merger_input,
                         actual_collection_merger_input)

    def test_isNotISFRunAndHITSMergingRequired_expectCollectionMergerAlgorithmUntouched(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        flags.Sim.ISFRun = False
        mergeDict = {'ID':True, 'CALO':True, 'MUON':True}
        flags.Sim.ISF.HITSMergingRequired = mergeDict
        flags.Detector.EnableBCM = False
        flags.lock()

        bare_collection_name = 'aTestCollection'
        mergeable_collection_suffix = '_TESTSUFFIX'
        merger_input_property = 'PixelHits'
        region = 'ID'
        result, actual_collection_name = CollectionMergerCfg(flags,
                                                             bare_collection_name,
                                                             mergeable_collection_suffix,
                                                             merger_input_property,
                                                             region)
        print('***************** TEST 8 ***********************')
        result.printConfig(withDetails=True, summariseProps=True, prefix='CATest')
        actual_collection_merger_input = None
        try:
            actual_collection_merger_input = result.getEventAlgo("ISF_CollectionMerger").InputPixelHits
        except ConfigurationError:
            pass
        expected_collection_merger_input = None # we don't expect ISF_CollectionMerger to have been created
        self.assertEqual(expected_collection_merger_input,
                         actual_collection_merger_input)


if __name__ == '__main__':
    unittest.main()
