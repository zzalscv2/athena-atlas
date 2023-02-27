#!/usr/bin/env python
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Script testing whether hadd (and the way that it auto-loads dictionaries) can
# deal with asg::AsgComponentConfig.
#

# Necessary import(s).
import subprocess
import ROOT

def writeConfigFile(fname, oname):
   '''Write a ROOT file with an asg::AsgComponentConfig file in it

   fname  --  The name of the file to write
   oname  --  The name of the configuration object
   '''

   # Open an output file.
   ofile = ROOT.TFile.Open(fname, 'RECREATE')
   if not ofile or ofile.IsZombie():
      print('Failed to open output file "%s"' % fname)
      return 1

   # Create a not completely trivial asg::AsgComponentConfig object.
   conf = ROOT.asg.AsgComponentConfig('Foo/Bar')

   # Write out the object.
   if ofile.WriteObjectAny(conf, ROOT.TClass.GetClass('asg::AsgComponentConfig'), oname) <= 0:
      print('Something went wrong with writing to "%s"' % fname)
      return 1

   # Close the output file.
   ofile.Close()

   # Return gracefully.
   return 0


def main():
   '''C(++) style main function
   '''

   # Write the input file(s).
   ret = writeConfigFile('AsgComponentConfig-input1.root', 'conf1')
   if ret != 0:
      return ret
   ret = writeConfigFile('AsgComponentConfig-input2.root', 'conf2')
   if ret != 0:
      return ret

   # Use hadd on them.
   ret = subprocess.call('hadd -f AsgComponentConfig-output1.root AsgComponentConfig-input1.root AsgComponentConfig-input2.root',
                         shell = True)
   if ret != 0:
      return ret

   # Return gracefully.
   return 0

# Execute the main() function by default.
if __name__ == '__main__':
   import sys
   sys.exit(main())
   pass
