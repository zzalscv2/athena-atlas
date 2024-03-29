# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
'''
@file parseRunArgs.py
@author Riley Xu - riley.xu@cern.ch
@date July 30, 2020
@brief Configuration file that parses run arguments from transforms

This file collects all transform run argument parsing into a centralized area
for configuring various FPGATrackSim transforms.

Note that the parse functions return a dictionary that contain keys to correspond
to those in the various package tags. We don't want to use the tags directly here
to maintain good factorization. However, any changes to the tag keys must now be
copied to this file.

Usage:
    transform:
        addFPGATrackSimBanksArgs(trf.parser)
    skeleton:
        opts = parseFPGATrackSimArgs(runArgs)
        FPGATrackSimTagConfig.getTags(opts)
'''

import itertools
import PyJobTransforms.trfArgClasses as trfArgClasses
from PyJobTransforms.trfLogger import msg

###############################################################################
## FPGATrackSimMaps
###############################################################################

_FPGATrackSimMapsArgs = [
    ('mapTag', trfArgClasses.argString, 'Switch to a different tag than the default listed in FPGATrackSimMapTags'),
    ('region', trfArgClasses.argString, 'Region index or name'),
    ('pmap', trfArgClasses.argString, "plane map (first stage)"),
    ('layerOverride', trfArgClasses.argIntList, "Overrides the selection of the 1st stage logical layers in the plane map. Each entry declares a detector layer to use as a logical layer. Specify a detector layer with { SilictonTech * 1000 + DetectorZone * 100 + PhysicalLayer }"),
    ('rmap', trfArgClasses.argString, "region map"),
    ('subrmap', trfArgClasses.argString, "Subregion (slice) map"),
    ('NNmap', trfArgClasses.argString, "NN weighting map"),
    ("sampleType", trfArgClasses.argString, "Specify the type of sample being used"),
    ("withPU", trfArgClasses.argBool, "specify if sample has pile-up or not"),
]


def addFPGATrackSimMapsArgs(parser):
    '''
    Adds arguments used by FPGATrackSimMaps to a transform parser.
    '''

    parser.defineArgGroup('FPGATrackSimMaps', 'Options for FPGATrackSimMaps. '
            'These options will override the default configuration specified in FPGATrackSimMapTags.')

    def addArg(argName, argType, helpText=""):
        parser.add_argument('--' + argName,
                type=trfArgClasses.argFactory(argType, runarg=True),
                help=helpText,
                group='FPGATrackSimMaps')

    for arg in _FPGATrackSimMapsArgs:
        addArg(*arg)

###############################################################################
## FPGATrackSimBanks
###############################################################################

_FPGATrackSimBanksArgs = [
    ("bankTag", trfArgClasses.argString, "Switch to a different tag than the default listed in FPGATrackSimBankTags"),
    ("bankDir", trfArgClasses.argString, "Common directory path to be prepended to all filenames below. If not all files are in the same directory, you can change bankDir to a common parent directory or just '' and use fully-qualified paths for the individual filenames."),
    ("constants_1st", trfArgClasses.argString, "Filename of the 1st stage constants"),
    ("constants_2nd", trfArgClasses.argString, "Filename of the 2nd stage constants"),
    ("sectorBank_1st", trfArgClasses.argString, "Filename of the first stage sector bank"),
    ("sectorBank_2nd", trfArgClasses.argString, "Filename of the second stage sector bank"),
    ("sectorSlices", trfArgClasses.argString, "Filename of the sector slices")
]


def addFPGATrackSimBanksArgs(parser):
    '''
    Adds arguments used by FPGATrackSimBanks to a transform parser.
    '''

    parser.defineArgGroup('FPGATrackSimBanks', 'Optional overrides for FPGATrackSimBanks. '
            'These options will override the default configuration specified in FPGATrackSimBankTags.')

    def addArg(argName, argType, helpText=""):
        parser.add_argument('--' + argName,
                type=trfArgClasses.argFactory(argType, runarg=True),
                help=helpText,
                group='FPGATrackSimBanks')

    for arg in _FPGATrackSimBanksArgs:
        addArg(*arg)


###############################################################################
## FPGATrackSimAlgorithms
###############################################################################

_FPGATrackSimAlgorithmsArgs = [
    ("algoTag", trfArgClasses.argString, "Tag to use for algorithm configuration"),
    ("FirstInputToolN", trfArgClasses.argInt, "number of times to reuse each event in first input tool, i.e. for overlaying"),
    ("realHitsOverlay", trfArgClasses.argBool, "Use a second real hits tool"),
    ("hough", trfArgClasses.argBool, "Use Hough transform for road finding"),
    ("hough_1d", trfArgClasses.argBool, "Use Elliot's Hough transform for road finding"),
    ("hough_d0phi0", trfArgClasses.argBool, "Use 2d accumulator in d0 vs phi0 for road finding"),
    ("hough_rootoutput", trfArgClasses.argBool, "Output root output from the Hough transform"),
    ("fastMon", trfArgClasses.argBool, "only do fast monitoring"),
    ("canExtendHistRanges", trfArgClasses.argBool, "set kCanExtend bit on some monitoring histograms"),
    ("HitFiltering", trfArgClasses.argBool, "do hit / cluster filtering"),
    ("SpacePoints", trfArgClasses.argBool, "do spacepoint formation"),
    ("SpacePointFiltering", trfArgClasses.argBool, "remove hits not part of a spacepoint"),
    ("TrackNNAnalysis", trfArgClasses.argBool, "Do NN track filtering"),
    ("doFastOR", trfArgClasses.argBool, "Use fast overlap removal algorithm instead of default"),
    ("doTracking", trfArgClasses.argBool, "Do the track fitting"),
    ("secondStage", trfArgClasses.argBool, "Run second stage fitting"),
    ("EtaPatternRoadFilter", trfArgClasses.argString, "Apply a road filter"),
    ("PhiRoadFilter", trfArgClasses.argString, "Turn on PhiRoad filter with window sizes given as a list of nLayers numbers"),
    ("IdealCoordFitType", trfArgClasses.argInt, "Do ideal coordinate fits? 0 means don't do this, 1 means yes with 1st order correction, 2 means yes with 2nd order correction"),
    ("DoDeltaGPhis", trfArgClasses.argBool, "Do the track fitting based on the Hough transform, idealized geometry and delta phis"),
    ("outputHitTxt", trfArgClasses.argBool, "write out road hits to text file"),
    ("barcodeFracMatch", trfArgClasses.argFloat, "barcode fraction at which we accept a truth match"),
    ("writeOutputData", trfArgClasses.argBool, "write out Tree in the output file")
]

_FPGATrackSimHoughTransformArgs = [
    ('phi_min', trfArgClasses.argFloat),
    ('phi_max', trfArgClasses.argFloat),
    ('qpt_min', trfArgClasses.argFloat),
    ('qpt_max', trfArgClasses.argFloat),
    ('d0_min', trfArgClasses.argFloat),
    ('d0_max', trfArgClasses.argFloat),
    ('d0_slices', trfArgClasses.argIntList, "Create separate accumulators for each of the specified d0"),
    ('xBins', trfArgClasses.argInt),
    ('yBins', trfArgClasses.argInt),
    ('xBufferBins', trfArgClasses.argInt),
    ('yBufferBins', trfArgClasses.argInt),
    ('slicing', trfArgClasses.argBool),
    ('threshold', trfArgClasses.argIntList),
    ('convolution', trfArgClasses.argIntList),
    ('convSize_x', trfArgClasses.argInt),
    ('convSize_y', trfArgClasses.argInt),
    ('hitExtend_x', trfArgClasses.argString, "Hit lines will fill extra bins in x by this amount on each side, size == nLayers"),
    ('roadSliceOR', trfArgClasses.argBool, "Do slice-wise road overlap removal"),
    ('phiRangeCut', trfArgClasses.argBool),
    ('localMaxWindowSize', trfArgClasses.argInt, "Only create roads that are a local maximum within this window size. Set this to 0 to turn off local max filtering"),
    ("splitpt", trfArgClasses.argInt, "divide into multiple pt ranges"),
    ("radiiFile", trfArgClasses.argString, "file with mean layer radii for Hough1D"),
    ("fieldCorrection", trfArgClasses.argBool, "Apply corrections to hough equation due to field nonuniformity"),
]

_FPGATrackSimLRTArgs = [
    ('lrt', trfArgClasses.argBool, "Enable LRT stage"),
    ('lrt_use_basicHitFilter', trfArgClasses.argBool, "Enable basic hits filtering"),
    ('lrt_use_mlHitFilter', trfArgClasses.argBool, "Enable ML ts filtering"),
    ('lrt_use_straightTrackHT', trfArgClasses.argBool, "Use stright track approximation HT"),
    ('lrt_use_doubletHT', trfArgClasses.argBool, "Use hits doublet HT"),
    ('lrt_skip_hit_filtering', trfArgClasses.argBool, "Skip hit filtering"),
    ('lrt_straighttrack_slicing', trfArgClasses.argBool, "Turn off slicing for straight track HT"),
    ('lrt_doublet_d0_range', trfArgClasses.argFloat, "coverage in d0"),
    ('lrt_doublet_d0_bins', trfArgClasses.argInt, "number of bins in d0 dimenstion"),
    ('lrt_doublet_qpt_range', trfArgClasses.argFloat, "coverage of q/pT - effectively min pt limit"),
    ('lrt_doublet_qpt_bins', trfArgClasses.argInt, "number of bins in q/pt"),
    ('lrt_ptmin',trfArgClasses.argFloat, "minimum pT to use in evaluating efficiencies"),
    ('m_LRT_pdgID',trfArgClasses.argInt, "if only some PDGID desired in truth matching, add it here."),
    ('allowHighTruthBarcode', trfArgClasses.argBool, "Allow truth matching to take place for truth particles with barcodes > 200000. This is important for some LLP samples where the LLP decays in Geant.")
    #TODO add options for stright track HT and hit filter tools
]

def addFPGATrackSimAlgorithmsArgs(parser):
    '''
    Adds arguments used by FPGATrackSimAlgorithms to a transform parser.
    '''

    parser.defineArgGroup('FPGATrackSimAlgorithms', 'Options for FPGATrackSimAlgorithms configuration')
    parser.defineArgGroup('FPGATrackSimHoughTransform', 'Options for FPGATrackSim Hough transform')
    parser.defineArgGroup('FPGATrackSimLRT', 'Options for FPGATrackSim Hough transform')

    def addArg(group, argName, argType, helpText=""):
        parser.add_argument('--' + argName,
                type=trfArgClasses.argFactory(argType, runarg=True),
                help=helpText,
                group=group)

    for arg in _FPGATrackSimAlgorithmsArgs:
        addArg('FPGATrackSimAlgorithms', *arg)

    for arg in _FPGATrackSimHoughTransformArgs:
        addArg('FPGATrackSimHoughTransform', *arg)

    for arg in _FPGATrackSimLRTArgs:
        addArg('FPGATrackSimLRT', *arg)

_FPGATrackSimHitFilteringArgs = [
    ('doRandomRemoval', trfArgClasses.argBool),
    ('pixelHitRmFrac', trfArgClasses.argFloat),
    ('pixelClusRmFrac', trfArgClasses.argFloat),
    ('stripHitRmFrac', trfArgClasses.argFloat),
    ('stripClusRmFrac', trfArgClasses.argFloat),

    ('doStubs', trfArgClasses.argBool),
    ('stubCutsFile', trfArgClasses.argString),
    ('barrelStubDphiCut', trfArgClasses.argFloat),
    ('endcapStubDphiCut', trfArgClasses.argFloat),
    ('useNstrips', trfArgClasses.argBool),
]

def addFPGATrackSimHitFilteringArgs(parser):
    '''
    Adds arguments used by FPGATrackSimHitFiltering to a transform parser.
    '''

    parser.defineArgGroup('FPGATrackSimHitFiltering', 'Options for FPGATrackSimHitFiltering configuration')

    def addArg(group, argName, argType, helpText=""):
        parser.add_argument('--' + argName,
                type=trfArgClasses.argFactory(argType, runarg=True),
                help=helpText,
                group=group)

    for arg in _FPGATrackSimHitFilteringArgs:
        addArg('FPGATrackSimHitFiltering', *arg)



###############################################################################
## Parsing
###############################################################################

def parseFPGATrackSimArgs(runArgs):
    '''
    Collects FPGATrackSim arguments from runArgs into a dictionary of (parameter: value) pairs.
    Use the corresponding add***Args() functions above to add arguments to a transform parser.
    '''
    opts = {}
    for arg in itertools.chain(_FPGATrackSimMapsArgs, _FPGATrackSimBanksArgs, _FPGATrackSimAlgorithmsArgs, _FPGATrackSimHoughTransformArgs, _FPGATrackSimHitFilteringArgs, _FPGATrackSimLRTArgs):
        key = arg[0]
        if hasattr(runArgs, key):
            opts[key] = getattr(runArgs, key)
    msg.info("Final pars: ", opts)
    return opts
