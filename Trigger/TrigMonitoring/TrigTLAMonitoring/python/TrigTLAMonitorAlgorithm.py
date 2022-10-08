#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

'''@file TrigTLAMonitorAlgorithm.py
@author E. Nagy
@author K. Krizka
@date 2022-06-14
@brief Example trigger python configuration for the Run III AthenaMonitoring package, based on the example by C Burton and P Onyisi
'''

def histdefs_eventinfo(pprefix):
    """
    Return a list of histogram definitions for global event variables

    @param pprefix Prefix of histogram names
    """
    histograms=[
        {'name'  :f'{pprefix}_AvgMu',
         'ylabel':'events',
         'xlabel':'AvgMu',
         'xmin'  :-0.5,
         'xmax'  :69.5,
         'xbins' :70},
        {'name'  :f'{pprefix}_NumPV',
         'ylabel':'events',
         'xlabel':'NumPV',
         'xmin'  :-0.5,
         'xmax'  :39.5,
         'xbins' :40},
        {'name'  :f'{pprefix}_JetDensityEMTopo',
         'ylabel':'events',
         'xlabel':'JetDensityEMTopo',
         'xmin'  :-0.5,
         'xmax'  :25000,
         'xbins' :100},
        {'name'  :f'{pprefix}_JetDensityEMPFlow',
         'ylabel':'events',
         'xlabel':'JetDensityEMPFlow',
         'xmin'  :-0.5,
         'xmax'  :25000,
         'xbins' :100}
    ]
    return histograms

def histdefs_jetvariables(pprefix, plabel, pflow=False):
    """
    Return a list of jet-specific histogram definitions for jet moment distributions

    @param pprefix Prefix of histogram names
    @param plabel Human-readable name of the jet to use in labeling axes
    @param pflow bool to indicate that this is the PFlow jet collection, so that the track-based variables are added. 
                False indicates that this is the calo jet collection, and additional calo variables are monitored.
    """

    histograms=[
       {'name'  :f'{pprefix}ActiveArea',
        'ylabel':f'{plabel}s',
        'xlabel':f'{plabel} ActiveArea',
        'xunit' :'',
        'xmin'  :0,
        'xmax'  :2.,
        'xbins' :200},
    ]
    if pflow:
        histograms+=[
           {'name'  :f'{pprefix}TrackWidthPt1000',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} TrackWidthPt1000',
            'xmin'  :-0.5,
            'xmax'  :0.95,
            'xbins' :50},
           {'name'  :f'{pprefix}NumTrkPt1000',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} NumTrkPt1000',
            'xmin'  :-0.5,
            'xmax'  :39.5,
            'xbins' :40},
           {'name'  :f'{pprefix}SumPtTrkPt500',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} SumPtTrkPt500',
            'xmin'  :-0.5,
            'xmax'  :500e3,
            'xunit' : 'MeV',
            'xbins' :200},
           {'name'  :f'{pprefix}SumPtChargedPFOPt500',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} SumPtChargedPFOPt500',
            'xmin'  :-0.5,
            'xmax'  :500e3,
            'xunit' : 'MeV',
            'xbins' :200},
           {'name'  :f'{pprefix}Jvt',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} Jvt',
            'xmin'  :-0.2,
            'xmax'  :1.2,
            'xbins' :70},
           {'name'  :f'{pprefix}JvtRpt',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} JvtRpt',
            'xmin'  :-0.1,
            'xmax'  :1.4,
            'xbins' :75},
           {'name'  :f'{pprefix}fastDIPS20211215_pu',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} fastDips_pu',
            'xmin'  :-0.2,
            'xmax'  :1.2,
            'xbins' :70},
           {'name'  :f'{pprefix}fastDIPS20211215_pb',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} fastDips_pb',
            'xmin'  :-0.2,
            'xmax'  :1.2,
            'xbins' :70},
           {'name'  :f'{pprefix}fastDIPS20211215_pc',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} fastDips_pc',
            'xmin'  :-0.2,
            'xmax'  :1.2,
            'xbins' :70},
        ]
    else:
        histograms+=[
           {'name'  :f'{pprefix}EMFrac',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} EMFrac',
            'xmin'  :-0.1,
            'xmax'  :1.4,
            'xbins' :75},
           {'name'  :f'{pprefix}HECFrac',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} HECFrac',
            'xmin'  :-0.1,
            'xmax'  :1.4,
            'xbins' :75},
           {'name'  :f'{pprefix}Timing',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} Timing',
            'xunit' :'ns',
            'xmin'  :-50,
            'xmax'  :50,
            'xbins' :50},
           {'name'  :f'{pprefix}N90Constituents',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} N90Constituents',
            'xunit' :'',
            'xmin'  :0,
            'xmax'  :20,
            'xbins' :20},
        ]
    return histograms

def histdefs_jetcalibscales(pprefix, plabel, pflow=False):
    """
    Return a list of jet-specific histogram definitions for jet pT distributions at different calibration scales

    @param pprefix Prefix of histogram names
    @param plabel Human-readable name of the jet to use in labeling axes
    @param pflow bool to indicate that this is the PFlow jet collection, so that the track-based calibration scales are added.
    """
    histograms=[
       {'name'  :f'{pprefix}JetConstitScaleMomentum_pt',
        'ylabel':f'{plabel}s',
        'xlabel':f'{plabel} JetConstitScaleMomentum_pt',
        'xunit' :'GeV',
        'xmin'  :0,
        'xmax'  :750,
        'xbins' :1500},
       {'name'  :f'{pprefix}JetPileupScaleMomentum_pt',
        'ylabel':f'{plabel}s',
        'xlabel':f'{plabel} JetPileupScaleMomentum_pt',
        'xunit' :'GeV',
        'xmin'  :0,
        'xmax'  :750,
        'xbins' :1500},
       {'name'  :f'{pprefix}JetEtaJESScaleMomentum_pt',
        'ylabel':f'{plabel}s',
        'xlabel':f'{plabel} JetEtaJESScaleMomentum_pt',
        'xunit' :'GeV',
        'xmin'  :0,
        'xmax'  :750,
        'xbins' :1500},
    ]
    if pflow:
        histograms+= [
          {'name'  :f'{pprefix}JetGSCScaleMomentum_pt',
           'ylabel':f'{plabel}s',
           'xlabel':f'{plabel} JetGSCScaleMomentum_pt',
           'xunit' :'GeV',
           'xmin'  :0,
           'xmax'  :750,
           'xbins' :1500}
        ]
    return histograms

def histdefs_particle(pprefix, plabel):
    """
    Return a list of histogram definitions for particle kinematics.

    @param pprefix Prefix of histogram names
    @param plabel Human-readable name of the particle to use in labeling axes
    """
    histograms=[
        {'name'  :f'n{pprefix}',
         'ylabel':'events',
         'xlabel':f'number of {plabel}s',
         'xmin'  :-0.5,
         'xmax'  :19.5,
         'xbins' :20},
        {'name'  :f'{pprefix}pt',
         'ylabel':f'{plabel}s',
         'xlabel':f'{plabel} p_{{T}}',
         'xunit' :'GeV',
         'xmin'  :0,
         'xmax'  :750,
         'xbins' :150},
        {'name'  :f'{pprefix}eta',
         'ylabel':f'{plabel}s',
         'xlabel':f'{plabel} #eta',
         'xmin'  :-4,
         'xmax'  : 4,
         'xbins' :100},
        {'name'  :f'{pprefix}phi',
         'ylabel':f'{plabel}s',
         'xlabel':f'leading {plabel} #phi',
         'xmin'  :-3.5,
         'xmax'  : 3.5,
         'xbins' :100},
        {'name'  :f'{pprefix}0pt',
         'ylabel':'events',
         'xlabel':f'leading {plabel} p_{{T}}',
         'xunit' :'GeV',
         'xmin'  :0,
         'xmax'  :750,
         'xbins' :150},
        {'name'  :f'{pprefix}0eta',
         'ylabel':'events',
         'xlabel':f'leading {plabel} #eta',
         'xmin'  :-4,
         'xmax'  : 4,
         'xbins' :100},
        {'name'  :f'{pprefix}0phi',
         'ylabel':'events',
         'xlabel':f'leading {plabel} #phi',
         'xmin'  :-3.5,
         'xmax'  : 3.5,
         'xbins' :100}
    ]
    return histograms

def histdefs_dR(p0prefix,p1prefix,p0label,p1label):
    """
    Return a list of histogram definitions for particle dR

    @param p0prefix Prefix of first particle in histogram
    @param p1prefix Prefix of second particle in histogram
    @param p0label Human-readable name of the first particle to use in labeling axes
    @param p1label Human-readable name of the second particle to use in labeling axes
    """
    histograms=[
        {'name'  :f'{p0prefix}{p1prefix}dr',
         'ylabel':'events',
         'xlabel':f'#DeltaR_{{{p0label}, {p1label}}}',
         'xmin'  :0,
         'xmax'  :6,
         'xbins' :60}]
    return histograms

def TrigTLAMonConfig(inputFlags):
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

    ### STEP 4 ###
    # Read in the TLA trigger chain names as submitted to the MonGroups

    tla_triglist = []
    # Trigger list from monitoring groups: signatures = TLAMon to be agreed w/ MonGroups management
    from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
    moniAccess=getHLTMonitoringAccess(inputFlags)
    TLAChainsE=moniAccess.monitoredChains(signatures="tlaMon",monLevels=["t0"])
    log.info (" ==> tla_chainlist t0: %s", TLAChainsE)
    for chain in TLAChainsE:
        chain = "E_"+chain
        tla_triglist.append(chain)
    TLAChainsS=moniAccess.monitoredChains(signatures="tlaMon",monLevels=["shifter"])
    log.info (" ==> tla_chainlist shifter:  %s", TLAChainsS)
    for chain in TLAChainsS:
        chain = "S_"+chain
        tla_triglist.append(chain)
    log.info (" ==> tla_triglist:  %s", tla_triglist)

    # Add a generic monitoring tool (a "group" in old language). The returned 
    # object here is the standard GenericMonitoringTool.
    TLAMonGroup = helper.addGroup(trigTLAMonAlg,'TrigTLAMonitor','HLT/TLAMon/')


    ### STEP 5 ###
    # Configure histograms
    #NB! The histograms defined here must match the ones in the cxx file exactly
    histdefs_global=[]
    histdefs_global+=histdefs_eventinfo('eventInfo')
    histdefs=[]
    histdefs+=histdefs_particle('jet'  ,'jet')
    histdefs+=histdefs_particle('pfjet','particle-flow jet')
    histdefs+=histdefs_particle('ph'   ,'photon')
    histdefs+=histdefs_particle('muon' ,'muon')
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
    from AthenaConfiguration.AllConfigFlags import ConfigFlags

    # Set execute flags (number of events to process)
    ConfigFlags.Exec.MaxEvents = args.nevents

    # Input files (AOD or other files, e.g. costumized RAW file, to be defined 
    ConfigFlags.Input.Files = [args.input]
    ConfigFlags.Input.isMC = not args.data

    # Output file (root)
    ConfigFlags.Output.HISTFileName = 'TrigTLAMonitorOutput.root'

    # ConfigFlags.Trigger.triggerMenuSetup="Physics_pp_v7_primaries"
    ConfigFlags.Trigger.triggerMenuSetup = 'Physics_pp_run3_v1'
    ConfigFlags.Trigger.triggerConfig = 'DB'                   
    
    ConfigFlags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(ConfigFlags)
    cfg.merge(PoolReadCfg(ConfigFlags))

    trigTLAMonitorAcc = TrigTLAMonConfig(ConfigFlags)
    cfg.merge(trigTLAMonitorAcc)

    # If you want to turn on more detailed messages ...
    #trigTLAMonitorAcc.getEventAlgo('TrigTLAMonAlg').OutputLevel = 2 # DEBUG
    cfg.printConfig(withDetails=False) # set True for exhaustive info

    cfg.run()
