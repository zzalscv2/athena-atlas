#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#


def MistimedStreamMonitorConfig(flags):
    '''Function to configure LVL1 Mistimed Stream algorithm in the monitoring system.'''


    # get the component factory - used for getting the algorithms
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    # make the athena monitoring helper
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags,'MistimedStreamMonitorCfg')

    # get any algorithms
    MistimedMonAlg = helper.addAlgorithm(CompFactory.MistimedStreamMonitorAlgorithm,'MistimedStreamMonitorAlg')
    
    # import tools
    from DetDescrCnvSvc.DetDescrCnvSvcConfig import DetDescrCnvSvcCfg
    helper.result().merge(DetDescrCnvSvcCfg(flags))

    from TrigT1CaloCondSvc.L1CaloCondConfig import L1CaloCondAlgCfg 
    helper.result().merge(L1CaloCondAlgCfg(flags,Physics=True, Calib1=False, Calib2=False)) 
    
    from TrigConfxAOD.TrigConfxAODConfig import getxAODConfigSvc
    helper.result().getPrimaryAndMerge(getxAODConfigSvc(flags))


    

    # add any steering
    groupName = 'MistimedStreamMonitor' # the monitoring group name is also used for the package name
    MistimedMonAlg.PackageName = groupName
    
    # Histogram paths
    mainDir = 'L1Calo'
    trigPath = 'MistimedStream'

    # Trigger tower plots: eta-phi granularity
    etabins = [-4.9,-4.475,-4.050,-3.625,-3.2,-3.1,-2.9,
                     -2.7,-2.5,-2.4,-2.3,-2.2,-2.1,-2.0,-1.9,
                     -1.8,-1.7,-1.6,-1.5,-1.4,-1.3,-1.2,-1.1,
                     -1.0,-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,
                     -0.2,-0.1,0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,
                     0.8,0.9,1.0,1.1,1.2,1.3,1.4,1.5,1.6,1.7,
                     1.8,1.9,2.0,2.1,2.2,2.3,2.4,2.5,2.7,2.9,
                     3.1,3.2,3.625,4.050,4.475,4.9]


                            

    phibins = 64
    phimin = 0

    
    NumberOfGlobalErrors=10
    globalStatus_xlabels = [
        "All",
        "unsuitable readout",
        "HLT_mistimemonj400",
        "L1_J100",
        "<= 4 bad peak TT",
        "<= 4 bad central TT",
        "<= 4 bad late TT",
        ">= 2 late TT",
        "<= 3 in-time",
        ">= 1 significant TT in EM layer",
    ]


    NumberofEvents = MistimedMonAlg.MaxEvents

    # add monitoring algorithm to group, with group name and main directory 
    myGroup = helper.addGroup(MistimedMonAlg, groupName , mainDir)
    

    
    myGroup.defineHistogram('cutFlowX;1d_cutFlow_mistimedStreamAna',title='Total events selected by the MistimedStream analysis for this run',
                            type='TH1F',
                            path=trigPath,
                            xbins=NumberOfGlobalErrors,xmin=0.,xmax=NumberOfGlobalErrors,xlabels=globalStatus_xlabels,
                            opt='kAlwaysCreate')
    
    groupMapsEM         = helper.addGroup(MistimedMonAlg, 'Event_', mainDir)
    groupMapsEM.defineHistogram('eventMonitor,lbMonitor;1d_selectedEvents_mistimedStreamAna', title='Events of interest - Lumi Block;Events of interest;Lumi Block',type='TH2I', xbins=NumberofEvents, ybins=NumberofEvents, path=trigPath)


    # add monitoring algorithm to group, with group name and main directory
    histPath = trigPath+'/EventofInterest'

    for i in range(0, NumberofEvents):
        # LUT per BCN (Both EM & HAD together)
        groupMapsEM         = helper.addGroup(MistimedMonAlg, 'EventofInterest_'+str(i)+'_TT_EM', mainDir)
        groupMapsEM.defineHistogram('etaTT_2D,phiTT_2D,pulseCat;EventofInterest_'+str(i)+'_em_2d_etaPhi_tt_classification_mistimedStreamAna', title='#eta - #phi Map of TT classification, EM layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins, type='TProfile2D', path=histPath)

        groupMapsEM.defineHistogram('etaTT_2D,phiTT_2D,bcidWord;EventofInterest_'+str(i)+'_em_2d_etaPhi_tt_pseBits_mistimedStreamAna', title='#eta - #phi Map of TT PSE Bits, EM layer: event  of interest no. '+str(i)+';#eta;#phi', xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins,type='TProfile2D', path=histPath)
        
        
        groupMapsHAD         = helper.addGroup(MistimedMonAlg, 'EventofInterest_'+str(i)+'_TT_HAD', mainDir)
        groupMapsHAD.defineHistogram('etaTT_2D,phiTT_2D,pulseCat;EventofInterest_'+str(i)+'_had_2d_etaPhi_tt_classification_mistimedStreamAna', title='#eta - #phi Map of TT classification, HAD layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins,  type='TProfile2D', path=histPath)
        groupMapsHAD.defineHistogram('etaTT_2D,phiTT_2D,bcidWord;EventofInterest_'+str(i)+'_had_2d_etaPhi_tt_pseBits_mistimedStreamAna', title='#eta - #phi Map of TT PSE Bits, HAD layer: event of interest no. '+str(i)+';#eta;#phi', xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins, type='TProfile2D', path=histPath)

    
        groupMapsEM0         = helper.addGroup(MistimedMonAlg,  'EventofInterest_'+str(i)+'_lut_EM0', mainDir)
        groupMapsEM0.defineHistogram('etalut,philut,emLUT0;EventofInterest_'+str(i)+'_em_2d_etaPhi_tt_lut0_mistimedStreamAna', title='#eta - #phi Map of TT LUT in timeslice 0 = BCID-1, EM layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins, type='TProfile2D', path=histPath)
        groupMapsEM1         = helper.addGroup(MistimedMonAlg,  'EventofInterest_'+str(i)+'_lut_EM1', mainDir)
        groupMapsEM1.defineHistogram('etalut,philut,emLUT1;EventofInterest_'+str(i)+'_em_2d_etaPhi_tt_lut1_mistimedStreamAna', title='#eta - #phi Map of TT LUT in timeslice 1 = BCID-1, EM layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins, type='TProfile2D', path=histPath)
        groupMapsEM2         = helper.addGroup(MistimedMonAlg,  'EventofInterest_'+str(i)+'_lut_EM2', mainDir)
        groupMapsEM2.defineHistogram('etalut,philut,emLUT2;EventofInterest_'+str(i)+'_em_2d_etaPhi_tt_lut2_mistimedStreamAna', title='#eta - #phi Map of TT LUT in timeslice 2 = BCID-1, EM layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins,  type='TProfile2D', path=histPath)
    
        groupMapsHAD0         = helper.addGroup(MistimedMonAlg, 'EventofInterest_'+str(i)+'_lut_HAD0', mainDir)
        groupMapsHAD0.defineHistogram('etalut,philut,hadLUT0;EventofInterest_'+str(i)+'_had_2d_etaPhi_tt_lut0_mistimedStreamAna', title='#eta - #phi Map of TT LUT in timeslice 0 = BCID-1, HAD layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins, type='TProfile2D', path=histPath)
        groupMapsHAD1         = helper.addGroup(MistimedMonAlg, 'EventofInterest_'+str(i)+'_lut_HAD1', mainDir)
        groupMapsHAD1.defineHistogram('etalut,philut,hadLUT1;EventofInterest_'+str(i)+'_had_2d_etaPhi_tt_lut1_mistimedStreamAna', title='#eta - #phi Map of TT LUT in timeslice 1 = BCID-1, HAD layer: event of interest no. '+str(i)+';#eta;#phi'  , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins, type='TProfile2D', path=histPath)
        groupMapsHAD2         = helper.addGroup(MistimedMonAlg, 'EventofInterest_'+str(i)+'_lut_HAD2', mainDir)
        groupMapsHAD2.defineHistogram('etalut,philut,hadLUT2;EventofInterest_'+str(i)+'_had_2d_etaPhi_tt_lut2_mistimedStreamAna', title='#eta - #phi Map of TT LUT in timeslice 2 = BCID-1, HAD layer: eventof interest no. '+str(i)+';#eta;#phi'   , xbins=etabins, ybins=phibins, ymin=phimin, ymax=phibins, type='TProfile2D', path=histPath)
        
       
               





    result.merge(helper.result())
    return result


    


if __name__=='__main__':
   
    import sys 

    from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
    flags.Exec.MaxEvents = -1
    flags.IOVDb.GlobalTag = 'CONDBR2-BLKPA-2022-02'
    flags.Input.Files = ["/eos/atlas/atlastier0/rucio/data22_13p6TeV/express_express/00423433/data22_13p6TeV.00423433.express_express.recon.ESD.x653/data22_13p6TeV.00423433.express_express.recon.ESD.x653._lb0015._SFO-ALL._0001.2"]

    

    flags.Output.HISTFileName = 'ExampleMonitorOutput_LVL1.root'
    
    flags.lock()

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc = MainServicesCfg(flags)
    acc.merge(PoolReadCfg(flags))
    

    MistimedStreamMonitorCfg = MistimedStreamMonitorConfig(flags)
    acc.merge(MistimedStreamMonitorCfg)

    MistimedStreamMonitorCfg.getEventAlgo('MistimedStreamMonitorAlg').OutputLevel = 2 # 1/2 INFO/DEBUG
    acc.printConfig(withDetails=True, summariseProps = True)

    sys.exit(acc.run().isFailure())
