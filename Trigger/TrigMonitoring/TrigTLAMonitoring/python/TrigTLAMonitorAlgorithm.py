#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''@file TrigTLAMonitorAlgorithm.py
@author E. Nagy
@author K. Krizka
@date 2022-06-14
@brief Example trigger python configuration for the Run III AthenaMonitoring package, based on the example by C Burton and P Onyisi
'''

def tla_chains(inputFlags,log=None):
    '''
    Read in the TLA trigger chain names as submitted to the MonGroups 
    '''
    tla_triglist = []
    # Trigger list from monitoring groups: signatures = TLAMon to be agreed w/ MonGroups management
    from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
    moniAccess=getHLTMonitoringAccess(inputFlags)
    TLAChainsE=moniAccess.monitoredChains(signatures="tlaMon",monLevels=["t0"])
    if log is not None:
        log.info (" ==> tla_chainlist t0: %s", TLAChainsE)
    for chain in TLAChainsE:
        chain = "E_"+chain
        tla_triglist.append(chain)
    TLAChainsS=moniAccess.monitoredChains(signatures="tlaMon",monLevels=["shifter"])
    if log is not None:
        log.info (" ==> tla_chainlist shifter:  %s", TLAChainsS)
    for chain in TLAChainsS:
        chain = "S_"+chain
        tla_triglist.append(chain)
    if log is not None:
        log.info (" ==> tla_triglist:  %s", tla_triglist)
    return tla_triglist


def TrigTLAMonConfig(inputFlags, tdt):
    '''Function to configures some algorithms in the monitoring system.'''

    from AthenaCommon.Logging import logging
    log = logging.getLogger( 'TrigTLAMonitorAlgorithm.py' )

    ### STEP 1 ###
    # Define one top-level monitoring algorithm. The new configuration 
    # framework uses a component accumulator.
    # EN: Not needed for the moment
    # from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    # result = ComponentAccumulator()

    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'TrigTLAAthMonitorCfg')

    ### STEP 2 ###
    # Adding an algorithm to the helper.

    from AthenaConfiguration.ComponentFactory import CompFactory
    trigTLAMonAlg = helper.addAlgorithm(CompFactory.TrigTLAMonitorAlgorithm,'TrigTLAMonAlg')

    ### STEP 3 ###
    # Edit properties of a algorithm
    # to enable a trigger filter, for example:
    #trigTLAMonAlg.TriggerChain = 'HLT_mu26_ivarmedium'
    #trigTLAMonAlg.TriggerChain = 'HLT_e24_lhtight_nod0'
    trigTLAMonAlg.TriggerChain = ''
    trigTLAMonAlg.TriggerDecisionTool = tdt

    ### STEP 4 ###
    tla_triglist=tla_chains(inputFlags,log=None)

    # Add a generic monitoring tool (a "group" in old language). The returned 
    # object here is the standard GenericMonitoringTool.
    TLAMonGroup = helper.addGroup(trigTLAMonAlg,'TrigTLAMonitor','HLT/TLAMon/')

    ### STEP 5 ###
    from TrigTLAMonitoring.TrigTLAMonitorHistograms import (
        histdefs_eventinfo, histdefs_particle, histdefs_tracks, histdefs_dR, histdefs_jetcalibscales, histdefs_jetvariables
    )
    # Configure histograms
    #NB! The histograms defined here must match the ones in the cxx file exactly
    histdefs_global=[]
    histdefs_global+=histdefs_eventinfo('eventInfo')
    histdefs=[]
    histdefs+=histdefs_particle('jet'  ,'jet')
    histdefs+=histdefs_particle('pfjet','particle-flow jet')
    histdefs+=histdefs_particle('ph'   ,'photon')
    histdefs+=histdefs_particle('muon' ,'muon')
    histdefs+=histdefs_tracks  ('trk'  ,'track')
    histdefs+=histdefs_dR      ('jet0'  ,'jet1'  , 'leading jet'   ,'subleading jet'   )
    histdefs+=histdefs_dR      ('pfjet0','pfjet1', 'leading pf jet','subleading pf jet')
    histdefs+=histdefs_dR      ('jet0'  ,'ph0'   , 'leading jet'   ,'leading photon'   )
    histdefs+=histdefs_dR      ('pfjet0','ph0'   , 'leading pf jet','leading photon'   )
    histdefs+=histdefs_jetcalibscales('jet'  ,'jet')
    histdefs+=histdefs_jetcalibscales('pfjet','particle-flow jet', True)
    histdefs+=histdefs_jetvariables('jet', 'jet')
    histdefs+=histdefs_jetvariables('pfjet', 'particle-flow jet', True)

    ## per chain histograms added here
    AllChains = []
    for chain in tla_triglist :
        AllChains.append(chain[2:])

        for histdef in histdefs:
            HistName = histdef['name'] + '_' + chain[2:]

            xlabel=histdef.get('xlabel',histdef['name'  ])
            if 'xunit' in histdef:
                xlabel+=f' [{histdef["xunit"]}'
            ylabel=histdef.get('ylabel',histdef['ylabel'])

            if chain[0:1] == "E" :
                TLAMonGroup.defineHistogram(HistName, title=f'Distribution of {histdef["name"]};{xlabel};{ylabel}',
                                            path='Expert/' +chain[2:],xbins=histdef['xbins'],xmin=histdef['xmin'],xmax=histdef['xmax'])
            if chain[0:1] == "S" :
                TLAMonGroup.defineHistogram(HistName, title=f'Distribution of {histdef["name"]};{xlabel};{ylabel}',
                                            path='Shifter/'+chain[2:],xbins=histdef['xbins'],xmin=histdef['xmin'],xmax=histdef['xmax'])

    # global histograms added here
    for histdef in histdefs_global:
        HistName = histdef['name']
        xlabel=histdef.get('xlabel',histdef['name'  ])
        if 'xunit' in histdef:
            xlabel+=f' [{histdef["xunit"]}'
        ylabel=histdef.get('ylabel',histdef['ylabel'])
        TLAMonGroup.defineHistogram(HistName, title=f'Distribution of {histdef["name"]};{xlabel};{ylabel}',
                                        path='Expert/EventInfo',xbins=histdef['xbins'],xmin=histdef['xmin'],xmax=histdef['xmax'])

    log.info (" ==> In TrigTLAMonitorAlgorithm.py: AllChains list:  %s", AllChains)
    trigTLAMonAlg.AllChains = AllChains

    ### STEP 6 ###
    # Finalize. The return value should be a tuple of the ComponentAccumulator
    # and the sequence containing the created algorithms. If we haven't called
    # any configuration other than the AthMonitorCfgHelper here, then we can 
    # just return directly (and not create "result" above)
    return helper.result()


if __name__=='__main__':
    # Get run options
    import argparse
    parser = argparse.ArgumentParser()
    # AOD file copied from Bjets, just to avoid compilation warning, for TLA need to be changed!!!!
    parser.add_argument('input',nargs='?',default='/afs/cern.ch/work/e/enagy/public/ARTfiles/MCtest160322.AOD.pool.root')
    parser.add_argument('--data',action='store_true',help='Input file is data.')
    parser.add_argument('--nevents',type=int,default=-1,help='Number of events to process.')
    args = parser.parse_args()

    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)
    # from AthenaCommon.Constants import INFO
    # log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # Set execute flags (number of events to process)
    flags.Exec.MaxEvents = args.nevents

    # Input files (AOD or other files, e.g. costumized RAW file, to be defined 
    flags.Input.Files = [args.input]
    flags.Input.isMC = not args.data

    # Output file (root)
    flags.Output.HISTFileName = 'TrigTLAMonitorOutput.root'

    # flags.Trigger.triggerMenuSetup="Physics_pp_v7_primaries"
    flags.Trigger.triggerMenuSetup = 'Physics_pp_run3_v1'
    flags.Trigger.triggerConfig = 'DB'

    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    # initialize the trigger decision tool
    from TrigDecisionTool.TrigDecisionToolConfig import TrigDecisionToolCfg
    tdt = cfg.getPrimaryAndMerge(TrigDecisionToolCfg(flags))

    # Use sequeces to handle algorithms dependencies
    from AthenaCommon.CFElements import seqAND, seqOR
    cfg.addSequence( seqOR("monSeq")  )

    # Sequence for augmenting tracks - needs to run only for pflow chains
    cfg.addSequence( seqAND("trackAugmentSeq"), parentName="monSeq")
    pfChains=[trig[2:] for trig in tla_chains(flags) if 'pf_ftf' in trig]
    from AthenaConfiguration.ComponentFactory import CompFactory
    cfg.addEventAlgo(
        CompFactory.CP.TrigEventSelectionAlg(triggers=pfChains,tool=tdt,noL1=True),
        sequenceName="trackAugmentSeq",
    )
    from BTagging.BTagTrackAugmenterAlgConfig import BTagTrackAugmenterAlgCfg
    cfg.merge(
        BTagTrackAugmenterAlgCfg(
            flags,
            TrackCollection='HLT_IDTrack_FS_FTF',
            PrimaryVertexCollectionName='HLT_IDVertex_FS',
        ),
        sequenceName="trackAugmentSeq"
    )
    
    # Add Monitoring Algorithm, after augmentations
    trigTLAMonitorAcc = TrigTLAMonConfig(flags,tdt)
    cfg.merge(
        trigTLAMonitorAcc,
        sequenceName="monSeq"
    )

    # If you want to turn on more detailed messages ...
    #trigTLAMonitorAcc.getEventAlgo('TrigTLAMonAlg').OutputLevel = 2 # DEBUG
    #cfg.getService("MessageSvc").debugLimit=100000 # for extensive print outs
    cfg.printConfig(withDetails=False) # set True for exhaustive info

    cfg.run()
