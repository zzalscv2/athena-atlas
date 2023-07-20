"""ComponentAccumulator configuration for MT pileup digitization

Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
"""
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from SGComps.AddressRemappingConfig import InputRenameCfg
from Digitization.RunDependentConfig import (
    maxNevtsPerXing,
    LumiProfileSvcCfg,
    NoProfileSvcCfg,
)
from Digitization.PileUpConfig import (
    LowPtMinBiasEventSelectorCfg,
    HighPtMinBiasEventSelectorCfg,
    CavernEventSelectorCfg,
    BeamGasEventSelectorCfg,
    BeamHaloEventSelectorCfg,
    StepArrayBMCfg,
    FixedArrayBMCfg,
    ArrayBMCfg,
)

from enum import Enum

def numBkgToLoad(nPerBC):
    "Given number of bkg needed (on average) per BC, returns number to load per batch"
    from math import ceil
    import numpy as np
    # We don't have scipy in the release so we have a lookup table
    # Lookup table computed using scipy.stats.poisson.isf to determine how many events need
    # to be loaded to have a probability of running out of events < 10^-6.
    boundaries = np.array([1.74752840e-03, 4.03701726e-03, 7.74263683e-03, 1.23284674e-02,
                           1.78864953e-02, 2.36448941e-02, 3.12571585e-02, 3.76493581e-02,
                           4.53487851e-02, 5.46227722e-02, 6.57933225e-02, 7.92482898e-02,
                           8.69749003e-02, 9.54548457e-02, 1.04761575e-01, 1.14975700e-01,
                           1.26185688e-01, 1.38488637e-01, 1.51991108e-01, 1.66810054e-01,
                           1.83073828e-01, 2.00923300e-01, 2.20513074e-01, 2.42012826e-01,
                           2.65608778e-01, 2.91505306e-01, 3.19926714e-01, 3.51119173e-01,
                           3.85352859e-01, 4.22924287e-01, 4.64158883e-01, 5.09413801e-01,
                           5.59081018e-01, 6.13590727e-01, 6.73415066e-01, 7.39072203e-01,
                           8.11130831e-01, 8.90215085e-01, 9.77009957e-01, 1.07226722e+00,
                           1.17681195e+00, 1.29154967e+00, 1.41747416e+00, 1.55567614e+00,
                           1.70735265e+00, 1.87381742e+00, 2.05651231e+00, 2.25701972e+00,
                           2.47707636e+00, 2.71858824e+00, 2.98364724e+00, 3.27454916e+00,
                           3.59381366e+00, 3.94420606e+00, 4.32876128e+00, 4.75081016e+00,
                           5.21400829e+00, 5.72236766e+00, 6.28029144e+00, 6.89261210e+00,
                           7.56463328e+00, 8.30217568e+00, 9.11162756e+00, 1.00000000e+01])
    n_to_load = np.array([  3.,   4.,   5.,   6.,   7.,   8.,   9.,  10.,  11.,  12.,  13.,
                           14.,  15.,  16.,  17.,  18.,  19.,  20.,  21.,  22.,  23.,  24.,
                           26.,  27.,  29.,  31.,  33.,  35.,  37.,  39.,  42.,  44.,  47.,
                           51.,  54.,  58.,  62.,  66.,  71.,  76.,  81.,  88.,  94., 101.,
                          109., 117., 126., 136., 147., 158., 171., 185., 200., 216., 234.,
                          253., 275., 298., 323., 350., 380., 413., 448., 487.])
    if nPerBC > 10:
        return int(ceil(1.224 * nPerBC * 39)) # Table only goes up to 10 per bunch crossing
    return int(n_to_load[np.searchsorted(boundaries, nPerBC, side='right')])

class PUBkgKind(Enum):
    LOWPT = 1
    HIGHPT = 2
    CAVERN = 3
    BEAMGAS = 4
    BEAMHALO = 5


def HSInputRenameCfg():
    acc = ComponentAccumulator()
    acc.merge(InputRenameCfg("xAOD::EventInfo", "EventInfo", "HSEventInfo"))
    acc.merge(InputRenameCfg("xAOD::EventAuxInfo", "EventInfoAux.", "HSEventInfoAux."))
    acc.merge(InputRenameCfg("EventInfo", "EventInfo", "HSEventInfo"))
    acc.merge(InputRenameCfg("McEventCollection", "TruthEvent", "HSTruthEvent"))
    acc.merge(InputRenameCfg("TrackRecordCollection", "MuonEntryLayer", "HSMuonEntryLayer"))
    return acc


def BatchedMinbiasSvcCfg(flags, name="LowPtMinbiasSvc", kind=PUBkgKind.LOWPT, **kwargs):
    flags.dump()
    acc = ComponentAccumulator()
    skip = flags.Exec.SkipEvents
    n_evt = flags.Exec.MaxEvents
    n_bc = (
        flags.Digitization.PU.FinalBunchCrossing
        - flags.Digitization.PU.InitialBunchCrossing
        + 1
    )

    if kind == PUBkgKind.LOWPT:
        acc.merge(LowPtMinBiasEventSelectorCfg(flags))
        kwargs.setdefault("OnDemandMB", False)
        kwargs.setdefault("MBBatchSize", 10000)
        # kwargs.setdefault("MBBatchSize", 1.3 * flags.Digitization.PU.NumberOfLowPtMinBias * n_bc)
        kwargs.setdefault("NSimultaneousBatches", 1)
        kwargs.setdefault("SkippedHSEvents", skip)
        kwargs.setdefault("HSBatchSize", 128)
        evt_per_batch = kwargs["HSBatchSize"]
        actualNHSEventsPerBatch = (
            (skip // evt_per_batch) * [0]
            + (skip % evt_per_batch != 0) * [evt_per_batch - (skip % evt_per_batch)]
            + (n_evt // evt_per_batch) * [evt_per_batch]
            + (n_evt % evt_per_batch != 0) * [n_evt % evt_per_batch]
        )
        kwargs["actualNHSEventsPerBatch"] = actualNHSEventsPerBatch
        kwargs.setdefault(
            "BkgEventSelector", acc.getService("LowPtMinBiasEventSelector")
        )
    elif kind == PUBkgKind.HIGHPT:
        acc.merge(HighPtMinBiasEventSelectorCfg(flags))
        kwargs.setdefault("OnDemandMB", False)
        # load enough events that the probability of running out for any given event is no more than 1e-6
        kwargs.setdefault(
            "MBBatchSize", numBkgToLoad(flags.Digitization.PU.NumberOfHighPtMinBias)
        )
        kwargs.setdefault("NSimultaneousBatches", flags.Concurrency.NumConcurrentEvents)
        kwargs.setdefault("SkippedHSEvents", skip)
        kwargs.setdefault("HSBatchSize", 1)
        evt_per_batch = kwargs["HSBatchSize"]
        actualNHSEventsPerBatch = (
            (skip // evt_per_batch) * [0]
            + (skip % evt_per_batch != 0) * [evt_per_batch - (skip % evt_per_batch)]
            + (n_evt // evt_per_batch) * [evt_per_batch]
            + (n_evt % evt_per_batch != 0) * [n_evt % evt_per_batch]
        )
        kwargs["actualNHSEventsPerBatch"] = actualNHSEventsPerBatch
        kwargs.setdefault(
            "BkgEventSelector", acc.getService("HighPtMinBiasEventSelector")
        )
    elif kind == PUBkgKind.CAVERN:
        acc.merge(CavernEventSelectorCfg(flags))
        kwargs.setdefault("OnDemandMB", False)
        kwargs.setdefault("MBBatchSize", flags.Digitization.PU.NumberOfCavern * n_bc)
        kwargs.setdefault("NSimultaneousBatches", flags.Concurrency.NumConcurrentEvents)
        kwargs.setdefault("SkippedHSEvents", skip)
        kwargs.setdefault("HSBatchSize", 1)
        evt_per_batch = kwargs["HSBatchSize"]
        actualNHSEventsPerBatch = (
            (skip // evt_per_batch) * [0]
            + (skip % evt_per_batch != 0) * [evt_per_batch - (skip % evt_per_batch)]
            + (n_evt // evt_per_batch) * [evt_per_batch]
            + (n_evt % evt_per_batch != 0) * [n_evt % evt_per_batch]
        )
        kwargs["actualNHSEventsPerBatch"] = actualNHSEventsPerBatch
        kwargs.setdefault("BkgEventSelector", acc.getService("CavernEventSelector"))
    elif kind == PUBkgKind.BEAMGAS:
        acc.merge(BeamGasEventSelectorCfg(flags))
        kwargs.setdefault("OnDemandMB", False)
        kwargs.setdefault(
            "MBBatchSize", numBkgToLoad(flags.Digitization.PU.NumberOfBeamGas)
        )
        kwargs.setdefault("NSimultaneousBatches", flags.Concurrency.NumConcurrentEvents)
        kwargs.setdefault("SkippedHSEvents", skip)
        kwargs.setdefault("HSBatchSize", 1)
        evt_per_batch = kwargs["HSBatchSize"]
        actualNHSEventsPerBatch = (
            (skip // evt_per_batch) * [0]
            + (skip % evt_per_batch != 0) * [evt_per_batch - (skip % evt_per_batch)]
            + (n_evt // evt_per_batch) * [evt_per_batch]
            + (n_evt % evt_per_batch != 0) * [n_evt % evt_per_batch]
        )
        kwargs["actualNHSEventsPerBatch"] = actualNHSEventsPerBatch
        kwargs.setdefault("BkgEventSelector", acc.getService("BeamGasEventSelector"))
    elif kind == PUBkgKind.BEAMHALO:
        acc.merge(BeamHaloEventSelectorCfg(flags))
        kwargs.setdefault("OnDemandMB", False)
        kwargs.setdefault(
            "MBBatchSize", numBkgToLoad(flags.Digitization.PU.NumberOfBeamHalo)
        )
        kwargs.setdefault("NSimultaneousBatches", flags.Concurrency.NumConcurrentEvents)
        kwargs.setdefault("SkippedHSEvents", skip)
        kwargs.setdefault("HSBatchSize", 1)
        evt_per_batch = kwargs["HSBatchSize"]
        actualNHSEventsPerBatch = (
            (skip // evt_per_batch) * [0]
            + (skip % evt_per_batch != 0) * [evt_per_batch - (skip % evt_per_batch)]
            + (n_evt // evt_per_batch) * [evt_per_batch]
            + (n_evt % evt_per_batch != 0) * [n_evt % evt_per_batch]
        )
        kwargs["actualNHSEventsPerBatch"] = actualNHSEventsPerBatch
        kwargs.setdefault("BkgEventSelector", acc.getService("BeamHaloEventSelector"))

    acc.addService(CompFactory.BatchedMinbiasSvc(name, **kwargs), primary=True)
    return acc


def PileUpMTAlgCfg(flags, **kwargs):
    kwargs.setdefault("Cardinality", flags.Concurrency.NumThreads)
    acc = ComponentAccumulator()
    # acc = BeamSpotFixerAlgCfg(flags)  # Needed currently for running on 21.0 HITS

    assert (
        not flags.Digitization.DoXingByXingPileUp
    ), "PileUpMTAlg does not support XingByXing pile-up!"
    # Bunch Structure
    if flags.Digitization.PU.BeamIntensityPattern:
        if flags.Digitization.PU.SignalPatternForSteppingCache:
            # Simulate Bunch Structure with events sliding backwards on a conveyor belt
            acc.merge(StepArrayBMCfg(flags))
            kwargs.setdefault("BeamIntSvc", acc.getService("StepArrayBM"))
        elif flags.Digitization.PU.FixedT0BunchCrossing:
            # Simulate Bunch Structure using a fixed point for the central bunch crossing
            acc.merge(FixedArrayBMCfg(flags))
            kwargs.setdefault("BeamIntSvc", acc.getService("FixedArrayBM"))
        else:
            # Simulate Bunch Structure and allow the central bunch crossing to vary
            acc.merge(ArrayBMCfg(flags))
            kwargs.setdefault("BeamIntSvc", acc.getService("ArrayBM"))

    # define inputs
    assert not flags.Input.SecondaryFiles, (
        "Found ConfigFlags.Input.SecondaryFiles = %r; "
        "double event selection is not supported "
        "by PileUpMTAlg" % (not flags.Input.SecondaryFiles)
    )
    acc.merge(HSInputRenameCfg())
    acc.merge(PoolReadCfg(flags))
    # add minbias service(s)
    if flags.Digitization.PU.LowPtMinBiasInputCols:
        svc = acc.getPrimaryAndMerge(
            BatchedMinbiasSvcCfg(flags, name="LowPtMinBiasSvc", kind=PUBkgKind.LOWPT)
        )
        kwargs.setdefault("LowPtMinbiasSvc", svc)
        kwargs.setdefault(
            "FracLowPt",
            flags.Digitization.PU.NumberOfLowPtMinBias
            / flags.Digitization.PU.NumberOfCollisions,
        )
    if flags.Digitization.PU.HighPtMinBiasInputCols:
        svc = acc.getPrimaryAndMerge(
            BatchedMinbiasSvcCfg(flags, name="HighPtMinBiasSvc", kind=PUBkgKind.HIGHPT)
        )
        kwargs.setdefault("HighPtMinbiasSvc", svc)
        kwargs.setdefault(
            "FracHighPt",
            flags.Digitization.PU.NumberOfHighPtMinBias
            / flags.Digitization.PU.NumberOfCollisions,
        )
    if flags.Digitization.PU.CavernInputCols:
        svc = acc.getPrimaryAndMerge(
            BatchedMinbiasSvcCfg(flags, name="CavernMinBiasSvc", kind=PUBkgKind.CAVERN)
        )
        kwargs.setdefault("CavernMinbiasSvc", svc)
        kwargs.setdefault("NumCavern", flags.Digitization.PU.NumberOfCavern)
    if flags.Digitization.PU.BeamGasInputCols:
        svc = acc.getPrimaryAndMerge(
            BatchedMinbiasSvcCfg(
                flags, name="BeamGasMinBiasSvc", kind=PUBkgKind.BEAMGAS
            )
        )
        kwargs.setdefault("BeamGasMinbiasSvc", svc)
        kwargs.setdefault("NumBeamGas", flags.Digitization.PU.NumberOfBeamGas)
    if flags.Digitization.PU.BeamHaloInputCols:
        svc = acc.getPrimaryAndMerge(
            BatchedMinbiasSvcCfg(
                flags, name="BeamHaloMinBiasSvc", kind=PUBkgKind.BEAMHALO
            )
        )
        kwargs.setdefault("BeamHaloMinbiasSvc", svc)
        kwargs.setdefault("NumBeamHalo", flags.Digitization.PU.NumberOfBeamHalo)

    kwargs.setdefault("SkippedHSEvents", flags.Exec.SkipEvents)
    kwargs.setdefault("BCSpacing", flags.Digitization.PU.BunchSpacing)
    kwargs.setdefault("EarliestDeltaBC", flags.Digitization.PU.InitialBunchCrossing)
    kwargs.setdefault("LatestDeltaBC", flags.Digitization.PU.FinalBunchCrossing)
    if flags.Input.RunAndLumiOverrideList:
        acc.merge(LumiProfileSvcCfg(flags))
        kwargs.setdefault("BeamLumiSvc", acc.getService("LumiProfileSvc"))
        kwargs.setdefault("AverageMu", maxNevtsPerXing(flags))
    else:
        acc.merge(NoProfileSvcCfg(flags))
        kwargs.setdefault("BeamLumiSvc", acc.getService("NoProfileSvc"))
        kwargs.setdefault("AverageMu", flags.Digitization.PU.NumberOfCollisions)
    presampling = flags.Common.ProductionStep == ProductionStep.PileUpPresampling
    if presampling:
        kwargs.setdefault("EventInfoKey", flags.Overlay.BkgPrefix + "EventInfo")
    else:
        kwargs.setdefault("EventInfoKey", "EventInfo")

    acc.addEventAlgo(
        CompFactory.PileUpMTAlg(flags.Digitization.DigiSteeringConf, **kwargs)
    )

    # write PileUpEventInfo
    if flags.Output.doWriteRDO:
        from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg

        acc.merge(
            OutputStreamCfg(
                flags,
                "RDO",
                ItemList=[
                    "xAOD::EventInfoContainer#PileUpEventInfo",
                    "xAOD::EventInfoAuxContainer#PileUpEventInfo*",
                ],
            )
        )

    return acc
