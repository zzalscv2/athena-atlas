# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Pythonized version of MadGraph steering executables
#    written by Zach Marshall <zach.marshall@cern.ch>
#    updates for aMC@NLO by Josh McFayden <mcfayden@cern.ch>
#    updates to LHE handling and SUSY functionality by Emma Kuwertz <ekuwertz@cern.ch>
#  Attempts to remove path-dependence of MadGraph

import os,time,subprocess,glob,re,sys
from AthenaCommon import Logging
mglog = Logging.logging.getLogger('MadGraphUtils')

# Name of python executable
python='python'
# Magic name of gridpack directory
MADGRAPH_GRIDPACK_LOCATION='madevent'
# Name for the run (since we only have 1, just needs consistency)
MADGRAPH_RUN_NAME='run_01'
# For error handling
MADGRAPH_CATCH_ERRORS=True
# PDF setting (global setting)
MADGRAPH_PDFSETTING=None
MADGRAPH_COMMAND_STACK = []

patched_shutil_loc='/cvmfs/atlas.cern.ch/repo/sw/Generators/madgraph/models/latest/shutil_patch'
if 'PYTHONPATH' in os.environ and patched_shutil_loc not in os.environ['PYTHONPATH']:
    # add shutil_patch in first place so that the patched version of shutil.py is picked up by MG instead of original version
    # the patched version does not throw the errors that has made running MG and this code impossible on some file systems
    os.environ['PYTHONPATH'] = patched_shutil_loc+':'+os.environ['PYTHONPATH']
    MADGRAPH_COMMAND_STACK += ['export PYTHONPATH='+patched_shutil_loc+':${PYTHONPATH}']
# we need to remove shutil from modules before we can use our version
if 'shutil' in sys.modules:
    sys.modules.pop('shutil')
# make sure this python instance uses fixed shutil
sys.path.insert(0,patched_shutil_loc)
import shutil

from MadGraphControl.MadGraphUtilsHelpers import checkSettingExists,checkSetting,checkSettingIsTrue,getDictFromCard,get_runArgs_info,get_physics_short,is_version_or_newer
from MadGraphControl.MadGraphParamHelpers import do_PMG_updates,check_PMG_updates

def stack_subprocess(command,**kwargs):
    global MADGRAPH_COMMAND_STACK
    MADGRAPH_COMMAND_STACK += [' '.join(command)]
    return subprocess.Popen(command,**kwargs)


def setup_path_protection():
    # Addition for models directory
    global MADGRAPH_COMMAND_STACK
    if 'PYTHONPATH' in os.environ:
        if not any( [('Generators/madgraph/models' in x and 'shutil_patch' not in x) for x in os.environ['PYTHONPATH'].split(':') ]):
            os.environ['PYTHONPATH'] += ':/cvmfs/atlas.cern.ch/repo/sw/Generators/madgraph/models/latest'
            MADGRAPH_COMMAND_STACK += ['export PYTHONPATH=${PYTHONPATH}:/cvmfs/atlas.cern.ch/repo/sw/Generators/madgraph/models/latest']
    # Make sure that gfortran doesn't write to somewhere it shouldn't
    if 'GFORTRAN_TMPDIR' in os.environ:
        return
    if 'TMPDIR' in os.environ:
        os.environ['GFORTRAN_TMPDIR']=os.environ['TMPDIR']
        MADGRAPH_COMMAND_STACK += ['export GFORTRAN_TMPDIR=${TMPDIR}']
        return
    if 'TMP' in os.environ:
        os.environ['GFORTRAN_TMPDIR']=os.environ['TMP']
        MADGRAPH_COMMAND_STACK += ['export GFORTRAN_TMPDIR=${TMP}']
        return


def config_only_check():
    try:
        from __main__ import opts
        if opts.config_only:
            mglog.info('Athena running on config only mode: not executing MadGraph')
            return True
    except ImportError:
        pass
    return False


def generate_prep(process_dir):
    global MADGRAPH_COMMAND_STACK
    if not os.access('Cards_bkup',os.R_OK):
        shutil.copytree(process_dir+'/Cards','Cards_bkup')
        shutil.copyfile(process_dir+'/Source/make_opts','Cards_bkup/make_opts_bkup')
        MADGRAPH_COMMAND_STACK += ['# In case this fails, Cards_bkup should be in your original run directory']
        MADGRAPH_COMMAND_STACK += ['# And ${MGaMC_PROCESS_DIR} can be replaced with whatever process directory exists in your stand-alone test']
        MADGRAPH_COMMAND_STACK += ['cp '+os.getcwd()+'/Cards_bkup/*dat ${MGaMC_PROCESS_DIR}/Cards/']
        MADGRAPH_COMMAND_STACK += ['cp '+os.getcwd()+'/Cards_bkup/make_opts_bkup ${MGaMC_PROCESS_DIR}/Source/make_opts']
    else:
        mglog.warning('Found Cards_bkup directory existing. Suggests you are either running generation twice (a little funny) or are not using a clean directory.')
        bkup_v = 1
        while os.access('Cards_bkup_'+str(bkup_v),os.R_OK) and bkup_v<100:
            bkup_v += 1
        if bkup_v<100:
            shutil.copytree(process_dir+'/Cards','Cards_bkup_'+str(bkup_v))
            shutil.copyfile(process_dir+'/Source/make_opts','Cards_bkup_'+str(bkup_v)+'/make_opts_bkup')
            MADGRAPH_COMMAND_STACK += ['# In case this fails, Cards_bkup should be in your original run directory']
            MADGRAPH_COMMAND_STACK += ['# And ${MGaMC_PROCESS_DIR} can be replaced with whatever process directory exists in your stand-alone test']
            MADGRAPH_COMMAND_STACK += ['cp '+os.getcwd()+'/Cards_bkup_'+str(bkup_v)+'/*dat ${MGaMC_PROCESS_DIR}/Cards/']
            MADGRAPH_COMMAND_STACK += ['cp '+os.getcwd()+'/Cards_bkup_'+str(bkup_v)+'/make_opts_bkup ${MGaMC_PROCESS_DIR}/Source/make_opts']
        else:
            mglog.warning('Way too many Cards_bkup* directories found. Giving up -- standalone script may not work.')


def error_check(errors_a, return_code):
    global MADGRAPH_CATCH_ERRORS
    if not MADGRAPH_CATCH_ERRORS:
        return
    unmasked_error = False
    my_debug_file = None
    bad_variables = []
    # Make sure we are getting a string and not a byte string (python3 ftw)
    errors = errors_a
    if type(errors)==bytes:
        errors = errors.decode('utf-8')
    if len(errors):
        mglog.info('Some errors detected by MadGraphControl - checking for serious errors')
        for err in errors.split('\n'):
            if len(err.strip())==0:
                continue
            # Errors to do with I/O... not clear on their origin yet
            if 'Inappropriate ioctl for device' in err:
                mglog.info(err)
                continue
            if 'stty: standard input: Invalid argument' in err:
                mglog.info(err)
                continue
            # Errors for PDF sets that should be fixed in MG5_aMC 2.7
            if 'PDF already installed' in err:
                mglog.info(err)
                continue
            if 'Read-only file system' in err:
                mglog.info(err)
                continue
            if 'HTML' in err:
                # https://bugs.launchpad.net/mg5amcnlo/+bug/1870217
                mglog.info(err)
                continue
            if 'impossible to set default multiparticles' in err:
                # https://answers.launchpad.net/mg5amcnlo/+question/690004
                mglog.info(err)
                continue
            if 'More information is found in' in err:
                my_debug_file = err.split("'")[1]
            if err.startswith('tar'):
                mglog.info(err)
                continue
            if 'python2 support will be removed' in err:
                mglog.info(err)
                continue
            # silly ghostscript issue in 21.6.46 nightly
            if 'required by /lib64/libfontconfig.so' in err or\
               'required by /lib64/libgs.so' in err:
                mglog.info(err)
                continue
            if 'Error: Symbol' in err and 'has no IMPLICIT type' in err:
                bad_variables += [ err.split('Symbol ')[1].split(' at ')[0] ]
            # error output from tqdm (progress bar)
            if 'it/s' in err:
                mglog.info(err)
                continue
            mglog.error(err)
            unmasked_error = True
    # This is a bit clunky, but needed because we could be several places when we get here
    if my_debug_file is None:
        debug_files = glob.glob('*debug.log')+glob.glob('*/*debug.log')
        for debug_file in debug_files:
            # This protects against somebody piping their output to my_debug.log and it being caught here
            has_subproc = os.access(os.path.join(os.path.dirname(debug_file),'SubProcesses'),os.R_OK)
            if has_subproc:
                my_debug_file = debug_file
                break

    if my_debug_file is not None:
        if not unmasked_error:
            mglog.warning('Found a debug file at '+my_debug_file+' but no apparent error. Will terminate.')
        mglog.error('MadGraph5_aMC@NLO appears to have crashed. Debug file output follows.')
        with open(my_debug_file,'r') as error_output:
            for l in error_output:
                mglog.error(l.replace('\n',''))
        mglog.error('End of debug file output')

    if bad_variables:
        mglog.warning('Appeared to detect variables in your run card that MadGraph did not understand:')
        mglog.warning('  Check your run card / JO settings for %s',bad_variables)

    # Check the return code
    if return_code!=0:
        mglog.error(f'Detected a bad return code: {return_code}')
        unmasked_error = True

    # Now raise an error if we were in either of the error states
    if unmasked_error or my_debug_file is not None:
        write_test_script()
        raise RuntimeError('Error detected in MadGraphControl process')
    return


# Write a short test script for standalone debugging
def write_test_script():
    mglog.info('Beta feature: Stand-alone debugging script')
    mglog.info('This is an attempt to provide you commands that you can use')
    mglog.info('to reproduce the error locally\n\n')
    global MADGRAPH_COMMAND_STACK
    mglog.info('# Script start; trim off columns left of the "#"')
    # Beta feature: offline stand-alone reproduction script
    with open('standalone_script.sh','w') as standalone_script:
        for command in MADGRAPH_COMMAND_STACK:
            for line in command.split('\n'):
                mglog.info(line)
                standalone_script.write(line+'\n')
    mglog.info('# Script end')
    mglog.info('Script also written to %s/standalone_script.sh',os.getcwd())


def new_process(process='generate p p > t t~\noutput -f', plugin=None, keepJpegs=False, usePMGSettings=False):
    """ Generate a new process in madgraph.
    Pass a process string.
    Optionally request JPEGs to be kept and request for PMG settings to be used in the param card
    Return the name of the process directory.
    """
    if config_only_check():
        # Give some directories to work on
        try:
            os.makedirs('dummy_proc/Cards')
        except os.error:
            pass
        return 'dummy_proc'

    # Don't run if generating events from gridpack
    if is_gen_from_gridpack():
        return MADGRAPH_GRIDPACK_LOCATION

    # Actually just sent the process card contents - let's make a card
    card_loc='proc_card_mg5.dat'
    mglog.info('Writing process card to '+card_loc)
    a_card = open( card_loc , 'w' )
    for l in process.split('\n'):
        if 'output' not in l:
            a_card.write(l+'\n')
        elif '-nojpeg' in l or keepJpegs:
            a_card.write(l+'\n')
        elif '#' in l:
            a_card.write(l.split('#')[0]+' -nojpeg #'+l.split('#')[1]+'\n')
        else:
            a_card.write(l+' -nojpeg\n')
    a_card.close()

    madpath=os.environ['MADPATH']
    # Just in case
    setup_path_protection()

    # Check if we have a special output directory
    process_dir = ''
    for l in process.split('\n'):
        # Look for an output line
        if 'output' not in l.split('#')[0].split():
            continue
        # Check how many things before the options start
        tmplist = l.split('#')[0].split(' -')[0]
        # if two things, second is the directory
        if len(tmplist.split())==2:
            process_dir = tmplist.split()[1]
        # if three things, third is the directory (second is the format)
        elif len(tmplist.split())==3:
            process_dir = tmplist.split()[2]
        # See if we got a directory
        if ''!=process_dir:
            mglog.info('Saw that you asked for a special output directory: '+str(process_dir))
        break

    mglog.info('Started process generation at '+str(time.asctime()))

    plugin_cmd = '--mode='+plugin if plugin is not None else ''

    # Note special handling here to explicitly print the process
    global MADGRAPH_COMMAND_STACK
    MADGRAPH_COMMAND_STACK += ['# All jobs should start in a clean directory']
    MADGRAPH_COMMAND_STACK += ['mkdir standalone_test; cd standalone_test']
    MADGRAPH_COMMAND_STACK += [' '.join([python,madpath+'/bin/mg5_aMC '+plugin_cmd+' << EOF\n'+process+'\nEOF\n'])]
    global MADGRAPH_CATCH_ERRORS
    generate = subprocess.Popen([python,madpath+'/bin/mg5_aMC',plugin_cmd,card_loc],stdin=subprocess.PIPE,stderr=subprocess.PIPE if MADGRAPH_CATCH_ERRORS else None)
    (out,err) = generate.communicate()
    error_check(err,generate.returncode)

    mglog.info('Finished process generation at '+str(time.asctime()))

    # at this point process_dir is for sure defined - it's equal to '' in the worst case
    if process_dir == '': # no user-defined value, need to find the directory created by MadGraph5
        for adir in sorted(glob.glob( os.getcwd()+'/*PROC*' ),reverse=True):
            if os.access('%s/SubProcesses/subproc.mg'%adir,os.R_OK):
                if process_dir=='':
                    process_dir=adir
                else:
                    mglog.warning('Additional possible process directory, '+adir+' found. Had '+process_dir)
                    mglog.warning('Likely this is because you did not run from a clean directory, and this may cause errors later.')
    else: # user-defined directory
        if not os.access('%s/SubProcesses/subproc.mg'%process_dir,os.R_OK):
            raise RuntimeError('No diagrams for this process in user-define dir='+str(process_dir))
    if process_dir=='':
        raise RuntimeError('No diagrams for this process from list: '+str(sorted(glob.glob(os.getcwd()+'/*PROC*'),reverse=True)))

    # Special catch related to path setting and using afs
    needed_options = ['ninja','collier','fastjet','lhapdf','syscalc_path']
    in_config = open(os.environ['MADPATH']+'/input/mg5_configuration.txt','r')
    option_paths = {}
    for l in in_config.readlines():
        for o in needed_options:
            if o+' =' in l.split('#')[0] and 'MCGenerators' in l.split('#')[0]:
                old_path = l.split('#')[0].split('=')[1].strip().split('MCGenerators')[1]
                old_path = old_path[ old_path.find('/') : ]
                if o =='lhapdf' and 'LHAPATH' in os.environ:
                    # Patch for LHAPDF version
                    version = os.environ['LHAPATH'].split('lhapdf/')[1].split('/')[0]
                    old_version = old_path.split('lhapdf/')[1].split('/')[0]
                    old_path = old_path.replace(old_version,version)
                if o=='ninja':
                    # Patch for stupid naming problem
                    old_path.replace('gosam_contrib','gosam-contrib')
                option_paths[o] = os.environ['MADPATH'].split('madgraph5amc')[0]+old_path
            # Check to see if the option has been commented out
            if o+' =' in l and o+' =' not in l.split('#')[0]:
                mglog.info('Option '+o+' appears commented out in the config file')

    in_config.close()
    for o in needed_options:
        if o not in option_paths:
            mglog.warning('Path for option '+o+' not found in original config')

    mglog.info('Modifying config paths to avoid use of afs:')
    mglog.info(option_paths)

    # Set the paths appropriately
    modify_config_card(process_dir=process_dir,settings=option_paths,set_commented=False)
    # Done modifying paths

    # If requested, apply PMG default settings
    if usePMGSettings:
        do_PMG_updates(process_dir)

    # After 2.9.3, enforce the standard default sde_strategy, so that this won't randomly change on the user
    if is_version_or_newer([2,9,3]) and not is_NLO_run(process_dir=process_dir):
        mglog.info('Setting default sde_strategy to old default (1)')
        my_settings = {'sde_strategy':1}
        modify_run_card(process_dir=process_dir,settings=my_settings,skipBaseFragment=True)

    # Make sure we store the resultant directory
    MADGRAPH_COMMAND_STACK += ['export MGaMC_PROCESS_DIR='+os.path.basename(process_dir)]

    return process_dir


def get_default_runcard(process_dir=MADGRAPH_GRIDPACK_LOCATION):
    """ Copy the default runcard from one of several locations
    to a local file with name run_card.tmp.dat"""
    output_name = 'run_card.tmp.dat'
    if config_only_check():
        mglog.info('Athena running on config only mode: grabbing run card the old way, as there will be no proc dir')
        mglog.info('Fetching default LO run_card.dat')
        if os.access(os.environ['MADPATH']+'/Template/LO/Cards/run_card.dat',os.R_OK):
            shutil.copy(os.environ['MADPATH']+'/Template/LO/Cards/run_card.dat',output_name)
            return 'run_card.dat'
        elif os.access(os.environ['MADPATH']+'/Template/Cards/run_card.dat',os.R_OK):
            shutil.copy(os.environ['MADPATH']+'/Template/Cards/run_card.dat',output_name)
            return output_name
        else:
            raise RuntimeError('Cannot find default LO run_card.dat!')

    # Get the run card from the installation
    run_card=process_dir+'/Cards/run_card.dat'
    if os.access(run_card,os.R_OK):
        mglog.info('Copying default run_card.dat from '+str(run_card))
        shutil.copy(run_card,output_name)
        return output_name
    else:
        run_card=process_dir+'/Cards/run_card_default.dat'
        mglog.info('Fetching default run_card.dat from '+str(run_card))
        if os.access(run_card,os.R_OK):
            shutil.copy(run_card,output_name)
            return output_name
        else:
            raise RuntimeError('Cannot find default run_card.dat or run_card_default.dat! I was looking here: %s'%run_card)


def generate(process_dir='PROC_mssm_0', grid_pack=False, gridpack_compile=False, extlhapath=None, required_accuracy=0.01, runArgs=None, bias_module=None, requirePMGSettings=False):
    if config_only_check():
        return

    # Just in case
    setup_path_protection()

    # Set consistent mode and number of jobs
    mode = 0
    njobs = 1
    if 'ATHENA_PROC_NUMBER' in os.environ and int(os.environ['ATHENA_PROC_NUMBER'])>0:
        njobs = int(os.environ['ATHENA_PROC_NUMBER'])
        mglog.info('Lucky you - you are running on a full node queue.  Will re-configure for '+str(njobs)+' jobs.')
        mode = 2

    cluster_type = get_cluster_type(process_dir=process_dir)
    if cluster_type is not None:
        mode = 1

    if is_gen_from_gridpack():
        mglog.info('Running event generation from gridpack (using smarter mode from generate() function)')
        generate_from_gridpack(runArgs=runArgs,extlhapath=extlhapath,gridpack_compile=gridpack_compile,requirePMGSettings=requirePMGSettings)
        return
    else:
        mglog.info('Did not identify an input gridpack.')
        if grid_pack:
            mglog.info('The grid_pack flag is set, so I am expecting to create a gridpack in this job')

    # Now get a variety of info out of the runArgs
    beamEnergy,random_seed = get_runArgs_info(runArgs)

    # Check if process is NLO or LO
    isNLO=is_NLO_run(process_dir=process_dir)

    # temporary fix of makefile, needed for 3.3.1., remove in future
    if isNLO:
        fix_fks_makefile(process_dir=process_dir)

    # if f2py not available
    if get_reweight_card(process_dir=process_dir) is not None:
        from distutils.spawn import find_executable
        if find_executable('f2py') is not None:
            mglog.info('Found f2py, will use it for reweighting')
        else:
            raise RuntimeError('Could not find f2py, needed for reweighting')
        check_reweight_card(process_dir)

    if grid_pack:
        #Running in gridpack mode
        mglog.info('Started generating gridpack at '+str(time.asctime()))
        mglog.warning(' >>>>>> THIS KIND OF JOB SHOULD ONLY BE RUN LOCALLY - NOT IN GRID JOBS <<<<<<')

        # Some events required if we specify MadSpin usage!
        my_settings = {'nevents':'1000'}
        if isNLO:
            my_settings['req_acc']=str(required_accuracy)
        else:
            my_settings = {'gridpack':'true'}
        modify_run_card(process_dir=process_dir,settings=my_settings,skipBaseFragment=True)

    else:
        #Running in on-the-fly mode
        mglog.info('Started generating at '+str(time.asctime()))

    mglog.info('Run '+MADGRAPH_RUN_NAME+' will be performed in mode '+str(mode)+' with '+str(njobs)+' jobs in parallel.')

    # Ensure that things are set up normally
    if not os.access(process_dir,os.R_OK):
        raise RuntimeError('No process directory found at '+process_dir)
    if not os.access(process_dir+'/bin/generate_events',os.R_OK):
        raise RuntimeError('No generate_events module found in '+process_dir)

    mglog.info('For your information, the libraries available are (should include LHAPDF):')
    ls_dir(process_dir+'/lib')

    setupFastjet(process_dir=process_dir)
    if bias_module is not None:
        setup_bias_module(bias_module,process_dir)

    mglog.info('Now I will hack the make files a bit.  Apologies, but there seems to be no good way around this.')
    shutil.copyfile(process_dir+'/Source/make_opts',process_dir+'/Source/make_opts_old')
    old_opts = open(process_dir+'/Source/make_opts_old','r')
    new_opts = open(process_dir+'/Source/make_opts','w')
    for aline in old_opts:
        if 'FC=g' in aline:
            mglog.info('Configuring the fancy gfortran compiler instead of g77 / f77')
            new_opts.write('  FC=gfortran\n')
        elif 'FFLAGS+= -ffixed-line-length-132' in aline and 'i686' in os.environ['CMTCONFIG']:
            mglog.info('Setting you up for a 32-bit compilation')
            new_opts.write('FFLAGS+= -ffixed-line-length-132 -m32\n')
        else:
            new_opts.write(aline)
    old_opts.close()
    new_opts.close()
    mglog.info('Make file hacking complete.')

    # Change directories
    currdir=os.getcwd()
    os.chdir(process_dir)
    # Record the change
    global MADGRAPH_COMMAND_STACK
    MADGRAPH_COMMAND_STACK += [ 'cd ${MGaMC_PROCESS_DIR}' ]


    # Check the run card
    run_card_consistency_check(isNLO=isNLO)

    # Since the consistency check can update some settings, print the cards now
    print_cards_from_dir(process_dir=os.getcwd())

    # Check the param card
    code = check_PMG_updates(process_dir=os.getcwd())
    if requirePMGSettings and code!=0:
        raise RuntimeError('Settings are not compliant with PMG defaults! Please use do_PMG_updates function to get PMG default params.')

    # Build up the generate command
    # Use the new-style way of passing things: just --name, everything else in config
    command = [python,'bin/generate_events']
    if isNLO:
        command += ['--name='+MADGRAPH_RUN_NAME]
        mglog.info('Removing Cards/shower_card.dat to ensure we get parton level events only')
        os.unlink('Cards/shower_card.dat')
    else:
        command += [MADGRAPH_RUN_NAME]
    # Set the number of cores to be used
    setNCores(process_dir=os.getcwd(), Ncores=njobs)
    # Special handling for mode 1
    if mode==1:
        mglog.info('Setting up cluster running')
        modify_config_card(process_dir=os.getcwd(),settings={'run_mode':1})
        if cluster_type=='pbs':
            mglog.info('Modifying bin/internal/cluster.py for PBS cluster running')
            os.system("sed -i \"s:text += prog:text += './'+prog:g\" bin/internal/cluster.py")
    elif mode==2:
        mglog.info('Setting up multi-core running on '+os.environ['ATHENA_PROC_NUMBER']+' cores')
    elif mode==0:
        mglog.info('Setting up serial generation.')

    generate_prep(process_dir=os.getcwd())
    global MADGRAPH_CATCH_ERRORS
    generate = stack_subprocess(command,stdin=subprocess.PIPE, stderr=subprocess.PIPE if MADGRAPH_CATCH_ERRORS else None)
    (out,err) = generate.communicate()
    error_check(err,generate.returncode)

    # Get back to where we came from
    os.chdir(currdir)
    MADGRAPH_COMMAND_STACK += [ 'cd -' ]

    if grid_pack:
        # Name dictacted by https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/PmgMcSoftware
        gridpack_name='mc_'+str(int(beamEnergy*2/1000))+'TeV.'+get_physics_short()+'.GRID.tar.gz'
        mglog.info('Tidying up gridpack '+gridpack_name)

        if not isNLO:
            ### LO RUN - names with and without madspin ###
            MADGRAPH_COMMAND_STACK += ['cp '+glob.glob(process_dir+'/'+MADGRAPH_RUN_NAME+'_*gridpack.tar.gz')[0]+' '+gridpack_name]
            shutil.copy(glob.glob(process_dir+'/'+MADGRAPH_RUN_NAME+'_*gridpack.tar.gz')[0],gridpack_name)

            if gridpack_compile:
                MADGRAPH_COMMAND_STACK += ['mkdir tmp%i/'%os.getpid(),'cd tmp%i/'%os.getpid()]
                os.mkdir('tmp%i/'%os.getpid())
                os.chdir('tmp%i/'%os.getpid())
                mglog.info('untar gridpack')
                untar = stack_subprocess(['tar','xvzf',('../'+gridpack_name)])
                untar.wait()
                mglog.info('compile and clean up')
                MADGRAPH_COMMAND_STACK += ['cd madevent']
                os.chdir('madevent/')
                compilep = stack_subprocess(['./bin/compile'],stderr=subprocess.PIPE if MADGRAPH_CATCH_ERRORS else None)
                (out,err) = compilep.communicate()
                error_check(err,compilep.returncode)
                clean = stack_subprocess(['./bin/clean4grid'],stderr=subprocess.PIPE if MADGRAPH_CATCH_ERRORS else None)
                (out,err) = clean.communicate()
                error_check(err,clean.returncode)
                clean.wait()
                MADGRAPH_COMMAND_STACK += ['cd ..','rm ../'+gridpack_name]
                os.chdir('../')
                mglog.info('remove old tarball')
                os.unlink('../'+gridpack_name)
                mglog.info('Package up new tarball')
                tar = stack_subprocess(['tar','cvzf','../'+gridpack_name,'--exclude=SubProcesses/P*/G*/*_results.dat','--exclude=SubProcesses/P*/G*/*.log','--exclude=SubProcesses/P*/G*/*.txt','.'])
                tar.wait()
                MADGRAPH_COMMAND_STACK += ['cd ..','rm -r tmp%i/'%os.getpid()]
                os.chdir('../')
                mglog.info('Remove temporary directory')
                shutil.rmtree('tmp%i/'%os.getpid())
                mglog.info('Tidying up complete!')

        else:

            ### NLO RUN ###
            mglog.info('Package up process_dir')
            MADGRAPH_COMMAND_STACK += ['mv '+process_dir+' '+MADGRAPH_GRIDPACK_LOCATION]
            os.rename(process_dir,MADGRAPH_GRIDPACK_LOCATION)
            tar = stack_subprocess(['tar','czf',gridpack_name,MADGRAPH_GRIDPACK_LOCATION,'--exclude=Events/*/*events*gz','--exclude=SubProcesses/P*/G*/log*txt','--exclude=SubProcesses/P*/G*/events.lhe*','--exclude=*/*.o','--exclude=*/*/*.o','--exclude=*/*/*/*.o','--exclude=*/*/*/*/*.o'])
            tar.wait()
            MADGRAPH_COMMAND_STACK += ['mv '+MADGRAPH_GRIDPACK_LOCATION+' '+process_dir]
            os.rename(MADGRAPH_GRIDPACK_LOCATION,process_dir)

        mglog.info('Gridpack sucessfully created, exiting the transform')
        if hasattr(runArgs,'outputTXTFile'):
            mglog.info('Touching output TXT (LHE) file for the transform')
            open(runArgs.outputTXTFile, 'w').close()
        from AthenaCommon.AppMgr import theApp
        theApp.finalize()
        theApp.exit()

    mglog.info('Finished at '+str(time.asctime()))
    return 0


def generate_from_gridpack(runArgs=None, extlhapath=None, gridpack_compile=None, requirePMGSettings=False):

    # Get of info out of the runArgs
    beamEnergy,random_seed = get_runArgs_info(runArgs)

    # Just in case
    setup_path_protection()

    isNLO=is_NLO_run(process_dir=MADGRAPH_GRIDPACK_LOCATION)

    setupFastjet(process_dir=MADGRAPH_GRIDPACK_LOCATION)

    # This is hard-coded as a part of MG5_aMC :'(
    gridpack_run_name = 'GridRun_'+str(random_seed)

    # Ensure that we only do madspin at the end
    if os.access(MADGRAPH_GRIDPACK_LOCATION+'/Cards/madspin_card.dat',os.R_OK):
        os.rename(MADGRAPH_GRIDPACK_LOCATION+'/Cards/madspin_card.dat',MADGRAPH_GRIDPACK_LOCATION+'/Cards/backup_madspin_card.dat')
        do_madspin=True
    else:
        do_madspin=False

    if get_reweight_card(process_dir=MADGRAPH_GRIDPACK_LOCATION) is not None:
        check_reweight_card(MADGRAPH_GRIDPACK_LOCATION)

    # Check the param card
    code = check_PMG_updates(process_dir=MADGRAPH_GRIDPACK_LOCATION)
    if requirePMGSettings and code!=0:
        raise RuntimeError('Settings are not compliant with PMG defaults! Please use do_PMG_updates function to get PMG default params.')

    # Modify run card, then print
    settings={'iseed':str(random_seed)}
    if not isNLO:
        settings['python_seed']=str(random_seed)
    modify_run_card(process_dir=MADGRAPH_GRIDPACK_LOCATION,settings=settings,skipBaseFragment=True)

    mglog.info('Generating events from gridpack')

    # Ensure that things are set up normally
    if not os.path.exists(MADGRAPH_GRIDPACK_LOCATION):
        raise RuntimeError('Gridpack directory not found at '+MADGRAPH_GRIDPACK_LOCATION)

    nevents = getDictFromCard(MADGRAPH_GRIDPACK_LOCATION+'/Cards/run_card.dat')['nevents']
    mglog.info('>>>> FOUND GRIDPACK <<<<  <- This will be used for generation')
    mglog.info('Generation of '+str(int(nevents))+' events will be performed using the supplied gridpack with random seed '+str(random_seed))
    mglog.info('Started generating events at '+str(time.asctime()))

    #Remove addmasses if it's there
    if os.access(MADGRAPH_GRIDPACK_LOCATION+'/bin/internal/addmasses.py',os.R_OK):
        os.remove(MADGRAPH_GRIDPACK_LOCATION+'/bin/internal/addmasses.py')

    currdir=os.getcwd()

    # Make sure we've set the number of processes appropriately
    setNCores(process_dir=MADGRAPH_GRIDPACK_LOCATION)
    global MADGRAPH_CATCH_ERRORS

    # Run the consistency check, print some useful info
    ls_dir(currdir)
    ls_dir(MADGRAPH_GRIDPACK_LOCATION)

    # Update the run card according to consistency checks
    run_card_consistency_check(isNLO=isNLO,process_dir=MADGRAPH_GRIDPACK_LOCATION)

    # Now all done with updates, so print the cards with the final settings
    print_cards_from_dir(process_dir=MADGRAPH_GRIDPACK_LOCATION)

    if isNLO:
        #turn off systematics for gridpack generation and store settings for standalone run
        run_card_dict=getDictFromCard(MADGRAPH_GRIDPACK_LOCATION+'/Cards/run_card.dat')
        systematics_settings=None
        if checkSetting('systematics_program','systematics',run_card_dict):
            if not checkSettingIsTrue('store_rwgt_info',run_card_dict):
                raise RuntimeError('Trying to run NLO systematics but reweight info not stored')
            if checkSettingExists('systematics_arguments',run_card_dict):
                systematics_settings=MadGraphControl.MadGraphSystematicsUtils.parse_systematics_arguments(run_card_dict['systematics_arguments'])
            else:
                systematics_settings={}
            mglog.info('Turning off systematics for now, running standalone later')
            modify_run_card(process_dir=MADGRAPH_GRIDPACK_LOCATION,settings={'systematics_program':'none'},skipBaseFragment=True)

    global MADGRAPH_COMMAND_STACK
    if not isNLO:
        ### LO RUN ###
        if not os.access(MADGRAPH_GRIDPACK_LOCATION+'/bin/gridrun',os.R_OK):
            mglog.error('/bin/gridrun not found at '+MADGRAPH_GRIDPACK_LOCATION)
            raise RuntimeError('Could not find gridrun executable')
        else:
            mglog.info('Found '+MADGRAPH_GRIDPACK_LOCATION+'/bin/gridrun, starting generation.')
        generate_prep(MADGRAPH_GRIDPACK_LOCATION)
        granularity=1
        mglog.info("Now generating {} events with random seed {} and granularity {}".format(int(nevents),int(random_seed),granularity))
        # not sure whether this is needed but it is done in the old "run.sh" script
        new_ld_path=":".join([os.environ['LD_LIBRARY_PATH'],os.getcwd()+'/'+MADGRAPH_GRIDPACK_LOCATION+'/madevent/lib',os.getcwd()+'/'+MADGRAPH_GRIDPACK_LOCATION+'/HELAS/lib'])
        os.environ['LD_LIBRARY_PATH']=new_ld_path
        MADGRAPH_COMMAND_STACK+=["export LD_LIBRARY_PATH="+":".join(['${LD_LIBRARY_PATH}',new_ld_path])]
        generate = stack_subprocess([python,MADGRAPH_GRIDPACK_LOCATION+'/bin/gridrun',str(int(nevents)),str(int(random_seed)),str(granularity)],stdin=subprocess.PIPE,stderr=subprocess.PIPE if MADGRAPH_CATCH_ERRORS else None)        
        (out,err) = generate.communicate()
        error_check(err,generate.returncode)
        gp_events=MADGRAPH_GRIDPACK_LOCATION+"/Events/GridRun_{}/unweighted_events.lhe.gz".format(int(random_seed))
        if not os.path.exists(gp_events):
            mglog.error('Error in gp generation, did not find events at '+gp_events)

        # add reweighting, which is not run automatically from LO GPs
        reweight_card=get_reweight_card(MADGRAPH_GRIDPACK_LOCATION)
        if reweight_card is not None:
            pythonpath_backup=os.environ['PYTHONPATH']
            # workaround as madevent crashes when path to mg in PYTHONPATH
            os.environ['PYTHONPATH']=':'.join([p for p in pythonpath_backup.split(':') if 'madgraph5amc' not in p])
            add_reweighting('GridRun_{}'.format(int(random_seed)))
            os.environ['PYTHONPATH']=pythonpath_backup

        shutil.move(gp_events,'events.lhe.gz')

    else:
        ### NLO RUN ###
        if not os.access(MADGRAPH_GRIDPACK_LOCATION+'/bin/generate_events',os.R_OK):
            raise RuntimeError('Could not find generate_events executable at '+MADGRAPH_GRIDPACK_LOCATION)
        else:
            mglog.info('Found '+MADGRAPH_GRIDPACK_LOCATION+'/bin/generate_events, starting generation.')

        ls_dir(MADGRAPH_GRIDPACK_LOCATION+'/Events/')
        if os.access(MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name, os.F_OK):
            mglog.info('Removing '+MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name+' directory from gridpack generation')
            MADGRAPH_COMMAND_STACK += ['rm -rf '+MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name]
            shutil.rmtree(MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name)

        # Delete events generated when setting up MadSpin during gridpack generation
        if os.access(MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name+'_decayed_1', os.F_OK):
            mglog.info('Removing '+MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name+'_decayed_1 directory from gridpack generation')
            MADGRAPH_COMMAND_STACK += ['rm -rf '+MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name+'_decayed_1']
            shutil.rmtree(MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name+'_decayed_1')

        ls_dir(MADGRAPH_GRIDPACK_LOCATION+'/Events/')

        if not gridpack_compile:
            mglog.info('Copying make_opts from Template')
            shutil.copy(os.environ['MADPATH']+'/Template/LO/Source/make_opts',MADGRAPH_GRIDPACK_LOCATION+'/Source/')

            generate_prep(MADGRAPH_GRIDPACK_LOCATION)
            generate = stack_subprocess([python,MADGRAPH_GRIDPACK_LOCATION+'/bin/generate_events','--parton','--nocompile','--only_generation','-f','--name='+gridpack_run_name],stdin=subprocess.PIPE,stderr=subprocess.PIPE if MADGRAPH_CATCH_ERRORS else None)
            (out,err) = generate.communicate()
            error_check(err,generate.returncode)
        else:
            mglog.info('Allowing recompilation of gridpack')
            if os.path.islink(MADGRAPH_GRIDPACK_LOCATION+'/lib/libLHAPDF.a'):
                mglog.info('Unlinking '+MADGRAPH_GRIDPACK_LOCATION+'/lib/libLHAPDF.a')
                os.unlink(MADGRAPH_GRIDPACK_LOCATION+'/lib/libLHAPDF.a')

            generate_prep(MADGRAPH_GRIDPACK_LOCATION)
            generate = stack_subprocess([python,MADGRAPH_GRIDPACK_LOCATION+'/bin/generate_events','--parton','--only_generation','-f','--name='+gridpack_run_name],stdin=subprocess.PIPE,stderr=subprocess.PIPE if MADGRAPH_CATCH_ERRORS else None)
            (out,err) = generate.communicate()
            error_check(err,generate.returncode)
    if isNLO and systematics_settings is not None:
        # run systematics
        mglog.info('Running systematics standalone')
        systematics_path=MADGRAPH_GRIDPACK_LOCATION+'/bin/internal/systematics.py'
        events_location=MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name+'/events.lhe.gz'
        syst_cmd=[python,systematics_path]+[events_location]*2+["--"+k+"="+systematics_settings[k] for k in systematics_settings]
        mglog.info('running: '+' '.join(syst_cmd))
        systematics = stack_subprocess(syst_cmd)
        systematics.wait()


    # See if MG5 did the job for us already
    if not os.access('events.lhe.gz',os.R_OK):
        mglog.info('Copying generated events to '+currdir)
        if not os.path.exists(MADGRAPH_GRIDPACK_LOCATION+'Events/'+gridpack_run_name):
            shutil.copy(MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name+'/events.lhe.gz','events.lhe.gz')
    else:
        mglog.info('Events were already in place')

    ls_dir(currdir)

    mglog.info('Moving generated events to be in correct format for arrange_output().')
    mglog.info('Unzipping generated events.')
    unzip = stack_subprocess(['gunzip','-f','events.lhe.gz'])
    unzip.wait()

    mglog.info('Moving file over to '+MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name+'/unweighted_events.lhe')
    mkdir = stack_subprocess(['mkdir','-p',(MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name)])
    mkdir.wait()
    shutil.move('events.lhe',MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name+'/unweighted_events.lhe')

    mglog.info('Re-zipping into dataset name '+MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name+'/unweighted_events.lhe.gz')
    rezip = stack_subprocess(['gzip',MADGRAPH_GRIDPACK_LOCATION+'/Events/'+gridpack_run_name+'/unweighted_events.lhe'])
    rezip.wait()

    os.chdir(currdir)

    # Now consider MadSpin:
    if do_madspin:
        # Move card back
        os.rename(MADGRAPH_GRIDPACK_LOCATION+'/Cards/backup_madspin_card.dat',MADGRAPH_GRIDPACK_LOCATION+'/Cards/madspin_card.dat')
        mglog.info('Decaying with MadSpin.')
        add_madspin(process_dir=MADGRAPH_GRIDPACK_LOCATION)

    mglog.info('Finished at '+str(time.asctime()))

    return 0


def setupFastjet(process_dir=None):

    isNLO=is_NLO_run(process_dir=process_dir)

    mglog.info('Path to fastjet install dir: '+os.environ['FASTJETPATH'])
    fastjetconfig = os.environ['FASTJETPATH']+'/bin/fastjet-config'

    mglog.info('fastjet-config --version:      '+str(subprocess.Popen([fastjetconfig, '--version'],stdout = subprocess.PIPE).stdout.read().strip()))
    mglog.info('fastjet-config --prefix:       '+str(subprocess.Popen([fastjetconfig, '--prefix'],stdout = subprocess.PIPE).stdout.read().strip()))

    if not isNLO:
        config_card=process_dir+'/Cards/me5_configuration.txt'
    else:
        config_card=process_dir+'/Cards/amcatnlo_configuration.txt'

    oldcard = open(config_card,'r')
    newcard = open(config_card+'.tmp','w')

    for line in oldcard:
        if 'fastjet = ' in line:
            newcard.write('fastjet = '+fastjetconfig+'\n')
            mglog.info('Setting fastjet = '+fastjetconfig+' in '+config_card)
        else:
            newcard.write(line)
    oldcard.close()
    newcard.close()
    shutil.move(config_card+'.tmp',config_card)

    return


def get_LHAPDF_DATA_PATH():
    return get_LHAPDF_PATHS()[1]


def get_LHAPDF_PATHS():
    LHADATAPATH=None
    LHAPATH=None
    for p in os.environ['LHAPATH'].split(':')+os.environ['LHAPDF_DATA_PATH'].split(':'):
        if os.path.exists(p+"/../../lib/") and LHAPATH is None:
            LHAPATH=p
    for p in os.environ['LHAPDF_DATA_PATH'].split(':')+os.environ['LHAPATH'].split(':'):
        if os.path.exists(p) and LHADATAPATH is None and p!=LHAPATH:
            LHADATAPATH=p
    if LHADATAPATH is None:
        LHADATAPATH=LHAPATH
    if LHAPATH is None:
        mglog.error('Could not find path to LHAPDF installation')
    return LHAPATH,LHADATAPATH


# function to get lhapdf id and name from either id or name
def get_lhapdf_id_and_name(pdf):
    pdfname=''
    pdfid=-999
    LHADATAPATH=get_LHAPDF_DATA_PATH()
    pdflist = open(LHADATAPATH+'/pdfsets.index','r')
    if isinstance(pdf,int) or pdf.isdigit():
        pdf=int(pdf)
        pdfid=pdf
        for line in pdflist:
            splitline=line.split()
            if int(splitline[0]) == pdfid:
                pdfname=splitline[1]
                break
    else:
        pdfname=pdf
        for line in pdflist:
            splitline=line.split()
            if splitline[1] == pdfname:
                pdfid=int(splitline[0])
                break
    pdflist.close()

    if pdfname=='':
        err='Couldn\'t find PDF name associated to ID %i in %s.'%(pdfid,LHADATAPATH+'/pdfsets.index')
        mglog.error(err)
        raise RuntimeError(err)
    if pdfid<0:
        err='Couldn\'t find PDF ID associated to name %s in %s.'%(pdfname,LHADATAPATH+'/pdfsets.index')
        mglog.error(err)
        raise RuntimeError(err)

    return pdfid,pdfname


def setupLHAPDF(process_dir=None, extlhapath=None, allow_links=True):

    isNLO=is_NLO_run(process_dir=process_dir)

    origLHAPATH=os.environ['LHAPATH']
    origLHAPDF_DATA_PATH=os.environ['LHAPDF_DATA_PATH']

    LHAPATH,LHADATAPATH=get_LHAPDF_PATHS()

    pdfname=''
    pdfid=-999

    ### Reading LHAPDF ID from run card
    run_card=process_dir+'/Cards/run_card.dat'
    mydict=getDictFromCard(run_card)

    if mydict["pdlabel"].replace("'","") == 'lhapdf':
        #Make local LHAPDF dir
        mglog.info('creating local LHAPDF dir: MGC_LHAPDF/')
        if os.path.islink('MGC_LHAPDF/'):
            os.unlink('MGC_LHAPDF/')
        elif os.path.isdir('MGC_LHAPDF/'):
            shutil.rmtree('MGC_LHAPDF/')

        newMGCLHA='MGC_LHAPDF/'

        mkdir = subprocess.Popen(['mkdir','-p',newMGCLHA])
        mkdir.wait()

        pdfs_used=[ int(x) for x in mydict['lhaid'].replace(' ',',').split(',') ]
        # included systematics pdfs here
        if 'sys_pdf' in mydict:
            sys_pdf=mydict['sys_pdf'].replace('&&',' ').split()
            for s in sys_pdf:
                if s.isdigit():
                    idx=int(s)
                    if idx>1000: # the sys_pdf syntax is such that small numbers are used to specify the subpdf index
                        pdfs_used.append(idx)
                else:
                    pdfs_used.append(s)
        if 'systematics_arguments' in mydict:
            systematics_arguments=MadGraphControl.MadGraphSystematicsUtils.parse_systematics_arguments(mydict['systematics_arguments'])
            if 'pdf' in systematics_arguments:
                sys_pdf=systematics_arguments['pdf'].replace(',',' ').replace('@',' ').split()
                for s in sys_pdf:
                    if s.isdigit():
                        idx=int(s)
                        if idx>1000: # the sys_pdf syntax is such that small numbers are used to specify the subpdf index
                            pdfs_used.append(idx)
                    else:
                        pdfs_used.append(s)
        for pdf in pdfs_used:
            if isinstance(pdf,str) and (pdf.lower()=='errorset' or pdf.lower()=='central'):
                continue
            # new function to get both lhapdf id and name
            pdfid,pdfname=get_lhapdf_id_and_name(pdf)
            mglog.info("Found LHAPDF ID="+str(pdfid)+", name="+pdfname)

            if not os.path.exists(newMGCLHA+pdfname) and not os.path.lexists(newMGCLHA+pdfname):
                if not os.path.exists(LHADATAPATH+'/'+pdfname):
                    mglog.warning('PDF not installed at '+LHADATAPATH+'/'+pdfname)
                if allow_links:
                    mglog.info('linking '+LHADATAPATH+'/'+pdfname+' --> '+newMGCLHA+pdfname)
                    os.symlink(LHADATAPATH+'/'+pdfname,newMGCLHA+pdfname)
                else:
                    mglog.info('copying '+LHADATAPATH+'/'+pdfname+' --> '+newMGCLHA+pdfname)
                    shutil.copytree(LHADATAPATH+'/'+pdfname,newMGCLHA+pdfname)

        if allow_links:
            mglog.info('linking '+LHADATAPATH+'/pdfsets.index --> '+newMGCLHA+'pdfsets.index')
            os.symlink(LHADATAPATH+'/pdfsets.index',newMGCLHA+'pdfsets.index')

            atlasLHADATAPATH=LHADATAPATH.replace('sft.cern.ch/lcg/external/lhapdfsets/current','atlas.cern.ch/repo/sw/Generators/lhapdfsets/current')
            mglog.info('linking '+atlasLHADATAPATH+'/lhapdf.conf --> '+newMGCLHA+'lhapdf.conf')
            os.symlink(atlasLHADATAPATH+'/lhapdf.conf',newMGCLHA+'lhapdf.conf')
        else:
            mglog.info('copying '+LHADATAPATH+'/pdfsets.index --> '+newMGCLHA+'pdfsets.index')
            shutil.copy2(LHADATAPATH+'/pdfsets.index',newMGCLHA+'pdfsets.index')

            atlasLHADATAPATH=LHADATAPATH.replace('sft.cern.ch/lcg/external/lhapdfsets/current','atlas.cern.ch/repo/sw/Generators/lhapdfsets/current')
            mglog.info('copying '+atlasLHADATAPATH+'/lhapdf.conf -->'+newMGCLHA+'lhapdf.conf')
            shutil.copy2(atlasLHADATAPATH+'/lhapdf.conf',newMGCLHA+'lhapdf.conf')


        LHADATAPATH=os.getcwd()+'/MGC_LHAPDF'

    else:
        mglog.info('Not using LHAPDF')
        return (LHAPATH,origLHAPATH,origLHAPDF_DATA_PATH)


    if isNLO:
        os.environ['LHAPDF_DATA_PATH']=LHADATAPATH

    mglog.info('Path to LHAPDF install dir: '+LHAPATH)
    mglog.info('Path to LHAPDF data dir: '+LHADATAPATH)
    if not os.path.isdir(LHADATAPATH):
        raise RuntimeError('LHAPDF data dir not accesible: '+LHADATAPATH)
    if not os.path.isdir(LHAPATH):
        raise RuntimeError('LHAPDF path dir not accesible: '+LHAPATH)

    # Dealing with LHAPDF
    if extlhapath:
        lhapdfconfig=extlhapath
        if not os.access(lhapdfconfig,os.X_OK):
            raise RuntimeError('Failed to find valid external lhapdf-config at '+lhapdfconfig)
        LHADATAPATH=subprocess.Popen([lhapdfconfig, '--datadir'],stdout = subprocess.PIPE).stdout.read().strip()
        mglog.info('Changing LHAPDF_DATA_PATH to '+LHADATAPATH)
        os.environ['LHAPDF_DATA_PATH']=LHADATAPATH
    else:
        getlhaconfig = subprocess.Popen(['get_files','-data','lhapdf-config'])
        getlhaconfig.wait()
        #Get custom lhapdf-config
        if not os.access(os.getcwd()+'/lhapdf-config',os.X_OK):
            mglog.error('Failed to get lhapdf-config from MadGraphControl')
            return 1
        lhapdfconfig = os.getcwd()+'/lhapdf-config'

    mglog.info('lhapdf-config --version:      '+str(subprocess.Popen([lhapdfconfig, '--version'],stdout = subprocess.PIPE).stdout.read().strip()))
    mglog.info('lhapdf-config --prefix:       '+str(subprocess.Popen([lhapdfconfig, '--prefix'],stdout = subprocess.PIPE).stdout.read().strip()))
    mglog.info('lhapdf-config --libdir:       '+str(subprocess.Popen([lhapdfconfig, '--libdir'],stdout = subprocess.PIPE).stdout.read().strip()))
    mglog.info('lhapdf-config --datadir:      '+str(subprocess.Popen([lhapdfconfig, '--datadir'],stdout = subprocess.PIPE).stdout.read().strip()))
    mglog.info('lhapdf-config --pdfsets-path: '+str(subprocess.Popen([lhapdfconfig, '--pdfsets-path'],stdout = subprocess.PIPE).stdout.read().strip()))

    modify_config_card(process_dir=process_dir,settings={'lhapdf':lhapdfconfig,'lhapdf_py3':lhapdfconfig})

    mglog.info('Creating links for LHAPDF')
    if os.path.islink(process_dir+'/lib/PDFsets'):
        os.unlink(process_dir+'/lib/PDFsets')
    elif os.path.isdir(process_dir+'/lib/PDFsets'):
        shutil.rmtree(process_dir+'/lib/PDFsets')
    if allow_links:
        os.symlink(LHADATAPATH,process_dir+'/lib/PDFsets')
    else:
        shutil.copytree(LHADATAPATH,process_dir+'/lib/PDFsets')
    mglog.info('Available PDFs are:')
    mglog.info( sorted( [ x for x in os.listdir(process_dir+'/lib/PDFsets') if ".tar.gz" not in x ] ) )

    global MADGRAPH_COMMAND_STACK
    MADGRAPH_COMMAND_STACK += [ '# Copy the LHAPDF files locally' ]
    MADGRAPH_COMMAND_STACK += [ 'cp -r '+os.getcwd()+'/MGC_LHAPDF .' ]
    MADGRAPH_COMMAND_STACK += [ 'cp -r '+process_dir+'/lib/PDFsets ${MGaMC_PROCESS_DIR}/lib/' ]

    return (LHAPATH,origLHAPATH,origLHAPDF_DATA_PATH)


# Function to set the number of cores and the running mode in the run card
def setNCores(process_dir, Ncores=None):
    my_Ncores = Ncores
    my_runMode = 2 if 'ATHENA_PROC_NUMBER' in os.environ else 0
    if Ncores is None and 'ATHENA_PROC_NUMBER' in os.environ and int(os.environ['ATHENA_PROC_NUMBER'])>0:
        my_Ncores = int(os.environ['ATHENA_PROC_NUMBER'])
        my_runMode = 2
    if my_Ncores is None:
        mglog.info('Setting up for serial run')
        my_Ncores = 1

    modify_config_card(process_dir=process_dir,settings={'nb_core':my_Ncores,'run_mode':my_runMode,'automatic_html_opening':'False'})


def resetLHAPDF(origLHAPATH='',origLHAPDF_DATA_PATH=''):
    mglog.info('Restoring original LHAPDF env variables:')
    os.environ['LHAPATH']=origLHAPATH
    os.environ['LHAPDF_DATA_PATH']=origLHAPDF_DATA_PATH
    mglog.info('LHAPATH='+os.environ['LHAPATH'])
    mglog.info('LHAPDF_DATA_PATH='+os.environ['LHAPDF_DATA_PATH'])


def get_mg5_executable():
    madpath=os.environ['MADPATH']
    if not os.access(madpath+'/bin/mg5_aMC',os.R_OK):
        raise RuntimeError('mg5_aMC executable not found in '+madpath)
    return madpath+'/bin/mg5_aMC'


def add_lifetimes(process_dir,threshold=None):
    """ Add lifetimes to the generated LHE file.  Should be
    called after generate_events is called.
    """
    if config_only_check():
        return

    me_exec=get_mg5_executable()

    if len(glob.glob(process_dir+'/Events/*'))<1:
        mglog.error('Process dir '+process_dir+' does not contain events?')
    run = glob.glob(process_dir+'/Events/*')[0].split('/')[-1]

    # Note : This slightly clunky implementation is needed for the time being
    # See : https://answers.launchpad.net/mg5amcnlo/+question/267904

    tof_c = open('time_of_flight_exec_card','w')
    tof_c.write('launch '+process_dir+''' -i
add_time_of_flight '''+run+((' --threshold='+str(threshold)) if threshold is not None else ''))
    tof_c.close()

    mglog.info('Started adding time of flight info '+str(time.asctime()))

    global MADGRAPH_CATCH_ERRORS
    generate = stack_subprocess([python,me_exec,'time_of_flight_exec_card'],stdin=subprocess.PIPE,stderr=subprocess.PIPE if MADGRAPH_CATCH_ERRORS else None)
    (out,err) = generate.communicate()
    error_check(err,generate.returncode)

    mglog.info('Finished adding time of flight information at '+str(time.asctime()))

    # Re-zip the file if needed
    lhe_gz = glob.glob(process_dir+'/Events/*/*lhe.gz')[0]
    if not os.access(lhe_gz,os.R_OK):
        mglog.info('LHE file needs to be zipped')
        lhe = glob.glob(process_dir+'/Events/*/*lhe.gz')[0]
        rezip = stack_subprocess(['gzip',lhe])
        rezip.wait()
        mglog.info('Zipped')
    else:
        mglog.info('LHE file zipped by MadGraph automatically. Nothing to do')

    return True


def add_madspin(madspin_card=None,process_dir=MADGRAPH_GRIDPACK_LOCATION):
    """ Run madspin on the generated LHE file.  Should be
    run when you have inputGeneratorFile set.
    Only requires a simplified process with the same model that you are
    interested in (needed to set up a process directory for MG5_aMC)
    """
    if config_only_check():
        return

    me_exec=get_mg5_executable()

    if madspin_card is not None:
        shutil.copyfile(madspin_card,process_dir+'/Cards/madspin_card.dat')

    if len(glob.glob(process_dir+'/Events/*'))<1:
        mglog.error('Process dir '+process_dir+' does not contain events?')
    proc_dir_list = glob.glob(process_dir+'/Events/*')
    run=None
    for adir in proc_dir_list:
        if 'GridRun_' in adir:
            run=adir.split('/')[-1]
            break
    else:
        run=proc_dir_list[0].split('/')[-1]

    # Note : This slightly clunky implementation is needed for the time being
    # See : https://answers.launchpad.net/mg5amcnlo/+question/267904

    ms_c = open('madspin_exec_card','w')
    ms_c.write('launch '+process_dir+''' -i
decay_events '''+run)
    ms_c.close()

    mglog.info('Started running madspin at '+str(time.asctime()))

    global MADGRAPH_CATCH_ERRORS
    generate = stack_subprocess([python,me_exec,'madspin_exec_card'],stdin=subprocess.PIPE,stderr=subprocess.PIPE if MADGRAPH_CATCH_ERRORS else None)
    (out,err) = generate.communicate()
    error_check(err,generate.returncode)
    if len(glob.glob(process_dir+'/Events/'+run+'_decayed_*/')) == 0:
        mglog.error('No '+process_dir+'/Events/'+run+'_decayed_*/ can be found')
        raise RuntimeError('Problem while running MadSpin')

    mglog.info('Finished running madspin at '+str(time.asctime()))

    # Re-zip the file if needed
    lhe_gz = glob.glob(process_dir+'/Events/*/*lhe.gz')[0]
    if not os.access(lhe_gz,os.R_OK):
        mglog.info('LHE file needs to be zipped')
        lhe = glob.glob(process_dir+'/Events/*/*lhe.gz')[0]
        rezip = stack_subprocess(['gzip',lhe])
        rezip.wait()
        mglog.info('Zipped')
    else:
        mglog.info('LHE file zipped by MadGraph automatically. Nothing to do')


def madspin_on_lhe(input_LHE,madspin_card,runArgs=None,keep_original=False):
    """ Run MadSpin on an input LHE file. Takes the process
    from the LHE file, so you don't need to have a process directory
    set up in advance. Runs MadSpin and packs the LHE file up appropriately
    Needs runArgs for the file handling"""
    if not os.access(input_LHE,os.R_OK):
        raise RuntimeError('Could not find LHE file '+input_LHE)
    if not os.access(madspin_card,os.R_OK):
        raise RuntimeError('Could not find input MadSpin card '+madspin_card)
    if keep_original:
        shutil.copy(input_LHE,input_LHE+'.original')
        mglog.info('Put backup copy of LHE file at '+input_LHE+'.original')
    # Start writing the card for execution
    madspin_exec_card = open('madspin_exec_card','w')
    madspin_exec_card.write('import '+input_LHE+'\n')
    # Based on the original card
    input_madspin_card = open(madspin_card,'r')
    has_launch = False
    for l in input_madspin_card.readlines():
        commands = l.split('#')[0].split()
        # Skip import of a file name that isn't our file
        if len(commands)>1 and 'import'==commands[0] and not 'model'==commands[1]:
            continue
        # Check for a launch command
        if len(commands)>0 and 'launch' == commands[0]:
            has_launch = True
        madspin_exec_card.write(l.strip()+'\n')
    if not has_launch:
        madspin_exec_card.write('launch\n')
    madspin_exec_card.close()
    input_madspin_card.close()
    # Now get the madspin executable
    madpath=os.environ['MADPATH']
    if not os.access(madpath+'/MadSpin/madspin',os.R_OK):
        raise RuntimeError('madspin executable not found in '+madpath)
    mglog.info('Starting madspin at '+str(time.asctime()))
    global MADGRAPH_CATCH_ERRORS
    generate = stack_subprocess([python,madpath+'/MadSpin/madspin','madspin_exec_card'],stdin=subprocess.PIPE,stderr=subprocess.PIPE if MADGRAPH_CATCH_ERRORS else None)
    (out,err) = generate.communicate()
    error_check(err,generate.returncode)
    mglog.info('Done with madspin at '+str(time.asctime()))
    # Should now have a re-zipped LHE file
    # We now have to do a shortened version of arrange_output below
    # Clean up in case a link or file was already there
    if os.path.exists(os.getcwd()+'/events.lhe'):
        os.remove(os.getcwd()+'/events.lhe')

    mglog.info('Unzipping generated events.')
    unzip = stack_subprocess(['gunzip','-f',input_LHE+'.gz'])
    unzip.wait()

    mglog.info('Putting a copy in place for the transform.')
    mod_output = open(os.getcwd()+'/events.lhe','w')

    #Removing empty lines in LHE
    nEmpty=0
    with open(input_LHE,'r') as fileobject:
        for line in fileobject:
            if line.strip():
                mod_output.write(line)
            else:
                nEmpty=nEmpty+1
    mod_output.close()

    mglog.info('Removed '+str(nEmpty)+' empty lines from LHEF')

    # Actually move over the dataset - this first part is horrible...
    if runArgs is None:
        raise RuntimeError('Must provide runArgs to madspin_on_lhe')

    outputDS = runArgs.outputTXTFile if hasattr(runArgs,'outputTXTFile') else 'tmp_LHE_events'

    mglog.info('Moving file over to '+outputDS.split('.tar.gz')[0]+'.events')
    shutil.move(os.getcwd()+'/events.lhe',outputDS.split('.tar.gz')[0]+'.events')

    mglog.info('Re-zipping into dataset name '+outputDS)
    rezip = stack_subprocess(['tar','cvzf',outputDS,outputDS.split('.tar.gz')[0]+'.events'])
    rezip.wait()

    # shortening the outputDS in the case of an output TXT file
    if hasattr(runArgs,'outputTXTFile') and runArgs.outputTXTFile is not None:
        outputDS = outputDS.split('.TXT')[0]
    # Do some fixing up for them
    if runArgs is not None:
        mglog.debug('Setting inputGenerator file to '+outputDS)
        runArgs.inputGeneratorFile=outputDS


def arrange_output(process_dir=MADGRAPH_GRIDPACK_LOCATION,lhe_version=None,saveProcDir=False,runArgs=None,fixEventWeightsForBridgeMode=False):
    if config_only_check():
        return

    # NLO is not *really* the question here, we need to know if we should look for weighted or
    #  unweighted events in the output directory.  MadSpin (above) only seems to give weighted
    #  results for now?
    if len(glob.glob(os.path.join(process_dir, 'Events','*')))<1:
        mglog.error('Process dir '+process_dir+' does not contain events?')
    proc_dir_list = glob.glob(os.path.join(process_dir, 'Events', '*'))
    this_run_name=None
    # looping over possible directories to find the right one
    for adir in proc_dir_list:
        if 'decayed' in adir:# skipping '*decayed*' directories produced by MadSpin, will be picked later if they exist
            continue
        else:
            if 'GridRun_' in adir:
                this_run_name=adir
                break # GridRun_* directories have priority
            elif os.path.join(process_dir, 'Events',MADGRAPH_RUN_NAME) in adir:
                this_run_name=adir
    if not os.access(this_run_name,os.R_OK):
        raise RuntimeError('Unable to locate run directory')

    hasUnweighted = os.access(this_run_name+'/unweighted_events.lhe.gz',os.R_OK)

    hasRunMadSpin=False
    madspinDirs=sorted(glob.glob(this_run_name+'_decayed_*/'))
    if len(madspinDirs):
        hasRunMadSpin=True
    if hasRunMadSpin and not hasUnweighted:
        # check again:
        hasUnweighted = os.access(madspinDirs[-1]+'/unweighted_events.lhe.gz',os.R_OK)

    global MADGRAPH_COMMAND_STACK
    if hasRunMadSpin:
        if len(madspinDirs):
            if hasUnweighted:
                # so this is a bit of a mess now...
                # if madspin is run from an NLO grid pack the correct lhe events are at both
                #      madevent/Events/run_01/unweighted_events.lhe.gz
                # and  madevent/Events/run_01_decayed_1/events.lhe.gz
                # so there are unweighted events but not in the madspinDir...
                if os.path.exists(madspinDirs[-1]+'/unweighted_events.lhe.gz'):
                    MADGRAPH_COMMAND_STACK += ['mv '+madspinDirs[-1]+'/unweighted_events.lhe.gz'+' '+this_run_name+'/unweighted_events.lhe.gz']
                    shutil.move(madspinDirs[-1]+'/unweighted_events.lhe.gz',this_run_name+'/unweighted_events.lhe.gz')
                    mglog.info('Moving MadSpin events from '+madspinDirs[-1]+'/unweighted_events.lhe.gz to '+this_run_name+'/unweighted_events.lhe.gz')
                elif os.path.exists(madspinDirs[-1]+'/events.lhe.gz'):
                    MADGRAPH_COMMAND_STACK += ['mv '+madspinDirs[-1]+'/events.lhe.gz'+' '+this_run_name+'/unweighted_events.lhe.gz']
                    shutil.move(madspinDirs[-1]+'/events.lhe.gz',this_run_name+'/unweighted_events.lhe.gz')
                    mglog.info('Moving MadSpin events from '+madspinDirs[-1]+'/events.lhe.gz to '+this_run_name+'/unweighted_events.lhe.gz')
                else:
                    raise RuntimeError('MadSpin was run but can\'t find files :(')

            else:
                MADGRAPH_COMMAND_STACK += ['mv '+madspinDirs[-1]+'/events.lhe.gz '+this_run_name+'/events.lhe.gz']
                shutil.move(madspinDirs[-1]+'/events.lhe.gz',this_run_name+'/events.lhe.gz')
                mglog.info('Moving MadSpin events from '+madspinDirs[-1]+'/events.lhe.gz to '+this_run_name+'/events.lhe.gz')

        else:
            mglog.error('MadSpin was run but can\'t find output folder '+(this_run_name+'_decayed_1/'))
            raise RuntimeError('MadSpin was run but can\'t find output folder '+(this_run_name+'_decayed_1/'))

        if fixEventWeightsForBridgeMode:
            mglog.info("Fixing event weights after MadSpin... initial checks.")

            # get the cross section from the undecayed LHE file
            spinmodenone=False
            MGnumevents=-1
            MGintweight=-1

            if hasUnweighted:
                eventsfilename="unweighted_events"
            else:
                eventsfilename="events"
            unzip = stack_subprocess(['gunzip','-f',this_run_name+'/%s.lhe.gz' % eventsfilename])
            unzip.wait()

            for line in open(process_dir+'/Events/'+MADGRAPH_RUN_NAME+'/%s.lhe'%eventsfilename):
                if "Number of Events" in line:
                    sline=line.split()
                    MGnumevents=int(sline[-1])
                elif "Integrated weight (pb)" in line:
                    sline=line.split()
                    MGintweight=float(sline[-1])
                elif "set spinmode none" in line:
                    spinmodenone=True
                elif "</header>" in line:
                    break

            if spinmodenone and MGnumevents>0 and MGintweight>0:
                mglog.info("Fixing event weights after MadSpin... modifying LHE file.")
                newlhe=open(this_run_name+'/%s_fixXS.lhe'%eventsfilename,'w')
                initlinecount=0
                eventlinecount=0
                inInit=False
                inEvent=False

                # new default for MG 2.6.1+ (https://its.cern.ch/jira/browse/AGENE-1725)
                # but verified from LHE below.
                event_norm_setting="average" 

                for line in open(this_run_name+'/%s.lhe'%eventsfilename):

                    newline=line
                    if "<init>" in line:                         
                        inInit=True
                        initlinecount=0
                    elif "</init>" in line:
                        inInit=False
                    elif inInit and initlinecount==0:
                        initlinecount=1
                        # check event_norm setting in LHE file, deteremines how Pythia interprets event weights
                        sline=line.split()
                        if abs(int(sline[-2])) == 3:
                            event_norm_setting="sum"
                        elif abs(int(sline[-2])) == 4:
                            event_norm_setting="average"
                    elif inInit and initlinecount==1:
                        sline=line.split()
                        # update the global XS info
                        relunc=float(sline[1])/float(sline[0])
                        sline[0]=str(MGintweight)                
                        sline[1]=str(float(sline[0])*relunc)     
                        if event_norm_setting=="sum":
                            sline[2]=str(MGintweight/MGnumevents)
                        elif event_norm_setting=="average":
                            sline[2]=str(MGintweight)            
                        newline=' '.join(sline)
                        newline+="\n"
                        initlinecount+=1
                    elif inInit and initlinecount>1:
                        initlinecount+=1
                    elif "<event>" in line:                      
                        inEvent=True
                        eventlinecount=0
                    elif "</event>" in line:
                        inEvent=False
                    elif inEvent and eventlinecount==0:
                        sline=line.split()
                        # next change the per-event weights
                        if event_norm_setting=="sum":
                            sline[2]=str(MGintweight/MGnumevents)
                        elif event_norm_setting=="average":
                            sline[2]=str(MGintweight)            
                        newline=' '.join(sline)
                        newline+="\n"
                        eventlinecount+=1
                    newlhe.write(newline)
                newlhe.close()

                mglog.info("Fixing event weights after MadSpin... cleaning up.")
                shutil.copyfile(this_run_name+'/%s.lhe' % eventsfilename,
                                this_run_name+'/%s_badXS.lhe' % eventsfilename)

                shutil.move(this_run_name+'/%s_fixXS.lhe' % eventsfilename,
                            this_run_name+'/%s.lhe' % eventsfilename)

                rezip = stack_subprocess(['gzip',this_run_name+'/%s.lhe' % eventsfilename])
                rezip.wait()

                rezip = stack_subprocess(['gzip',this_run_name+'/%s_badXS.lhe' % eventsfilename])
                rezip.wait()

    # Clean up in case a link or file was already there
    if os.path.exists(os.getcwd()+'/events.lhe'):
        os.remove(os.getcwd()+'/events.lhe')

    mglog.info('Unzipping generated events.')
    if hasUnweighted:
        unzip = stack_subprocess(['gunzip','-f',this_run_name+'/unweighted_events.lhe.gz'])
        unzip.wait()
    else:
        unzip = stack_subprocess(['gunzip','-f',this_run_name+'/events.lhe.gz'])
        unzip.wait()

    mglog.info('Putting a copy in place for the transform.')
    if hasUnweighted:
        orig_input = this_run_name+'/unweighted_events.lhe'
        mod_output = open(os.getcwd()+'/events.lhe','w')
    else:
        orig_input = this_run_name+'/events.lhe'
        mod_output = open(os.getcwd()+'/events.lhe','w')

    #Removing empty lines and bad comments in LHE
    #and check for existence of weights
    initrwgt=None
    nEmpty=0
    lhe_weights=[]
    with open(orig_input,'r') as fileobject:
        for line in fileobject:
            if line.strip():
                # search for bad characters (neccessary until at least MG5 2.8.1)
                newline=line
                if '#' not in newline:
                    newline=newline
                elif '>' not in newline[ newline.find('#'): ]:
                    newline=newline
                else:
                    mglog.warning('Found bad LHE line with an XML mark in a comment: "'+newline.strip()+'"')
                    newline=newline[:newline.find('#')]+'#'+ (newline[newline.find('#'):].replace('>','-'))
                # check for weightnames that should exist, simplify nominal weight names
                if initrwgt is False:
                    pass
                elif "</initrwgt>" in newline:
                    initrwgt=False
                elif "<initrwgt>" in newline:
                    initrwgt=True
                elif initrwgt is not None:
                    newline=newline.replace('_DYNSCALE-1','')
                    if '</weight>' in newline:
                        iend=newline.find('</weight>')
                        istart=newline[:iend].rfind('>')
                        lhe_weights+=[newline[istart+1:iend].strip()]
                mod_output.write(newline)           
            else:
                nEmpty=nEmpty+1
    mod_output.close()
    mglog.info('Removed '+str(nEmpty)+' empty lines from LHEF')
    
    mglog.info("The following  "+str(len(lhe_weights))+" weights have been written to the LHE file: "+",".join(lhe_weights))
    expected_weights=get_expected_reweight_names(get_reweight_card(process_dir))
    expected_weights+=get_expected_systematic_names(MADGRAPH_PDFSETTING)
    mglog.info("Checking whether the following expected weights are in LHE file: "+",".join(expected_weights))
    for w in expected_weights:
        if w not in lhe_weights:
            raise RuntimeError("Did not find expected weight "+w+" in lhe file. Did the reweight or systematics module crash?")
    mglog.info("Found all required weights!")
    
    if lhe_version:
        mod_output2 = open(os.getcwd()+'/events.lhe','r')
        test=mod_output2.readline()
        if 'version="' in test:
            mglog.info('Applying LHE version hack')
            final_file = open(os.getcwd()+'/events.lhe.copy','w')
            final_file.write('<LesHouchesEvents version="%i.0">\n'%lhe_version)
            shutil.copyfileobj(mod_output2, final_file)
            final_file.close()
            shutil.copy(os.getcwd()+'/events.lhe.copy',os.getcwd()+'/events.lhe')
            # Clean up after ourselves
            os.remove(os.getcwd()+'/events.lhe.copy')
        mod_output2.close()

    # Actually move over the dataset
    if runArgs is None:
        raise RuntimeError('Must provide runArgs to arrange_output')

    if hasattr(runArgs,'outputTXTFile'):
        outputDS = runArgs.outputTXTFile
    else:
        outputDS = 'tmp_LHE_events'

    mglog.info('Moving file over to '+outputDS.split('.tar.gz')[0]+'.events')

    shutil.move(os.getcwd()+'/events.lhe',outputDS.split('.tar.gz')[0]+'.events')

    mglog.info('Re-zipping into dataset name '+outputDS)
    rezip = stack_subprocess(['tar','cvzf',outputDS,outputDS.split('.tar.gz')[0]+'.events'])
    rezip.wait()

    if not saveProcDir:
        mglog.info('Removing the process directory')
        shutil.rmtree(process_dir,ignore_errors=True)

        if os.path.isdir('MGC_LHAPDF/'):
            shutil.rmtree('MGC_LHAPDF/',ignore_errors=True)

    # shortening the outputDS in the case of an output TXT file
    if hasattr(runArgs,'outputTXTFile') and runArgs.outputTXTFile is not None:
        outputDS = outputDS.split('.TXT')[0]
    # Do some fixing up for them
    if runArgs is not None:
        mglog.debug('Setting inputGenerator file to '+outputDS)
        runArgs.inputGeneratorFile=outputDS

    mglog.info('All done with output arranging!')
    return outputDS

def get_expected_reweight_names(reweight_card_loc):
    if reweight_card_loc is None:
        return []
    names=[]
    f_rw=open(reweight_card_loc)
    for line in f_rw:
        if 'launch' not in line:
            continue
        match=re.match(r'launch.*--rwgt_info\s*=\s*(\S+).*',line.strip())
        if len(match.groups())!=1:
            raise RuntimeError('Unexpected format of reweight card in line'+line)
        else:
            names+=[match.group(1)]
    f_rw.close()
    return names

def get_expected_systematic_names(syst_setting):
    names=[]
    if syst_setting is None or 'central_pdf' not in syst_setting:
        mglog.warning("Systematics have not been defined via base fragment or 'MADGRAPH_PDFSETTING', cannot check for expected weights")
        return []
    if 'pdf_variations' in syst_setting and isinstance(syst_setting['pdf_variations'],list):
        names+=[MadGraphControl.MadGraphSystematicsUtils.SYSTEMATICS_WEIGHT_INFO%{'mur':1.0,'muf':1.0,'pdf':syst_setting['central_pdf']}]
        for pdf in syst_setting['pdf_variations']:
            names+=[MadGraphControl.MadGraphSystematicsUtils.SYSTEMATICS_WEIGHT_INFO%{'mur':1.0,'muf':1.0,'pdf':pdf+1}]
    if 'alternative_pdfs' in syst_setting and isinstance(syst_setting['alternative_pdfs'],list):
        for pdf in syst_setting['alternative_pdfs']:
            names+=[MadGraphControl.MadGraphSystematicsUtils.SYSTEMATICS_WEIGHT_INFO%{'mur':1.0,'muf':1.0,'pdf':pdf}]
    if 'scale_variations' in syst_setting and isinstance(syst_setting['scale_variations'],list):
        for mur in syst_setting['scale_variations']:
            for muf in syst_setting['scale_variations']:
                names+=[MadGraphControl.MadGraphSystematicsUtils.SYSTEMATICS_WEIGHT_INFO%{'mur':mur,'muf':muf,'pdf':syst_setting['central_pdf']}]
    return names

def setup_bias_module(bias_module,process_dir):
    run_card = process_dir+'/Cards/run_card.dat'
    if isinstance(bias_module,tuple):
        mglog.info('Using bias module '+bias_module[0])
        the_run_card = open(run_card,'r')
        for line in the_run_card:
            if 'bias_module' in line and not bias_module[0] in line:
                raise RuntimeError('You need to add the bias module '+bias_module[0]+' to the run card to actually run it')
        the_run_card.close()
        if len(bias_module)!=3:
            raise RuntimeError('Please give a 3-tuple of strings containing bias module name, bias module, and makefile. Alternatively, give path to bias module tarball.')
        bias_module_newpath=process_dir+'/Source/BIAS/'+bias_module[0]
        os.makedirs(bias_module_newpath)
        bias_module_file=open(bias_module_newpath+'/'+bias_module[0]+'.f','w')
        bias_module_file.write(bias_module[1])
        bias_module_file.close()
        bias_module_make_file=open(bias_module_newpath+'/Makefile','w')
        bias_module_make_file.write(bias_module[2])
        bias_module_make_file.close()
    else:
        mglog.info('Using bias module '+bias_module)
        bias_module_name=bias_module.split('/')[-1].replace('.gz','')
        bias_module_name=bias_module_name.replace('.tar','')
        the_run_card = open(run_card,'r')
        for line in the_run_card:
            if 'bias_module' in line and bias_module_name not in line:
                raise RuntimeError('You need to add the bias module '+bias_module_name+' to the run card to actually run it')
        the_run_card.close()

        if os.path.exists(bias_module+'.tar.gz'):
            bias_module_path=bias_module+'.tar.gz'
        elif os.path.exists(bias_module+'.gz'):
            bias_module_path=bias_module+'.gz'
        elif os.path.exists(bias_module):
            bias_module_path=bias_module
        else:
            mglog.error('Did not find bias module '+bias_module+' , this path should point to folder or tarball.  Alternatively give a tuple of strings containing module name, module, and makefile')
            return 1
        bias_module_newpath=process_dir+'/Source/BIAS/'+bias_module_path.split('/')[-1]
        mglog.info('Copying bias module into place: '+bias_module_newpath)
        shutil.copy(bias_module_path,bias_module_newpath)
        mglog.info('Unpacking bias module')
        if bias_module_newpath.endswith('.tar.gz'):
            untar = stack_subprocess(['tar','xvzf',bias_module_newpath,'--directory='+process_dir+'/Source/BIAS/'])
            untar.wait()
        elif bias_module_path.endswith('.gz'):
            gunzip = stack_subprocess(['gunzip',bias_module_newpath])
            gunzip.wait()


def get_reweight_card(process_dir=MADGRAPH_GRIDPACK_LOCATION):
    if os.access(process_dir+'/Cards/reweight_card.dat',os.R_OK):
        return process_dir+'/Cards/reweight_card.dat'
    return None


def check_reweight_card(process_dir=MADGRAPH_GRIDPACK_LOCATION):
    reweight_card=get_reweight_card(process_dir=process_dir)
    shutil.move(reweight_card,reweight_card+'.old')
    oldcard = open(reweight_card+'.old','r')
    newcard = open(reweight_card,'w')
    changed = False
    info_expression=r'launch.*--rwgt_info\s*=\s*(\S+).*'
    name_expression=info_expression.replace('info','name')
    goodname_expression=r'^[A-Za-z0-9_\-.]+$'
    for line in oldcard:
        # we are only interested in the 'launch' line
        if not line.strip().startswith('launch') :
            newcard.write(line)
        else:
            rwgt_name_match=re.match(name_expression,line.strip())
            rwgt_info_match=re.match(info_expression,line.strip())
            if rwgt_name_match is None and rwgt_info_match is None:
                raise RuntimeError('Every reweighting should have a --rwgt_info (see https://cp3.irmp.ucl.ac.be/projects/madgraph/wiki/Reweight), please update your reweight_card accordingly. Line to fix: '+line)
            for match in [rwgt_info_match,rwgt_name_match]:
                if match is None:
                    continue
                if len(match.groups())!=1:
                    raise RuntimeError('Unexpected format of reweight card in line: '+line)
                if not re.match(goodname_expression,match.group(1)):
                    raise RuntimeError('No special character in reweighting info/name, only allowing '+goodname_expression)
            if rwgt_info_match is not None:
                newcard.write(line)
            elif rwgt_name_match is not None:
                newcard.write(line.strip()+' --rwgt_info={0}\n'.format(rwgt_name_match.group(1)))
                changed=True
    if changed:
        mglog.info('Updated reweight_card')
    newcard.close()
    oldcard.close()


def helpful_SUSY_definitions():
    return """
# Define multiparticle labels
define p = g u c d s u~ c~ d~ s~
define j = g u c d s u~ c~ d~ s~
define pb = g u c d s b u~ c~ d~ s~ b~
define jb = g u c d s b u~ c~ d~ s~ b~
define l+ = e+ mu+
define l- = e- mu-
define vl = ve vm vt
define vl~ = ve~ vm~ vt~
define fu = u c e+ mu+ ta+
define fu~ = u~ c~ e- mu- ta-
define fd = d s ve~ vm~ vt~
define fd~ = d~ s~ ve vm vt
define susystrong = go ul ur dl dr cl cr sl sr t1 t2 b1 b2 ul~ ur~ dl~ dr~ cl~ cr~ sl~ sr~ t1~ t2~ b1~ b2~
define susyweak = el- el+ er- er+ mul- mul+ mur- mur+ ta1- ta1+ ta2- ta2+ n1 n2 n3 n4 x1- x1+ x2- x2+ sve sve~ svm svm~ svt svt~
define susylq = ul ur dl dr cl cr sl sr
define susylq~ = ul~ ur~ dl~ dr~ cl~ cr~ sl~ sr~
define susysq = ul ur dl dr cl cr sl sr t1 t2 b1 b2
define susysq~ = ul~ ur~ dl~ dr~ cl~ cr~ sl~ sr~ t1~ t2~ b1~ b2~
define susysl = el- el+ er- er+ mul- mul+ mur- mur+ ta1- ta1+ ta2- ta2+
define susyv = sve svm svt
define susyv~ = sve~ svm~ svt~
"""


def get_SUSY_variations( process , masses , syst_mod , ktdurham = None ):
    # Don't override an explicit setting from the run card!
    if ktdurham is None:
        prod_particles = []
        if process is not None:
            id_map = {'go':'1000021','dl':'1000001','ul':'1000002','sl':'1000003','cl':'1000004','b1':'1000005','t1':'1000006',
                      'dr':'2000001','ur':'2000002','sr':'2000003','cr':'2000004','b2':'2000005','t2':'2000006',
                      'n1':'1000022','n2':'1000023','x1':'1000024','x2':'1000037','n3':'1000025','n4':'1000035',
                      'el':'1000011','mul':'1000013','ta1':'1000015','sve':'1000012','svm':'1000014','svt':'1000016',
                      'er':'2000011','mur':'2000013','ta2':'2000015'}
            for l in process:
                if 'generate' in l or 'add process' in l:
                    clean_proc = l.replace('generate','').replace('+','').replace('-','').replace('~','').replace('add process','').split('>')[1].split(',')[0]
                    for particle in clean_proc.split():
                        if particle not in id_map:
                            mglog.info(f'Particle {particle} not found in PDG ID map - skipping')
                        else:
                            prod_particles += id_map[particle]
        # If we don't specify a process, then all we can do is guess based on available masses
        # Same if we failed to identify the right particles
        my_mass = 10000.
        if len(prod_particles)>0:
            for x in prod_particles:
                if x in masses:
                    my_mass = min(my_mass,abs(float(masses[x])))
                else:
                    mglog.info(f'Seem to ask for production of PDG ID {x}, but {x} not in mass dictionary?')
        if my_mass>9999.:
            strong_ids = ['1000001','1000002','1000003','1000004','1000005','1000006','2000001','2000002','2000003','2000004','2000005','2000006','1000021']
            weak_ids = ['1000023','1000024','1000025','1000011','1000013','1000015','2000011','2000013','2000015','1000012','1000014','1000016']
            # First check the lightest of the heavy sparticles - all squarks and gluino
            my_mass = min([abs(float(masses[x])) for x in strong_ids if x in masses])
            # Now check if strong production was not the key mode
            if my_mass>10000.:
                # This is a little tricky, but: we want the heaviest non-decoupled mass
                my_mass = max([abs(float(masses[x])) for x in weak_ids if x in masses and float(masses[x])<10000.])
            # Final check for N1N1 with everything else decoupled
            if my_mass>10000. and '1000022' in masses:
                my_mass = masses['1000022']
            if my_mass>10000.:
                raise RuntimeError('Could not understand which mass to use for matching cut in '+str(masses))

        # Now set the matching scale accordingly
        ktdurham = min(my_mass*0.25,500)
        # Should not be weirdly low - can't imagine a situation where you'd really want the scale below 15 GeV
        ktdurham = max(ktdurham,15)
        if syst_mod is not None and 'qup' in syst_mod.lower():
            ktdurham = ktdurham*2.
        elif syst_mod is not None and 'qdown' in syst_mod.lower():
            ktdurham = ktdurham*0.5

    mglog.info('For matching, will use ktdurham of '+str(ktdurham))

    alpsfact = 1.0
    scalefact = 1.0
    if syst_mod is not None and 'alpsfactup' in syst_mod.lower():
        alpsfact = 2.0
    elif syst_mod is not None and 'alpsfactdown' in syst_mod.lower():
        alpsfact = 0.5

    if syst_mod is not None and 'scalefactup' in syst_mod.lower():
        scalefact = 2.0
    elif syst_mod is not None and 'scalefactdown' in syst_mod.lower():
        scalefact = 0.5

    return abs(ktdurham) , alpsfact , scalefact


def SUSY_process(process=''):
    # Generate the new process!
    if 'import model' in process:
        mglog.info('Assuming that you have specified the model in your process string already')
        full_proc = ''
        for l in process.split('\n'):
            if 'import model' in l:
                full_proc += l+'\n'
                break
        # Only magically add helpful definitions if we are in the right model
        if 'MSSM_SLHA2' in full_proc:
            full_proc+=helpful_SUSY_definitions()
        for l in process.split('\n'):
            if 'import model' not in l:
                full_proc += l+'\n'
        full_proc+="""
# Output processes to MadEvent directory
output -f
"""
    else:
        full_proc = "import model MSSM_SLHA2\n"+helpful_SUSY_definitions()+"""
# Specify process(es) to run

"""+process+"""
# Output processes to MadEvent directory
output -f
"""
    return full_proc


def SUSY_Generation(runArgs = None, process=None, plugin=None,\
                    syst_mod=None, keepOutput=False, param_card=None, writeGridpack=False,\
                    madspin_card=None, run_settings={}, params={}, fixEventWeightsForBridgeMode=False, add_lifetimes_lhe=False, usePMGSettings=True):

    """
    Keyword Arguments:
        usePMGSettings (bool): See :py:func:`new_process`. Will set SM parameters to the appropriate values. Default: True.
    """
    ktdurham = run_settings['ktdurham'] if 'ktdurham' in run_settings else None
    ktdurham , alpsfact , scalefact = get_SUSY_variations( process, params['MASS'] , syst_mod , ktdurham=ktdurham )

    process_dir = MADGRAPH_GRIDPACK_LOCATION
    if not is_gen_from_gridpack():
        full_proc = SUSY_process(process)
        process_dir = new_process(full_proc, plugin=plugin, usePMGSettings=usePMGSettings)
    mglog.info('Using process directory '+str(process_dir))

    # Grab the param card and move the new masses into place
    modify_param_card(param_card_input=param_card,process_dir=process_dir,params=params)

    # Set up the extras dictionary
    settings = {'ktdurham':ktdurham,'scalefact':scalefact,'alpsfact':alpsfact}
    settings.update(run_settings) # This allows explicit settings in the input to override these settings

    # Set up the run card
    modify_run_card(process_dir=process_dir,runArgs=runArgs,settings=settings)

    # Set up madspin if needed
    if madspin_card is not None:
        if not os.access(madspin_card,os.R_OK):
            raise RuntimeError('Could not locate madspin card at '+str(madspin_card))
        shutil.copy(madspin_card,process_dir+'/Cards/madspin_card.dat')

    # Generate events!
    if is_gen_from_gridpack():
        generate_from_gridpack(runArgs=runArgs)
    else:
        # Grab the run card and move it into place
        generate(runArgs=runArgs,process_dir=process_dir,grid_pack=writeGridpack)

    # Add lifetimes to LHE before arranging output if requested
    if add_lifetimes_lhe :
        mglog.info('Requested addition of lifetimes to LHE files: doing so now.')
        if is_gen_from_gridpack():
            add_lifetimes()
        else:
            add_lifetimes(process_dir=process_dir)

    # Move output files into the appropriate place, with the appropriate name
    arrange_output(process_dir=process_dir,saveProcDir=keepOutput,runArgs=runArgs,fixEventWeightsForBridgeMode=fixEventWeightsForBridgeMode)

    mglog.info('All done generating events!!')
    return settings['ktdurham']


def update_lhe_file(lhe_file_old,param_card_old=None,lhe_file_new=None,masses={},delete_old_lhe=True):
    """Build a new LHE file from an old one and an updated param card.
    The masses of some particles can be changed via the masses dictionary.  No particles that appear in the events
    may have their masses changed.
    If the param card is provided, the decay block in the LHE file will be replaced with the one in the param card.
    By default, the old LHE file is removed.
    If None is provided as a new LHE file name, the new file will replace the old one."""
    # If we want to just use a temp file, then put in a little temp holder
    lhe_file_new_tmp = lhe_file_new if lhe_file_new is not None else lhe_file_old+'.tmp'
    # Make sure the LHE file is there
    if not os.access(lhe_file_old,os.R_OK):
        raise RuntimeError('Could not access old LHE file at '+str(lhe_file_old)+'. Please check the file location.')
    # Grab the old param card
    if param_card_old is not None:
        paramcard = subprocess.Popen(['get_files','-data',param_card_old])
        paramcard.wait()
        if not os.access(param_card_old,os.R_OK):
            raise RuntimeError('Could not get param card '+param_card_old)
    # Don't overwrite old param cards
    if os.access(lhe_file_new_tmp,os.R_OK):
        raise RuntimeError('Old file at'+str(lhe_file_new_tmp)+' in the current directory. Dont want to clobber it. Please move it first.')

    newlhe = open(lhe_file_new_tmp,'w')
    blockName = None
    decayEdit = False
    eventRead = False
    particles_in_events = []
    # Decay block ends with </slha>

    with open(lhe_file_old,'r') as fileobject:
        for line in fileobject:
            if decayEdit and '</slha>' not in line:
                continue
            if decayEdit and '</slha>' in line:
                decayEdit = False
            if line.strip().upper().startswith('BLOCK') or line.strip().upper().startswith('DECAY')\
                        and len(line.strip().split()) > 1:
                pos = 0 if line.strip().startswith('DECAY') else 1
                blockName = line.strip().upper().split()[pos]

            akey = None
            if blockName != 'DECAY' and len(line.strip().split()) > 0:
                akey = line.strip().split()[0]
            elif blockName == 'DECAY' and len(line.strip().split()) > 1:
                akey = line.strip().split()[1]

            # Replace the masses with those in the dictionary
            if akey is not None and blockName == 'MASS'  and akey in masses:
                newlhe.write('   '+akey+'    '+str(masses[akey])+'  # \n')
                mglog.info('   '+akey+'    '+str(masses[akey])+'  #')
                decayEdit = False
                continue

            # Replace the entire decay section of the LHE file with the one from the param card
            if blockName == 'DECAY' and param_card_old is not None:
                # We are now reading the decay blocks!  Take them from the param card
                oldparam = open(param_card_old,'r')
                newDecays = False
                for old_line in oldparam.readlines():
                    newBlockName = None
                    if old_line.strip().upper().startswith('DECAY') and len(old_line.strip().split()) > 1:
                        newBlockName = line.strip().upper().split()[pos]
                    if newDecays:
                        newlhe.write(old_line)
                    elif newBlockName == 'DECAY':
                        newDecays = True
                        newlhe.write(old_line)
                oldparam.close()
                # Done adding the decays
                decayEdit = True
                blockName = None
                continue

            # Keep a record of the particles that are in the events
            if not eventRead and '<event>' in line:
                eventRead = True
            if eventRead:
                if len(line.split())==11:
                    aparticle = line.split()[0]
                    if aparticle not in particles_in_events:
                        particles_in_events += [aparticle]

            # Otherwise write the line again
            newlhe.write(line)

    # Check that none of the particles that we were setting the masses of appear in the LHE events
    for akey in masses:
        if akey in particles_in_events:
            mglog.error('Attempted to change mass of a particle that was in an LHE event!  This is not allowed!')
            return -1

    # Close up and return
    newlhe.close()

    # Move the new file to the old file location
    if lhe_file_new is None:
        os.remove(lhe_file_old)
        shutil.move(lhe_file_new_tmp,lhe_file_old)
        lhe_file_new_tmp = lhe_file_old
    # Delete the old file if requested
    elif delete_old_lhe:
        os.remove(lhe_file_old)

    return lhe_file_new_tmp


def remap_lhe_pdgids(lhe_file_old,lhe_file_new=None,pdgid_map={},delete_old_lhe=True):
    """Update the PDG IDs used in an LHE file. This is a bit finicky, as we have to
    both touch the LHE file metadata _and_ modify the events themselves. But since this
    is "just" a remapping, it should be safe assuming Pythia8 is told the correct thing
    afterwards and can get the showering right."""
    # If we want to just use a temp file, then put in a little temp holder
    lhe_file_new_tmp = lhe_file_new if lhe_file_new is not None else lhe_file_old+'.tmp'
    # Make sure the LHE file is there
    if not os.access(lhe_file_old,os.R_OK):
        raise RuntimeError('Could not access old LHE file at '+str(lhe_file_old)+'. Please check the file location.')

    # Convert the map into a str:str map, no matter what we started with
    pdgid_map_str = { str(x) : str(pdgid_map[x]) for x in pdgid_map }
    # Add anti-particles if they aren't already there
    pdgid_map_str.update( { '-'+str(x) : '-'+str(pdgid_map[x]) for x in pdgid_map if '-'+str(x) not in pdgid_map } )

    newlhe = open(lhe_file_new_tmp,'w')
    blockName = None
    eventRead = False
    with open(lhe_file_old,'r') as fileobject:
        for line in fileobject:
            # In case we're reading the param section and we have a block, read the block name
            if line.strip().upper().startswith('BLOCK') or line.strip().upper().startswith('DECAY')\
                        and len(line.strip().split()) > 1:
                pos = 0 if line.strip().startswith('DECAY') else 1
                blockName = line.strip().upper().split()[pos]
            elif '</slha>' in line:
                blockName = None
            # Check for comments - just write those and move on
            if len(line.split('#')[0].strip())==0:
                line_mod = line
                for pdgid in pdgid_map_str:
                    if pdgid in line_mod.split():
                        line_mod = line_mod.replace( pdgid , pdgid_map_str[pdgid] )
                newlhe.write(line_mod)
                continue
            # Replace the PDG ID in the mass block
            if blockName=='MASS' and line.split()[0] in pdgid_map_str:
                newlhe.write( line.replace( line.split()[0] , pdgid_map_str[ line.split()[0] ] , 1 ) )
                continue
            if blockName=='DECAY' and line.split()[1] in pdgid_map_str:
                newlhe.write( line.replace( line.split()[1] , pdgid_map_str[ line.split()[1] ] , 1 ) )
                continue
            if blockName=='QNUMBERS' and line.split()[2] in pdgid_map_str:
                newlhe.write( line.replace( line.split()[2] , pdgid_map_str[ line.split()[2] ] , 1 ) )
                continue
            if '<event>' in line:
                eventRead = True
            if eventRead and len(line.split())==13 and line.split()[0] in pdgid_map_str:
                newlhe.write( line.replace( line.split()[0] , pdgid_map_str[ line.split()[0] ] , 1 ) )
                continue

            # Otherwise write the line again
            newlhe.write(line)

    # Move the new file to the old file location
    if lhe_file_new is None:
        os.remove(lhe_file_old)
        shutil.move(lhe_file_new_tmp,lhe_file_old)
        lhe_file_new_tmp = lhe_file_old
    # Delete the old file if requested
    elif delete_old_lhe:
        os.remove(lhe_file_old)

    return lhe_file_new_tmp


def find_key_and_update(akey,dictionary):
    """ Helper function when looking at param cards
    In some cases it's tricky to match keys - they may differ
    only in white space. This tries to sort out when we have
    a match, and then uses the one in blockParams afterwards.
    In the case of no match, it returns the original key.
    """
    test_key = ' '.join(akey.strip().replace('\t',' ').split())
    for key in dictionary:
        mod_key = ' '.join(key.strip().replace('\t',' ').split())
        if mod_key==test_key:
            return key
    return akey


def modify_param_card(param_card_input=None,param_card_backup=None,process_dir=MADGRAPH_GRIDPACK_LOCATION,params={},output_location=None):
    """Build a new param_card.dat from an existing one.
    Params should be a dictionary of dictionaries. The first key is the block name, and the second in the param name.
    Keys can include MASS (for masses) and DECAY X (for decays of particle X)"""
    # Grab the old param card and move it into place

    # Check for the default run card location
    if param_card_input is None:
        param_card_input=process_dir+'/Cards/param_card.dat'
    elif param_card_input is not None and not os.access(param_card_input,os.R_OK):
        paramcard = subprocess.Popen(['get_files','-data',param_card_input])
        paramcard.wait()
        if not os.access(param_card_input,os.R_OK):
            raise RuntimeError('Could not get param card '+param_card_input)
        mglog.info('Using input param card at '+param_card_input)

    #ensure all blocknames and paramnames are upper case
    for blockName in list(params.keys()):
       params[blockName.upper()] = params.pop(blockName)
       for paramName in list(params[blockName.upper()].keys()):
          params[blockName.upper()][paramName.upper()] = params[blockName.upper()].pop(paramName)

    if param_card_backup is not None:
        mglog.info('Keeping backup of original param card at '+param_card_backup)
        param_card_old = param_card_backup
    else:
        param_card_old = param_card_input+'.old_to_be_deleted'
    if os.path.isfile(param_card_old):
        os.unlink(param_card_old) # delete old backup
    os.rename(param_card_input, param_card_old) # change name of original card

    oldcard = open(param_card_old,'r')
    param_card_location= process_dir+'/Cards/param_card.dat' if output_location is None else output_location
    newcard = open(param_card_location,'w')
    decayEdit = False #only becomes true in a DECAY block when specifying the BR
    blockName = ""
    doneParams = {} #tracks which params have been done
    for linewithcomment in oldcard:
        line=linewithcomment.split('#')[0]
        if line.strip().upper().startswith('BLOCK') or line.strip().upper().startswith('DECAY')\
                    and len(line.strip().split()) > 1:
            if decayEdit and blockName == 'DECAY':
                decayEdit = False # Start a new DECAY block
            pos = 0 if line.strip().startswith('DECAY') else 1
            if blockName=='MASS' and 'MASS' in params:
                # Any residual masses to set?
                if "MASS" in doneParams:
                    leftOvers = [ x for x in params['MASS'] if x not in doneParams['MASS'] ]
                else:
                    leftOvers = [ x for x in params['MASS'] ]

                for pdg_id in leftOvers:
                    mglog.warning('Adding mass line for '+str(pdg_id)+' = '+str(params['MASS'][pdg_id])+' which was not in original param card')
                    newcard.write('   '+str(pdg_id)+'  '+str(params['MASS'][pdg_id])+'\n')
                    doneParams['MASS'][pdg_id]=True
            if blockName=='DECAY' and 'DECAY' not in line.strip().upper() and 'DECAY' in params:
                # Any residual decays to include?
                leftOvers = [ x for x in params['DECAY'] if x not in doneParams['DECAY'] ]
                for pdg_id in leftOvers:
                    mglog.warning('Adding decay for pdg id '+str(pdg_id)+' which was not in the original param card')
                    newcard.write( params['DECAY'][pdg_id].strip()+'\n' )
                    doneParams['DECAY'][pdg_id]=True
            blockName = line.strip().upper().split()[pos]
        if decayEdit:
            continue #skipping these lines because we are in an edit of the DECAY BR

        akey = None
        if blockName != 'DECAY' and len(line.strip().split()) > 0:
            # The line is already without the comment.
            # In the case of mixing matrices this is a bit tricky
            if len(line.split())==2:
                akey = line.upper().strip().split()[0]
            else:
                # Take everything but the last word
                akey = line.upper().strip()[:line.strip().rfind(' ')].strip()
        elif blockName == 'DECAY' and len(line.strip().split()) > 1:
            akey = line.strip().split()[1]
        if akey is None:
           newcard.write(linewithcomment)
           continue

        #check if we have params for this block
        if blockName not in params:
           newcard.write(linewithcomment)
           continue
        blockParams = params[blockName]
        # Check the spacing in the key
        akey = find_key_and_update(akey,blockParams)

        # look for a string key, which would follow a #
        stringkey = None
        if '#' in linewithcomment: #ignores comment lines
           stringkey = linewithcomment[linewithcomment.find('#')+1:].strip()
           if len(stringkey.split()) > 0:
               stringkey = stringkey.split()[0].upper()

        if akey not in blockParams and not (stringkey is not None and stringkey in blockParams):
           newcard.write(linewithcomment)
           continue

        if akey in blockParams and (stringkey is not None and stringkey in blockParams):
           raise RuntimeError('Conflicting use of numeric and string keys '+akey+' and '+stringkey)

        theParam = blockParams.get(akey,blockParams[stringkey] if stringkey in blockParams else None)
        if blockName not in doneParams:
            doneParams[blockName] = {}
        if akey in blockParams:
            doneParams[blockName][akey]=True
        elif stringkey is not None and stringkey in blockParams:
            doneParams[blockName][stringkey]=True

        #do special case of DECAY block
        if blockName=="DECAY":
           if theParam.splitlines()[0].split()[0].upper()=="DECAY":
               #specifying the full decay block
               for newline in theParam.splitlines():
                    newcard.write(newline+'\n')
                    mglog.info(newline)
               decayEdit = True
           else: #just updating the total width
              newcard.write('DECAY   '+akey+'    '+str(theParam)+'  # '+(linewithcomment[linewithcomment.find('#')+1:].strip() if linewithcomment.find('#')>0 else "")+'\n')
              mglog.info('DECAY   '+akey+'    '+str(theParam)+'  # '+(linewithcomment[linewithcomment.find('#')+1:].strip() if linewithcomment.find('#')>0 else "")+'\n')
        # second special case of QNUMBERS
        elif blockName=='QNUMBERS':
           #specifying the full QNUMBERS block
           for newline in theParam.splitlines():
                newcard.write(newline+'\n')
                mglog.info(newline)
           decayEdit = True
        else: #just updating the parameter
           newcard.write('   '+akey+'    '+str(theParam)+'  # '+(linewithcomment[linewithcomment.find('#')+1:].strip() if linewithcomment.find('#')>0 else "")+'\n')
           mglog.info('   '+akey+'    '+str(theParam)+'  # '+(linewithcomment[linewithcomment.find('#')+1:].strip() if linewithcomment.find('#')>0 else "")+'\n')
        # Done editing the line!

    #check that all specified parameters have been updated (helps to catch typos)
    for blockName in params:
       if blockName not in doneParams and len(params[blockName].keys())>0:
          raise RuntimeError('Did not find any of the parameters for block '+blockName+' in param_card')
       for paramName in params[blockName]:
          if paramName not in doneParams[blockName]:
            raise RuntimeError('Was not able to replace parameter '+paramName+' in param_card')

    # Close up and return
    oldcard.close()
    newcard.close()


def modify_run_card(run_card_input=None,run_card_backup=None,process_dir=MADGRAPH_GRIDPACK_LOCATION,runArgs=None,settings={},skipBaseFragment=False):
    """Build a new run_card.dat from an existing one.
    This function can get a fresh runcard from DATAPATH or start from the process directory.
    Settings is a dictionary of keys (no spaces needed) and values to replace.
    """
    if config_only_check():
        mglog.info('Running config-only. No proc card, so not operating on the run card.')
        return

    # Operate on lower case settings, and choose the capitalization MG5 has as the default (or all lower case)
    for s in list(settings.keys()):
        if s.lower() not in settings:
            settings[s.lower()] = settings[s]
            del settings[s]

    # Check for the default run card location
    if run_card_input is None:
        run_card_input=get_default_runcard(process_dir)
    elif run_card_input is not None and not os.access(run_card_input,os.R_OK):
        runcard = subprocess.Popen(['get_files','-data',run_card_input])
        runcard.wait()
        if not os.access(run_card_input,os.R_OK):
            raise RuntimeError('Could not get run card '+run_card_input)

    # guess NLO
    isNLO=is_NLO_run(process_dir=process_dir)
    # add gobal PDF and scale uncertainty config to extras, except PDF or weights for syscal config are explictly set
    if not skipBaseFragment:
        MadGraphControl.MadGraphSystematicsUtils.setup_pdf_and_systematic_weights(MADGRAPH_PDFSETTING,settings,isNLO)

    # Get some info out of the runArgs
    if runArgs is not None:
        beamEnergy,rand_seed = get_runArgs_info(runArgs)
        if 'iseed' not in settings:
            settings['iseed']=rand_seed
        if not isNLO and 'python_seed' not in settings:
            settings['python_seed']=rand_seed
        if 'beamenergy' in settings:
            mglog.warning('Do not set beam energy in MG settings. The variables are ebeam1 and ebeam2. Will use your setting of '+str(settings['beamenergy']))
            beamEnergy=settings['beamenergy']
            settings.pop('beamenergy')
        if 'ebeam1' not in settings:
            settings['ebeam1']=beamEnergy
        if 'ebeam2' not in settings:
            settings['ebeam2']=beamEnergy
    # Make sure nevents is an integer
    if 'nevents' in settings:
        settings['nevents'] = int(settings['nevents'])

    mglog.info('Modifying run card located at '+run_card_input)
    if run_card_backup is not None:
        mglog.info('Keeping backup of original run card at '+run_card_backup)
        run_card_old = run_card_backup
    else:
        run_card_old = run_card_input+'.old_to_be_deleted'
    mglog.debug('Modifying runcard settings: '+str(settings))
    if os.path.isfile(run_card_old):
        os.unlink(run_card_old) # delete old backup
    os.rename(run_card_input, run_card_old) # change name of original card

    oldCard = open(run_card_old, 'r')
    newCard = open(process_dir+'/Cards/run_card.dat', 'w')
    used_settings = []
    for line in iter(oldCard):
        if not line.strip().startswith('#'): # line commented out
            command = line.split('!', 1)[0]
            comment = line.split('!', 1)[1] if '!' in line else ''
            if '=' in command:
                setting = command.split('=')[-1] #.strip()
                stripped_setting = setting.strip()
                oldValue = '='.join(command.split('=')[:-1])
                if stripped_setting.lower() in settings:
                    # if setting set to 'None' it will be removed from run_card
                    if settings[stripped_setting.lower()] is None:
                        line=''
                        mglog.info('Removing '+stripped_setting+'.')
                        used_settings += [ stripped_setting.lower() ]
                    else:
                        line = oldValue.replace(oldValue.strip(), str(settings[stripped_setting.lower()]))+'='+setting
                        if comment != '':
                            line += '  !' + comment
                        mglog.info('Setting '+stripped_setting+' = '+str(settings[stripped_setting.lower()]))
                        used_settings += [ stripped_setting.lower() ]
        newCard.write(line.strip()+'\n')

    # Clean up unused options
    for asetting in settings:
        if asetting in used_settings:
            continue
        if settings[asetting] is None:
            continue
        mglog.warning('Option '+asetting+' was not in the default run_card.  Adding by hand a setting to '+str(settings[asetting]) )
        newCard.write( ' '+str(settings[asetting])+'   = '+str(asetting)+'\n')
    # close files
    oldCard.close()
    newCard.close()
    mglog.info('Finished modification of run card.')
    if run_card_backup is None:
        os.unlink(run_card_old)


def modify_config_card(config_card_backup=None,process_dir=MADGRAPH_GRIDPACK_LOCATION,settings={},set_commented=True):
    """Build a new configuration from an existing one.
    This function can get a fresh runcard from DATAPATH or start from the process directory.
    Settings is a dictionary of keys (no spaces needed) and values to replace.
    """
    # Check for the default config card location
    config_card=get_default_config_card(process_dir=process_dir)

    # The format is similar to the run card, but backwards
    mglog.info('Modifying config card located at '+config_card)
    if config_card_backup is not None:
        mglog.info('Keeping backup of original config card at '+config_card_backup)
        config_card_old = config_card_backup
    else:
        config_card_old = config_card+'.old_to_be_deleted'
    mglog.debug('Modifying config card settings: '+str(settings))
    if os.path.isfile(config_card_old):
        os.unlink(config_card_old) # delete old backup
    os.rename(config_card, config_card_old) # change name of original card

    oldCard = open(config_card_old, 'r')
    newCard = open(config_card, 'w')
    used_settings = []
    for line in iter(oldCard):
        lmod = line if set_commented else line.split('#')[0]
        if '=' in lmod:
            modified = False
            for setting in settings:
                if setting not in lmod:
                    continue
                # Assume we hit
                mglog.info('Setting '+setting.strip()+' to '+str(settings[setting]))
                newCard.write(' '+str(setting.strip())+' = '+str(settings[setting])+'\n')
                used_settings += [ setting.strip() ]
                modified = True
                break
            if modified:
                continue
        newCard.write(line)

    # Clean up unused options
    for asetting in settings:
        if asetting in used_settings:
            continue
        if settings[asetting] is None:
            continue
        mglog.warning('Option '+asetting+' was not in the default config card.  Adding by hand a setting to '+str(settings[asetting]) )
        newCard.write(' '+str(asetting)+' = '+str(settings[asetting])+'\n')
    # close files
    oldCard.close()
    newCard.close()
    mglog.info('Finished modification of config card.')
    if config_card_backup is None:
        os.unlink(config_card_old)


def print_cards_from_dir(process_dir=MADGRAPH_GRIDPACK_LOCATION):
    card_dir=process_dir+'/Cards/'
    print_cards(proc_card=card_dir+'proc_card_mg5.dat',run_card=card_dir+'run_card.dat',param_card=card_dir+'param_card.dat',\
                madspin_card=card_dir+'madspin_card.dat',reweight_card=card_dir+'reweight_card.dat',warn_on_missing=False)


def print_cards(proc_card='proc_card_mg5.dat',run_card=None,param_card=None,madspin_card=None,reweight_card=None,warn_on_missing=True):
    if os.access(proc_card,os.R_OK):
        mglog.info("proc_card:")
        procCard = subprocess.Popen(['cat',proc_card])
        procCard.wait()
    elif warn_on_missing:
        mglog.warning('No proc_card: '+proc_card+' found')

    if run_card is not None and os.access(run_card,os.R_OK):
        mglog.info("run_card:")
        runCard = subprocess.Popen(['cat',run_card])
        runCard.wait()
    elif run_card is not None and warn_on_missing:
        mglog.warning('No run_card: '+run_card+' found')
    else:
        mglog.info('Default run card in use')

    if param_card is not None and os.access(param_card,os.R_OK):
        mglog.info("param_card:")
        paramCard = subprocess.Popen(['cat',param_card])
        paramCard.wait()
    elif param_card is not None and warn_on_missing:
        mglog.warning('No param_card: '+param_card+' found')
    else:
        mglog.info('Default param card in use')

    if madspin_card is not None and os.access(madspin_card,os.R_OK):
        mglog.info("madspin_card:")
        madspinCard = subprocess.Popen(['cat',madspin_card])
        madspinCard.wait()
    elif madspin_card is not None and warn_on_missing:
        mglog.warning('No madspin_card: '+madspin_card+' found')
    else:
        mglog.info('No madspin card in use')

    if reweight_card is not None and os.access(reweight_card,os.R_OK):
        mglog.info("reweight_card:")
        madspinCard = subprocess.Popen(['cat',reweight_card])
        madspinCard.wait()
    elif reweight_card is not None and warn_on_missing:
        mglog.warning('No reweight_card: '+reweight_card+' found')
    else:
        mglog.info('No reweight card in use')


def is_gen_from_gridpack():
    """ Simple function for checking if there is a grid pack.
    Relies on the specific location of the unpacked gridpack (madevent)
    which is here set as a global variable. The gridpack is untarred by
    the transform (Gen_tf.py) and no sign is sent to the job itself
    that there is a gridpack in use except the file's existence"""
    if os.access(MADGRAPH_GRIDPACK_LOCATION,os.R_OK):
        mglog.info('Located input grid pack area')
        return True
    return False


def get_default_config_card(process_dir=MADGRAPH_GRIDPACK_LOCATION):
    if config_only_check():
        mglog.info('Athena running on config only mode: grabbing config card the old way, as there will be no proc dir')
        if os.access(os.environ['MADPATH']+'/input/mg5_configuration.txt',os.R_OK):
            shutil.copy(os.environ['MADPATH']+'/input/mg5_configuration.txt','local_mg5_configuration.txt')
            return 'local_mg5_configuration.txt'

    lo_config_card=process_dir+'/Cards/me5_configuration.txt'
    nlo_config_card=process_dir+'/Cards/amcatnlo_configuration.txt'

    if os.access(lo_config_card,os.R_OK) and not os.access(nlo_config_card,os.R_OK):
        return lo_config_card
    elif os.access(nlo_config_card,os.R_OK) and not os.access(lo_config_card,os.R_OK):
        return nlo_config_card
    elif os.access(nlo_config_card,os.R_OK) and os.access(lo_config_card,os.R_OK):
        mglog.error('Found both types of config card in '+process_dir)
    else:
        mglog.error('No config card in '+process_dir)
    raise RuntimeError('Unable to locate configuration card')


def get_cluster_type(process_dir=MADGRAPH_GRIDPACK_LOCATION):
    card_in = open(get_default_config_card(process_dir=process_dir),'r')
    for l in card_in.readlines():
        if 'cluster_type' not in l.split('#')[0]:
            continue
        cluster_type = l.split('#')[0].split('=')[1]
        mglog.info('Returning cluster type: '+cluster_type)
        return cluster_type
    return None


def is_NLO_run(process_dir=MADGRAPH_GRIDPACK_LOCATION):
    # Very simple check based on the above config card grabbing
    if config_only_check():
        return False
    return get_default_config_card(process_dir=process_dir)==process_dir+'/Cards/amcatnlo_configuration.txt'


def run_card_consistency_check(isNLO=False,process_dir='.'):
    cardpath=process_dir+'/Cards/run_card.dat'
    mydict=getDictFromCard(cardpath)

    # We should always use event_norm = average [AGENE-1725] otherwise Pythia cross sections are wrong
    # Modification: average or bias is ok; sum is incorrect. Change the test to set sum to average
    if checkSetting('event_norm','sum',mydict):
        modify_run_card(process_dir=process_dir,settings={'event_norm':'average'},skipBaseFragment=True)
        mglog.warning("setting event_norm to average, there is basically no use case where event_norm=sum is a good idea")

    if not isNLO:
        #Check CKKW-L setting
        if 'ktdurham' in mydict and float(mydict['ktdurham']) > 0 and int(mydict['ickkw']) != 0:
            log='Bad combination of settings for CKKW-L merging! ktdurham=%s and ickkw=%s.'%(mydict['ktdurham'],mydict['ickkw'])
            mglog.error(log)
            raise RuntimeError(log)

        # Check if user is trying to use deprecated syscalc arguments with the other systematics script
        if 'systematics_program' not in mydict or mydict['systematics_program']=='systematics':
            syscalc_settings=['sys_pdf', 'sys_scalefact', 'sys_alpsfact', 'sys_matchscale']
            found_syscalc_setting=False
            for s in syscalc_settings:
                if s in mydict:
                    mglog.warning('Using syscalc setting '+s+' with new systematics script. Systematics script is default from 2.6.2 and steered differently (https://cp3.irmp.ucl.ac.be/projects/madgraph/wiki/Systematics#Systematicspythonmodule)')
                    found_syscalc_setting=True
            if found_syscalc_setting:
                syst_arguments=MadGraphControl.MadGraphSystematicsUtils.convertSysCalcArguments(mydict)
                mglog.info('Converted syscalc arguments to systematics arguments: '+syst_arguments)
                syst_settings_update={'systematics_arguments':syst_arguments}
                for s in syscalc_settings:
                    syst_settings_update[s]=None
                modify_run_card(process_dir=process_dir,settings=syst_settings_update,skipBaseFragment=True)


    # usually the pdf and systematics should be set during modify_run_card
    # but check again in case the user did not call the function or provides a different card here
    mglog.info('Checking PDF and systematics settings')
    if not MadGraphControl.MadGraphSystematicsUtils.base_fragment_setup_check(MADGRAPH_PDFSETTING,mydict,isNLO):
        # still need to set pdf and systematics
        syst_settings=MadGraphControl.MadGraphSystematicsUtils.get_pdf_and_systematic_settings(MADGRAPH_PDFSETTING,isNLO)
        modify_run_card(process_dir=process_dir,settings=syst_settings,skipBaseFragment=True)

    mydict_new=getDictFromCard(cardpath)
    if 'systematics_arguments' in mydict_new:
        systematics_arguments=MadGraphControl.MadGraphSystematicsUtils.parse_systematics_arguments(mydict_new['systematics_arguments'])
        if 'weight_info' not in systematics_arguments:
            mglog.info('Enforcing systematic weight name convention')
            dyn = None
            if '--dyn' in systematics_arguments or ' dyn' in systematics_arguments:
                if '--dyn' in systematics_arguments:
                    dyn = systematics_arguments.split('--dyn')[1]
                if ' dyn' in systematics_arguments:
                    dyn = systematics_arguments.split(' dyn')[1]
                dyn = dyn.replace('\'',' ').replace('=',' ').split()[0]
            if dyn is not None and len(dyn.split(','))>1:
                systematics_arguments['weight_info']=MadGraphControl.MadGraphSystematicsUtils.SYSTEMATICS_WEIGHT_INFO_ALTDYNSCALES
            else:
                systematics_arguments['weight_info']=MadGraphControl.MadGraphSystematicsUtils.SYSTEMATICS_WEIGHT_INFO
            modify_run_card(process_dir=process_dir,settings={'systematics_arguments':MadGraphControl.MadGraphSystematicsUtils.write_systematics_arguments(systematics_arguments)},skipBaseFragment=True)

    if not isNLO:
        if 'python_seed' not in mydict:
            mglog.warning('No python seed set in run_card -- adding one with same value as iseed')
            modify_run_card(process_dir=process_dir,settings={'python_seed':mydict['iseed']},skipBaseFragment=True)

    mglog.info('Finished checking run card - All OK!')

def add_reweighting(run_name,reweight_card=None,process_dir=MADGRAPH_GRIDPACK_LOCATION):
    mglog.info('Running reweighting module on existing events')
    if reweight_card is not None:
        mglog.info('Copying new reweight card from '+reweight_card)
        shutil.move(reweight_card,process_dir+'/Cards/reweight_card.dat')
    reweight_cmd='{}/bin/madevent reweight {} -f'.format(process_dir,run_name)
    global MADGRAPH_CATCH_ERRORS
    reweight = stack_subprocess([python]+reweight_cmd.split(),stdin=subprocess.PIPE,stderr=subprocess.PIPE if MADGRAPH_CATCH_ERRORS else None)
    (out,err) = reweight.communicate()
    error_check(err,reweight.returncode)
    mglog.info('Finished reweighting')

def check_reset_proc_number(opts):
    if 'ATHENA_PROC_NUMBER' in os.environ and int(os.environ['ATHENA_PROC_NUMBER'])>0:
        mglog.info('Noticed that you have run with an athena MP-like whole-node setup.  Will re-configure now to make sure that the remainder of the job runs serially.')
        # Try to modify the opts underfoot
        if not hasattr(opts,'nprocs'):
            mglog.warning('Did not see option!')
        else:
            opts.nprocs = 0
        mglog.debug(str(opts))


def ls_dir(directory):
    mglog.info('For your information, ls of '+directory+':')
    mglog.info( sorted( os.listdir( directory ) ) )

# Final import of some code used in these functions
import MadGraphControl.MadGraphSystematicsUtils

# To be removed once we moved past MG5 3.3.1
def fix_fks_makefile(process_dir):
    makefile_fks=process_dir+'/SubProcesses/makefile_fks_dir'
    mglog.info('Fixing '+makefile_fks)
    shutil.move(makefile_fks,makefile_fks+'_orig')
    fin=open(makefile_fks+'_orig')
    fout=open(makefile_fks,'w')
    edit=False
    for line in fin:
        if 'FKSParams.mod' in line:
            fout.write(line.replace('FKSParams.mod','FKSParams.o'))
            edit=True
        elif edit and 'driver_mintFO' in line:
            fout.write('driver_mintFO.o: weight_lines.o mint_module.o FKSParams.o\n')
        elif edit and 'genps_fks.o' in line:
            fout.write('genps_fks.o: mint_module.o FKSParams.o\n')
        elif edit and 'test_soft_col_limits' in line:
            fout.write(line)
            fout.write('madfks_plot.o: mint_module.o\n')
            fout.write('cluster.o: weight_lines.o\n')
        else:
            fout.write(line)
    fin.close()
    fout.close()
