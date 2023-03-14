# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.AccumulatorCache import AccumulatorCache

def resolveTwissBeamFilePath(twiss_beam, msg):
    import os
    if not isinstance(twiss_beam, str):
        return None
    if os.access(twiss_beam,os.R_OK):
        return twiss_beam
    twiss_path = os.getenv('TwissFilesPATH')
    if not twiss_path:
        msg.warning("resolveTwissBeamFilePath: TwissFilesPATH environment variable is empty.")
    twiss_beam_path = twiss_path + '/' + twiss_beam
    if os.access(twiss_beam_path,os.R_OK):
        return twiss_beam_path
    msg.warning(f'resolveTwissBeamFilePath: Could not find twiss beam file at {twiss_beam} or {twiss_beam_path}')
    return None


def buildTwissFilePath(flags, msg, filename, twiss_path=None):
    twiss_energy = '%1.1fTeV'%(float(flags.Sim.TwissEnergy)*0.000001) # flags.Sim.TwissEnergy possibly obsolete?
    twiss_beta = '%07.2fm'%(0.001*flags.Sim.TwissFileBeta) # assumes flags.Sim.TwissFileBeta is in mm
    if not (flags.Sim.TwissFileNomReal and flags.Sim.TwissFileVersion):
        msg.error(f"buildTwissFilePath: Need to either provide file names or set file name (currently {filename}) and file version flags (currently flags.Sim.TwissFileNomReal = {flags.Sim.TwissFileNomReal} and flags.Sim.TwissFileVersion = {flags.Sim.TwissFileVersion}.")
        raise Exception('Not enough information to locate Twiss files. Need to either provide file names or set file name and file version flags.')
    twiss_nomreal = flags.Sim.TwissFileNomReal
    twiss_version = flags.Sim.TwissFileVersion
    import os
    if not twiss_path:
        twiss_path = os.getenv('TwissFilesPATH')
    if not twiss_path:
        msg.warning("buildTwissFilePath: TwissFilesPATH environment variable is empty.")
    twiss_beam = os.path.join(twiss_path, twiss_energy, twiss_beta, twiss_nomreal, twiss_version, filename)
    if not os.access(twiss_beam,os.R_OK):
        raise Exception(f'Failed to find {filename} at {twiss_beam}')
    return twiss_beam


# NB Reconstruction jobs seem to use the default version of the tool,
# so maintain that behaviour for now.
@AccumulatorCache
def ForwardRegionPropertiesCfg(flags, name="ForwardRegionProperties", **kwargs):
    from AthenaCommon.Logging import logging
    msg = logging.getLogger("ForwardRegionPropertiesCfg")
    result = ComponentAccumulator()
    # Settings of optics to be used
    twiss_beam1 = resolveTwissBeamFilePath(flags.Sim.TwissFileBeam1, msg)
    twiss_beam2 = resolveTwissBeamFilePath(flags.Sim.TwissFileBeam2, msg)
    twiss_momentum = -1.
    if twiss_beam1 is None or twiss_beam2 is None:
        msg.info("Attempting to build TwissFileBeam paths manually")
        # Getting paths to the twiss files, momentum calculation; you can switch to local files
        twiss_beam1 = buildTwissFilePath(flags, msg, 'beam1.tfs')
        twiss_beam2 = buildTwissFilePath(flags, msg, 'beam2.tfs')
        import re,math
        twiss_energy = '%1.1fTeV'%(float(flags.Sim.TwissEnergy)*0.000001)
        twiss_momentum =  math.sqrt(float(re.findall("\\d+.\\d+", twiss_energy)[0])**2 - (0.938e-3)**2)*1e3
    else:
        # Have to sort out twiss momentum based on file name
        tmp = twiss_beam1.split('TeV')[0]
        tmp_spot = len(tmp)
        if flags.Sim.TwissEnergy:
            twiss_energy = '%1.1fTeV'%(float(flags.Sim.TwissEnergy)*0.000001)
        else:
            while True:
                try:
                    tmp_energy = float( tmp[tmp_spot:] ) # noqa: F841
                    tmp_spot -= 1
                except ValueError:
                    twiss_energy = float( tmp[tmp_spot+1:] )
                    break
                pass
        import re,math
        twiss_momentum =  math.sqrt(float(re.findall("\\d+.\\d+", twiss_energy)[0])**2 - (0.938e-3)**2)*1e3

    # properties of the field set according to the optics settings above
    kwargs.setdefault("twissFileB1", twiss_beam1)
    kwargs.setdefault("twissFileB2", twiss_beam2)
    kwargs.setdefault("momentum", twiss_momentum)
    result.setPrivateTools(CompFactory.ForwardRegionProperties(name, **kwargs))
    return result
