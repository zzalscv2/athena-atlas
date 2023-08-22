#!/usr/bin/env python
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

if __name__=='__main__':

   import os,sys
   import argparse

   # now process the CL options and assign defaults
   parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
   #parser.add_argument('-r','--runlist', dest='runlist', default=RunNumberList, nargs='+', help='Run numbers string list', type=str)
   parser.add_argument('-r','--run', dest='run', default='00408920', help='Run number string as in input filename', type=str)
   parser.add_argument('-g','--gain', dest='gain', default="MEDIUM", help='Gain string', type=str)
   parser.add_argument('-p','--partition', dest='partition', default="Em", help='Data taking partition string', type=str)
   parser.add_argument('-f','--fileprefix', dest='fprefix', default="data23_calib", help='File prefix string', type=str)
   parser.add_argument('-i','--indirprefix', dest='dprefix', default="/eos/atlas/atlastier0/rucio/", help='Input directory prefix string', type=str)
   parser.add_argument('-d','--indir', dest='indir', default="", help='Full input dir string', type=str)
   parser.add_argument('-t','--trigger', dest='trig', default='calibration_', help='Trigger string in filename', type=str)
   parser.add_argument('-o','--outrprefix', dest='outrprefix', default="LArRamp", help='Prefix of output ramp root filename', type=str)
   parser.add_argument('-j','--outpprefix', dest='outpprefix', default="LArRamp", help='Prefix of output ramp pool filename', type=str)
   parser.add_argument('-e','--outrdir', dest='outrdir', default="/eos/atlas/atlascerngroupdisk/det-larg/Temp/Weekly/ntuples", help='Output root file directory', type=str)
   parser.add_argument('-k','--outpdir', dest='outpdir', default="/eos/atlas/atlascerngroupdisk/det-larg/Temp/Weekly/poolFiles", help='Output pool file directory', type=str)
   parser.add_argument('-u','--inpsqlite', dest='inpsql', default="mysql.db", help='Input sqlite file with pedestals, in pool output dir.', type=str)
   parser.add_argument('-l','--inofcsqlite', dest='inofcsql', default="mysql_delay.db", help='Input sqlite file with ofcs, in pool output dir', type=str)
   parser.add_argument('-n','--outsqlite', dest='outsql', default="mysql_ramp.db", help='Output sqlite file, in pool output dir.', type=str)
   parser.add_argument('-m','--subdet', dest='subdet', default="EMB", help='Subdetector, EMB, EMEC, HEC or FCAL', type=str)
   parser.add_argument('-s','--side', dest='side', default="C", help='Detector side empty (means both), C or A', type=str)
   parser.add_argument('-c','--isSC', dest='supercells', default=False, action="store_true", help='is SC data ?')
   parser.add_argument('-a','--isRawdata', dest='rawdata', default=False, action="store_true", help='is raw data ?')
   parser.add_argument('-b','--badchansqlite', dest='badsql', default="SnapshotBadChannel.db", help='Output sqlite file, in pool output dir.', type=str)

   args = parser.parse_args()
   if help in args and args.help is not None and args.help:
      parser.print_help()
      sys.exit(0)

   for _, value in args._get_kwargs():
    if value is not None:
        print(value)

   if len(args.run) < 8:
      args.run = args.run.zfill(8)

   # now set flags according parsed options
   if args.indir != "":
      InputDir = args.indir
   else:
      gain=args.gain.lower().capitalize()

      if not args.supercells:
         partstr = args.partition
      else:
         partstr = args.partition+"-DT"
      if args.rawdata:
            partstr += "-RawData"
      # here - add optional nsamples
      InputDir = args.dprefix+args.fprefix+"/calibration_LArElec-Ramp-32s-"+gain+"-"+partstr+"/"+args.run+"/"+args.fprefix+"."+args.run+".calibration_LArElec-Ramp-32s-"+gain+"-"+partstr+".daq.RAW/"

   
   #Import the configution-method we want to use (here: Pedestal and AutoCorr)
   from LArCalibProcessing.LArCalib_RampConfig import LArRampCfg
   
   #Import the MainServices (boilerplate)
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   
   #Import the flag-container that is the arguemnt to the configuration methods
   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
   flags=initConfigFlags()
   addLArCalibFlags(flags, args.supercells)
   
   #Now we set the flags as required for this particular job:
   #The following flags help finding the input bytestream files: 
   flags.LArCalib.Input.Dir = InputDir
   flags.LArCalib.Input.Type = args.trig
   flags.LArCalib.Input.RunNumbers = [int(args.run),]
   flags.LArCalib.Input.Database = args.outpdir + "/" +args.inpsql
   flags.LArCalib.Input.Database2 = args.outpdir + "/" +args.inofcsql
   gainNumMap={"HIGH":0,"MEDIUM":1,"LOW":2}
   flags.LArCalib.Gain=gainNumMap[args.gain.upper()]

   # Input files
   flags.Input.Files=flags.LArCalib.Input.Files
   #Print the input files we found 
   print ("Input files to be processed:")
   for f in flags.Input.Files:
       print (f)

   if len(flags.Input.Files) == 0 :
      print("Unable to find any input files. Please check the input directory:",InputDir)
      sys.exit(0)
   
   #Some configs depend on the sub-calo in question
   #(sets also the preselection of LArRawCalibDataReadingAlg)
   if not flags.LArCalib.isSC:
      if args.subdet == 'EMB' or args.subdet == 'EMEC':
         flags.LArCalib.Input.SubDet="EM"
      elif args.subdet:   
         flags.LArCalib.Input.SubDet=args.subdet
 
      if not args.side:   
         flags.LArCalib.Preselection.Side = [0,1]
      elif args.side == "C":
         flags.LArCalib.Preselection.Side = [0]
      elif args.side == "A":   
         flags.LArCalib.Preselection.Side = [1]
      else:   
         print("unknown side ",args.side)
         sys.exit(-1)
 
      if args.subdet != "EM":
         if args.subdet == 'EMB':
            flags.LArCalib.Preselection.BEC = [0]
         else:   
            flags.LArCalib.Preselection.BEC = [1]
 
      if args.subdet == 'FCAL':
         flags.LArCalib.Preselection.FT = [6]
      elif args.subdet == 'HEC':
         flags.LArCalib.Preselection.FT = [3,10,16,22]
      elif args.subdet == 'HECFCAL':
         flags.LArCalib.Preselection.FT = [3,6,10,16,22]
   
   #Configure the Bad-Channel database we are reading 
   #(the AP typically uses a snapshot in an sqlite file
   flags.LArCalib.BadChannelDB = args.outpdir + "/" + args.badsql
   flags.LArCalib.BadChannelTag = "-RUN2-UPD3-00"
   
   #Output of this job:
   OutputRampRootFileName = args.outrprefix + "_" + args.run
   OutputRampPoolFileName = args.outpprefix + "_" + args.run

   if args.subdet != "" and not flags.LArCalib.isSC:
      OutputRampRootFileName += "_"+args.subdet
      OutputRampPoolFileName += "_"+args.subdet
      if flags.LArCalib.Input.SubDet=="EM":
         OutputRampRootFileName += args.side
         OutputRampPoolFileName += args.side
   OutputRampRootFileName += ".root"
   OutputRampPoolFileName += ".pool.root"

   flags.LArCalib.Output.ROOTFile = args.outrdir + "/" + OutputRampRootFileName
   flags.LArCalib.Output.POOLFile = args.outpdir + "/" + OutputRampPoolFileName
   flags.IOVDb.DBConnection="sqlite://;schema="+args.outpdir + "/" + args.outsql +";dbname=CONDBR2"

   #The global tag we are working with
   flags.IOVDb.GlobalTag = "LARCALIB-RUN2-00"
   

   #Other potentially useful flags-settings:
   
   #Define the global output Level:
   from AthenaCommon.Constants import INFO 
   flags.Exec.OutputLevel = INFO
   
   from AthenaConfiguration.Enums import LHCPeriod
   flags.GeoModel.Run = LHCPeriod.Run3

   flags.lock()
   
   cfg=MainServicesCfg(flags)
   
   cfg.merge(LArRampCfg(flags))

   # adding new patching, again needed in summer 2023
   if flags.LArCalib.CorrectBadChannels:
      if flags.LArCalib.doValidation:
         cfg.getEventAlgo("RampVal").PatchCBs=[0x3df70000]

      # block standard patching for this CB
      cfg.getEventAlgo("LArRampPatcher").DoNotPatchCBs=[0x3df70000]


   #run the application
   cfg.run() 

   #build tag hierarchy in output sqlite file
   import subprocess
   cmdline = (['/afs/cern.ch/user/l/larcalib/LArDBTools/python/BuildTagHierarchy.py',args.outpdir + "/" + args.outsql , flags.IOVDb.GlobalTag])
   print(cmdline)
   try:
      subprocess.run(cmdline, check=True)
   except Exception as e:
      print('Could not create tag hierarchy in output sqlite file !!!!')
      sys.exit(-1)


