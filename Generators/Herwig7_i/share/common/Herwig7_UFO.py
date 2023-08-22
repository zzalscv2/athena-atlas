# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Python steering script for Herwig7_i to shower input LHE files
# generated with a given UFO model
#   written by Yoran Yeh <yoran.yeh@cern.ch>

import os, shutil

def generateFRModel(UFOModel, overWriteLocalDir=False):
    """
        UFOModel (string) :: name of UFO model that is imported for the event generation
    """

    # find UFO directory
    MGModelsLatest = '/cvmfs/atlas.cern.ch/repo/sw/Generators/madgraph/models/latest/'
    MGModelDirCVMFS = os.path.join(MGModelsLatest, UFOModel)
    if not MGModelDirCVMFS:
        athMsgLog.error('could not find UFO model ' + UFOModel + ' on cvmfs.' \
                        ' Are you sure it is available on ' + MGModelsLatest + '?')
        exit(1)

    try:
        shutil.copytree(MGModelDirCVMFS, UFOModel)
    except OSError:
        if not overWriteLocalDir:
            athMsgLog.error(UFOModel + ' already exists in your local directory.' \
                            ' Please (re)move this directory yourself to avoid it' \
                            ' being overwritten, or set `overWriteLocalDir` to True.' \
                            ' Exiting gracefully...')
            exit(1)
        else:
            shutil.rmtree(UFOModel)
            shutil.copytree(MGModelDirCVMFS, UFOModel)

    # find ufo2herwig command
    HERWIG7_PATH = os.environ['HERWIG7_PATH']
    ufo2herwig   = os.path.join(HERWIG7_PATH, 'bin/ufo2herwig')
    if not ufo2herwig: 
        athMsgLog.error('could not find Herwig7 ufo2herwig command: ' + ufo2herwig)
        exit(1)

    # generate FRModel.model file
    try:
        os.system(ufo2herwig + ' ' + UFOModel)
    
    # it might be needed to convert python2 models to python3
    except:
    
        # fresh copy of UFO model, as the previous attempt for `ufo2herwig` 
        # will have made modifications in this directory
        shutil.rmtree(UFOModel)
        shutil.copytree(MGModelDirCVMFS, UFOModel)
        os.system(ufo2herwig + ' --convert ' + UFOModel)

    os.system('make')
    
    # add "read FRModel.model" to the Herwig7 command
    Herwig7Config.add_commands("""
read FRModel.model
""")

  

