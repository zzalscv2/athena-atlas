# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CFElements import parOR, seqAND

def getSecondStageBjetTracking( flags, inputRoI, inputVertex, inputJets ):
    algSequence = []


    # Second stage of Fast tracking (for precision tracking preparation)
    from TrigInDetConfig.ConfigSettings import getInDetTrigConfig
    IDTrigConfig = getInDetTrigConfig( 'bjet' )

    from TrigInDetConfig.InDetTrigFastTracking import makeInDetTrigFastTracking

    viewAlgs, viewVerify = makeInDetTrigFastTracking(flags, config = IDTrigConfig, rois=inputRoI)

    viewVerify.DataObjects += [( 'TrigRoiDescriptorCollection' , 'StoreGateSvc+%s' % inputRoI ),
                               ( 'xAOD::VertexContainer' , 'StoreGateSvc+%s' % inputVertex ),
                               ( 'xAOD::JetContainer' , 'StoreGateSvc+%s' % inputJets )]


    algSequence.append( parOR("SecondStageFastTrackingSequence",viewAlgs) )

    # Precision Tracking
    from TrigInDetConfig.InDetTrigPrecisionTracking import makeInDetTrigPrecisionTracking
    PTTracks, PTTrackParticles, PTAlgs = makeInDetTrigPrecisionTracking( flags, config = IDTrigConfig, rois=inputRoI )
    algSequence.append( seqAND("PrecisionTrackingSequence",PTAlgs) )

    return [ algSequence, PTTrackParticles ]
