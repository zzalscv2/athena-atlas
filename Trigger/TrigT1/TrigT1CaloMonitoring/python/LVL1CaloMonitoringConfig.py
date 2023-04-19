#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

def LVL1CaloMonitoringConfig(flags):
    '''Function to call l1calo DQ monitoring algorithms'''
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.Enums import Format
    import logging

    # local printing
    local_logger = logging.getLogger('AthenaMonitoringCfg')
    info = local_logger.info
    info('In LVL1CaloMonitoringConfig')

    result = ComponentAccumulator()

    # If we're not putting trigger objects in event store, can't monitor them
    if not flags.Trigger.Online.isPartition:
        if not flags.DQ.triggerDataAvailable:
            return result

    isData = not flags.Input.isMC

    # check if validation requested
    validation=flags.DQ.Steering.LVL1Calo.doValidation

    # monitoring algorithm configs
    # do not run on MC or  RAW->ESD(tier0), or AOD-only
    if not validation and isData and flags.DQ.Environment not in ('tier0Raw', 'AOD'):

        from TrigT1CaloMonitoring.PprMonitorAlgorithm import PprMonitoringConfig
        from TrigT1CaloMonitoring.JepJemMonitorAlgorithm import JepJemMonitoringConfig

        # Use metadata to check Run3 compatible trigger info is available  
        from AthenaConfiguration.AutoConfigFlags import GetFileMD
        md = GetFileMD(flags.Input.Files)
        inputContainsRun3FormatConfigMetadata = ("metadata_items" in md and any(('TriggerMenuJson' in key) for key in md["metadata_items"].keys()))
        result.merge(PprMonitoringConfig(flags))
        result.merge(JepJemMonitoringConfig(flags))
        if flags.Input.Format is not Format.POOL or inputContainsRun3FormatConfigMetadata:
            # L1 menu available in the POOL file
            from TrigT1CaloMonitoring.CpmMonitorAlgorithm import CpmMonitoringConfig
            from TrigT1CaloMonitoring.CpmSimMonitorAlgorithm import CpmSimMonitoringConfig
            from TrigT1CaloMonitoring.JepCmxMonitorAlgorithm import JepCmxMonitoringConfig
            from TrigT1CaloMonitoring.OverviewMonitorAlgorithm import OverviewMonitoringConfig
            from TrigT1CaloMonitoring.PPMSimBSMonitorAlgorithm import PPMSimBSMonitoringConfig
            from TrigT1CaloMonitoring.JetEfficiencyMonitorAlgorithm import JetEfficiencyMonitoringConfig
            
            result.merge(CpmMonitoringConfig(flags))
            result.merge(CpmSimMonitoringConfig(flags))
            result.merge(JepCmxMonitoringConfig(flags))
            result.merge(PPMSimBSMonitoringConfig(flags))
            result.merge(JetEfficiencyMonitoringConfig(flags))
            result.merge(OverviewMonitoringConfig(flags))

            if  flags.Input.TriggerStream == "physics_Mistimed":
                from TrigT1CaloMonitoring.MistimedStreamMonitorAlgorithm import MistimedStreamMonitorConfig
                result.merge(MistimedStreamMonitorConfig(flags))

        # For running on bytestream data
        if flags.Input.Format is Format.BS:
            from TrigT1CaloByteStream.LVL1CaloRun2ByteStreamConfig import LVL1CaloRun2ReadBSCfg
            result.merge(LVL1CaloRun2ReadBSCfg(flags))

        # Phase 1 monitoring
        if flags.Trigger.enableL1CaloPhase1:
            #efex monitoring
            from TrigT1CaloMonitoring.EfexMonitorAlgorithm import EfexMonitoringConfig
            EfexMonitorCfg = EfexMonitoringConfig(flags)
            result.merge(EfexMonitorCfg)

            #  Need to pass the algorithm to the histogram booking
            EfexMonAlg = result.getEventAlgo('EfexMonAlg')  
            from TrigT1CaloMonitoring.EfexMonitorAlgorithm import EfexMonitoringHistConfig
            EfexMonitorHistCfg = EfexMonitoringHistConfig(flags,EfexMonAlg)
            result.merge(EfexMonitorHistCfg)

            #gfex monitoring 
            from TrigT1CaloMonitoring.GfexMonitorAlgorithm import GfexMonitoringConfig
            result.merge(GfexMonitoringConfig(flags))

            #efex input monitoring 
            from TrigT1CaloMonitoring.EfexInputMonitorAlgorithm import EfexInputMonitoringConfig
            result.merge(EfexInputMonitoringConfig(flags))

            #gfex input monitoring 
            from TrigT1CaloMonitoring.GfexInputMonitorAlgorithm import GfexInputMonitoringConfig
            result.merge(GfexInputMonitoringConfig(flags))

            #jfex input monitoring 
            from TrigT1CaloMonitoring.JfexInputMonitorAlgorithm import JfexInputMonitoringConfig
            result.merge(JfexInputMonitoringConfig(flags))




    # algorithms for validation checks
    if validation:
        from TrigT1CaloMonitoring.L1CaloLegacyEDMMonitorAlgorithm import L1CaloLegacyEDMMonitoringConfig
        result.merge(L1CaloLegacyEDMMonitoringConfig(flags))
        # Phase 1 systems
        from TrigT1CaloMonitoring.EfexMonitorAlgorithm import EfexMonitoringConfig
        EfexMonitorCfg = EfexMonitoringConfig(flags)
        result.merge(EfexMonitorCfg)
        # algorithm required for dynamic histogram booking
        EfexMonAlg = result.getEventAlgo('EfexMonAlg')
        EfexMonAlg.eFexEMTobKeyList = ['L1_eEMRoI', 'L1_eEMxRoI']
        EfexMonAlg.eFexTauTobKeyList = ['L1_eTauRoI', 'L1_eTauxRoI'] 
        from TrigT1CaloMonitoring.EfexMonitorAlgorithm import EfexMonitoringHistConfig
        EfexMonitorHistCfg = EfexMonitoringHistConfig(flags,EfexMonAlg)
        result.merge(EfexMonitorHistCfg)
        #
        from TrigT1CaloMonitoring.GfexMonitorAlgorithm import GfexMonitoringConfig
        result.merge(GfexMonitoringConfig(flags))
        from TrigT1CaloMonitoring.JfexMonitorAlgorithm import JfexMonitoringConfig
        result.merge(JfexMonitoringConfig(flags))

        from TrigT1CaloMonitoring.JetEfficiencyMonitorAlgorithm import JetEfficiencyMonitoringConfig
        result.merge(JetEfficiencyMonitoringConfig(flags))


    return result
