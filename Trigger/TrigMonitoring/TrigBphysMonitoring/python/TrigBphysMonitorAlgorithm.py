#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

def TrigBphysMonConfig(inputFlags):
    '''Function to configures some algorithms in the monitoring system.'''
    
    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'TrigBphysAthMonitorCfg')
    
    from TrigBphysMonitoring.TrigBphysMonitoringConfig import TrigBphysMonAlgBuilder
    monAlgCfg = TrigBphysMonAlgBuilder( helper, useMonGroups = True ) 
    
    # build monitor and book histograms
    monAlgCfg.configure()
    
    return helper.result()
    

if __name__=='__main__':
    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    nightly = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CommonInputs/'
    file = 'data16_13TeV.00311321.physics_Main.recon.AOD.r9264/AOD.11038520._000001.pool.root.1'

    flags = initConfigFlags()
    flags.Input.Files = [nightly+file]
    flags.Input.isMC = False
    flags.Output.HISTFileName = 'TrigBphysMonitorOutput.root'
    
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    trigBphysMonitorAcc = TrigBphysMonConfig(flags)
    cfg.merge(trigBphysMonitorAcc)

    # If you want to turn on more detailed messages ...
    #trigBphysMonitorAcc.getEventAlgo('TrigBphysMonAlg').OutputLevel = 2 # DEBUG
    cfg.printConfig(withDetails=True) # set True for exhaustive info

    cfg.run() #use cfg.run(20) to only run on first 20 events

