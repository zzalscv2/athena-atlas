#!/usr/bin/env python3

# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

# This aim of this script is to wrap execution of the web display transform when
# the input file doesn't need to be merged or postprocessed
# Need to set up AMI ("lsetup pyami") before use

from argparse import ArgumentParser
from pyAMI.atlas import api as ami
from pyAMI.client import Client
import json
import os
import glob
import sys
import subprocess
import shutil

PTAG = 'data22_13p6TeV'
EOS_TOP = f'/eos/atlas/atlastier0/rucio/{PTAG}/'
CODEDIR = '/afs/cern.ch/user/a/atlasdqm/dqmdisk1/20221117_rescue_displays'

parser = ArgumentParser()
parser.add_argument('run')
parser.add_argument('stream')
parser.add_argument('reco_tag')
parser.add_argument('htag')

opts = parser.parse_args()

client = Client('atlas')
# client.config.read()
# ami.init()
amiinfo = ami.get_ami_tag(client, opts.htag)

argdict = { 'skipMerge': 'True',
            'allowCOOLUpload': 'True',
            'doWebDisplay': 'True',
            'incrementalMode': 'False',
            'postProcessing': 'False',
        }
# print(eval(amiinfo[1]['phconfig']))
argdict.update(eval(amiinfo[1]['phconfig']))
print('argdict', argdict)

script = amiinfo[1]['trfsetupcmd'].split()[0]
preExec = ' '.join([os.path.join(CODEDIR, os.path.basename(script))] + amiinfo[1]['trfsetupcmd'].split()[1:])
print('preExec', preExec)

inpath = os.path.join(EOS_TOP, opts.stream, f'*{opts.run}', 
                        f'{PTAG}.*{opts.run}.{opts.stream}.merge.HIST.{opts.reco_tag}_{opts.htag}/*')
flist = glob.glob(inpath)
if len(flist) != 1:
    print(f'Unable to locate unique file in {inpath}')
    sys.exit(1)

# os.symlink(flist[0], os.path.basename(flist[0]))
shutil.copy(flist[0], '.')

pfx, lfn = os.path.split(flist[0])
_, dsn = os.path.split(pfx)
argdict['inputHistFiles'] = [{'dsn': dsn, 'dstype': 'HIST', 'events': 0, 'lfn': lfn}]

with open('argdict.json', 'w') as outf:
    json.dump(argdict, outf)

cmd = f"source {preExec}; python3 -u {CODEDIR}/DQM_Tier0Wrapper_tf.py --argJSON=argdict.json"
print(f'To execute {cmd}')
chk=subprocess.run(cmd, shell=True)

os.unlink(os.path.basename(flist[0]))
chk.check_returncode()