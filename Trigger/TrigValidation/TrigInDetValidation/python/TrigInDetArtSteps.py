#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''
Definitions of additional validation steps in Trigger ART tests relevant only for TrigInDetValidation
The main common check steps are defined in the TrigValSteering.CheckSteps module.
'''

import os
import subprocess
import json

from TrigValTools.TrigValSteering.ExecStep import ExecStep
from TrigValTools.TrigValSteering.Step import Step
from TrigValTools.TrigValSteering.CheckSteps import RefComparisonStep
from TrigValTools.TrigValSteering.Common import find_file
from AthenaCommon.Utils.unixtools import FindFile

##################################################
# Exec (athena) steps for Reco_tf
##################################################

class TrigInDetReco(ExecStep):

    def __init__(self, name='TrigInDetReco', postinclude_file='', preinclude_file='' ):
        ExecStep.__init__(self, name)
##      super(TrigInDetReco, self).__init__(name)
        self.type = 'Reco_tf'
        self.max_events=-1
        self.required = True
        self.threads = 1 # TODO: change to 4
        self.concurrent_events = 1 # TODO: change to 4
        self.perfmon = False
        self.timeout = 18*3600
        self.slices = []
        self.preexec_trig = ''
        self.postinclude_trig = postinclude_file
        self.preinclude_trig  = preinclude_file
        self.release = 'current'
        self.preexec_reco =  ';'.join([
            'flags.Reco.EnableEgamma=True',
            'flags.Reco.EnableCombinedMuon=True',
            'flags.Reco.EnableJet=False',
            'flags.Reco.EnableMet=False',
            'flags.Reco.EnableBTagging=False',
            'flags.Reco.EnablePFlow=False',
            'flags.Reco.EnableTau=False',
            'flags.Reco.EnablePostProcessing=False',
        ])

        self.preexec_all = ';'.join([
            'ConfigFlags.Trigger.AODEDMSet=\'AODFULL\'',
        ])
        self.postexec_trig = "from AthenaCommon.AppMgr import ServiceMgr; ServiceMgr.AthenaPoolCnvSvc.MaxFileSizes=['tmp.RDO_TRIG=100000000000']"
        # no longer needed if we don't write ESDs?
        #self.postexec_reco = "from AthenaCommon.AppMgr import ServiceMgr; ServiceMgr.AthenaPoolCnvSvc.MaxFileSizes=['tmp.ESD=100000000000']"
        self.postexec_reco = ''
        self.args = '--outputAODFile=AOD.pool.root --steering "doRDO_TRIG"'
        self.args += ' --CA "default:True" "RDOtoRDOTrigger:False"'
       
        if ( self.postinclude_trig != '' ) : 
            print( "postinclude_trig: ", self.postinclude_trig )

        if ( self.preinclude_trig != '' ) : 
            print( "preinclude_trig:  ", self.preinclude_trig  )


    def configure(self, test):
        chains = '['
        flags = ''
        for i in self.slices:
            if (i=='L2muonLRT') :
                chains += "'HLT_mu20_LRT_idperf_L1MU14FCH',"
                chains += "'HLT_mu6_LRT_idperf_L1MU5VF',"
                chains += "'HLT_mu6_idperf_L1MU5VF',"
                chains += "'HLT_mu24_idperf_L1MU14FCH',"
                flags += 'doMuonSlice=True;'
            if (i=='FSLRT') :
                chains += "'HLT_fslrt0_L1J100',"
                flags  += 'doUnconventionalTrackingSlice=True;'
            if (i=='muon') :
                chains += "'HLT_mu6_idperf_L1MU5VF',"
                chains += "'HLT_mu24_idperf_L1MU14FCH',"
                chains += "'HLT_mu26_ivarperf_L1MU14FCH',"
                flags += 'doMuonSlice=True;'
            if (i=='muon-tnp') :
                chains += "'HLT_mu14_mu14_idtp_idZmumu_L12MU8F'," 
                chains += "'HLT_mu14_mu14_idperf_50invmAB130_L12MU8F',"
                flags += 'doMuonSlice=True;'
            if (i=='L2electronLRT') :
                chains += "'HLT_e20_idperf_loose_lrtloose_L1eEM18L',"
                chains += "'HLT_e30_idperf_loose_lrtloose_L1eEM26M',"
                chains += "'HLT_e26_lhtight_ivarloose_e5_idperf_loose_lrtloose_probe_L1eEM26M',"
                chains += "'HLT_e5_idperf_loose_lrtloose_probe_g25_medium_L1eEM24L',"
                flags += 'doEgammaSlice=True;'
            if (i=='electron') :
                # chains +=  "'HLT_e5_etcut_L1EM3',"  ## need an idperf chain once one is in the menu
                # chains +=  "'HLT_e17_lhvloose_nod0_L1EM15VH',"
                # chains += "'HLT_e26_idperf_gsf_tight_L1EM22VHI',"
                chains += "'HLT_e26_idperf_loose_L1eEM26M',"
                chains += "'HLT_e5_idperf_tight_L1EM3',"
                flags += 'doEgammaSlice=True;'
            if (i=='electron-tnp') :
                chains += "'HLT_e26_lhtight_e14_idperf_tight_probe_50invmAB130_L1eEM26M',"
                chains += "'HLT_e26_lhtight_e14_idperf_tight_nogsf_probe_50invmAB130_L1eEM26M',"
                flags += 'doEgammaSlice=True;'
            if (i=='tau') :
                chains +=  "'HLT_tau25_idperf_tracktwoMVA_L1TAU12IM',"
                chains +=  "'HLT_mu24_ivarmedium_tau25_idperf_tracktwoMVA_probe_03dRAB_L1MU14FCH',"
                flags += 'doTauSlice=True;'
            if (i=='tauLRT') :
                chains +=  "'HLT_tau25_idperf_tracktwoMVA_L1TAU12IM',"
                chains +=  "'HLT_tau25_idperf_tracktwoLLP_L1TAU12IM',"
                chains +=  "'HLT_tau25_idperf_trackLRT_L1TAU12IM',"
                flags += 'doTauSlice=True;'
            if (i=='bjet') :
#               chains += "'HLT_j80_pf_ftf_preselj20b95_L1J20',"
                chains += "'HLT_j20_roiftf_preselj20_L1RD0_FILLED',"
                chains += "'HLT_j45_pf_ftf_preselj20_L1jJ40',"
#               chains += "'HLT_j45_subjesgscIS_ftf_boffperf_split_L1J20',"
                chains += "'HLT_j45_0eta290_020jvt_boffperf_pf_ftf_L1J20',"
#               chains += "'HLT_j75_0eta290_020jvt_bdl1r60_3j75_pf_ftf_preselj50b85XX3j50_L14J20',"
                flags  += 'doBjetSlice=True;'
            if ( i=='fsjet' or i=='fs' or i=='jet' ) :
                chains += "'HLT_j45_pf_ftf_preselj20_L1J15',"
                flags  += 'doJetSlice=True;'
            if (i=='beamspot') :
                chains += "'HLT_beamspot_allTE_trkfast_BeamSpotPEB_L1J15','HLT_beamspot_trkFS_trkfast_BeamSpotPEB_L1J15',"
                flags  += 'doBeamspotSlice=True;'
            if (i=='minbias') :
                chains += "'HLT_mb_sptrk_L1RD0_FILLED',"
                flags  += "doMinBiasSlice=True;setMenu='PhysicsP1_pp_lowMu_run3_v1';"
            if (i=='cosmic') :
                chains += "'HLT_mu4_cosmic_L1MU3V_EMPTY'"
                flags  += "doMuonSlice=True;doCosmics=True;setMenu='Cosmic_run3_v1';"
            if (i=='bphys') :
                chains += "'HLT_mu6_idperf_L1MU5VF',"
                chains += "'HLT_2mu4_bBmumux_BsmumuPhi_L12MU3V',"
                chains += "'HLT_mu11_mu6_bBmumux_Bidperf_L1MU8VF_2MU5VF',"
                flags += 'doMuonSlice=True;doBphysicsSlice=True;'
        if ( flags=='' ) : 
            print( "ERROR: no chains configured" )

        chains += ']'
        self.preexec_trig = 'doEmptyMenu=True;'+flags+'selectChains='+chains

        AVERSION = ""
        ### # temporary hack until we get to the bottom of why the tests are really failing
        ### self.release = 'latest'
        if (self.release != 'current'):
            # get the current atlas base release, and the previous base release
            import os
            DVERSION=os.getenv('Athena_VERSION')
            if (self.release == 'latest'):
                if ( DVERSION is None ) :
                    AVERSION = "22.0.20"
                else:
                    AVERSION=str(subprocess.Popen(["getrelease.sh",DVERSION],stdout=subprocess.PIPE).communicate()[0],'utf-8')
                    if AVERSION == "":
                        print( "cannot get last stable release - will use current release" )
            else:
                AVERSION = self.release

        # would use AVERSION is not None, but the return from a shell function with no printout 
        # gets set as an empty string rather than None        
        if AVERSION != "":
            self.args += ' --asetup "RAWtoALL:Athena,'+AVERSION+'" '
            print( "remapping athena base release version for offline Reco steps: ", DVERSION, " -> ", AVERSION )
        else:
            print( "Using current release for offline Reco steps  " )

        if self.preexec_trig != '' or self.preexec_reco != '' or self.preexec_all != '':
            self.args += ' --preExec'
            if self.preexec_trig != '':
                self.args += ' "RDOtoRDOTrigger:{:s};"'.format(self.preexec_trig)
            if self.preexec_reco != '':
                self.args += ' "RAWtoALL:{:s};"'.format(self.preexec_reco)
            if self.preexec_all != '':
                self.args += ' "all:{:s};"'.format(self.preexec_all)
        if self.postexec_trig != '' or self.postexec_reco != '':
            self.args += ' --postExec'
            if self.postexec_trig != '':
                self.args += ' "RDOtoRDOTrigger:{:s};"'.format(self.postexec_trig)
            if self.postexec_reco != '':
                self.args += ' "RAWtoALL:{:s};"'.format(self.postexec_reco)
        if (self.postinclude_trig != ''):
            self.args += ' --postInclude "{:s}"'.format(self.postinclude_trig)
        if (self.preinclude_trig != ''):
            self.args += ' --preInclude "{:s}"'.format(self.preinclude_trig)
        super(TrigInDetReco, self).configure(test)


##################################################
# Additional exec (athena) steps - AOD to TrkNtuple
##################################################

class TrigInDetAna(ExecStep):
    def __init__(self, name='TrigInDetAna', extraArgs=None):
        ExecStep.__init__(self, name )
        self.type = 'athena'
        self.job_options = 'TrigInDetValidation/TrigInDetValidation_AODtoTrkNtuple.py'
        self.max_events=-1
        self.required = True
        self.depends_on_previous = True
        #self.input = 'AOD.pool.root'
        self.input = ''
        self.perfmon=False
        self.imf=False
        if extraArgs is not None:
            self.args = extraArgs


##################################################
# Additional exec (athena) steps - RDO to CostMonitoring
##################################################

class TrigCostStep(Step):
    def __init__(self, name='TrigCostStep'):
        super(TrigCostStep, self).__init__(name)
        self.required = True
        self.depends_on_previous = True
        self.input = 'tmp.RDO_TRIG'
        self.args = '  --monitorChainAlgorithm --MCCrossSection=0.5 Input.Files=\'["tmp.RDO_TRIG"]\' '
        self.executable = 'RunTrigCostAnalysis.py'


##################################################
# Exec (athenaHLT) step running runHLT_Standalone.py on data
##################################################
class TrigInDetRecoData(ExecStep):
    def __init__(self, name='TrigInDetRecoData'):
#        super(TrigInDetRecoData, self).__init__(name)
        ExecStep.__init__(self, name)
        self.type = 'athenaHLT'
        self.job_options = 'TriggerJobOpts/runHLT_standalone.py'
        self.max_events=-1
        self.required = True
        self.threads = 1 # TODO: change to 4
        self.concurrent_events = 1 # TODO: change to 4
        self.perfmon = False
        self.timeout = 18*3600
        self.input = ''
        self.perfmon=False
        self.imf=False
        self.args = '-c "setMenu=\'Cosmic_run3_v1\';doCosmics=True;doL1Sim=True;rewriteLVL1=True;"'
        self.args += ' -o output'


##################################################
# Additional exec (athena) steps - extract Physics_Main when running on data
##################################################

class TrigBSExtr(ExecStep):
    def __init__(self, name='TrigBSExtr'):
        super(TrigBSExtr, self).__init__(name)
        self.type = 'other'
        self.executable = 'trigbs_extractStream.py'
        self.input = ''
        self.args = '-s Main ' + find_file('*_HLTMPPy_output.*.data')


##################################################
# Additional exec (athena) steps - Tier0 Reco (BS->AOD)
##################################################

class TrigTZReco(ExecStep):
    def __init__(self, name='TrigTZReco'):
        super(TrigTZReco, self).__init__(name)
        self.type = 'Reco_tf'
        tzrecoPreExec = ' '.join([
            "flags.Trigger.triggerMenuSetup=\'Cosmic_run3_v1\';",
            "flags.Trigger.AODEDMSet=\'AODFULL\';",
            ])
        self.threads = 1
        self.concurrent_events = 1
        self.input = ''
        self.explicit_input = True
        self.max_events = -1
        self.args = '--inputBSFile=' + find_file('*.physics_Main*._athenaHLT*.data')  # output of the previous step
        self.args += ' --outputAODFile=AOD.pool.root'
        self.args += ' --conditionsTag=\'CONDBR2-BLKPA-2022-08\' --geometryVersion=\'ATLAS-R3S-2021-03-00-00\''
        self.args += ' --preExec="{:s}"'.format(tzrecoPreExec)
        self.args += ' --CA'


##################################################
# Additional post-processing steps
##################################################

class TrigInDetRdictStep(Step):
    '''
    Execute TIDArdict for TrkNtuple files.
    '''
    def __init__(self, name='TrigInDetdict', args=None, testbin='Test_bin.dat', config=False ):
        super(TrigInDetRdictStep, self).__init__(name)
        self.args=args + "  -b " + testbin + " "
        self.auto_report_result = True
        self.required = True
        self.executable = 'TIDArdict'
        self.timeout = 10*60
        self.config = config

    def configure(self, test):
        if not self.config :
            os.system( 'get_files -data TIDAbeam.dat &> /dev/null' )
            os.system( 'get_files -data Test_bin.dat &> /dev/null' )
            os.system( 'get_files -data Test_bin_larged0.dat &> /dev/null' )
            os.system( 'get_files -data Test_bin_lrt.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-larged0.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-larged0-el.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-lrt.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-fslrt.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-minbias.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata_cuts.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-offline.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-offline-rzMatcher.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-offline-vtxtrack.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-offline-larged0.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-offline-larged0-el.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-offline-lrt.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-offline-fslrt.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-offline-vtx.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-minbias-offline.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-run3-offline-cosmic.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata_cuts-offline.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-chains-run3.dat &> /dev/null' )
            os.system( 'get_files -data TIDAdata-chains-run3-lrt.dat &> /dev/null' )
        super(TrigInDetRdictStep, self).configure(test)


def json_chains( slice ) : 
    json_file     = 'TrigInDetValidation/comparitor.json'
    json_fullpath = FindFile(json_file, os.environ['DATAPATH'].split(os.pathsep), os.R_OK)

    if not json_fullpath:
        print('Failed to determine full path for input JSON %s', json_file)
        return None
        
    with open(json_fullpath) as f:
        try:
            data = json.load(f)
        except json.decoder.JSONDecodeError as e:
            print(f"Failed to load json file {json_fullpath}")
            raise e

    chainmap = data[slice]

    return chainmap['chains']



class TrigInDetCompStep(RefComparisonStep):
    '''
    Execute TIDAcomparitor for data.root files.
    '''
    def __init__( self, name='TrigInDetComp', slice=None, args=None, file=None ):
        super(TrigInDetCompStep, self).__init__(name)

        self.input_file = file    
        self.slice = slice 
        self.auto_report_result = True
        self.required   = True
        self.args = args
        self.executable = 'TIDAcomparitor'
        os.system( 'get_files -data TIDAhisto-panel.dat &> /dev/null' )
        os.system( 'get_files -data TIDAhisto-panel-vtx.dat &> /dev/null' )
        os.system( 'get_files -data TIDAhistos-vtx.dat &> /dev/null' )
        os.system( 'get_files -data TIDAhisto-panel-TnP.dat &> /dev/null' )
        os.system( 'get_files -data TIDAhisto-tier0.dat &> /dev/null' )
        os.system( 'get_files -data TIDAhisto-tier0-vtx.dat &> /dev/null' )
        os.system( 'get_files -data TIDAhisto-tier0-TnP.dat &> /dev/null' )    

    def configure(self, test):
        RefComparisonStep.configure(self, test)
        if self.reference is None :
            self.args  = self.args + " " + self.input_file + "  " + self.input_file + " --noref --oldrms "
        else:
            self.args  = self.args + " " + self.input_file + "  " + self.reference + " --oldrms "
        self.chains = json_chains( self.slice )
        self.args += " " + self.chains
        print( "\033[0;32mTIDAcomparitor "+self.args+" \033[0m" )
        Step.configure(self, test)




class TrigInDetCpuCostStep(RefComparisonStep):
    '''
    Execute TIDAcpucost for data.root files.
    '''
    def __init__( self, name='TrigInDetCpuCost', outdir=None, infile=None, extra=None ):
        super(TrigInDetCpuCostStep, self).__init__(name)

        self.input_file = infile
        self.output_dir = outdir
        self.auto_report_result = True
        self.required   = True
        self.extra = extra
        self.executable = 'TIDAcpucost'
    

    def configure(self, test):
        RefComparisonStep.configure(self, test)
        if self.reference is None :
            self.args  = self.input_file + " -o " + self.output_dir + " " + self.extra + " --noref  "
        else:
            self.args  = self.input_file + " " + self.reference + " -o " + self.output_dir + " " + self.extra
        Step.configure(self, test)
