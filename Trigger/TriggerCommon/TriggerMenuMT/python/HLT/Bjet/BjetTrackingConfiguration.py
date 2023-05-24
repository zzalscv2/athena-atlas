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

    # Make sure the required objects are still available at whole-event level
    if flags.Input.isMC:
      from AthenaCommon.AlgSequence import AlgSequence
      topSequence = AlgSequence()
      viewVerify.DataObjects += [( 'TRT_RDO_Container' , 'StoreGateSvc+TRT_RDOs' )]
      topSequence.SGInputLoader.Load += [( 'TRT_RDO_Container' , 'StoreGateSvc+TRT_RDOs' )]
    else:
      viewVerify.DataObjects += [( 'TRT_RDO_Cache' , 'StoreGateSvc+TrtRDOCache' )]

    algSequence.append( parOR("SecondStageFastTrackingSequence",viewAlgs) )

    # Precision Tracking
    from TrigInDetConfig.InDetTrigPrecisionTracking import makeInDetTrigPrecisionTracking
    PTTracks, PTTrackParticles, PTAlgs = makeInDetTrigPrecisionTracking( flags, config = IDTrigConfig, rois=inputRoI )
    algSequence.append( seqAND("PrecisionTrackingSequence",PTAlgs) )

    return [ algSequence, PTTrackParticles ]
