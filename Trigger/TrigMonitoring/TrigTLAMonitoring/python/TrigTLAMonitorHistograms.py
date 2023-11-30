#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''@file TrigTLAMonitorHistograms.py
@author S. Franchellucci
@date 2023-11-28
@brief Definitions of groups of histograms to monitor
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
            {'name'  :f'{pprefix}GN120230331_pu',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} GN120230331_pu',
            'xmin'  :-0.2,
            'xmax'  :1.2,
            'xbins' :70},
            {'name'  :f'{pprefix}GN120230331_pb',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} GN120230331_pb',
            'xmin'  :-0.2,
            'xmax'  :1.2,
            'xbins' :70},
            {'name'  :f'{pprefix}GN120230331_pc',
            'ylabel':f'{plabel}s',
            'xlabel':f'{plabel} GN120230331_pc',
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
         'xbins' :300},
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

def histdefs_tracks(prefix='trk',plabel='track'):
    """
    Return a list of histogram definitions for tracks features.

    @param pprefix Prefix of histogram names
    @param plabel Human-readable name of the particle to use in labeling axes
    """
    histograms = histdefs_particle(prefix,plabel)
    # Modifify the ntracks histogram -> first on the list
    histograms[0]['xmax']=199.5
    histograms[0]['xbins']=200
    
    histograms += [
        {'name' :prefix+'d0',
        'ylabel':'entries',
        'xlabel':f'{plabel} d0',
        'xmin'  :-5.,
        'xmax'  :5.,
        'xbins' :400},
        {'name' :prefix+'z0',
        'ylabel':'entries',
        'xlabel':plabel+' z0',
        'xmin'  :-50.,
        'xmax'  :50.,
        'xbins' :400},
        {'name' :prefix+'qOverP',
        'ylabel':'entries',
        'xlabel':plabel+' qOverP',
        'xmin'  :-0.003,
        'xmax'  :0.003,
        'xbins' :150},
        {'name' :prefix+'chiSquared',
        'ylabel':'entries',
        'xlabel':plabel+' chiSquared',
        'xmin'  :0.,
        'xmax'  :150.,
        'xbins' :300},
        {'name' :prefix+'numberDoF',
        'ylabel':'entries',
        'xlabel':plabel+' numberDoF',
        'xmin'  :-0.5,
        'xmax'  :50.5,
        'xbins' :51},
        {'name' :f'{prefix}btagIp-d0',
        'ylabel':'entries',
        'xlabel':'trk d0',
        'xmin'  :-5.,
        'xmax'  :5.,
        'xbins' :400},
        {'name' :f'{prefix}btagIp-d0Uncertainty',
        'ylabel':'entries',
        'xlabel':'trk d0 Uncertainty',
        'xmin'  :0.,
        'xmax'  :10.,
        'xbins' :200},
        {'name' :f'{prefix}btagIp-z0SinTheta',
        'ylabel':'entries',
        'xlabel':'trk z0SinTheta',
        'xmin'  :-100.,
        'xmax'  :100.,
        'xbins' :500},
        {'name' :f'{prefix}btagIp-z0SinThetaUncertainty',
        'ylabel':'entries',
        'xlabel':'trk z0SinTheta Uncertainty',
        'xmin'  :0.,
        'xmax'  :10.,
        'xbins' :200},
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
