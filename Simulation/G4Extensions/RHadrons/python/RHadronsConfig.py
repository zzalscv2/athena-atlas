# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon import Logging
import os

rhlog = Logging.logging.getLogger('RHadronConfig')

def create_rhadron_particles_file(input_param_card='SLHA_INPUT.DAT',spectrum=1):
    """Create a list of particles for custom particle creation"""
    # Just use our helper function
    from RHadrons.RHadronMasses import update_particle_table
    update_particle_table(input_param_card, 'particles.txt', mass_spectrum=spectrum)
    if not os.path.isfile('particles.txt'):
        raise RuntimeError('Failed to create particles.txt file - will abort')


def create_rhadron_pdgtable(input_param_card='SLHA_INPUT.DAT',spectrum=1):
    """Add lines to the PDG table"""

    from ExtraParticles.PDGHelpers import getPDGTABLE
    if getPDGTABLE('PDGTABLE.MeV'): # FIXME make configurable
        # Update the PDG table using our helper function
        from RHadrons.RHadronMasses import update_PDG_table
        update_PDG_table(input_param_card,'PDGTABLE.MeV',spectrum)


def addLineToPhysicsConfiguration(KEY, VALUE):
    """Add lines to the physics configuration"""
    os.system('touch PhysicsConfiguration.txt')
    newphysconfig = "{key} = {value}".format(key=KEY, value=VALUE)
    os.system('echo "%s" >> PhysicsConfiguration.txt' % newphysconfig)


@AccumulatorCache
def load_files_for_rhadrons_scenario(input_param_card='SLHA_INPUT.DAT',spectrum=1):
    """ Load all the files needed for a given scenario"""
    if not os.path.isfile(input_param_card):
        raise RuntimeError('input_param_card file is missing - will abort')
     # Create custom PDGTABLE.MeV file
    create_rhadron_pdgtable(input_param_card,spectrum)
    # Create particles.txt file
    create_rhadron_particles_file(input_param_card,spectrum)
    from RHadrons.RHadronMasses import get_interaction_list
    get_interaction_list(input_param_card, interaction_file='ProcessList.txt', mass_spectrum=spectrum)
    # Remove existing physics configuration file ([MDJ]: FIXME: Is this happening earlier, or is it needed?)
    if os.path.isfile('PhysicsConfiguration.txt'):
        rhlog.warning("load_files_for_rhadrons_scenario() Found pre-existing PhysicsConfiguration.txt file - deleting.")
        os.remove('PhysicsConfiguration.txt')


def SG_StepNtupleTool(flags, name="G4UA::SG_StepNtupleTool", **kwargs):
    result = ComponentAccumulator()
    if flags.Concurrency.NumThreads >1:
        log=Logging.logging.getLogger(name)
        log.fatal(' Attempt to run '+name+' with more than one thread, which is not supported')
        return False
    # Get the PDG IDs for RHadrons
    from RHadronMasses import offset_options
    kwargs.setdefault('RHadronPDGIDList',offset_options.keys())
    ## if name in simFlags.UserActionConfig.get_Value().keys(): ## FIXME missing functionality
    ##     for prop,value in simFlags.UserActionConfig.get_Value()[name].items():
    ##         kwargs.setdefault(prop,value)
    result.setPrivateTools( CompFactory.G4UA__SG_StepNtupleTool(name, **kwargs) )
    return result


def RHadronsPhysicsToolCfg(flags, name='RHadronsPhysicsTool', **kwargs):
    result = ComponentAccumulator()
    result.setPrivateTools( CompFactory.RHadronsPhysicsTool(name,**kwargs) )
    return result


def determineLegacyGeneratorJobOptions(runNumber):
    # From the run number, load up the configuration.  Not the most beautiful thing, but this works.
    from glob import glob
    # Default position: look in cvmfs for job options
    if(runNumber ==  449497 ):
       runNumber =  421442
       rhlog.info('run number changed from 449497 to ', str(runNumber))

    cvmfs_mc16 = '/cvmfs/atlas.cern.ch/repo/sw/Generators/MC16JobOptions/'

    JO = glob(cvmfs_mc16+str(int(runNumber/1000))+'xxx/'+str(runNumber)+'/mc.'+'*.py')

    JO_path = cvmfs_mc16+str(int(runNumber/1000))+'xxx/'+str(runNumber)
    os.environ["JOBOPTSEARCHPATH"] = JO_path +":"+os.environ["JOBOPTSEARCHPATH"]
    os.environ["DATAPATH"] = JO_path +":"+os.environ["DATAPATH"]
    if len(JO)>0:
        JO = JO[0]
    else:
        # Miss.  Try local in dir=DSID
        JO = glob(str(runNumber)+'/mc.'+'*.py')
        if len(JO)>0: JO=JO[0]
        else:
            # Miss.  Try one directory deeper (any name)
            JO = glob('*/mc.'+str(runNumber)+'*.py')
            if len(JO)>0: JO=JO[0]
            else:
                # Miss.  Try local
                JO = glob('mc.'+'*.py')
                if len(JO)>0: JO=JO[0]
                else:
                    # Miss.  Fall back to datapath
                    for adir in os.environ['DATAPATH'].split(":"):
                       JO = glob(adir+'/mc.'+'*.py')
                       if len(JO)>0:
                          JO=JO[0]
                          break
    if not JO:
        raise RuntimeError('Could not locate job options for DSID '+str(runNumber))
    return JO


def buildGeneratorConfigurationFiles(flags):
    UseLegacyConfig = False # Blocked for now as we need a non-legacy configuration alternative
    if UseLegacyConfig:
        ## Legacy configuration starts here:
        JO = determineLegacyGeneratorJobOptions(flags.Input.MCChannelNumber)
        from PyJobTransforms.trfJobOptions import RunArguments
        runArgs = RunArguments()
        runArgs.trfSubstepName = 'EVNTtoHITS'
        # add any necessary elements to the runArgs here!
        runArgs.jobConfig = [JO.split('/')[-2] if '/' in JO else JO]
        runArgs.runNumber = flags.Input.MCChannelNumber
        runArgs.ecmEnergy = 13000. # TODO Best to use (2.0 * flags.Beam.Energy) instead?
        runArgs.randomSeed = 1234
        runArgs.generatorJobNumber = 0 # Workaround for ATLASSIM-6687
        # Set up evgenLog logger - use this one
        evgenLog=rhlog  # noqa: F841
        # Set up evgenConfig just for a holder
        class dummyClass():
            def __init(self):
                pass
            keywords = [] # So that they can be +='d in
        evgenConfig = dummyClass()
        evgenConfig.generators = []
        evgenConfig.auxfiles = []
        evgenConfig.nEventsPerJob = flags.Exec.MaxEvents # Workaround for ATLASSIM-6687
        # Set up a fake pythia8...
        genSeq = dummyClass()
        genSeq.Pythia8 = dummyClass()
        # Standard list of commands stolen from the Pythia8 base fragment
        genSeq.Pythia8.Commands = [
            "6:m0 = 172.5",
            "23:m0 = 91.1876",
            "23:mWidth = 2.4952",
            "24:m0 = 80.399",
            "24:mWidth = 2.085",
            "StandardModel:sin2thetaW = 0.23113",
            "StandardModel:sin2thetaWbar = 0.23146",
            "ParticleDecays:limitTau0 = on",
            "ParticleDecays:tau0Max = 10.0"]
        # Set up a fake TestHepMC
        testSeq = dummyClass()
        testSeq.TestHepMC = dummyClass()
        # Block includes that we don't want running
        from AthenaCommon import Include
        include = Include.include # HACK??
        include.block('MadGraphControl/MadGraphControl_SimplifiedModelPostInclude.py')
        include.block('Pythia8_i/Pythia8_Base_Fragment.py')
        include.block('Pythia8_i/Pythia8_EvtGen.py')
        include.block('Pythia8_i/Pythia8_LHEF.py')

        # Updating JOBOPTSEARCHPATH env var on the athena side
        import re, os
        Include.optionsPathEnv = os.environ[ 'JOBOPTSEARCHPATH' ]
        Include.optionsPath = re.split( ',|' + os.pathsep, Include.optionsPathEnv )

        # Include the job options themselves
        include(JO) # FIXME currently fails in CA one layer of includes down.

        from glob import glob
        # Add param_cards.dat to the DATAPATH
        cwd_path = os.getcwd()
        cwd_sub = glob.glob(cwd_path + "/PROC*/Cards", recursive = True)
        cwd_sub_str = ' '.join(str(e)+":" for e in cwd_sub)
        os.environ["DATAPATH"] = cwd_sub_str +":"+os.environ["DATAPATH"]

        # Build the param card, aka SLHA file
        from MadGraphControl.MadGraphUtils import modify_param_card
        modify_param_card(param_card_input='param_card.dat',params={'MASS': masses,'DECAY':decays},output_location='SLHA_INPUT.DAT') # noqa: F821

        simdict = flags.Input.SpecialConfiguration
        # Get the spectrum number if it's in the metadata
        spectrum = 1 if 'SPECTRUM' not in simdict else simdict['SPECTRUM']

        # Last step, load up the files
        load_files_for_rhadrons_scenario('SLHA_INPUT.DAT',spectrum)

        # Add any lines that were missing
        # In case we want to use Pythia8 for decays during simulation
        lifetime = float(simdict['LIFETIME']) if "LIFETIME" in simdict else -1.
        if lifetime>0.:
            if lifetime<1. and hasattr(runArgs,'outputEVNT_TRFile'): # FIXME this will not work in CA
                rhlog.warning('Lifetime specified at <1ns, but you are writing stopped particle positions.')
                rhlog.warning('Assuming that you mean to use infinite lifetimes, and ignoring the setting')
            else:
                addLineToPhysicsConfiguration("DoDecays","1")
                addLineToPhysicsConfiguration("HadronLifeTime", str(lifetime))
            # If we are reading particle records, and the lifetime is short, stop them as well
            if lifetime<1. and hasattr(runArgs,'inputEVNT_TRFile'): # FIXME this will not work in CA
                addLineToPhysicsConfiguration("DoDecays","1")
                addLineToPhysicsConfiguration("HadronLifeTime", 0.000001)
        else:
            # Stable case. Can be unset lifetime or lifetime=0 or lifetime=-1
            addLineToPhysicsConfiguration("DoDecays","0")
            addLineToPhysicsConfiguration("HadronLifeTime", -1)

        # Set up R-hadron masses in Pythia8
        from RHadrons.RHadronMasses import get_Pythia8_commands
        genSeq.Pythia8.Commands += get_Pythia8_commands('SLHA_INPUT.DAT',spectrum)
        f = open('PYTHIA8_COMMANDS.TXT','w')
        f.write('\n'.join(genSeq.Pythia8.Commands))
        f.close()
        # Done with the Pythia8 setup


def RHadronsPreInclude(flags):
    print ("Start of RHadronsPreInclude")
    if 'Py8' not in flags.Input.GeneratorsInfo and 'Pythia8' not in flags.Input.GeneratorsInfo:
        raise RuntimeError('Pythia8 not found in generator metadata - will abort')
    simdict = flags.Input.SpecialConfiguration

    ## Eventually this method should create SLHA_INPUT.DAT,
    ## PhysicsConfiguration.txt and PYTHIA8_COMMANDS.TXT in the run
    ## directory
    buildGeneratorConfigurationFiles(flags)

    # Get the spectrum number if it's in the metadata
    spectrum = 1 if 'SPECTRUM' not in simdict else simdict['SPECTRUM']

    # Last step, load up the files
    load_files_for_rhadrons_scenario('SLHA_INPUT.DAT',spectrum)

    # Add any lines that were missing
    # In case we want to use Pythia8 for decays during simulation
    lifetime = float(simdict['LIFETIME']) if "LIFETIME" in simdict else -1.
    if lifetime>0.:
        addLineToPhysicsConfiguration("DoDecays","1")
        addLineToPhysicsConfiguration("HadronLifeTime", str(lifetime))
    else:
        # Stable case. Can be unset lifetime or lifetime=0 or lifetime=-1
        addLineToPhysicsConfiguration("DoDecays","0")
        addLineToPhysicsConfiguration("HadronLifeTime", -1)

    # Check for the presence of the other files needed at run-time
    if not os.path.isfile('ProcessList.txt'):
        raise RuntimeError('ProcessList.txt (needed by G4ProcessHelper) is missing - will abort')
    if not os.path.isfile('PhysicsConfiguration.txt'):
        raise RuntimeError('PhysicsConfiguration.txt (needed by G4ProcessHelper) is missing - will abort')
    print ("End of RHadronsPreInclude")


def RHadronsCfg(flags):
    result = ComponentAccumulator()
    print("Running RHadronsCfg")
    ## simdict = flags.Input.SpecialConfiguration # TODO will need this!
    from AthenaConfiguration.Enums import ProductionStep
    if flags.Common.ProductionStep == ProductionStep.Simulation:
        from G4AtlasServices.G4AtlasServicesConfig import PhysicsListSvcCfg
        result.merge(PhysicsListSvcCfg(flags))
        physicsOptions = [ result.popToolsAndMerge(RHadronsPhysicsToolCfg(flags)) ]
        result.getService("PhysicsListSvc").PhysOption += physicsOptions
    return result
