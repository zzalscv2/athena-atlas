#! /usr/bin/env python

# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# Run a RAWMerge job and test key metadata in the output
#

import glob
import json
import subprocess
import os
import os.path
import sys
import unittest

from PyJobTransforms.trfLogger import msg

sourceFiles = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecJobTransformTests/data22_13p6TeV/data22_13p6TeV.00431885.physics_Main.recon.DRAW_EGZ.r14710/*'

class RAWMergetest(unittest.TestCase):
    
    def test_runRAWMerge(self):
        inputs = glob.glob(sourceFiles)
        self.assertEqual(len(inputs), 2) # 2 input files
        cmd = ['RAWMerge_tf.py', '--inputBSFile']
        cmd.extend(inputs)
        cmd.extend(['--outputBS_MRGFile', 'merged.DRAW_ZEE._0001.data'])
        msg.info('Will run this transform: {0}'.format(cmd))
        p = subprocess.Popen(cmd, shell = False, stdout = subprocess.PIPE, stderr = subprocess.STDOUT, bufsize = 1)
        while p.poll() is None:
            line = p.stdout.readline()
            sys.stdout.write(line.decode())
        # Hoover up remaining buffered output lines
        for line in p.stdout:
            sys.stdout.write(line)
        self.assertEqual(p.returncode, 0)
        
        # Now load metadata and test a few important values
        with open('jobReport.json') as jr:
            md = json.load(jr)
            self.assertEqual(isinstance(md, dict), True)
            self.assertEqual(md['files']['output'][0]['subFiles'][0]['nentries'], 123) #123 total events
            self.assertEqual(md['files']['output'][0]['subFiles'][0]['name'], 'merged.DRAW_ZEE._0001.data')            

if __name__ == '__main__':
    unittest.main()
