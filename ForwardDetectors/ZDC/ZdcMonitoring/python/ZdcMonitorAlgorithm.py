#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

'''@file ZdcMonitorAlgorithm.py
@author Y. Guo
@author S. Mohapatra
@date 2023-08-01
@brief Zdc python configuration for the Run III AthenaMonitoring package
'''

import numpy as np

def create_log_bins(min_value, max_value, num_bins):
    # Calculate the logarithmic bin edges
    log_min = np.log10(min_value)
    log_max = np.log10(max_value)
    log_bin_edges = np.linspace(log_min, log_max, num_bins + 1)
    bin_edges = [10 ** edge for edge in log_bin_edges]

    return bin_edges


def ZdcMonitoringConfig(inputFlags, run_type):

    '''Function to configures some algorithms in the monitoring system.'''

    ### STEP 1 ###
    # If you need to set up special tools, etc., you will need your own ComponentAccumulator;
    # uncomment the following 2 lines and use the last three lines of this function instead of the ones
    # just before
    # from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    # result = ComponentAccumulator()

    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'ZdcAthMonitorCfg')


    ### STEP 2 ###
    # Adding an algorithm to the helper. Here, we will use the example 
    # algorithm in the AthenaMonitoring package. Just pass the type to the 
    # helper. Then, the helper will instantiate an instance and set up the 
    # base class configuration following the inputFlags. The returned object 
    # is the algorithm.
    # This uses the new Configurables object system.
    from AthenaConfiguration.ComponentFactory import CompFactory
    zdcMonAlg = helper.addAlgorithm(CompFactory.ZdcMonitorAlgorithm,'ZdcMonAlg')

    # You can actually make multiple instances of the same algorithm and give 
    # them different configurations
    # anotherZdcMonAlg = helper.addAlgorithm(CompFactory.ZdcMonitorAlgorithm,'AnotherZdcMonAlg')

    # # If for some really obscure reason you need to instantiate an algorithm
    # # yourself, the AddAlgorithm method will still configure the base 
    # # properties and add the algorithm to the monitoring sequence.
    # helper.AddAlgorithm(myExistingAlg)


    ### STEP 3 ###
    # Edit properties of a algorithm
    # some generic property
    # zdcMonAlg.CalInfoOn = False
    # to enable a trigger filter, for example:
    #zdcMonAlg.TriggerChain = 'HLT_mu26_ivarmedium'

    ### STEP 4 ###
    # Add some tools. N.B. Do not use your own trigger decion tool. Use the
    # standard one that is included with AthMonitorAlgorithm.

    # # Then, add a tool that doesn't have its own configuration function. In
    # # this example, no accumulator is returned, so no merge is necessary.
    # from MyDomainPackage.MyDomainPackageConf import MyDomainTool
    # zdcMonAlg.MyDomainTool = MyDomainTool()

    # Add a generic monitoring tool (a "group" in old language). The returned 
    # object here is the standard GenericMonitoringTool.
    genZdcMonTool = helper.addGroup(
        zdcMonAlg,
        'genZdcMonTool'
    )

    ### STEP 5 ###
    # Configure histograms

    # (potentially run-type dependent) range settings
    lumi_block_max = 2000
    module_chisq_min = 0.1
    module_chisq_max = 800000
    module_chisq_over_amp_min = 0.01
    module_chisq_over_amp_max = 100000

    if run_type == "lhcf":
        print ("looking at 2022 lhcf data")
        energy_sum_xmax = 3000
        energy_sum_zoomin_xmax = 0
        amp_sum_xmax = 3000
        module_amp_xmax = 2000
        module_calib_amp_xmax = 5000

    elif run_type == "pp":
        print ("looking at pp reference run")
        energy_sum_xmax = 2000
        energy_sum_zoomin_xmax = 0
        amp_sum_xmax = 2000
        module_amp_xmax = 2000
        module_calib_amp_xmax = 5000

    elif run_type == "pbpb":
        print ("looking at pbpb run")
        energy_sum_xmax = 250000.0
        energy_sum_zoomin_xmax = 20000.0
        amp_sum_xmax = 163840.0
        module_amp_xmax = 40960.0
        module_calib_amp_xmax = 100000.0


    genZdcMonTool.defineHistogram('statusBits',title=';;Events',
                            xbins=18,xmin=0.0,xmax=18,opt='kVec',
                            xlabels=['PulseBit', 'LowGainBit', 'FailBit', 'HGOverflowBit', 'HGUnderflowBit', 'PSHGOverUnderflowBit', 'LGOverflowBit', 'LGUnderflowBit', 'PrePulseBit', 'PostPulseBit', 'FitFailedBit', 'BadChisqBit', 'BadT0Bit', 'ExcludeEarlyLGBit', 'ExcludeLateLGBit', 'preExpTailBit', 'FitMinAmpBit', 'RepassPulseBit']) # 2.5TeV * 160 neutrons

    genZdcMonTool.defineHistogram('zdcEnergySumA',title='ZDC Side A Energy Sum;E_{ZDC,A}[GeV];Events',
                            xbins=200,xmin=0.0,xmax=energy_sum_xmax) # 2.5TeV * 160 neutrons
    genZdcMonTool.defineHistogram('zdcEnergySumC',title='ZDC Side C Energy Sum;E_{ZDC,C}[GeV];Events',
                            xbins=200,xmin=0.0,xmax=energy_sum_xmax)
    genZdcMonTool.defineHistogram('zdcAvgTimeA',title='ZDC Side A Average Time;t_{ZDC,A}[ns];Events',
                            xbins=50,xmin=-5.0,xmax=5.0)
    genZdcMonTool.defineHistogram('zdcAvgTimeC',title='ZDC Side C Average Time;t_{ZDC,C}[ns];Events',
                            xbins=50,xmin=-5.0,xmax=5.0)
    if run_type == "pbpb":
        genZdcMonTool.defineHistogram('zdcEnergySumA; zdcEnergySumA_zoomin',title='ZDC Side A Energy Sum (zoomed in);E_{ZDC,A}[GeV];Events',
                            xbins=200,xmin=0.0,xmax=energy_sum_zoomin_xmax) # 2.5TeV * 8 neutrons
        genZdcMonTool.defineHistogram('zdcEnergySumC; zdcEnergySumC_zoomin',title='ZDC Side C Energy Sum (zoomed in);E_{ZDC,C}[GeV];Events',
                            xbins=200,xmin=0.0,xmax=energy_sum_zoomin_xmax)
    
    genZdcMonTool.defineHistogram('zdcUncalibSumA',title='ZDC Side A Uncalibrated Sum;[ADC Counts];Events',
                            xbins=200,xmin=0.0,xmax=amp_sum_xmax)
    genZdcMonTool.defineHistogram('zdcUncalibSumC',title='ZDC Side C Uncalibrated Sum;[ADC Counts];Events',
                            xbins=200,xmin=0.0,xmax=amp_sum_xmax)
    # A vs fCalETA
    # C vs fCalETC

    # vars: x vs y
    genZdcMonTool.defineHistogram('lumiBlock', title=';lumi block;Events', # as a sanity check - see if lumi block can be plotted
                            xbins=500,xmin=0.0,xmax=lumi_block_max)
    genZdcMonTool.defineHistogram('lumiBlock, zdcEnergySumA', type='TH2F', title=';lumi block;E_{ZDC,A} [GeV]',
                            xbins=500,xmin=0.0,xmax=lumi_block_max,
                            ybins=200,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcEnergySumC', type='TH2F', title=';lumi block;E_{ZDC,C} [GeV]',
                            xbins=400,xmin=0.0,xmax=lumi_block_max,
                            ybins=200,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('zdcEnergySumA, zdcEnergySumC', type='TH2F', title=';E_{ZDC,A} [GeV];E_{ZDC,C} [GeV]',
                            xbins=200,xmin=0.0,xmax=energy_sum_xmax,
                            ybins=200,ymin=0.0,ymax=energy_sum_xmax)

    if (zdcMonAlg.CalInfoOn):
        genZdcMonTool.defineHistogram('zdcEnergySumA, fcalEtA', type='TH2F', title=';E_{ZDC,A} [GeV];E_{FCal, A} [GeV]',
                            xbins=200,xmin=0.0,xmax=energy_sum_xmax,
                            ybins=200,ymin=0.0,ymax=5000)
        genZdcMonTool.defineHistogram('zdcEnergySumC, fcalEtC', type='TH2F', title=';E_{ZDC,C} [GeV];E_{FCal, C} [GeV]',
                            xbins=200,xmin=0.0,xmax=energy_sum_xmax,
                            ybins=200,ymin=0.0,ymax=5000)
        genZdcMonTool.defineHistogram('fcalEtA, fcalEtC', type='TH2F', title=';E_{FCal, A} [GeV];E_{FCal, C} [GeV]',
                            xbins=200,xmin=0.0,xmax=5000,
                            ybins=200,ymin=0.0,ymax=5000)

# --------------------------------------------------------------------------------------------------

    zdcModuleToolArr = helper.addArray([2,4],zdcMonAlg,'ZdcModuleMonitor')
    zdcModuleToolArr.defineHistogram('zdcModuleAmp',title=';Module Amplitude [ADC Counts];Events',
                            xbins=200,xmin=0.0,xmax=module_amp_xmax)
    zdcModuleToolArr.defineHistogram('zdcModuleAmpFract',title=';Module Amplitude Fraction;Events',
                            xbins=50,xmin=0.0,xmax=1.)
    zdcModuleToolArr.defineHistogram('zdcUncalibSumCurrentSide, zdcModuleAmpFract', type='TH2F', title=';Amplitude Sum Current Side [ADC Counts];Module Amplitude Fraction',
                            xbins=200,xmin=0.0,xmax=5000,
                            ybins=50,ymin=0.0,ymax=1.)


    # zdcModuleToolArr.defineHistogram('zdcModuleAmpFract, zdcUncalibSumCurrentSide',type='TH2F',title=';Module Amplitude Fraction;E_{side} [GeV]',
    #                         xbins=50,xmin=0.0,xmax=1.,
    #                         ybins=200,ymin=0.0,ymax=8000)
    zdcModuleToolArr.defineHistogram('zdcModuleCalibAmp',title=';Module Calibrated Amplitude [GeV];Events',
                            xbins=200,xmin=0.0,xmax=module_calib_amp_xmax) # 2.5TeV * 40
    zdcModuleToolArr.defineHistogram('zdcModuleTime',title=';Module Time [ns];Events',
                            xbins=40,xmin=-10.0,xmax=10.0)
    zdcModuleToolArr.defineHistogram('zdcModuleCalibTime',title=';Module Calibrated Time [ns];Events',
                            xbins=40,xmin=-10.0,xmax=10.0)
    zdcModuleToolArr.defineHistogram('zdcModuleChisq',title=';Module Chi-square;Events',
                            xbins=create_log_bins(module_chisq_min, module_chisq_max, 80))
    zdcModuleToolArr.defineHistogram('zdcModuleChisqOverAmp',title=';Module Chi-square / Amplitude;Events',
                            xbins=create_log_bins(module_chisq_over_amp_min, module_chisq_over_amp_max, 80))

# --------------------------------------------------------------------------------------------------

    rpdChannelToolArr = helper.addArray([2,16],zdcMonAlg,'RPDMonitor')

    # to be modified with RPD analysis
    # each tool has to define at least one histogram
    # defining a space filler for now
    rpdChannelToolArr.defineHistogram('rpdChannelAmp',title=';Channel Amplitude [ADC Counts];Events',
                            xbins=40960,xmin=0.0,xmax=40960.0)


    ### STEP 6 ###
    # Finalize. The return value should be a tuple of the ComponentAccumulator
    # and the sequence containing the created algorithms. If we haven't called
    # any configuration other than the AthMonitorCfgHelper here, then we can 
    # just return directly (and not create "result" above)
    return helper.result()
    
    # # Otherwise, merge with result object and return
    # acc = helper.result()
    # result.merge(acc)
    # return result

if __name__=='__main__':
    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import WARNING
    log.setLevel(WARNING)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    import sys
    directory = ''
    inputfile = 'myAOD.pool.root'
    ConfigFlags.Input.Files = [directory+inputfile]
    ConfigFlags.Input.isMC = False
    ConfigFlags.Output.HISTFileName = 'ZdcMonitorOutput_try_readdecorhandle.root'
    ConfigFlags.fillFromArgs(sys.argv[1:])
    
    ConfigFlags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(ConfigFlags)
    cfg.merge(PoolReadCfg(ConfigFlags))

    run_type = "lhcf"

    zdcMonitorAcc = ZdcMonitoringConfig(ConfigFlags, run_type)
    cfg.merge(zdcMonitorAcc)

    # If you want to turn on more detailed messages ...
    # zdcMonitorAcc.getEventAlgo('ZdcMonAlg').OutputLevel = 2 # DEBUG
    # If you want fewer messages ...
    zdcMonitorAcc.getEventAlgo('ZdcMonAlg').OutputLevel = 4 # WARNING
    cfg.printConfig(withDetails=False) # set True for exhaustive info

    # cfg.run() #use cfg.run(20) to only run on first 20 events
    cfg.run(20)
