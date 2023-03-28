#################################################################
#       preInclude.SLeptonsLLP.py - Emma Kuwertz, Dec 2017                #
#################################################################

def get_and_fix_PDGTABLE(replace):
    import os, shutil, re, sys

    # Download generic PDGTABLE (do not overwrite existing one if it exists, use existing one instead) 
    from G4AtlasApps.SimFlags import simFlags
    from ExtraParticles.PDGHelpers import getPDGTABLE
    if getPDGTABLE(simFlags.ExtraParticlesPDGTABLE.get_Value()):
        shutil.move('PDGTABLE.MeV', 'PDGTABLE.MeV.org')

    # an example line to illustrate the fixed format, see PDGTABLE.MeV for more details
        # M 1000022                          0.E+00         +0.0E+00 -0.0E+00 ~chi(0,1)     0

        lines = open('PDGTABLE.MeV.org').readlines()
        for pdgid,mass,name,charge in replace:
            if not re.search(r'[MW]\s+'+str(pdgid)+'\s+\S+', ''.join(lines)):
                lines.append('M' + str(pdgid).rjust(8) +''.ljust(26) +
                             ('%11.5E' % mass).ljust(15) +
                             '+0.0E+00'.ljust(9) + '-0.0E+00'.ljust(9) +
                             name.strip() + ''.ljust(6) + charge.strip()+''.rjust(20-len(name.strip())) + '\n')
                lines.append('W' + str(pdgid).rjust(8) +''.ljust(26) +
                             '0.E+00'.ljust(15) + '+0.0E+00'.ljust(9) + '-0.0E+00'.ljust(9) +
                             name.strip() + ''.ljust(6) + charge.strip()+''.rjust(20-len(name.strip())) + '\n')
            else:
                from past.builtins import xrange # Temporary workaround for python3 compatibility use range in CA-based config
                for i in xrange(len(lines)):
                    if re.search(r'M\s+'+str(pdgid)+'\s+\S+', lines[i]):
                        l = lines[i]
                        lines[i] = l[0:35] + ('%11.5E' % mass).ljust(14) + l[49:]

        update = open('PDGTABLE.MeV', 'w')
        update.write(''.join(lines))
        update.close()

        print ('modified PDGTABLE\n%s\n' % ''.join(lines))
        sys.stdout.flush()


def load_files_for_sleptonLLP_scenario(simdict):
    pdgcodes = []
    if "GMSBSlepton" in simdict:
        get_and_fix_PDGTABLE([
                (2000011, eval(simdict.get("GMSBSlepton",'0')), '~e(R)', '-'),
                (2000013, eval(simdict.get("GMSBSlepton",'0')), '~mu(R)', '-'),
                (1000011, eval(simdict.get("GMSBSlepton",'0')), '~e(L)', '-'),
                (1000013, eval(simdict.get("GMSBSlepton",'0')), '~mu(L)', '-'),
                ])
        pdgcodes += [-2000011,2000011,-2000013,2000013,-1000011,1000011,-1000013,1000013]
    if "GMSBStau" in simdict:
        get_and_fix_PDGTABLE([
                (2000015, eval(simdict.get("GMSBStau",'0')), '~tau(R)', '-'),
                (1000015, eval(simdict.get("GMSBStau",'0')), '~tau(L)', '-'),
                ])
        pdgcodes += [-2000015,2000015,-1000015,1000015]
    if "GMSBGravitino" in simdict:
        get_and_fix_PDGTABLE([
                (1000039, eval(simdict.get("GMSBGravitino",'0')), '~G', '0'),
                ])
        pdgcodes += [1000039]
    if "coannihilationStau" in simdict:
        get_and_fix_PDGTABLE([
                (2000015, eval(simdict.get("coannihilationStau",'0')), '~tau(R)', '-'),
                (1000015, eval(simdict.get("coannihilationStau",'0')), '~tau(L)', '-'),
                ])
        pdgcodes += [-2000015,2000015,-1000015,1000015]
    if "coannihilationSlepton" in simdict:
        get_and_fix_PDGTABLE([
            (2000011, eval(simdict.get("coannihilationSlepton", '0')), '~e(R)', '-'),
            (2000013, eval(simdict.get("coannihilationSlepton", '0')), '~mu(R)', '-'),
            (1000011, eval(simdict.get("coannihilationSlepton", '0')), '~e(L)', '-'),
            (1000013, eval(simdict.get("coannihilationSlepton", '0')), '~mu(L)', '-'),
        ])
        pdgcodes += [-2000011, 2000011, -2000013, 2000013, -1000011, 1000011, -1000013, 1000013]
    if "coannihilationNeutralino" in simdict:
        get_and_fix_PDGTABLE([
                (1000022, eval(simdict.get("coannihilationNeutralino", '0')), '~chi(0,1)', '0'),
                ])
        pdgcodes += [1000022]

    from ExtraParticles.PDGHelpers import updateExtraParticleWhiteList
    updateExtraParticleWhiteList('G4particle_whitelist_ExtraParticles.txt', pdgcodes)

doG4SimConfig = True
from AthenaCommon.AthenaCommonFlags import athenaCommonFlags
import PyUtils.AthFile as af
try:
    f = af.fopen(athenaCommonFlags.FilesInput()[0])

    if "StreamHITS" in f.infos["stream_names"]:
        from Digitization.DigitizationFlags import digitizationFlags
        simdict = digitizationFlags.specialConfiguration.get_Value()
        if simdict is None:
            # Here we are in a ReSim job, so the input is a HITS file
            raise ValueError
        doG4SimConfig = False
    else:
        from G4AtlasApps.SimFlags import simFlags
        simdict = simFlags.specialConfiguration.get_Value()
except:
    from G4AtlasApps.SimFlags import simFlags
    simdict = simFlags.specialConfiguration.get_Value()

load_files_for_sleptonLLP_scenario(simdict)

if doG4SimConfig:
    localPhysicsOptions = ["GauginosPhysicsTool","AllSleptonsPhysicsTool"]
    # Slepton decays from SleptonsConfig
    if "GMSBSlepton" in simdict:
        localPhysicsOptions += ["SElectronRPlusToElectronGravitino","SElectronLPlusToElectronGravitino"]
        localPhysicsOptions += ["SElectronRMinusToElectronGravitino","SElectronLMinusToElectronGravitino"]
        localPhysicsOptions += ["SMuonRPlusToMuonGravitino","SMuonLPlusToMuonGravitino"]
        localPhysicsOptions += ["SMuonRMinusToMuonGravitino","SMuonLMinusToMuonGravitino"]
    if "GMSBStau" in simdict:
        localPhysicsOptions += ["STauRPlusToTauGravitino","STauLPlusToTauGravitino"]
        localPhysicsOptions += ["STauRMinusToTauGravitino","STauLMinusToTauGravitino"]
    if "coannihilationStau" in simdict:
        MassStau = eval(simdict.get("coannihilationStau",'0'))
        MassNeutralino = eval(simdict.get("coannihilationNeutralino",'0'))
        MassTau = 1776. # MeV
        if MassStau > MassNeutralino + MassTau: ## Check that there is energy to decay to tau.
            localPhysicsOptions += ["STauRPlusToTauNeutralino", "STauLPlusToTauNeutralino"]
            localPhysicsOptions += ["STauRMinusToTauNeutralino", "STauLMinusToTauNeutralino"]
        else: ## Do off shell decay for tau.
            # FIXME Dislike hard-coded numbers here
            if abs(MassStau-MassNeutralino - 300) > 0.05 and abs(MassStau-MassNeutralino - 1700) > 0.05:
                print('Warning: Branching ratios are wrong. Mass splitting of stau neutralino %s currently has no available values.' % (MassStau - MassNeutralino))
            localPhysicsOptions += ["STauRMinusToPionMinusNeutralino", "STauRPlusToPionPlusNeutralino"]
            localPhysicsOptions += ["STauLMinusToPionMinusNeutralino", "STauLPlusToPionPlusNeutralino"]
            localPhysicsOptions += ["STauRMinusToRhoMinusNeutralino", "STauRPlusToRhoPlusNeutralino"]
            localPhysicsOptions += ["STauLMinusToRhoMinusNeutralino", "STauLPlusToRhoPlusNeutralino"]
            localPhysicsOptions += ["STauRMinusToEMinusNeutralino", "STauRPlusToEPlusNeutralino"]
            localPhysicsOptions += ["STauLMinusToEMinusNeutralino", "STauLPlusToEPlusNeutralino"]
            localPhysicsOptions += ["STauRMinusToMuMinusNeutralino", "STauRPlusToMuPlusNeutralino"]
            localPhysicsOptions += ["STauLMinusToMuMinusNeutralino", "STauLPlusToMuPlusNeutralino"]
            localPhysicsOptions += ["STauRMinusToa1MinusNeutralino", "STauRPlusToa1PlusNeutralino"]
            localPhysicsOptions += ["STauLMinusToa1MinusNeutralino", "STauLPlusToa1PlusNeutralino"]
    if "coannihilationSlepton" in simdict:
        localPhysicsOptions += ["SElectronRPlusToElectronNeutralino", "SElectronLPlusToElectronNeutralino"]
        localPhysicsOptions += ["SElectronRMinusToElectronNeutralino", "SElectronLMinusToElectronNeutralino"]
        localPhysicsOptions += ["SMuonRPlusToMuonNeutralino", "SMuonLPlusToMuonNeutralino"]
        localPhysicsOptions += ["SMuonRMinusToMuonNeutralino", "SMuonLMinusToMuonNeutralino"]

    simFlags.PhysicsOptions += localPhysicsOptions
del doG4SimConfig, simdict
