#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


## Propagate random seed and dsid arguments to the generators

dsid = ''.join(runArgs.jobConfig)
seed = str(runArgs.randomSeed)

if runArgs.trfSubstepName == 'afterburn':
    from PyJobTransforms.trfLogger import msg
    msg.info("Running in Afterburner mode   ..... for now dsid and seed not set")
elif 'Pythia8' in evgenConfig.generators:
    genSeq.Pythia8.RandomSeedTfArg=seed
    genSeq.Pythia8.Dsid=dsid
elif 'PythiaB' in evgenConfig.generators:
    genSeq.Pythia8B.RandomSeedTfArg=seed
    genSeq.Pythia8B.Dsid=dsid
elif 'Herwig7' in evgenConfig.generators:
    genSeq.Herwig7.RandomSeedTfArg=seed
    genSeq.Herwig7.Dsid=dsid
elif 'Sherpa' in evgenConfig.generators:
    genSeq.Sherpa_i.RandomSeedTfArg=seed              
    genSeq.Sherpa_i.Dsid=dsid
elif 'Epos' in evgenConfig.generators:
    genSeq.Epos.RandomSeedTfArg=seed              
    genSeq.Epos.Dsid=dsid
elif 'QGSJet' in evgenConfig.generators:
    genSeq.QGSJet.RandomSeedTfArg = seed
    genSeq.QGSJet.Dsid=dsid

del dsid,seed


