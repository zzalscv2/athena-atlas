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
    bcid_max = 3564
    n_energy_bins_default = 200
    n_time_centroid_bins_default = 100
    n_module_amp_zoomin_bins = 100
    module_chisq_min = 0.1
    module_chisq_max = 800000
    module_chisq_over_amp_min = 0.01
    module_chisq_over_amp_max = 100000

    if run_type == "LHCf2022":
        print ("looking at 2022 lhcf data")
        energy_sum_xmax = 3000
        energy_sum_zoomin_xmax = 3000
        uncalib_amp_sum_zoomin_xmax = 3000
        x_centroid_min = -500 #small amplitude sum --> large range for x, y position
        x_centroid_max = 500
        y_centroid_min = -50
        y_centroid_max = 750
        zdc_amp_sum_xmax = 3000
        rpd_channel_amp_min = - 200. 
        rpd_amp_sum_xmax = 3000
        rpd_max_adc_sum_xmax = 3000
        module_amp_xmax = 2000
        module_calib_amp_xmax = 5000
        module_amp_1Nmonitor_xmax = 2000 #about 5N / 4 * 2.7TeV
        module_calib_amp_1Nmonitor_xmax = 5000 #about 5N / 4 * 2.7TeV

    elif run_type == "pp2023":
        print ("looking at pp reference run")
        energy_sum_xmax = 3000
        energy_sum_zoomin_xmax = 3000
        uncalib_amp_sum_zoomin_xmax = 3000
        x_centroid_min = -500 #small amplitude sum --> large range for x, y position
        x_centroid_max = 500
        y_centroid_min = -50
        y_centroid_max = 750
        zdc_amp_sum_xmax = 3000
        rpd_channel_amp_min = - 200. 
        rpd_amp_sum_xmax = 3000
        rpd_max_adc_sum_xmax = 3000
        module_amp_xmax = 2000
        module_calib_amp_xmax = 5000
        module_amp_1Nmonitor_xmax = 2000 #about 5N / 4 * 2.7TeV
        module_calib_amp_1Nmonitor_xmax = 5000 #about 5N / 4 * 2.7TeV


    elif run_type == "PbPb2023":
        print ("looking at pbpb run")
        energy_sum_xmax = 200000.0
        energy_sum_zoomin_xmax = 20000.0
        uncalib_amp_sum_zoomin_xmax = 6000.0
        x_centroid_min = -20 #small amplitude sum --> large range for x, y position
        x_centroid_max = 20
        y_centroid_min = -20
        y_centroid_max = 20
        zdc_amp_sum_xmax = 163840.0
        rpd_channel_amp_min = - 2048. 
        rpd_amp_sum_xmax = 245760.0 #not the full range but a reasonable value
        rpd_max_adc_sum_xmax = 40960.0
        module_amp_xmax = 40960.0
        module_calib_amp_xmax = 100000.0 #about the full dynamic range: 160 N / 4 * 2.5TeV
        module_amp_1Nmonitor_xmax = 1250.0 #about 5N / 4 * 2.7TeV
        module_calib_amp_1Nmonitor_xmax = 3400.0 #about 5N / 4 * 2.7TeV

    # #bins for RPD channel amplitude, including negative values - determined by the ratio between the negative amplitude range & positive amplitude range
    n_rpd_amp_bins_full_range = int((abs(rpd_channel_amp_min) + module_amp_xmax) / module_amp_xmax * n_energy_bins_default)
    n_rpd_amp_bins_half_range = int((abs(rpd_channel_amp_min) + module_amp_xmax/2.) / (module_amp_xmax/2.) * n_energy_bins_default)
    rpd_sub_amp_min = - module_amp_xmax / 4.
    rpd_sub_amp_max = module_amp_xmax / 2.
    n_rpd_sub_amp_bins = int((abs(rpd_sub_amp_min) + rpd_sub_amp_max) / rpd_sub_amp_max * n_energy_bins_default)

    genZdcMonTool.defineHistogram('zdcEnergySumA',title='ZDC Side A Energy Sum;E_{ZDC,A}[GeV];Events',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_xmax) # 2.5TeV * 80 neutrons
    genZdcMonTool.defineHistogram('zdcEnergySumC',title='ZDC Side C Energy Sum;E_{ZDC,C}[GeV];Events',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_xmax)
    genZdcMonTool.defineHistogram('zdcEnergySumA;zdcEnergySumA_zoomin_wTrigSelec',title='ZDC Side A Energy Sum (few neutrons, triggered on side C);E_{ZDC,A}[GeV];Events',
                            cutmask = 'passTrigSideC', 
                            xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_zoomin_xmax) # 2.5TeV * 8 neutrons
    genZdcMonTool.defineHistogram('zdcEnergySumC;zdcEnergySumC_zoomin_wTrigSelec',title='ZDC Side C Energy Sum (few neutrons, triggered on side A);E_{ZDC,C}[GeV];Events',
                            cutmask = 'passTrigSideA', 
                            xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_zoomin_xmax)
    genZdcMonTool.defineHistogram('zdcEnergySumA;zdcEnergySumA_zoomin_noTrigSelec',title='ZDC Side A Energy Sum (few neutrons, no trigger selection);E_{ZDC,A}[GeV];Events',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_zoomin_xmax) # 2.5TeV * 8 neutrons
    genZdcMonTool.defineHistogram('zdcEnergySumC;zdcEnergySumC_zoomin_noTrigSelec',title='ZDC Side C Energy Sum (few neutrons, no trigger selection);E_{ZDC,C}[GeV];Events',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_zoomin_xmax)
    
    genZdcMonTool.defineHistogram('zdcUncalibSumA',title='ZDC Side A Uncalibrated Sum;[ADC Counts];Events',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=zdc_amp_sum_xmax)
    genZdcMonTool.defineHistogram('zdcUncalibSumC',title='ZDC Side C Uncalibrated Sum;[ADC Counts];Events',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=zdc_amp_sum_xmax)

    # 2D vars: x vs y

    genZdcMonTool.defineHistogram('lumiBlock, zdcEnergySumA;zdcEnergySumA_vs_lb_noTrig', type='TH2F', title=';lumi block;E_{ZDC,A} [GeV]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcEnergySumC;zdcEnergySumC_vs_lb_noTrig', type='TH2F', title=';lumi block;E_{ZDC,C} [GeV]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcEnergySumA;zdcEnergySumA_vs_lb_wTrig', type='TH2F', title=';lumi block;E_{ZDC,A} [GeV]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max, cutmask = 'passTrigSideC',
                            ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcEnergySumC;zdcEnergySumC_vs_lb_wTrig', type='TH2F', title=';lumi block;E_{ZDC,C} [GeV]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max, cutmask = 'passTrigSideA',
                            ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks

    genZdcMonTool.defineHistogram('lumiBlock, zdcUncalibSumA;zdcUncalibSumA_vs_lb_noTrig', type='TH2F', title=';lumi block;ZDC amp A[ADC counts]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcUncalibSumC;zdcUncalibSumC_vs_lb_noTrig', type='TH2F', title=';lumi block;ZDC amp C[ADC counts]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcUncalibSumA;zdcUncalibSumA_vs_lb_wTrig', type='TH2F', title=';lumi block;ZDC amp A[ADC counts]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max, cutmask = 'passTrigSideC',
                            ybins=n_energy_bins_default,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcUncalibSumC;zdcUncalibSumC_vs_lb_wTrig', type='TH2F', title=';lumi block;ZDC amp C[ADC counts]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max, cutmask = 'passTrigSideA',
                            ybins=n_energy_bins_default,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks

    genZdcMonTool.defineHistogram('bcid, zdcEnergySumA;zdcEnergySumA_vs_bcid_noTrig', type='TH2F', title=';BCID;E_{ZDC,A} [GeV]',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('bcid, zdcEnergySumC;zdcEnergySumC_vs_bcid_noTrig', type='TH2F', title=';BCID;E_{ZDC,C} [GeV]',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('bcid, zdcEnergySumA;zdcEnergySumA_vs_bcid_wTrig', type='TH2F', title=';BCID;E_{ZDC,A} [GeV]',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max, cutmask = 'passTrigSideC',
                            ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('bcid, zdcEnergySumC;zdcEnergySumC_vs_bcid_wTrig', type='TH2F', title=';BCID;E_{ZDC,C} [GeV]',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max, cutmask = 'passTrigSideA',
                            ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks

    genZdcMonTool.defineHistogram('bcid, zdcUncalibSumA;zdcUncalibSumA_vs_bcid_noTrig', type='TH2F', title=';BCID;ZDC amp A[ADC counts]',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('bcid, zdcUncalibSumC;zdcUncalibSumC_vs_bcid_noTrig', type='TH2F', title=';BCID;ZDC amp C[ADC counts]',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('bcid, zdcUncalibSumA;zdcUncalibSumA_vs_bcid_wTrig', type='TH2F', title=';BCID;ZDC amp A[ADC counts]',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max, cutmask = 'passTrigSideC',
                            ybins=n_energy_bins_default,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('bcid, zdcUncalibSumC;zdcUncalibSumC_vs_bcid_wTrig', type='TH2F', title=';BCID;ZDC amp C[ADC counts]',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max, cutmask = 'passTrigSideA',
                            ybins=n_energy_bins_default,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks

    genZdcMonTool.defineHistogram('lumiBlock, zdcUncalibSumA;zdcUncalibSumA_vs_lb_160bins_noTrig', type='TH2F', title=';lumi block;E_{ZDC,A} [GeV]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=160,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcUncalibSumC;zdcUncalibSumC_vs_lb_160bins_noTrig', type='TH2F', title=';lumi block;E_{ZDC,C} [GeV]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=160,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcUncalibSumA;zdcUncalibSumA_vs_lb_160bins_wTrig', type='TH2F', title=';lumi block;E_{ZDC,A} [GeV]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max, cutmask = 'passTrigSideC',
                            ybins=160,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    genZdcMonTool.defineHistogram('lumiBlock, zdcUncalibSumC;zdcUncalibSumC_vs_lb_160bins_wTrig', type='TH2F', title=';lumi block;E_{ZDC,C} [GeV]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max, cutmask = 'passTrigSideA',
                            ybins=160,ymin=0.0,ymax=uncalib_amp_sum_zoomin_xmax) # for lumi dependence, only focus on the few-neutron peaks
    
    genZdcMonTool.defineHistogram('zdcEnergySumA, zdcEnergySumC', type='TH2F', title=';E_{ZDC,A} [GeV];E_{ZDC,C} [GeV]',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_xmax,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_xmax)

    genZdcMonTool.defineHistogram('rpdCosDeltaReactionPlaneAngle', title=';Cos (#Delta #phi_{AorC});Events',
                            cutmask='bothSubAmpSumPositive', # only require subtracted amplitude sum on both sides to be positive
                            xbins=n_time_centroid_bins_default,xmin=-1,xmax=1)
    genZdcMonTool.defineHistogram('rpdCosDeltaReactionPlaneAngle_requireValid', title=';Cos (#Delta #phi_{AorC});Events',
                            cutmask='bothReactionPlaneAngleValid', # require centroid calculation on both sides to be valid
                            xbins=n_time_centroid_bins_default,xmin=-1,xmax=1)

    # FCal E_T vs ZDC E_T
    # to be run on min bias stream
    if (zdcMonAlg.CalInfoOn):
        genZdcMonTool.defineHistogram('fcalEtA, zdcEnergySumA', type='TH2F', title=';E_{FCal, A} [GeV];E_{ZDC,A} [GeV]',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=5000,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_xmax)
        genZdcMonTool.defineHistogram('fcalEtC, zdcEnergySumC', type='TH2F', title=';E_{FCal, C} [GeV];E_{ZDC,C} [GeV]',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=5000,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=energy_sum_xmax)
        genZdcMonTool.defineHistogram('fcalEtA, fcalEtC', type='TH2F', title=';E_{FCal, A} [GeV];E_{FCal, C} [GeV]',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=5000,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=5000)

# --------------------------------------------------------------------------------------------------
    nSides = 2
    nModules = 4
    nChannels = 16

    nZdcStatusBits = 18
    nRpdStatusBits = 3
    nRpdCentroidStatusBits = 17

# --------------------------------------------------------------------------------------------------

    zdcSideMonToolArr = helper.addArray([nSides],zdcMonAlg,'ZdcSideMonitor')


    zdcSideMonToolArr.defineHistogram('zdcAvgTime',title='ZDC Side Average Time;t[ns];Events', cutmask = 'zdcModuleMask',
                            xbins=n_time_centroid_bins_default,xmin=-10.0,xmax=10.0)

    zdcSideMonToolArr.defineHistogram('lumiBlock, zdcAvgTime;zdcAvgTime_vs_lb', type='TH2F', title='ZDC Side Average Time versus Lumi block;lumi block;t[ns]',
                            cutmask = 'zdcModuleMask',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_time_centroid_bins_default,ymin=-10.0,ymax=10.0)

    zdcSideMonToolArr.defineHistogram('centroidStatusBits',title=';;Events',
                            xbins=18,xmin=0.0,xmax=nRpdCentroidStatusBits,opt='kVec',
                            xlabels=['ValidBit', 'ZDCValidBit', 'RPDValidBit', 'MinimumZDCEnergyBit', 'ExcessiveZDCEnergyBit', 'PileupBit', 'ExcessivePileupBit', 'SubtrUnderflowBit', 'ZeroSumBit', 'ZeroSumRow0Bit', 'ZeroSumRow1Bit', 'ZeroSumRow2Bit', 'ZeroSumRow3Bit', 'ZeroSumCol0Bit', 'ZeroSumCol1Bit', 'ZeroSumCol2Bit', 'ZeroSumCol3Bit'])

    zdcSideMonToolArr.defineHistogram('xCentroid, yCentroid',type='TH2F',title=';Centroid x position [mm];Centroid y position [mm]',
                            xbins=n_time_centroid_bins_default,xmin=x_centroid_min,xmax=x_centroid_max,
                            ybins=n_time_centroid_bins_default,ymin=y_centroid_min,ymax=y_centroid_max)
    zdcSideMonToolArr.defineHistogram('xDetCentroidUnsub, yDetCentroidUnsub',type='TH2F',title=';Centroid x position unsubtracted;Centroid y position unsubtracted',
                            xbins=n_time_centroid_bins_default,xmin=x_centroid_min,xmax=x_centroid_max,
                            ybins=n_time_centroid_bins_default,ymin=y_centroid_min,ymax=y_centroid_max)
    zdcSideMonToolArr.defineHistogram('ReactionPlaneAngle',title=';Reaction Plane Angle;Events',
                            xbins=64,xmin=-3.141593,xmax=3.141593)

    zdcSideMonToolArr.defineHistogram('xCentroid, yCentroid;yCentroid_vs_xCentroid_requireValid',type='TH2F',title=';Centroid x position [mm];Centroid y position [mm]',
                            cutmask='centroidValid',
                            xbins=n_time_centroid_bins_default,xmin=x_centroid_min,xmax=x_centroid_max,
                            ybins=n_time_centroid_bins_default,ymin=y_centroid_min,ymax=y_centroid_max)
    zdcSideMonToolArr.defineHistogram('xDetCentroidUnsub, yDetCentroidUnsub;yDetCentroidUnsub_vs_xDetCentroidUnsub_requireValid',type='TH2F',title=';Centroid x position unsubtracted;Centroid y position unsubtracted',
                            cutmask='centroidValid',
                            xbins=n_time_centroid_bins_default,xmin=x_centroid_min,xmax=x_centroid_max,
                            ybins=n_time_centroid_bins_default,ymin=y_centroid_min,ymax=y_centroid_max)
    zdcSideMonToolArr.defineHistogram('ReactionPlaneAngle;ReactionPlaneAngle_requireValid',title=';Reaction Plane Angle;Events',
                            cutmask='centroidValid',
                            xbins=64,xmin=-3.141593,xmax=3.141593)


    zdcSideMonToolArr.defineHistogram('lumiBlock, xCentroid;xCentroid_vs_lb', type='TH2F', title=';lumi block;Centroid x position [mm]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_time_centroid_bins_default,ymin=x_centroid_min,ymax=x_centroid_max)
    zdcSideMonToolArr.defineHistogram('lumiBlock, yCentroid;yCentroid_vs_lb', type='TH2F', title=';lumi block;Centroid y position [mm]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_time_centroid_bins_default,ymin=y_centroid_min,ymax=y_centroid_max)

    zdcSideMonToolArr.defineHistogram('lumiBlock, xCentroid;xCentroid_vs_lb_requireValid', type='TH2F', title=';lumi block;Centroid x position [mm]',
                            cutmask='centroidValid',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_time_centroid_bins_default,ymin=x_centroid_min,ymax=x_centroid_max)
    zdcSideMonToolArr.defineHistogram('lumiBlock, yCentroid;yCentroid_vs_lb_requireValid', type='TH2F', title=';lumi block;Centroid y position [mm]',
                            cutmask='centroidValid',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_time_centroid_bins_default,ymin=y_centroid_min,ymax=y_centroid_max)

    zdcSideMonToolArr.defineHistogram('rpdSubAmpSum', title=';RPD Subtracted Amp Sum (AorC) [ADC counts]',
                            xbins=n_energy_bins_default,xmin = - rpd_amp_sum_xmax / 16., xmax=rpd_amp_sum_xmax / 4.) # try a value for now

    zdcSideMonToolArr.defineHistogram('zdcEnergySum, rpdMaxADCSum', type='TH2F', title=';E ZDC side [TeV];RPD Max ADC Sum (AorC) [ADC counts]',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_xmax,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=rpd_max_adc_sum_xmax) # try a value for now
    zdcSideMonToolArr.defineHistogram('zdcEnergySum, rpdAmplitudeCalibSum', type='TH2F', title=';E ZDC side [GeV];RPD Calib Amp Sum (AorC) [ADC counts]',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=energy_sum_xmax,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=rpd_amp_sum_xmax) # try a value for now
    zdcSideMonToolArr.defineHistogram('zdcEMModuleEnergy, rpdAmplitudeCalibSum', type='TH2F', title=';E EM module AorC [GeV];RPD Calib Amp Sum (AorC) [ADC counts]',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=module_calib_amp_xmax / 2., # divide by 2 to make a more zoomed-in plot (not full range)
                            ybins=n_energy_bins_default,ymin=0.0,ymax=rpd_amp_sum_xmax) # try a value for now

# --------------------------------------------------------------------------------------------------

    zdcModuleMonToolArr = helper.addArray([nSides,nModules],zdcMonAlg,'ZdcModuleMonitor')

    zdcModuleMonToolArr.defineHistogram('zdcStatusBits',title=';;Events',
                            xbins=18,xmin=0.0,xmax=nZdcStatusBits,opt='kVec',
                            xlabels=['PulseBit', 'LowGainBit', 'FailBit', 'HGOverflowBit', 'HGUnderflowBit', 'PSHGOverUnderflowBit', 'LGOverflowBit', 'LGUnderflowBit', 'PrePulseBit', 'PostPulseBit', 'FitFailedBit', 'BadChisqBit', 'BadT0Bit', 'ExcludeEarlyLGBit', 'ExcludeLateLGBit', 'preExpTailBit', 'FitMinAmpBit', 'RepassPulseBit'])


    zdcModuleMonToolArr.defineHistogram('zdcModuleAmp',title=';Module Amplitude [ADC Counts];Events',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=module_amp_xmax)
    zdcModuleMonToolArr.defineHistogram('zdcModuleAmp;zdcModuleAmp_halfrange',title=';Module Amplitude [ADC Counts];Events',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=module_amp_xmax / 2.)
    zdcModuleMonToolArr.defineHistogram('zdcModuleFract',title=';Module Amplitude Fraction;Events',
                            xbins=50,xmin=0.0,xmax=1.)
    zdcModuleMonToolArr.defineHistogram('zdcEnergySumCurrentSide, zdcModuleFract', type='TH2F', title=';Amplitude Sum Current Side [ADC Counts];Module Amplitude Fraction',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=5000,
                            ybins=50,ymin=0.0,ymax=1.)

    zdcModuleMonToolArr.defineHistogram('zdcModuleCalibAmp',title=';Module Calibrated Amplitude [GeV];Events',
                            xbins=2*n_energy_bins_default,xmin=0.0,xmax=module_calib_amp_xmax) # 2.5TeV * 40
    zdcModuleMonToolArr.defineHistogram('zdcModuleCalibAmp;zdcModuleCalibAmp_halfrange',title=';Module Calibrated Amplitude [GeV];Events',
                            xbins=2*n_energy_bins_default,xmin=0.0,xmax=module_calib_amp_xmax / 2.) # 2.5TeV * 40
    zdcModuleMonToolArr.defineHistogram('zdcModuleTime',title=';Module Time [ns];Events',
                            xbins=n_time_centroid_bins_default,xmin=-10.0,xmax=10.0)
    zdcModuleMonToolArr.defineHistogram('zdcModuleTime;zdcModuleTime_LG',title=';Module Time [ns];Events',
                            cutmask='zdcModuleLG',
                            xbins=n_time_centroid_bins_default,xmin=-10.0,xmax=10.0)
    zdcModuleMonToolArr.defineHistogram('zdcModuleTime;zdcModuleTime_HG',title=';Module Time [ns];Events',
                            cutmask='zdcModuleHG',
                            xbins=n_time_centroid_bins_default,xmin=-10.0,xmax=10.0)
    zdcModuleMonToolArr.defineHistogram('zdcModuleCalibTime',title=';Module Calibrated Time [ns];Events',
                            xbins=n_time_centroid_bins_default,xmin=-10.0,xmax=10.0)
    zdcModuleMonToolArr.defineHistogram('zdcModuleChisq',title=';Module Chi-square;Events',
                            xbins=create_log_bins(module_chisq_min, module_chisq_max, 80))
    zdcModuleMonToolArr.defineHistogram('zdcModuleChisqOverAmp',title=';Module Chi-square / Amplitude;Events',
                            xbins=create_log_bins(module_chisq_over_amp_min, module_chisq_over_amp_max, 80))

    zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleAmp;zdcModuleAmp_vs_lb', type='TH2F', title=';lumi block;Module Amplitude [ADC counts]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_module_amp_zoomin_bins, ymin=0.0, ymax=module_amp_1Nmonitor_xmax)
    zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleCalibAmp;zdcModuleCalibAmp_vs_lb', type='TH2F', title=';lumi block;Module Calib Amplitude',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_module_amp_zoomin_bins, ymin=0.0, ymax=module_calib_amp_1Nmonitor_xmax)

    zdcModuleMonToolArr.defineHistogram('bcid, zdcModuleCalibAmp', type='TH2F', title=';BCID;Module Calib Amplitude',
                            xbins=bcid_max,xmin=0.0,xmax=bcid_max,
                            ybins=n_module_amp_zoomin_bins, ymin=0.0, ymax=module_calib_amp_1Nmonitor_xmax)

    zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleTime;zdcModuleTime_vs_lb', type='TH2F', title=';lumi block;Module Time [ns]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_time_centroid_bins_default, ymin=-10.0, ymax=10.0)
    zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleTime;zdcModuleTime_LG_vs_lb', type='TH2F', title=';lumi block;Module Time [ns]',
                            cutmask='zdcModuleLG',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_time_centroid_bins_default, ymin=-10.0, ymax=10.0)
    zdcModuleMonToolArr.defineHistogram('lumiBlock, zdcModuleTime;zdcModuleTime_HG_vs_lb', type='TH2F', title=';lumi block;Module Time [ns]',
                            cutmask='zdcModuleHG',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_time_centroid_bins_default, ymin=-10.0, ymax=10.0)

# --------------------------------------------------------------------------------------------------

    rpdChannelMonToolArr = helper.addArray([nSides,nChannels],zdcMonAlg,'RpdChannelMonitor')

    rpdChannelMonToolArr.defineHistogram('RPDStatusBits',title=';;Events',
                        xbins=nRpdStatusBits,xmin=0,xmax=nRpdStatusBits,opt='kVec',
                        xlabels=['ChValidBit', 'ChOutOfTimePileupBit', 'ChOverflowBit'])

    rpdChannelMonToolArr.defineHistogram('RPDChannelAmplitudeCalib', title=';RPD Channel Calibrated Amplitude;Events',
                            xbins=n_rpd_amp_bins_full_range,xmin=rpd_channel_amp_min,xmax=module_amp_xmax) # NOT energy calibration - calibration factor is 1 for now
    rpdChannelMonToolArr.defineHistogram('RPDChannelAmplitudeCalib;RPDChannelAmplitudeCalib_halfrange', title=';RPD Channel Calibrated Amplitude;Events',
                            xbins=n_rpd_amp_bins_half_range,xmin=rpd_channel_amp_min,xmax=module_amp_xmax / 2.) # NOT energy calibration - calibration factor is 1 for now
    rpdChannelMonToolArr.defineHistogram('RPDChannelSubAmp', title=';RPD Channel Subtracted Amplitude;Events',
                            xbins=n_rpd_sub_amp_bins,xmin=rpd_sub_amp_min,xmax=rpd_sub_amp_max) # NOT energy calibration - calibration factor is 1 for now
    rpdChannelMonToolArr.defineHistogram('RPDChannelMaxADC', title=';Max ADC [ADC Counts];Events',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=4096.0)
    rpdChannelMonToolArr.defineHistogram('lumiBlock, RPDChannelAmplitudeCalib;RPDChannelAmplitudeCalib_vs_lb', type='TH2F', title=';lumi block;RPD Channel Calibrated Amplitude',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_rpd_amp_bins_full_range,ymin=rpd_channel_amp_min,ymax=module_amp_xmax) # NOT energy calibration - calibration factor is 1 for now
    rpdChannelMonToolArr.defineHistogram('lumiBlock, RPDChannelAmplitudeCalib;RPDChannelAmplitudeCalib_halfrange_vs_lb', type='TH2F', title=';lumi block;RPD Channel Calibrated Amplitude',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_rpd_amp_bins_half_range,ymin=rpd_channel_amp_min,ymax=module_amp_xmax / 2.) # NOT energy calibration - calibration factor is 1 for now
    rpdChannelMonToolArr.defineHistogram('lumiBlock, RPDChannelMaxADC;RPDChannelMaxADC_vs_lb', type='TH2F', title=';lumi block;Max ADC [ADC Counts]',
                            xbins=lumi_block_max,xmin=0.0,xmax=lumi_block_max,
                            ybins=n_energy_bins_default,ymin=0.0,ymax=4096.0)


    # study on EM module energy, max ADC and pile-up fit slope for RPD channels with negative amplitude
    rpdChannelMonToolArr.defineHistogram('RPDChannelPileupFrac;RPDChannelPileupFrac_negative_amp', title=';;Events',
                            cutmask='RPDChannelNegativeAmp',
                            xlabels=['pileup','no pileup'],
                            xbins=2,xmin=-1.5,xmax=0.5)

    rpdChannelMonToolArr.defineHistogram('zdcEMModuleSameSideHasPulse;zdcEMModuleSameSideHasPulse_negative_amp', title=';;Events',
                            cutmask='RPDChannelNegativeAmp',
                            xlabels=['EM pulse bit false','EM pulse bit true'],
                            xbins=2,xmin=-0.5,xmax=1.5)

    rpdChannelMonToolArr.defineHistogram('RPDChannelMaxADC;RPDChannelMaxADC_negative_amp', title=';Max ADC [ADC Counts];Events',
                            cutmask='RPDChannelNegativeAmp',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=4096.0)
    rpdChannelMonToolArr.defineHistogram('RPDChannelMaxADC;RPDChannelMaxADC_negative_amp_weighted', title=';Max ADC [ADC Counts];Events',
                            cutmask='RPDChannelNegativeAmp', weight='absRPDChannelAmplitude',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=4096.0)

    rpdChannelMonToolArr.defineHistogram('zdcEMModuleEnergySameSide;zdcEMModuleEnergySameSide_negative_amp', title=';E EM module AorC [GeV];Events',
                            cutmask='RPDChannelNegativeAmp',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=module_calib_amp_xmax / 2.)
    rpdChannelMonToolArr.defineHistogram('zdcEMModuleEnergySameSide;zdcEMModuleEnergySameSide_negative_amp_weighted', title=';E EM module AorC [GeV];Events',
                            cutmask='RPDChannelNegativeAmp', weight='absRPDChannelAmplitude',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=module_calib_amp_xmax / 2.)

    rpdChannelMonToolArr.defineHistogram('zdcEMModuleEnergySameSide, RPDChannelPileupFitSlope;RPDChannelPileupFitSlope_vs_zdcEMModuleEnergySameSide_negative_amp', type='TH2F', title=';E EM module AorC [GeV];RPD Pileup Fitted Slope',
                            cutmask='RPDChannelNegativePileup',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=module_calib_amp_xmax / 2., # divide by 2 to make a more zoomed-in plot (not full range)
                            ybins=n_energy_bins_default,ymin=-100.,ymax=10.) # try a value for now
    rpdChannelMonToolArr.defineHistogram('zdcEMModuleEnergySameSide, RPDChannelPileupFitSlope;RPDChannelPileupFitSlope_vs_zdcEMModuleEnergySameSide_negative_amp_weighted', type='TH2F', title=';E EM module AorC [GeV];RPD Pileup Fitted Slope',
                            cutmask='RPDChannelNegativePileup', weight='absRPDChannelAmplitude',
                            xbins=n_energy_bins_default,xmin=0.0,xmax=module_calib_amp_xmax / 2., # divide by 2 to make a more zoomed-in plot (not full range)
                            ybins=n_energy_bins_default,ymin=-100.,ymax=10.) # try a value for now

    # Study of effect of minimum EM module energy cut on RPD channel amplitude
    rpdChannelMonToolArr.defineHistogram('RPDChannelAmplitudeCalib;RPDChannelAmplitudeCalib_EMbelow0', title=';RPD Channel Calibrated Amplitude;Events',
                            cutmask='zdcEMModuleEnergySameSideBelow0',
                            xbins=n_rpd_amp_bins_half_range,xmin=rpd_channel_amp_min,xmax=module_amp_xmax / 2.) # NOT energy calibration - calibration factor is 1 for now
    rpdChannelMonToolArr.defineHistogram('RPDChannelAmplitudeCalib;RPDChannelAmplitudeCalib_EMbelow70', title=';RPD Channel Calibrated Amplitude;Events',
                            cutmask='zdcEMModuleEnergySameSideBelow70',
                            xbins=n_rpd_amp_bins_half_range,xmin=rpd_channel_amp_min,xmax=module_amp_xmax / 2.) # NOT energy calibration - calibration factor is 1 for now


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
    ConfigFlags.Output.HISTFileName = 'ZdcMonitorOutput_HI2023_462494.root'
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

    cfg.run() #use cfg.run(20) to only run on first 20 events
