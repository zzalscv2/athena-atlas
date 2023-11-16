#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration


## Propagate random seed and dsid arguments to the generators
import os
dsid = os.path.basename(runArgs.jobConfig[0])
if not dsid.isdigit():
    dsid = "999999"
dsid = int(dsid)
seed = int(runArgs.randomSeed)
if runArgs.trfSubstepName == 'afterburn':
    from PyJobTransforms.trfLogger import msg
    msg.info("Running in Afterburner mode   ..... for now dsid and seed not set")
else:
  if 'Pythia8' in evgenConfig.generators:
    genSeq.Pythia8.RandomSeed=seed
    genSeq.Pythia8.Dsid=dsid
  if 'Pythia8B' in evgenConfig.generators:
    genSeq.Pythia8B.RandomSeed=seed
    genSeq.Pythia8B.Dsid=dsid
  if 'Herwig7' in evgenConfig.generators:
    genSeq.Herwig7.RandomSeed=seed
    genSeq.Herwig7.Dsid=dsid
  if 'Sherpa' in evgenConfig.generators:
    genSeq.Sherpa_i.RandomSeed=seed
    genSeq.Sherpa_i.Dsid=dsid
  if 'Epos' in evgenConfig.generators:
    genSeq.Epos.RandomSeed=seed
    genSeq.Epos.Dsid=dsid
  if 'QGSJet' in evgenConfig.generators:
    genSeq.QGSJet.RandomSeed = seed
    genSeq.QGSJet.Dsid=dsid
  if 'EvtGen' in evgenConfig.generators:
    genSeq.EvtInclusiveDecay.RandomSeed=seed
    genSeq.EvtInclusiveDecay.Dsid=dsid
  if 'Photospp' in evgenConfig.generators:
    genSeq.Photospp.RandomSeed=seed
    genSeq.Photospp.Dsid=dsid
  if 'TauplaPP' in evgenConfig.generators:
    genSeq.TauolaPP.RandomSeed=seed
    genSeq.TauolaPP.Dsid=dsid
  if 'Hijing' in evgenConfig.generators:
    genSeq.Hijing.RandomSeed=seed
    genSeq.Hijing.Dsid=dsid
  if 'Starlight' in evgenConfig.generators:
    if hasattr(genSeq, 'Starlight'):
       genSeq.Starlight.RandomSeed=seed
       genSeq.Starlight.Dsid=dsid
    if hasattr(genSeq, 'ParticleDecayer'):
       genSeq.ParticleDecayer.RandomSeed=seed
       genSeq.ParticleDecayer.Dsid=dsid
  if 'CosmicGenerator' in evgenConfig.generators:
    genSeq.CosmicGenerator.RandomSeed=seed
    genSeq.CosmicGenerator.Dsid=dsid

del dsid,seed


