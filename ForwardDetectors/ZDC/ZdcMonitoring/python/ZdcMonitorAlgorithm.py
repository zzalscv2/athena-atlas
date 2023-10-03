#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

'''@file ZdcMonitorAlgorithm.py
@author Y. Guo
@author S. Mohapatra
@date 2023-08-01
@brief python configuration for ZDC monitoring under the Run III DQ framework
       will be run in the ZDC calibration stream & physics MinBias stream
       see https://acode-browser1.usatlas.bnl.gov/lxr/source/athenAControl/AthenaMonitoring/python/ExampleMonitorAlgorithm.py
       for details of structure of monitoring-configuration files
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

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'ZdcAthMonitorCfg')

    from AthenaConfiguration.ComponentFactory import CompFactory
    zdcMonAlg = helper.addAlgorithm(CompFactory.ZdcMonitorAlgorithm,'ZdcMonAlg')

    # Edit properties of a algorithm
    # zdcMonAlg.CalInfoOn = False

    genZdcMonTool = helper.addGroup(
        zdcMonAlg,
        'genZdcMonTool'
    )

# --------------------------------------------------------------------------------------------------
    # Configure histograms

    # (potentially run-type dependent) range settings
    lumi_block_max = 2000
    module_chisq_min = 0.1
    module_chisq_max = 800000
    module_chisq_over_amp_min = 0.01
    module_chisq_over_amp_max = 100000

    if run_type == "LHCf2022":
        print ("looking at 2022 lhcf data")
        energy_sum_xmax = 3000
        energy_sum_zoomin_xmax = 3000
        x_centroid_min = -500 #small amplitude sum --> large range for x, y position
        x_centroid_max = 500
        y_centroid_min = -50
        y_centroid_max = 750
        zdc_amp_sum_xmax = 3000
        rpd_amp_sum_xmax = 3000
        rpd_max_adc_sum_xmax = 3000
        module_amp_xmax = 2000
        module_calib_amp_xmax = 5000

    elif run_type == "pp2023":
        print ("looking at pp reference run")
        energy_sum_xmax = 3000
        energy_sum_zoomin_xmax = 3000
        x_centroid_min = -500 #small amplitude sum --> large range for x, y position
        x_centroid_max = 500
        y_centroid_min = -50
        y_centroid_max = 750
        zdc_amp_sum_xmax = 3000
        rpd_amp_sum_xmax = 3000
        rpd_max_adc_sum_xmax = 3000
        module_amp_xmax = 2000
        module_calib_amp_xmax = 5000

    elif run_type == "PbPb2023":
        print ("looking at pbpb run")
        energy_sum_xmax = 200000.0
        energy_sum_zoomin_xmax = 20000.0
        x_centroid_min = -20 #small amplitude sum --> large range for x, y position
        x_centroid_max = 20
        y_centroid_min = -20
        y_centroid_max = 20
        zdc_amp_sum_xmax = 163840.0
        rpd_amp_sum_xmax = 245760.0 #not the full range but a reasonable value
        rpd_max_adc_sum_xmax = 40960.0
        module_amp_xmax = 40960.0
        module_calib_amp_xmax = 100000.0

    genZdcMonTool.defineHistogram('zdcEnergySumA',title='ZDC Side A Energy Sum;E_{ZDC,A}[GeV];Events',
                            xbins=200,xmin=0.0,xmax=energy_sum_xmax) # 2.5TeV * 80 neutrons
    genZdcMonTool.defineHistogram('zdcEnergySumC',title='ZDC Side C Energy Sum;E_{ZDC,C}[GeV];Events',
                            xbins=200,xmin=0.0,xmax=energy_sum_xmax)
    genZdcMonTool.defineHistogram('zdcEnergySumA;zdcEnergySumA_zoomin_wTrigSelec',title='ZDC Side A Energy Sum (few neutrons, triggered on side C);E_{ZDC,A}[GeV];Events',
                            cutmask = 'passTrigSideC', 
                            xbins=200,xmin=0.0,xmax=energy_sum_zoomin_xmax) # 2.5TeV * 8 neutrons
    genZdcMonTool.defineHistogram('zdcEnergySumC;zdcEnergySumC_zoomin_wTrigSelec',title='ZDC Side C Energy Sum (few neutrons, triggered on side A);E_{ZDC,C}[GeV];Events',
                            cutmask = 'passTrigSideA', 
                            xbins=200,xmin=0.0,xmax=energy_sum_zoomin_xmax)
    genZdcMonTool.defineHistogram('zdcEnergySumA;zdcEnergySumA_zoomin_noTrigSelec',title='ZDC Side A Energy Sum (few neutrons, no trigger selection);E_{ZDC,A}[GeV];Events',
                            xbins=200,xmin=0.0,xmax=energy_sum_zoomin_xmax) # 2.5TeV * 8 neutrons
    genZdcMonTool.defineHistogram('zdcEnergySumC;zdcEnergySumC_zoomin_noTrigSelec',title='ZDC Side C Energy Sum (few neutrons, no trigger selection);E_{ZDC,C}[GeV];Events',
                            xbins=200,xmin=0.0,xmax=energy_sum_zoomin_xmax)
    
    genZdcMonTool.defineHistogram('zdcUncalibSumA',title='ZDC Side A Uncalibrated Sum;[ADC Counts];Events',
                            xbins=200,xmin=0.0,xmax=zdc_amp_sum_xmax)
    genZdcMonTool.defineHistogram('zdcUncalibSumC',title='ZDC Side C Uncalibrated Sum;[ADC Counts];Events',
                            xbins=200,xmin=0.0,xmax=zdc_amp_sum_xmax)

    # 2D vars: x vs y

    genZdcMonTool.defineHistogram('lumiBlock, zdcEnergySumA;zdcEnergySumA_vs_lb_noTrig', type='TH2F', title=';lumi block;E_{ZDC,A} [GeV]',
                            xbins=int(lumi_block_max),xmin=0.0,xmax=lumi_block_max,
                            ybins=200,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcEnergySumC;zdcEnergySumC_vs_lb_noTrig', type='TH2F', title=';lumi block;E_{ZDC,C} [GeV]',
                            xbins=int(lumi_block_max),xmin=0.0,xmax=lumi_block_max,
                            ybins=200,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcEnergySumA;zdcEnergySumA_vs_lb_wTrig', type='TH2F', title=';lumi block;E_{ZDC,A} [GeV]',
                            xbins=int(lumi_block_max),xmin=0.0,xmax=lumi_block_max, cutmask = 'passTrigSideC',
                            ybins=200,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcEnergySumC;zdcEnergySumC_vs_lb_wTrig', type='TH2F', title=';lumi block;E_{ZDC,C} [GeV]',
                            xbins=int(lumi_block_max),xmin=0.0,xmax=lumi_block_max, cutmask = 'passTrigSideA',
                            ybins=200,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcEnergySumA;zdcEnergySumA_vs_lb_160bins_noTrig', type='TH2F', title=';lumi block;E_{ZDC,A} [GeV]',
                            xbins=int(lumi_block_max),xmin=0.0,xmax=lumi_block_max,
                            ybins=160,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcEnergySumC;zdcEnergySumC_vs_lb_160bins_noTrig', type='TH2F', title=';lumi block;E_{ZDC,C} [GeV]',
                            xbins=int(lumi_block_max),xmin=0.0,xmax=lumi_block_max,
                            ybins=160,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcEnergySumA;zdcEnergySumA_vs_lb_160bins_wTrig', type='TH2F', title=';lumi block;E_{ZDC,A} [GeV]',
                            xbins=int(lumi_block_max),xmin=0.0,xmax=lumi_block_max, cutmask = 'passTrigSideC',
                            ybins=160,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcEnergySumC;zdcEnergySumC_vs_lb_160bins_wTrig', type='TH2F', title=';lumi block;E_{ZDC,C} [GeV]',
                            xbins=int(lumi_block_max),xmin=0.0,xmax=lumi_block_max, cutmask = 'passTrigSideA',
                            ybins=160,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('zdcEnergySumA, zdcEnergySumC', type='TH2F', title=';E_{ZDC,A} [GeV];E_{ZDC,C} [GeV]',
                            xbins=200,xmin=0.0,xmax=energy_sum_xmax,
                            ybins=200,ymin=0.0,ymax=energy_sum_xmax)

    genZdcMonTool.defineHistogram('rpdCosDeltaReactionPlaneAngle', title=';Cos (#Delta #phi_{AorC});Events',
                            xbins=100,xmin=-1,xmax=1)

    # FCal E_T vs ZDC E_T
    # to be run on min bias stream
    if (zdcMonAlg.CalInfoOn):
        genZdcMonTool.defineHistogram('fcalEtA, zdcEnergySumA', type='TH2F', title=';E_{FCal, A} [GeV];E_{ZDC,A} [GeV]',
                            xbins=200,xmin=0.0,xmax=5000,
                            ybins=200,ymin=0.0,ymax=energy_sum_xmax)
        genZdcMonTool.defineHistogram('fcalEtC, zdcEnergySumC', type='TH2F', title=';E_{FCal, C} [GeV];E_{ZDC,C} [GeV]',
                            xbins=200,xmin=0.0,xmax=5000,
                            ybins=200,ymin=0.0,ymax=energy_sum_xmax)
        genZdcMonTool.defineHistogram('fcalEtA, fcalEtC', type='TH2F', title=';E_{FCal, A} [GeV];E_{FCal, C} [GeV]',
                            xbins=200,xmin=0.0,xmax=5000,
                            ybins=200,ymin=0.0,ymax=5000)

# --------------------------------------------------------------------------------------------------
    nSides = 2
    nModules = 4
    nChannels = 16

    nZdcStatusBits = 18
    nRpdStatusBits = 3
    nRpdCentroidStatusBits = 17

# --------------------------------------------------------------------------------------------------

    zdcSideMonToolArr = helper.addArray([nSides],zdcMonAlg,'ZdcSideMonitor')


    zdcSideMonToolArr.defineHistogram('zdcAvgTime',title='ZDC Side Average Time;t[ns];Events',
                            xbins=50,xmin=-5.0,xmax=5.0)

    zdcSideMonToolArr.defineHistogram('lumiBlock, zdcAvgTime;zdcAvgTime_vs_lb', type='TH2F', title='ZDC Side Average Time versus Lumi block;lumi block;t[ns]',
                            xbins=int(lumi_block_max),xmin=0.0,xmax=lumi_block_max,
                            ybins=50,ymin=-5.0,ymax=5.0)

    zdcSideMonToolArr.defineHistogram('centroidStatusBits',title=';;Events',
                            xbins=18,xmin=0.0,xmax=nRpdCentroidStatusBits,opt='kVec',
                            xlabels=['ValidBit', 'ZDCValidBit', 'RPDValidBit', 'MinimumZDCEnergyBit', 'ExcessiveZDCEnergyBit', 'PileupBit', 'ExcessivePileupBit', 'SubtrUnderflowBit', 'ZeroSumBit', 'ZeroSumRow0Bit', 'ZeroSumRow1Bit', 'ZeroSumRow2Bit', 'ZeroSumRow3Bit', 'ZeroSumCol0Bit', 'ZeroSumCol1Bit', 'ZeroSumCol2Bit', 'ZeroSumCol3Bit'])

    zdcSideMonToolArr.defineHistogram('xCentroid, yCentroid',type='TH2F',title=';Centroid x position [mm];Centroid y position [mm]',
                            xbins=100,xmin=x_centroid_min,xmax=x_centroid_max,
                            ybins=100,ymin=y_centroid_min,ymax=y_centroid_max)
    zdcSideMonToolArr.defineHistogram('xDetCentroidUnsub, yDetCentroidUnsub',type='TH2F',title=';Centroid x position unsubtracted;Centroid y position unsubtracted',
                            xbins=100,xmin=x_centroid_min,xmax=x_centroid_max,
                            ybins=100,ymin=y_centroid_min,ymax=y_centroid_max)
    zdcSideMonToolArr.defineHistogram('ReactionPlaneAngle',title=';Reaction Plane Angle;Events',
                            xbins=64,xmin=-3.141593,xmax=3.141593)

    zdcSideMonToolArr.defineHistogram('lumiBlock, xCentroid;xCentroid_vs_lb', type='TH2F', title=';lumi block;Centroid x position [mm]',
                            xbins=int(lumi_block_max),xmin=0.0,xmax=lumi_block_max,
                            ybins=100,ymin=x_centroid_min,ymax=x_centroid_max)
    zdcSideMonToolArr.defineHistogram('lumiBlock, yCentroid;yCentroid_vs_lb', type='TH2F', title=';lumi block;Centroid y position [mm]',
                            xbins=int(lumi_block_max),xmin=0.0,xmax=lumi_block_max,
                            ybins=100,ymin=y_centroid_min,ymax=y_centroid_max)

    zdcSideMonToolArr.defineHistogram('zdcEnergySum, rpdMaxADCSum', type='TH2F', title=';E ZDC side [TeV];RPD Max ADC Sum (AorC) [ADC counts]',
                            xbins=200,xmin=0.0,xmax=energy_sum_xmax,
                            ybins=200,ymin=0.0,ymax=rpd_max_adc_sum_xmax) # try a value for now
    zdcSideMonToolArr.defineHistogram('zdcEnergySum, rpdAmplitudeCalibSum', type='TH2F', title=';E ZDC side [GeV];RPD Calib Amp Sum (AorC) [ADC counts]',
                            xbins=200,xmin=0.0,xmax=energy_sum_xmax,
                            ybins=200,ymin=0.0,ymax=rpd_amp_sum_xmax) # try a value for now
    zdcSideMonToolArr.defineHistogram('zdcEMModuleEnergy, rpdAmplitudeCalibSum', type='TH2F', title=';E EM module AorC [GeV];RPD Calib Amp Sum (AorC) [ADC counts]',
                            xbins=200,xmin=0.0,xmax=module_calib_amp_xmax / 2., # divide by 2 to make a more zoomed-in plot (not full range)
                            ybins=200,ymin=0.0,ymax=rpd_amp_sum_xmax) # try a value for now

# --------------------------------------------------------------------------------------------------

    zdcModuleMonToolArr = helper.addArray([nSides,nModules],zdcMonAlg,'ZdcModuleMonitor')

    zdcModuleMonToolArr.defineHistogram('zdcStatusBits',title=';;Events',
                            xbins=18,xmin=0.0,xmax=nZdcStatusBits,opt='kVec',
                            xlabels=['PulseBit', 'LowGainBit', 'FailBit', 'HGOverflowBit', 'HGUnderflowBit', 'PSHGOverUnderflowBit', 'LGOverflowBit', 'LGUnderflowBit', 'PrePulseBit', 'PostPulseBit', 'FitFailedBit', 'BadChisqBit', 'BadT0Bit', 'ExcludeEarlyLGBit', 'ExcludeLateLGBit', 'preExpTailBit', 'FitMinAmpBit', 'RepassPulseBit'])


    zdcModuleMonToolArr.defineHistogram('zdcModuleAmp',title=';Module Amplitude [ADC Counts];Events',
                            xbins=200,xmin=0.0,xmax=module_amp_xmax)
    zdcModuleMonToolArr.defineHistogram('zdcModuleFract',title=';Module Amplitude Fraction;Events',
                            xbins=50,xmin=0.0,xmax=1.)
    zdcModuleMonToolArr.defineHistogram('zdcEnergySumCurrentSide, zdcModuleFract', type='TH2F', title=';Amplitude Sum Current Side [ADC Counts];Module Amplitude Fraction',
                            xbins=200,xmin=0.0,xmax=5000,
                            ybins=50,ymin=0.0,ymax=1.)

    zdcModuleMonToolArr.defineHistogram('zdcModuleCalibAmp',title=';Module Calibrated Amplitude [GeV];Events',
                            xbins=200,xmin=0.0,xmax=module_calib_amp_xmax) # 2.5TeV * 40
    zdcModuleMonToolArr.defineHistogram('zdcModuleTime',title=';Module Time [ns];Events',
                            xbins=40,xmin=-10.0,xmax=10.0)
    zdcModuleMonToolArr.defineHistogram('zdcModuleCalibTime',title=';Module Calibrated Time [ns];Events',
                            xbins=40,xmin=-10.0,xmax=10.0)
    zdcModuleMonToolArr.defineHistogram('zdcModuleChisq',title=';Module Chi-square;Events',
                            xbins=create_log_bins(module_chisq_min, module_chisq_max, 80))
    zdcModuleMonToolArr.defineHistogram('zdcModuleChisqOverAmp',title=';Module Chi-square / Amplitude;Events',
                            xbins=create_log_bins(module_chisq_over_amp_min, module_chisq_over_amp_max, 80))


# --------------------------------------------------------------------------------------------------

    rpdChannelMonToolArr = helper.addArray([nSides,nChannels],zdcMonAlg,'RpdChannelMonitor')

    rpdChannelMonToolArr.defineHistogram('RPDStatusBits',title=';;Events',
                        xbins=nRpdStatusBits,xmin=0,xmax=nRpdStatusBits,opt='kVec',
                        xlabels=['ChValidBit', 'ChOutOfTimePileupBit', 'ChOverflowBit'])

    rpdChannelMonToolArr.defineHistogram('RPDChannelAmplitudeCalib', title=';RPD Channel Calibrated Amplitude [GeV];Events',
                            xbins=200,xmin=0.0,xmax=module_amp_xmax) # NOT energy calibration - calibration factor is 1 for now
    rpdChannelMonToolArr.defineHistogram('RPDChannelMaxADC', title=' Max ADC [ADC Counts];Events',
                            xbins=200,xmin=0.0,xmax=4096.0)


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
    ConfigFlags.Output.HISTFileName = 'ZdcMonitorOutput_HI2023_test.root'
    ConfigFlags.fillFromArgs(sys.argv[1:])
    
    ConfigFlags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(ConfigFlags)
    cfg.merge(PoolReadCfg(ConfigFlags))

    run_type = "PbPb2023"

    zdcMonitorAcc = ZdcMonitoringConfig(ConfigFlags, run_type)
    cfg.merge(zdcMonitorAcc)

    # If you want to turn on more detailed messages ...
    zdcMonitorAcc.getEventAlgo('ZdcMonAlg').OutputLevel = 2 # DEBUG
    # If you want fewer messages ...
    # zdcMonitorAcc.getEventAlgo('ZdcMonAlg').OutputLevel = 4 # WARNING
    cfg.printConfig(withDetails=False) # set True for exhaustive info

    cfg.run(20) #use cfg.run(20) to only run on first 20 events
