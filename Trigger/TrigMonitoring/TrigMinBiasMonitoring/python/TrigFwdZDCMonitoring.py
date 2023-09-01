# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaMonitoring import AthMonitorCfgHelper
from TrigConfigSvc.TriggerConfigAccess import getL1MenuAccess

def TrigFwdZDCMonitoringAlg(configFlags):
    """ Configure general ZDC chains monitoring algs """
    monConfig = AthMonitorCfgHelper(configFlags,'FwdZDCMonitoringAlgs')

    zdcMonAlg = monConfig.addAlgorithm(CompFactory.FwdZDCMonitoringAlg, 'FwdZDCMonitoringAlg')
    
    zdcMonAlg.triggerList = [c for c in getL1MenuAccess(configFlags) if 'L1_ZDC_COMB' in c]
    zdcMonAlg.triggerList += ['L1_ZDC_A','L1_ZDC_C','L1_ZDC_A_C','L1_ZDC_XOR']
    # this creates a 'Group' called 'zdcMonitor' which will put its histograms into the subdirectory 'L1/FwdZDC/'
    
    length = len(zdcMonAlg.triggerList)
    
    zdcCountsGroup = monConfig.addGroup(zdcMonAlg, 'ZDCall', topPath='L1/FwdZDC/')
    zdcCountsGroup.defineHistogram('TrigCounts', title='Trigger counts;;Event rate',
                              xbins=length, xmin=0, xmax=length, xlabels=list(zdcMonAlg.triggerList))
    zdcCountsGroup.defineHistogram('moduleNum,moduleEnergy', type='TH2F', title= ';channel ID;CalibEnergy per channel [GeV]',
                            xbins=8, xmin=-0.5, xmax=7.5,
                            ybins=120, ymin=0, ymax=6e4)
    zdcCountsGroup.defineHistogram('e_A,e_C', type='TH2F', title= ';Low Gain ADC counts on side A [GeV];Low Gain ADC counts on side C [GeV]',
                            xbins=160, xmin=0.0, xmax=8e4,
                            ybins=160, ymin=0.0, ymax=8e4)
    zdcCountsGroup.defineHistogram('e_A', title=';CalibEnergy on side A [GeV];counts',
                            xbins=160, xmin=0.0, xmax=8e4)
    zdcCountsGroup.defineHistogram('e_C', title=';CalibEnergy on side C [GeV];counts',
                           xbins=160, xmin=0.0, xmax=8e4)
    
    
    for chain in zdcMonAlg.triggerList:
      zdcExpGroup = monConfig.addGroup(zdcMonAlg, chain+'_expert', topPath='L1/FwdZDC/'+chain+'/' )
      zdcExpGroup.defineHistogram('moduleNum,moduleEnergy', type='TH2F', title= 'occupancy for {};channel ID;CalibEnergy per channel [GeV]'.format(chain),
                            xbins=8, xmin=-0.5, xmax=7.5,
                            ybins=120, ymin=0, ymax=6e4)
      zdcExpGroup.defineHistogram('e_A,e_C', type='TH2F', title= 'occupancy for {};CalibEnergy on side A [GeV];CalibEnergy on side C [GeV]'.format(chain),
                            xbins=160, xmin=0.0, xmax=8e4,
                            ybins=160, ymin=0.0, ymax=8e4)
      zdcExpGroup.defineHistogram('e_A', title='occupancy for {};CalibEnergy on side A [GeV];counts'.format(chain),
                            xbins=160,xmin=0.0,xmax=8e4)
      zdcExpGroup.defineHistogram('e_C', title='occupancy for {};CalibEnergy on side C [GeV];counts'.format(chain),
                            xbins=160,xmin=0.0,xmax=8e4)
                            

    return monConfig.result()


if __name__=='__main__':
    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import INFO, DEBUG
    log.setLevel(INFO)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Output.HISTFileName = 'ExampleMonitorOutput.root'
    flags.fillFromArgs()
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))
    cfg.merge(TrigFwdZDCMonitoringAlg(flags))

    # If you want to turn on more detailed messages ...
    cfg.getEventAlgo('FwdZDCMonitoringAlg').OutputLevel = DEBUG
    cfg.printConfig(withDetails=False) # set True for exhaustive info
    with open("cfg.pkl", "wb") as f:
        cfg.store(f)
        
    cfg.run() #use cfg.run(20) to only run on first 20 events
    # to run:
    # python -m TrigMinBiasMonitoring.TrigFwdZDCMonitoring --filesInput=
