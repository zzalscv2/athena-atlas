
# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

# New configuration for ATLAS extrapolator
# Based heavily on AtlasExtrapolationEngine.py

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg

# import the ExtrapolationEngine configurable
ExEngine=CompFactory.Trk.ExtrapolationEngine

def AtlasExtrapolationEngineCfg( flags, name = 'Extrapolation', nameprefix='Atlas' ):
    result=ComponentAccumulator()

    acc  = AtlasFieldCacheCondAlgCfg(flags)
    result.merge(acc)

    # get the correct TrackingGeometry setup
    from TrackingGeometryCondAlg.AtlasTrackingGeometryCondAlgConfig import TrackingGeometryCondAlgCfg
    result.merge( TrackingGeometryCondAlgCfg(flags) )
    geom_cond_key = 'AtlasTrackingGeometry'

    from TrkConfig.TrkExRungeKuttaPropagatorConfig import RungeKuttaPropagatorCfg
    AtlasRungeKuttaPropagator = acc.popToolsAndMerge(RungeKuttaPropagatorCfg(flags, name='AtlasRungeKuttaPropagator'))
       
    # from the Propagator create a Propagation engine to handle path length
    Trk__PropagationEngine=CompFactory.Trk.PropagationEngine
    staticPropagator = Trk__PropagationEngine(name = nameprefix+'StaticPropagation')
    # give the tools it needs 
    staticPropagator.Propagator               = AtlasRungeKuttaPropagator
    # configure output formatting               
    staticPropagator.OutputPrefix             = '[SP] - '
    staticPropagator.OutputPostfix            = ' - '
    result.addPublicTool(staticPropagator) #TODO remove one day
       
    # load the material effects engine
    Trk__MaterialEffectsEngine=CompFactory.Trk.MaterialEffectsEngine
    materialEffectsEngine = Trk__MaterialEffectsEngine(name = nameprefix+'MaterialEffects')
    # configure output formatting               
    materialEffectsEngine.OutputPrefix        = '[ME] - '
    materialEffectsEngine.OutputPostfix       = ' - '
    result.addPublicTool(materialEffectsEngine) #TODO remove one day

        
    # load the static navigation engine
    Trk__StaticNavigationEngine=CompFactory.Trk.StaticNavigationEngine
    staticNavigator = Trk__StaticNavigationEngine(name = nameprefix+'StaticNavigation')
    # give the tools it needs 
    staticNavigator.PropagationEngine        = staticPropagator
    staticNavigator.MaterialEffectsEngine    = materialEffectsEngine
    staticNavigator.TrackingGeometryReadKey     = geom_cond_key        
    # Geometry name
    # configure output formatting               
    staticNavigator.OutputPrefix             = '[SN] - '
    staticNavigator.OutputPostfix            = ' - '
    # add to tool service
    result.addPublicTool(staticNavigator) #TODO remove one day
    
    # load the Static ExtrapolationEngine
    Trk__StaticEngine=CompFactory.Trk.StaticEngine
    staticExtrapolator = Trk__StaticEngine(name = nameprefix+'StaticExtrapolation')
    # give the tools it needs 
    staticExtrapolator.PropagationEngine        = staticPropagator
    staticExtrapolator.MaterialEffectsEngine    = materialEffectsEngine
    staticExtrapolator.NavigationEngine         = staticNavigator
    # configure output formatting               
    staticExtrapolator.OutputPrefix             = '[SE] - '
    staticExtrapolator.OutputPostfix            = ' - '
    # add to tool service
    result.addPublicTool(staticExtrapolator) #TODO remove one day
    
    # call the base class constructor
    extrapolator = ExEngine(name=nameprefix+'Extrapolation',
                      ExtrapolationEngines   = [ staticExtrapolator ], 
                      PropagationEngine      = staticPropagator, 
                      NavigationEngine       = staticNavigator,
                      TrackingGeometryReadKey = geom_cond_key,
                      OutputPrefix           = '[ME] - ', 
                      OutputPostfix          = ' - ')
      
    result.addPublicTool(extrapolator, primary=True)
    return result

