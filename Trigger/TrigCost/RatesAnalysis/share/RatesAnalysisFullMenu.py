#!/usr/bin/env python
#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

if __name__=='__main__':
  import sys
  from argparse import ArgumentParser
  parser = ArgumentParser()
  parser.add_argument('--disableHistograms', action='store_false', help='Turn off histograming')
  parser.add_argument('--disableGlobalGroups', action='store_false', help='Turn off global groups')
  parser.add_argument('--disableTriggerGroups', action='store_false', help='Turn off per-trigger groups')
  parser.add_argument('--disableExpressGroup', action='store_false', help='Turn off express stream rates')
  parser.add_argument('--disableUniqueRates', action='store_false', help='Turn off unique rates (much faster!)')
  parser.add_argument('--disableLumiExtrapolation', action='store_false', help='Turn off luminosity extrapolation')
  #
  parser.add_argument('--doRatesVsPositionInTrain', action='store_true', help='Study rates vs BCID position in bunch train')
  parser.add_argument('--vetoStartOfTrain', default=0, type=int, help='Number of BCIDs at the start of the train to veto, implies doRatesVsPositionInTrain')
  #
  parser.add_argument('--outputHist', default='RatesHistograms.root', type=str, help='Histogram output ROOT file')
  parser.add_argument('--inputPrescalesHLTJSON', default='', type=str, help='JSON of HLT prescales to simulate applying when computing rates')
  parser.add_argument('--inputPrescalesL1JSON', default='', type=str, help='JSON of L1 prescales to simulate applying when computing rates')
  parser.add_argument('--ebWeightsDirectory', default='', type=str, help='Path to directory with local EB xml files')
  #
  parser.add_argument('--targetLuminosity', default=2e34, type=float)
  #
  parser.add_argument('--MCDatasetName', default='', type=str, help='For MC input: Name of the dataset, can be used instead of MCCrossSection, MCFilterEfficiency')
  parser.add_argument('--MCCrossSection', default=0.0, type=float, help='For MC input: Cross section of process in nb')
  parser.add_argument('--MCFilterEfficiency', default=1.0, type=float, help='For MC input: Filter efficiency of any MC filter (0.0 - 1.0)')
  parser.add_argument('--MCKFactor', default=1.0, type=float, help='For MC input: Additional multiplicitive fudge-factor to the supplied cross section.')
  parser.add_argument('--MCIgnoreGeneratorWeights', action='store_true', help='For MC input: Flag to disregard any generator weights.')
  #
  parser.add_argument('--maxEvents', type=int, help='Maximum number of events to process')
  parser.add_argument('--loglevel', type=int, default=3, help='Verbosity level')
  parser.add_argument('flags', nargs='*', help='Config flag overrides')
  args = parser.parse_args()

  # Set the Athena configuration flags
  from AthenaConfiguration.AllConfigFlags import ConfigFlags
  ConfigFlags.Exec.OutputLevel = args.loglevel
  ConfigFlags.fillFromArgs(args.flags)
  useBunchCrossingData = (args.doRatesVsPositionInTrain or args.vetoStartOfTrain > 0)

  ConfigFlags.lock()

  # Initialize configuration object, add accumulator, merge, and run.
  from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
  from AthenaConfiguration.ComponentFactory import CompFactory

  from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
  cfg = MainServicesCfg(ConfigFlags)
  cfg.merge(PoolReadCfg(ConfigFlags))

  histSvc = CompFactory.THistSvc()
  histSvc.Output += ["RATESTREAM DATAFILE='" + args.outputHist + "' OPT='RECREATE'"]
  cfg.addService(histSvc)

  # Minimal config needed to read metadata: MetaDataSvc & ProxyProviderSvc
  from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
  cfg.merge(MetaDataSvcCfg(ConfigFlags))

  cfgsvc = CompFactory.TrigConf.xAODConfigSvc('xAODConfigSvc')
  cfg.addService(cfgsvc)

  tdt = CompFactory.Trig.TrigDecisionTool('TrigDecisionTool')
  tdt.TrigConfigSvc = cfgsvc
  tdt.NavigationFormat = "TrigComposite"
  cfg.addPublicTool(tdt)

  # If the dataset name is in the input files path, then it will be fetched from there
  # Note to enable autolookup, first run "lsetup pyami; voms-proxy-init -voms atlas" and enter your grid pass phrase
  xsec = args.MCCrossSection
  fEff = args.MCFilterEfficiency
  dset = args.MCDatasetName
  if ConfigFlags.Input.isMC and xsec == 0: # If the input file is MC then make sure we have the needed info
    from RatesAnalysis.GetCrossSectionAMITool import GetCrossSectionAMI
    amiTool = GetCrossSectionAMI()
    if dset == "": # Can we get the dataset name from the input file path?
      dset = amiTool.getDatasetNameFromPath(ConfigFlags.Input.Files[0])
    amiTool.queryAmi(dset)
    xsec = amiTool.crossSection
    fEff = amiTool.filterEfficiency

  ebw = CompFactory.EnhancedBiasWeighter('EnhancedBiasRatesTool')
  ebw.RunNumber = ConfigFlags.Input.RunNumbers[0]
  ebw.UseBunchCrossingData = useBunchCrossingData
  ebw.IsMC = ConfigFlags.Input.isMC
  # The following three are only needed if isMC == true
  ebw.MCCrossSection = xsec
  ebw.MCFilterEfficiency = fEff
  ebw.MCKFactor = args.MCKFactor
  ebw.MCIgnoreGeneratorWeights = args.MCIgnoreGeneratorWeights
  ebw.EBWeightsDirectory = args.ebWeightsDirectory if args.ebWeightsDirectory else ""
  cfg.addPublicTool(ebw)

  rates = CompFactory.FullMenu()
  #rates.PrescalesJSON = args.inputPrescalesJSON
  rates.DoTriggerGroups = args.disableTriggerGroups
  rates.DoGlobalGroups = args.disableGlobalGroups
  rates.DoExpressRates = args.disableExpressGroup
  rates.DoUniqueRates = args.disableUniqueRates
  rates.DoHistograms = args.disableHistograms
  rates.UseBunchCrossingData = useBunchCrossingData
  rates.TargetLuminosity = args.targetLuminosity
  rates.VetoStartOfTrain = args.vetoStartOfTrain
  rates.EnableLumiExtrapolation = args.disableLumiExtrapolation
  rates.EnhancedBiasRatesTool = ebw
  rates.TrigDecisionTool = tdt
  rates.TrigConfigSvc = cfgsvc

  """Return all of the HLT items, and their lower chains
  in a JSON menu
  """
  import json
  from collections import OrderedDict as odict

  prescales = {}
  #ESprescalesHLT = {} #TVS: to be done
  
  inputFilePSL1JSON=args.inputPrescalesL1JSON
  inputFilePSHLTJSON=args.inputPrescalesHLTJSON

  if (inputFilePSL1JSON!="" and inputFilePSHLTJSON!=""):
    with open(inputFilePSL1JSON,'r') as fh:
      l1ps_json_file = json.load(fh, object_pairs_hook = odict)

    with open(inputFilePSHLTJSON,'r') as fh:
      hltps_json_file = json.load(fh, object_pairs_hook = odict)
    
    for chain_name, ch in l1ps_json_file['cutValues'].items():
      prescaleL1_input = ch['info'].split()[1]
      p = {'prescale': float(prescaleL1_input)}
      prescales[chain_name] = p
    
    for chain_name, ch in hltps_json_file['prescales'].items():
      prescaleHLT_input = ch['prescale']
      prescaleExpressHLT_input = ch['prescale_express'] if 'prescale_express' in ch else -1

      p = {
        'prescale': float(prescaleHLT_input),
        'prescale_express': float(prescaleExpressHLT_input)
      }
      prescales[chain_name] = p

    
  rates.PrescalesJSON = prescales
    

  cfg.addEventAlgo(rates)

  # Setup for accessing bunchgroup data from the DB
  if useBunchCrossingData:
    from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
    cfg.merge(BunchCrossingCondAlgCfg(ConfigFlags))

  eventLoop = CompFactory.AthenaEventLoopMgr()
  eventLoop.EventPrintoutInterval = 1000
  cfg.addService(eventLoop)

  # If you want to turn on more detailed messages ...
  # exampleMonitorAcc.getEventAlgo('ExampleMonAlg').OutputLevel = 2 # DEBUG
  cfg.printConfig(withDetails=False) # set True for exhaustive info

  sc = cfg.run(args.maxEvents)
  sys.exit(0 if sc.isSuccess() else 1)
