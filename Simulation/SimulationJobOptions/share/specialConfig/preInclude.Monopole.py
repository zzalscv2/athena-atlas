#########################################################################
#       preInclude.Monopole.py - Chiara Debenedetti, 3 Jun 2011      #
#########################################################################

def load_files_for_monopole_scenario(MASS, GCHARGE):
    import os, shutil, sys

    from G4AtlasApps.SimFlags import simFlags
    from ExtraParticles.PDGHelpers import getPDGTABLE
    if getPDGTABLE(simFlags.ExtraParticlesPDGTABLE.get_Value()):
        ALINE1="M 4110000                         {intmass}.E+03       +0.0E+00 -0.0E+00 Monopole         0".format(intmass=int(MASS))
        ALINE2="W 4110000                          0.E+00         +0.0E+00 -0.0E+00 Monopole         0"
        BLINE1="4110000 {intmass}.00 0.0 {gcharge} # Monopole".format(intmass=int(MASS), gcharge=GCHARGE)
        BLINE2="-4110000 {intmass}.00 0.0 -{gcharge} # MonopoleBar".format(intmass=int(MASS), gcharge=GCHARGE)

        f=open('PDGTABLE.MeV','a')
        f.writelines(str(ALINE1))
        f.writelines('\n')
        f.writelines(str(ALINE2))
        f.writelines('\n')
        f.close()
        partmod = os.path.isfile('particles.txt')
        if partmod is True:
            os.remove('particles.txt')
        f=open('particles.txt','w')
        f.writelines(str(BLINE1))
        f.writelines('\n')
        f.writelines(str(BLINE2))
        f.writelines('\n')
        f.close()

        del ALINE1
        del ALINE2
        del BLINE1
        del BLINE2


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
        if not "InteractingPDGCodes" in simFlags.specialConfiguration.get_Value():
            simFlags.specialConfiguration.get_Value()['InteractingPDGCodes'] = str([4110000,-4110000])
        simdict = simFlags.specialConfiguration.get_Value()
except:
    from G4AtlasApps.SimFlags import simFlags
    if not "InteractingPDGCodes" in simFlags.specialConfiguration.get_Value():
        simFlags.specialConfiguration.get_Value()['InteractingPDGCodes'] = str([4110000,-4110000])
    simdict = simFlags.specialConfiguration.get_Value()

assert "MASS" in simdict
assert "GCHARGE" in simdict
load_files_for_monopole_scenario(simdict["MASS"], simdict["GCHARGE"])
pdgcodes = eval(simdict['InteractingPDGCodes']) if 'InteractingPDGCodes' in simdict else []
from ExtraParticles.PDGHelpers import updateExtraParticleAcceptList
updateExtraParticleAcceptList('G4particle_acceptlist_ExtraParticles.txt', pdgcodes)

if doG4SimConfig:
    from G4AtlasApps.SimFlags import simFlags
    # FIXME ideally would include this file early enough, so that the unlocking is not required
    #simFlags.EquationOfMotion.unlock()
    simFlags.EquationOfMotion.set_On()
    simFlags.EquationOfMotion.set_Value_and_Lock("G4mplEqMagElectricField") #Monopole Equation of Motion
    simFlags.G4Stepper.set_Value_and_Lock('ClassicalRK4')
    simFlags.TightMuonStepping.set_Value_and_Lock(False)
    simFlags.PhysicsOptions += ["MonopolePhysicsTool"]
    # add monopole-specific configuration for looper killer
    simFlags.OptionalUserActionList.addAction('G4UA::MonopoleLooperKillerTool')
    # add default HIP killer
    simFlags.OptionalUserActionList.addAction('G4UA::HIPKillerTool')



del doG4SimConfig, simdict
