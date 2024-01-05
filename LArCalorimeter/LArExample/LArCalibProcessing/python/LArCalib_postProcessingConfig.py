# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from LArCalibProcessing.LArCalib_OFCPhysConfig import LArOFCPhysCfg
from LArCalibProcessing.LArCalib_PileUpAutoCorrConfig import LArPileUpAutoCorrCfg
from LArCalibProcessing.LArCalib_OFPhasePickerConfig import LArOFPhasePickerCfg


def finalOFCShapeCfg(flags):
    result=ComponentAccumulator()

    # Calculate pile-up AC for given mu
    result.merge(LArPileUpAutoCorrCfg(flags))

    #Calculate OFCs and Shape (various flavors)
    result.merge(LArOFCPhysCfg(flags,loadPhysAC=False))
    
    #Pick OFC-Phase + shape correction
    result.merge(LArOFPhasePickerCfg(flags,loadInputs=False))
    
    return result

if __name__=="__main__":
    import sys
    import argparse

    # now process the CL options and assign defaults
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('-r','--run', dest='run', default=str(0x7FFFFFFF), help='Run number to query input DB', type=str)
    parser.add_argument('-i','--insqlite', dest='insql', default="freshConstants_AP.db", help='Input sqlite file containing the (merged) output of the AP.', type=str)
    parser.add_argument('-o','--outsqlite', dest='outsql', default="freshConstants_merged.db", help='Output sqlite file', type=str)
    parser.add_argument('--poolfile', dest='poolfile', default="freshConstants_pp.pool.root", help='Output pool file', type=str)
    parser.add_argument('--rootfile', dest='rootfile', default="freshConstants_pp.root", help='Output ROOT file', type=str)
    parser.add_argument('--iovstart',dest="iovstart", default=0, help="IOV start (run-number)", type=int)
    parser.add_argument('--isSC', dest='supercells', default=False, help='is SC data ?', type=bool)
    parser.add_argument('--poolcat', dest='poolcat', default="freshConstants.xml", help='Catalog of POOL files', type=str)
    parser.add_argument('--Ncoll',dest='Ncoll', default=60, help='Number of MinBias collision assumed for pile-up OFCs', type=int)
    args = parser.parse_args()
    if help in args and args.help is not None and args.help:
        parser.print_help()
        sys.exit(0)



    
    #Import the MainServices (boilerplate)
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   
    #Import the flag-container that is the arguemnt to the configuration methods
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from LArCalibProcessing.LArCalibConfigFlags import addLArCalibFlags
    flags=initConfigFlags()
    addLArCalibFlags(flags, args.supercells)

    #Now we set the flags as required for this particular job:
    #The following flags help finding the input bytestream files: 
    flags.LArCalib.Input.RunNumbers = [int(args.run),]
    flags.LArCalib.Input.Database = args.insql
       
    flags.LArCalib.Output.ROOTFile = args.rootfile
    flags.LArCalib.Output.POOLFile = args.poolfile
    #flags.LArCalib.Output.ROOTFile2 ="freshConstants2_pp.pool.root"
    flags.IOVDb.DBConnection="sqlite://;schema="+args.outsql +";dbname=CONDBR2"

    #The global tag we are working with
    flags.IOVDb.GlobalTag = "LARCALIB-RUN2-00"

    flags.Input.Files=[]
    flags.LArCalib.Input.Files = [ ]
    flags.LArCalib.OFC.Ncoll = args.Ncoll
    flags.LArCalib.IOVStart = args.iovstart

    flags.LArCalib.PhysACuseHG=True
    flags.LArCalib.OFC.ShapeCorrection=True


    flags.LAr.doAlign=False
    flags.Input.RunNumbers=flags.LArCalib.Input.RunNumbers


    flags.lock()
   
    cfg=MainServicesCfg(flags)
    cfg.merge(finalOFCShapeCfg(flags))

    cfg.getService("PoolSvc").ReadCatalog+=["xmlcatalog_file:%s"%args.poolcat,]

    cfg.run(1)
