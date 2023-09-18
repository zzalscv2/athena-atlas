# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import Format


#This configures pflow + everything it needs
def PFFullCfg(inputFlags,**kwargs):
  
    result=ComponentAccumulator()

    StoreGateSvc=CompFactory.StoreGateSvc
    result.addService(StoreGateSvc("DetectorStore"))

    #Alias calibrated topoclusters, if they exist already, such that overwrite won't fial
    from SGComps.AddressRemappingConfig import InputRenameCfg
    result.merge(InputRenameCfg("xAOD::CaloClusterContainer","CaloCalTopoClusters",""))

    #setup magnetic field service
    from MagFieldServices.MagFieldServicesConfig import AtlasFieldCacheCondAlgCfg
    result.merge(AtlasFieldCacheCondAlgCfg(inputFlags))

    #Configure topocluster algorithmsm, and associated conditions
    from CaloRec.CaloTopoClusterConfig import CaloTopoClusterCfg
    result.merge(CaloTopoClusterCfg(inputFlags))

    from CaloTools.CaloNoiseCondAlgConfig import CaloNoiseCondAlgCfg
    result.merge(CaloNoiseCondAlgCfg(inputFlags,"totalNoise"))
    result.merge(CaloNoiseCondAlgCfg(inputFlags,"electronicNoise"))

    #Cache the track extrapolations
    from TrackToCalo.CaloExtensionBuilderAlgCfg import CaloExtensionBuilderAlgCfg
    # FIXME: This inversion to merge in CAs is a workaround, which can be removed once SiDetElementCondAlgs 
    # don't depend on Muons/TRT/alignment/otherSiSubdetectorAlignment anymore.
    tempCA = CaloExtensionBuilderAlgCfg(inputFlags)
    tempCA.merge(result)
    result = tempCA

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD
    #PFlow requires tracks, electrons, photons, muons and taus in order to have valid links to them. So lets add these objects to the AOD and ESD                                            
    #PFlow also requires calo clusters for links to work, but these are added to output streams elsewhere already
    toESDAndAOD = ["xAOD::TrackParticleContainer#InDetTrackParticles","xAOD::TrackParticleAuxContainer#InDetTrackParticlesAux."]
    toESDAndAOD += ["xAOD::ElectronContainer#Electrons","xAOD::ElectronAuxContainer#ElectronsAux."]
    toESDAndAOD += ["xAOD::PhotonContainer#Photons","xAOD::PhotonAuxContainer#PhotonsAux."]
    toESDAndAOD += ["xAOD::MuonContainer#Muons","xAOD::MuonAuxContainer#MuonsAux."]
    toESDAndAOD += ["xAOD::TauJetContainer#TauJets","xAOD::TauJetAuxContainer#TauJetsAux."]

    result.merge(addToESD(inputFlags, toESDAndAOD))
    result.merge(addToAOD(inputFlags, toESDAndAOD))

    result.merge(PFCfg(inputFlags))
    return result

#Configures only the pflow algorithms and tools - to be used from RecExCommon to avoid
#conflicts or if you only want to configure just the pflow algorithms and tools
def PFCfg(inputFlags,**kwargs):

    result=ComponentAccumulator()

    #Configure the pflow algorithms
    PFLeptonSelectorFactory=CompFactory.PFLeptonSelector
    PFLeptonSelector = PFLeptonSelectorFactory("PFLeptonSelector") 
    PFLeptonSelector.selectElectrons=False
    PFLeptonSelector.selectMuons=False
    result.addEventAlgo(PFLeptonSelector)

    from eflowRec.PFCfg import PFTrackSelectorAlgCfg
    useCaching = True
    #If reading ESD/AOD do not make use of caching of track extrapolations.
    if inputFlags.Input.Format is Format.POOL and "StreamRDO" not in inputFlags.Input.ProcessingTags:
        useCaching = False
    result.merge(PFTrackSelectorAlgCfg(inputFlags,"PFTrackSelector",useCaching))

    from eflowRec.PFCfg import getOfflinePFAlgorithm
    result.merge(getOfflinePFAlgorithm(inputFlags))

    # old PFO algorithm, keep gated behind a joboption but expect this is deprecated.    
    if(inputFlags.PF.useOldPFO):
        from eflowRec.PFCfg import getChargedPFOCreatorAlgorithm,getNeutralPFOCreatorAlgorithm
        result.addEventAlgo(getChargedPFOCreatorAlgorithm(inputFlags,""))
        result.addEventAlgo(getNeutralPFOCreatorAlgorithm(inputFlags,""))

    from eflowRec.PFCfg import getChargedFlowElementCreatorAlgorithm,getNeutralFlowElementCreatorAlgorithm,getLCNeutralFlowElementCreatorAlgorithm
    result.addEventAlgo(getChargedFlowElementCreatorAlgorithm(inputFlags,""))
    result.addEventAlgo(getNeutralFlowElementCreatorAlgorithm(inputFlags,""))
    result.addEventAlgo(getLCNeutralFlowElementCreatorAlgorithm(inputFlags,""))

    #Only do linking if not in eoverp mode
    if not inputFlags.PF.EOverPMode:
      if inputFlags.PF.useElPhotLinks:
          from eflowRec.PFCfg import getEGamFlowElementAssocAlgorithm        
          result.addEventAlgo(getEGamFlowElementAssocAlgorithm(inputFlags))

      if inputFlags.PF.useMuLinks and inputFlags.Detector.GeometryMuon:
          from eflowRec.PFCfg import getMuonFlowElementAssocAlgorithm
          result.addEventAlgo(getMuonFlowElementAssocAlgorithm(inputFlags))

    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD
    toESDAndAOD = ""
    if(inputFlags.PF.EOverPMode):
      toESDAndAOD = ["xAOD::FlowElementContainer#EOverPChargedParticleFlowObjects","xAOD::FlowElementAuxContainer#EOverPChargedParticleFlowObjectsAux."]
      toESDAndAOD += ["xAOD::FlowElementContainer#EOverPNeutralParticleFlowObjects","xAOD::FlowElementAuxContainer#EOverPNeutralParticleFlowObjectsAux."]
    else:
      toESDAndAOD = ["xAOD::FlowElementContainer#JetETMissChargedParticleFlowObjects", "xAOD::FlowElementAuxContainer#JetETMissChargedParticleFlowObjectsAux."]
      toESDAndAOD += ["xAOD::FlowElementContainer#JetETMissNeutralParticleFlowObjects","xAOD::FlowElementAuxContainer#JetETMissNeutralParticleFlowObjectsAux.-FEShowerSubtractedClusterLink."]
      toESDAndAOD += ["xAOD::FlowElementContainer#JetETMissLCNeutralParticleFlowObjects","xAOD::ShallowAuxContainer#JetETMissLCNeutralParticleFlowObjectsAux."]

    if inputFlags.PF.addCPData:
      #if CPData mode, then add PFCaloCluster to ESD and AOD
      #PFCaloCluster are the clusters modified by the PFlow algorithm
      toESDAndAOD += ["xAOD::CaloClusterContainer#PFCaloCluster","xAOD::CaloClusterAuxContainer#PFCaloClusterAux."]
      #also schedule an algoroithm to decorate each calo cluster with the cluster width in eta and phi
      #this allows clients of the AOD to calculate deltaRPrime for track-calocluster pairs.
      PFClusterWidthDecorator = CompFactory.PFClusterWidthDecorator()
      result.addEventAlgo(PFClusterWidthDecorator)

    result.merge(addToESD(inputFlags, toESDAndAOD))
    result.merge(addToAOD(inputFlags, toESDAndAOD))

    #If we read an ESD then we cannot run the thinning because e.g electrons in the ESD
    #have links to the neutral particle flow objects. If we run the thinning, then those
    #links become invalid. 
    if "StreamESD" not in inputFlags.Input.ProcessingTags:
      from ThinningUtils.ThinNegativeEnergyNeutralPFOCfg import ThinNegativeEnergyNeutralPFOCfg
      result.merge(ThinNegativeEnergyNeutralPFOCfg(inputFlags))

    return result

#Configure tau-FE link algorithm - this cannot be in PFCfg because
#pflow runs before taus in standard serial reco. Thus the links
#between taus and FE must happen after tau reco.
def PFTauFELinkCfg(inputFlags,**kwargs):
  result=ComponentAccumulator()
  from eflowRec.PFCfg import getTauFlowElementAssocAlgorithm
  result.addEventAlgo(getTauFlowElementAssocAlgorithm(inputFlags))
  return result

if __name__=="__main__":

    from AthenaConfiguration.AllConfigFlags import ConfigFlags as cfgFlags
    cfgFlags.Concurrency.NumThreads=8
    cfgFlags.Input.isMC=True
    cfgFlags.Input.Files = ["/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/RecExRecoTest/mc21_13p6TeV/ESDFiles/mc21_13p6TeV.421450.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep_fct.recon.ESD.e8445_e8447_s3822_r13565/ESD.28877240._000046.pool.root.1"]
    # Use latest MC21 tag to pick up latest muon folders apparently needed
    cfgFlags.IOVDb.GlobalTag = "OFLCOND-MC21-SDR-RUN3-10"
    cfgFlags.Output.AODFileName="output_AOD.root"
    cfgFlags.Output.doWriteAOD=True
    cfgFlags.fillFromArgs()
    cfgFlags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(cfgFlags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(cfgFlags))
    cfg.merge(PFFullCfg(cfgFlags))
    
    from eflowRec.PFRun3Remaps import ListRemaps

    list_remaps=ListRemaps()
    for mapping in list_remaps:
        cfg.merge(mapping)    

    cfg.run()
