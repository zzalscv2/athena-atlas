# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from __future__ import print_function
import InDetPhysValMonitoring.InDetPhysValMonitoringConf
from .ConfigUtils import createPublicTool
from .ConfigUtils import createExtendNameIfNotDefault
from .ConfigUtils import toolFactory
from InDetRecExample.TrackingCommon import setDefaults

# ---- definitions

def metaDataKey():
    '''
    Meta data key to store the file source on which the InDet decoration alg has been running.
    '''
    return 'InDetPhysValDecoration'

def monManName():
    '''
    Name of the monitoring manager
    '''
    return 'PhysValMonManager'


def findAlg(alg_name, search_outputstream_otherwise=True):
    '''
       Find the algorithm alg_name in the top sequence and return its index.
       if no algorithm of the given name is found the index just before the AhtenaOutputStream is returned.
       if all that fails None is returned.
    '''
    if not isinstance(alg_name, (list, tuple)):
        raise Exception(
            'logic error findAlg called with a non list argument %s / %s' % (alg_name, type(alg_name)))

    from AthenaCommon.AlgSequence import AlgSequence
    topSequence = AlgSequence()
    count = 0
    mon_man_index = None
    for child in topSequence.getChildren():
        # if child.getName() in alg_name :
        if child.getName() in alg_name:
            # print ('DEBUG findAlg %s =?=  %s ' %(child.getName(),alg_name))
            mon_man_index = count
            if len(alg_name) == 1:
                break
        count += 1

    if mon_man_index is None and search_outputstream_otherwise:
        count = 0
        exclude_streams = ['StreamBS']
        for child in topSequence.getChildren():
            if child.getType() == 'AthenaOutputStream' and child.getName() not in exclude_streams:
                # print ('DEBUG found %s at postion %i/%i' % (child.getFullName(),count,len(topSequence.getChildren() ) ))
                mon_man_index = count
                break
            count += 1
    return mon_man_index


def findMonMan():
    '''
    Find the monitoring manager and return its index in the top sequence or None
    '''

    return findAlg([monManName()], search_outputstream_otherwise=False)

def setMetaData():
    '''
    Write the input source on which the InDet decoration algorithm was running to the meta data
    '''

    from AthenaCommon.AppMgr import ServiceMgr as svcMgr
    if metaDataKey() in svcMgr.TagInfoMgr.ExtraTagValuePairs:
        return

    from RecExConfig.RecFlags import rec
    str = ''
    if rec.readRDO():
        str = 'fromRDO'
    elif rec.readESD():
        str = 'fromESD'
    elif rec.readAOD():
        str = 'fromAOD'

    if metaDataKey() not in svcMgr.TagInfoMgr.ExtraTagValuePairs:
        svcMgr.TagInfoMgr.ExtraTagValuePairs[metaDataKey()] = str
    # print ('DEBUG add meta data %s.' % svcMgr.TagInfoMgr.ExtraTagValuePairs)


def getPhysValMonInDetHoleSearchTool(**kwargs):
    from InDetRecExample.TrackingCommon import getInDetHoleSearchTool
    kwargs = setDefaults(kwargs,
                         name='PhysValMonInDetHoleSearchTool')

    return getInDetHoleSearchTool(**kwargs)


def getInDetPhysHitDecoratorAlg(**kwargs):
    '''
    create decoration algorithm which decorates track particles with the unbiased hit residuals and pulls.
    If the collection name TrackParticleContainerName is specified and differs from the default, the name
    of the algorithm will be extended by the collection name
    '''
    # @TODO use track particles from ? from InDetRecExample.InDetKeys import InDetKeys
    from InDetRecExample.TrackingCommon import getPixelLorentzAngleTool
    return createExtendNameIfNotDefault(InDetPhysValMonitoring.InDetPhysValMonitoringConf.InDetPhysHitDecoratorAlg,
                                        'TrackParticleContainerName', 'InDetTrackParticles',
                                        kwargs,
                                        InDetTrackHoleSearchTool=toolFactory(
                                            getPhysValMonInDetHoleSearchTool),
                                        Updator='Trk::KalmanUpdator/TrkKalmanUpdator',
                                        ResidualPullCalculator='Trk::ResidualPullCalculator/ResidualPullCalculator',
                                        LorentzAngleTool=getPixelLorentzAngleTool(),
                                        TrackParticleContainerName='InDetTrackParticles')


def getParameterErrDecoratorAlg(**kwargs):
    '''
    create decoration algorithm which decorates track particles with the uncertainties of the track parameters.
    If the collection name TrackParticleContainerName is specified and differs from the default, the name
    of the algorithm will be extended by the collection name
    '''
    return createExtendNameIfNotDefault(InDetPhysValMonitoring.InDetPhysValMonitoringConf.ParameterErrDecoratorAlg,
                                        'TrackParticleContainerName', 'InDetTrackParticles',
                                        kwargs)


def getInDetPhysValTruthDecoratorAlg(**kwargs):
    '''
    create decoration algorithm which decorates truth particles with track parameters at the perigee.
    If the collection name TruthParticleContainerName is specified and differs from the default, the name
    of the algorithm will be extended by the collection name
    '''
    return createExtendNameIfNotDefault(
        InDetPhysValMonitoring.InDetPhysValMonitoringConf.InDetPhysValTruthDecoratorAlg,
        'TruthParticleContainerName', 'TruthParticles',
        kwargs,
        Extrapolator='Trk::Extrapolator/AtlasExtrapolator')


def getInDetRttTruthSelectionTool(**kwargs):
    return createPublicTool(
        InDetPhysValMonitoring.InDetPhysValMonitoringConf.AthTruthSelectionTool,
        kwargs,
        # @TODO change name ? name = 'InDetRttTruthSelectionTool',
        requireStatus1=True,
        requireCharged=True,
        requireSiHit=0,
        requireOnlyPrimary=(
            True if kwargs.pop("OnlyDressPrimaryTracks", True) else False),
        maxProdVertRadius=300.,
        maxEta=2.5,
        minPt=500.)


def getInDetTruthSelectionTool(**kwargs):
    return getInDetRttTruthSelectionTool(
        **setDefaults(kwargs,
                      name='InDetTruthSelectionTool',
                      maxProdVertRadius=-1))  # disable production radius cut

def getHardScatterSelectionTool(**kwargs):
    from InDetHardScatterSelectionTool.InDetHardScatterSelectionToolConf import InDet__InDetHardScatterSelectionTool
    kwargs = setDefaults(kwargs, name="InDetHardScatterSelectionTool")
    kwargs = setDefaults(kwargs, JetContainer="AntiKt4EMTopoJets") 
    return InDet__InDetHardScatterSelectionTool(**kwargs)


def getInDetPhysValTruthDecoratorAlgStableParticles(**kwargs):
    '''
    Create a decoration algorithm which decorates stable truth particles only with track parameters at the perigee
    see @ref getInDetPhysValTruthDecoratorAlg for details.
    '''
    return getInDetPhysValTruthDecoratorAlg(**setDefaults(
        kwargs,
        TruthSelectionTool=getInDetTruthSelectionTool()))


def getTruthClassDecoratorAlg(**kwargs):
    '''
    create decoration algorithm which decorates truth particles with origin and type from truth classifier.
    if the collection name TruthParticleContainerName is specified and differs from the default, the name
    of the algorithm will be extended by the collection name
    '''
    return createExtendNameIfNotDefault(
        InDetPhysValMonitoring.InDetPhysValMonitoringConf.TruthClassDecoratorAlg,
        'TruthParticleContainerName', 'TruthParticles',
        kwargs)


def getTrackDecorators(**kwargs):
    '''
    Get track particle decorators needed for the InDetPhysValMonitoring tool
    If the collection name TrackParticleContainerName is specified and differs from the default, the name
    of the algorithms will be extended by the collection name.
    '''
    # only valid kwarg : TrackParticleContainerName
    from RecExConfig.AutoConfiguration import IsInInputFile
    if not IsInInputFile('TrackCollection', 'CombinedInDetTracks'):
        return [getParameterErrDecoratorAlg(**kwargs)]
    else:
        return [getInDetPhysHitDecoratorAlg(**kwargs),
                getParameterErrDecoratorAlg(**kwargs)]


def getTruthDecorators(**kwargs):
    '''
    Get truth particle decorators needed for the InDetPhysValMonitoring tool.
    If the collection name TruthParticleContainerName is specified and differs from the default, the name
    of the algorithms will be extended by the collection name.
    '''
    # only valid kwarg : TruthParticleContainerName
    return [getInDetPhysValTruthDecoratorAlg(**kwargs)
            # , getTruthClassDecoratorAlg(**kwargs)
            ]


def getDecorators(**kwargs):
    decorators = []
    import InDetPhysValMonitoring.InDetPhysValDecoration
    decorators += InDetPhysValMonitoring.InDetPhysValDecoration.getTrackDecorators(
        TrackParticleContainerName=kwargs.pop('TrackParticleContainerName', 'InDetTrackParticles'))
    truth_particle_container_name = kwargs.pop(
        'TruthParticleContainerName', 'TruthParticles')
    from InDetPhysValMonitoring.InDetPhysValJobProperties import isMC
    if isMC():
        decorators += InDetPhysValMonitoring.InDetPhysValDecoration.getTruthDecorators(
            TruthParticleContainerName=truth_particle_container_name)
    if len(kwargs) > 0:
        raise Exception('Unused kwargs: %s' % kwargs)
    return decorators


class InDetPhysValKeys:
    '''
    Keys to decorate and monitor GSF Tracks/TrackParticles
    '''
    # the name of unslimmed GSF Tracks defined here
    GSFTracksUnslimmed = 'GSFTracksUnslimmed'

    # the name of GSF Tracks given by egamma
    GSFTracks = 'GSFTracks'

    # the name of GSF TrackParticles given by egamma
    # @todo should be used also in InDetPhysValMonitoring.py
    GSFTrackParticles = 'GSFTrackParticles'


def getGSFTrackDecorators(**kwargs):
    return getTrackDecorators(
        TrackParticleContainerName=InDetPhysValKeys.GSFTrackParticles)

# Debugging
# import trace
# tracer = trace.Trace()
# tracer.runfunc( )


def _addDecorators(decorator_alg_list, add_after=None):
    '''
    Add the given decorator algorithms to the top sequence.
    The algorithm is to be run on RAW/RDO since it depends on full hit information
    which is generally not available at later stages. The decorations added by this
    algorithm are used by InDetPhysValMonitoring tool.
    '''

    if add_after is not None and not isinstance(add_after, (list, tuple)):
        raise Exception(' logic error _addDecorators called with a non list argument %s / %s' %
                        (add_after, type(add_after)))

    # Access the algorithm sequence:
    from AthenaCommon.AlgSequence import AlgSequence
    topSequence = AlgSequence()
    # print ('DEBUG add _addDecorators add after %s ' % (add_after))

    # if there is a monitoring manager add decorator before
    mon_man_index = findMonMan()
    if mon_man_index is None and add_after is not None:
        alg_index = findAlg(add_after)
        if alg_index is not None:
            # add after the found algorithm
            mon_man_index = alg_index + 1
    # print ('DEBUG _addDecorators after this %s ' % (mon_man_index))

    if mon_man_index is None:
        for decorator_alg in decorator_alg_list:
            if findAlg([decorator_alg.getName()], search_outputstream_otherwise=False) is not None:
                print('DEBUG decorator %s already in sequence. Not adding again.' % (
                    decorator_alg.getFullName()))
                continue
            print('DEBUG add decorator %s at end of top sequence:' %
                  (decorator_alg.getFullName()))
            topSequence += decorator_alg

    else:
        for decorator_alg in decorator_alg_list:
            if findAlg([decorator_alg.getName()], search_outputstream_otherwise=False) is not None:
                print('DEBUG decorator %s already in sequence. Not inserting again.' % (
                    decorator_alg.getFullName()))
                continue
            print('DEBUG insert decorator %s at position %i' %
                  (decorator_alg.getFullName(), mon_man_index))
            topSequence.insert(mon_man_index, decorator_alg)
            mon_man_index += 1
    setMetaData()


def addGSFTrackDecoratorAlg():
    '''
    Search egamma algorithm and add the GSF TrackParticle decorator after the it.
    '''
    from InDetPhysValMonitoring.InDetPhysValDecoration import _addDecorators

    from InDetPhysValMonitoring.InDetPhysValJobProperties import InDetPhysValFlags
    if InDetPhysValFlags.doValidateGSFTracks():
        # print ('DEBUG add addGSFTrackDecoratorAlg')
        decorators = getGSFTrackDecorators()
        from InDetPhysValMonitoring.InDetPhysValJobProperties import InDetPhysValFlags
        from InDetPhysValMonitoring.ConfigUtils import extractCollectionPrefix
        for col in InDetPhysValFlags.validateExtraTrackCollections():
            prefix = extractCollectionPrefix(col)
            decorators += getTrackDecorators(
                TrackParticleContainerName=prefix+"TrackParticles")
        # add the InDetPhysValDecoratorAlgGSF after the egamma algorithms ran
        # they build the GSF track particles.
        _addDecorators(decorators, ['egamma', 'egammaTruthAssociationAlg'])

        # To allow for proper decoration of GSF TrackParticles
        #  - have to switch of slimming for GSF Tracks at time of creation
        #  - must slim GSF Tracks after decoration since the unslimmed GSF Tracks cannot be persistified

        from AthenaCommon.AppMgr import ToolSvc
        # print (ToolSvc)
        # print ('DEBUG has EMBremCollectionBuilder %s' % hasattr(ToolSvc,'EMBremCollectionBuilder'))
        if hasattr(ToolSvc, 'EMBremCollectionBuilder'):
            decor_index = findAlg(
                [decorators[0].getName()], search_outputstream_otherwise=False)
            if decor_index is not None:
                from TrkTrackSlimmer.TrkTrackSlimmerConf import Trk__TrackSlimmer as ConfigurableTrackSlimmer
                slimmer = ConfigurableTrackSlimmer(
                    name="RealGSFTrackSlimmer",
                    TrackLocation=[
                        InDetPhysValKeys.GSFTracksUnslimmed],
                    SlimmedTrackLocation=[
                        InDetPhysValKeys.GSFTracks],
                    TrackSlimmingTool=ToolSvc.EMBremCollectionBuilder.TrackSlimmingTool)

                from InDetPhysValMonitoring.InDetPhysValMonitoringConf import DummyTrackSlimmingTool
                slimming_tool = DummyTrackSlimmingTool()
                ToolSvc += slimming_tool
                ToolSvc.EMBremCollectionBuilder.TrackSlimmingTool = slimming_tool
                ToolSvc.EMBremCollectionBuilder.OutputTrackContainerName = InDetPhysValKeys.GSFTracksUnslimmed
                # ToolSvc.ResidualPullCalculator.OutputLevel = 1

                from AthenaCommon.AlgSequence import AlgSequence
                topSequence = AlgSequence()
                topSequence.insert(decor_index+1, slimmer)
        # import sys
        # sys.exit(1)


def addDecorator():
    '''
    Add the track particle decoration algorithm to the top sequence.
    The algorithm is to be run on RAW/RDO since it depends on full hit information
    which is generally not available at later stages. The decorations added by this
    algorithm are used by InDetPhysValMonitoring tool.
    '''

    decorators = []

    decorators += getTrackDecorators()
    from InDetPhysValMonitoring.InDetPhysValJobProperties import isMC
    if isMC():
        decorators += getTruthDecorators()

    from InDetPhysValMonitoring.InDetPhysValJobProperties import InDetPhysValFlags
    InDetPhysValFlags.init()
    if InDetPhysValFlags.doValidateGSFTracks():
        # cannot add the decorator for GSF tracks at this moment because the egamma algorithm has not been
        # constructed yet.
        # so this does not work: decorators.append( InDetPhysValDecoratorAlg.InDetPhysValDecoratorAlgGSF() )
        # Can only schedule a user algorithm :
        from RecExConfig.RecFlags import rec
        rec.UserExecs += ['from InDetPhysValMonitoring.InDetPhysValDecoration import addGSFTrackDecoratorAlg;addGSFTrackDecoratorAlg();']

    _addDecorators(decorators)


def canAddDecorator():
    '''
    check whether the decorator can be added.

    A decorator can be added if a track particle converter alg is in the sequence or
    if ESDs or AODs are read.
    '''
    from RecExConfig.RecFlags import rec
    if not rec.doInDet:
        return False

    if rec.readAOD:
        return True

    if rec.readESD:
        return True

    if rec.readTAG:
        return False

    if rec.readRDO:
        try:
            from AthenaCommon.AlgSequence import AlgSequence
            topSequence = AlgSequence()
            import re
            pat = re.compile('.*(TrackParticleCnvAlg).*')
            for alg in topSequence.getChildren():
                if pat.match(alg.getFullName()) is not None:
                    return True

        except Exception:
            pass

    return False


def addDecoratorIfNeeded():
    '''
     Run the InDet decoration algorithm if it has not been ran yet.
    '''

    if not canAddDecorator():
        print('DEBUG addDecoratorIfNeeded ? Stage is too early or too late for running the decoration. Needs reconstructed tracks. Try again during next stage ?')
        return

    addDecorator()

    # if GSF tracks need to be monitored schedule addExtraMonitoring as user algorithm, so that
    # the monitoring manager exists already.
    from InDetPhysValMonitoring.InDetPhysValJobProperties import InDetPhysValFlags
    InDetPhysValFlags.init()

    # from AthenaCommon.AppMgr import ServiceMgr as svcMgr
    # print ('DEBUG addDecoratorIfNeeded add meta data %s.' % svcMgr.TagInfoMgr.ExtraTagValuePairs)
