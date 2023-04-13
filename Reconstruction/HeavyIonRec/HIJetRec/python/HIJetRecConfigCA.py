#
#  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod
from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD
from JetRecConfig.JetDefinition import JetInputConstitSeq,JetInputConstit, xAODType, JetDefinition
from JetRecConfig.JetDefinition import JetModifier, JetInputExternal
#from JetRecConfig.JetRecConfig import getJetModifierTools 
from JetRecConfig.StandardJetMods import stdJetModifiers
from JetRecConfig import JetRecConfig
from JetRecConfig.DependencyHelper import solveDependencies

def HIJetRecCfg(flags):
    """Configures Heavy IOn Jet reconstruction """
    acc = ComponentAccumulator()

    clustersKey = "HICluster"
    eventshapeKey_egamma = "HIEventShape"
    #get HIClusters
    acc.merge(HIClusterMakerCfg(flags,cluster_key=clustersKey))
    #set eventshape map
    theMapTool = CompFactory.HIEventShapeMapTool()
    #get weighted event shape
    HIEventShapeAlg = acc.getPrimaryAndMerge(HIEventShapeCfg(flags, map_tool=theMapTool)) 
    eventshapeKey = str(HIEventShapeAlg.OutputContainerKey)
   
    #jet definition
    #R=0.2 calojets are use as seeds for UE subtraction
    jetdef2 = defineHICaloJets(clustersKey,suffix="_Unsubtracted",jetradius=2)
    jetname2 = jetdef2.fullname()
    jetdef2 = solveDependencies(jetdef2)


    #Jets for physics
    jetDef = []
    jetname = []
    jetRlist = [2,3,4,6,10]
    for jetR in jetRlist:
        jetDef.append(defineHICaloJets(clustersKey,suffix="_Unsubtracted",jetradius=jetR))
    for i in range(len(jetRlist)):
        jetDef[i] = solveDependencies(jetDef[i])
        jetname.append(jetDef[i].fullname())
        print("HIJETCONFIG: Jet Collection for Reco: " + jetname[i])

    #get calo pseudojets
    pjcs = acc.getPrimaryAndMerge(JetInputCfg(flags))
    #build jet
    acc.merge(JetRecAlgCfg(flags, jetdef2, jetname2, pjcs.OutputContainer))
 
    for i in range(len(jetRlist)):
        acc.merge(JetRecAlgCfg(flags, jetDef[i], jetname[i], pjcs.OutputContainer))

    #START UE SUBTRACTION SEQUENCE
    associationName = "%s_DR8Assoc" % (clustersKey)
    #Modifiers for seed0
    stdJetModifiers.update(
        HIJetAssoc = JetModifier("HIJetDRAssociationTool","HIJetDRAssociation", ContainerKey=clustersKey, DeltaR=0.8, AssociationName=associationName),
        HIJetMaxOverMean = JetModifier("HIJetMaxOverMeanTool","HIJetMaxOverMean", JetContainer = jetname2),
        HIJetDiscrim = JetModifier("HIJetDiscriminatorTool","HIJetDiscriminator", MaxOverMeanCut = 4, MinimumETMaxCut=3000)
    )
    # Copy unsubtracted jets: seed0
    jetDef_seed0 = jetdef2.clone()
    jetDef_seed0.suffix = jetdef2.suffix.replace("Unsubtracted", "seed0")
    jetname_seed0 = jetDef_seed0.fullname()
    jetDef_seed0.modifiers=["HIJetAssoc", "HIJetMaxOverMean", "HIJetDiscrim", "Filter:5000"]
    jetDef_seed0 = solveDependencies(jetDef_seed0)
    #adding seed0 to CA
    acc.merge(JetCopyAlgCfg(flags,jetname2, jetDef_seed0)) 
 
    iter0 = acc.popToolsAndMerge(AddIterationCfg(flags,jetname_seed0, eventshapeKey, clustersKey, map_tool=theMapTool, assoc_name=associationName, suffix="iter0"))
    acc.merge(RunToolsCfg(flags,[iter0], "jetalgHI_iter0")) #this run iter0 and produce the new shape

    modulator0=iter0.Modulator #get modulator from iter0
    subtractor0=iter0.Subtractor #get subtractor from iter0

    #Seeting subtraction 0 and Jet energy calibration
    stdJetModifiers.update(
    subtr0 = JetModifier(
        "HIJetConstituentSubtractionTool",
        "HICS_HIEventShapeWeighted_iter0",
        Modulator=modulator0,
        EventShapeMapTool=theMapTool,
        Subtractor=subtractor0,
        EventShapeKey=iter0.OutputEventShapeKey,
        MomentName="JetSubtractedScaleMomentum",
        SetMomentOnly=False,
        ApplyOriginCorrection=True),
    HIJetCalib = JetModifier(
        "JetCalibrationTool",
        "HICalibTool_{modspec}",
        JetCollection="AntiKt4HI",
        PrimaryVerticesContainerName="",
        ConfigFile='JES_MC15c_HI_Nov2016.config',
        CalibSequence=lambda _, modspec: modspec.split('___')[0],
        IsData=lambda _, modspec: modspec.split('___')[1] == 'True') 
    )

    #Jet energy scale configuration
    JES_is_data=False
    calib_seq = "EtaJES"
    if not flags.Input.isMC :
        JES_is_data=True
        calib_seq = "EtaJES"+"_Insitu"
    
    # Copy unsubtracted jets: seed1
    jetDef_seed1 = jetdef2.clone()
    jetDef_seed1.suffix = jetdef2.suffix.replace("Unsubtracted", "seed1")
    jetname_seed1 = jetDef_seed1.fullname()
    jetDef_seed1.modifiers=["HIJetAssoc", "subtr0", "HIJetCalib:{}___{}".format(calib_seq, JES_is_data),"Filter:{}".format(flags.HeavyIon.Jet.SeedPtMin)]
    jetDef_seed1 = solveDependencies(jetDef_seed1)
    #adding seed1 to CA
    acc.merge(JetCopyAlgCfg(flags,jetname2, jetDef_seed1))

 ######## configuring TRACKJETS: seeds for second iteration
    if flags.HeavyIon.Jet.doTrackJetSeed : 
        jetdef_trk = defineHITrackJets(jetradius=4)
        jetdef_trk.modifiers=["HIJetAssoc", "Filter:{}".format(flags.HeavyIon.Jet.TrackJetPtMin)]
        jetdef_trk = solveDependencies(jetdef_trk)
        inputtracks = getTrackSelAlg()
        acc.addEventAlgo(inputtracks)
 
        pjcs_trk = CompFactory.PseudoJetAlgorithm(
             "TrackPseudoJets",
             InputContainer = inputtracks.OutputContainer,
             OutputContainer="PseudoTracks",
             Label="Tracks",
             SkipNegativeEnergy=True,
         )
        acc.addEventAlgo(pjcs_trk)
        acc.merge(JetRecAlgCfg(flags, jetdef_trk, jetdef_trk.fullname(), pjcs_trk.OutputContainer))
 
 ##############  

    #iter1 ( second iteration)
    if flags.HeavyIon.Jet.doTrackJetSeed :
        iter1 = acc.popToolsAndMerge(
            AddIterationCfg(
            flags,
            jetname_seed1, 
            eventshapeKey, 
            clustersKey, 
            map_tool=theMapTool, 
            assoc_name=associationName,
            track_jet_seeds=jetdef_trk.fullname(), 
            suffix="iter1")
        )
    else :
        iter1 = acc.popToolsAndMerge(
            AddIterationCfg(
            flags,
            jetname_seed1, 
            eventshapeKey, 
            clustersKey, 
            map_tool=theMapTool, 
            assoc_name=associationName, 
            suffix="iter1")
        )

    acc.merge(RunToolsCfg(flags,[iter1], "jetalgHI_iter1")) #this run iter1 and produce the new shape

    modulator1=iter1.Modulator
    subtractor1=iter1.Subtractor

#####EGAMMA iteration and subtration layer level (useClusters=False)
#///////////////////////////////////////////////

    #for egamma
    iter1_egamma = acc.popToolsAndMerge(
        AddIterationCfg(
            flags,
            jetname_seed1,
            eventshapeKey_egamma,
            clustersKey,
            map_tool=theMapTool,
            assoc_name=associationName,
            useClusters=False,
            suffix="iter_egamma")
    )

    acc.merge(RunToolsCfg(flags,[iter1_egamma], "jetalgHI_iter1_egamma")) #this run iter1_egamma and produce the new shape

    #Constituents subtraction for egamma, cell-level
    cluster_key_eGamma_deep=clustersKey+"_eGamma_deep"

    subtocelltool = acc.popToolsAndMerge(
        ApplySubtractionToClustersCfg(
        flags,
        name="HIClusterSubtraction_egamma",
        EventShapeKey=iter1_egamma.OutputEventShapeKey,
         ClusterKey=clustersKey,
         ClusterKey_out=cluster_key_eGamma_deep,
         modulator=modulator1,
         map_tool=theMapTool,
         CalculateMoments=True,
         useClusters=False,
         apply_origin_correction=False)
     )
    acc.merge(RunToolsCfg(flags,[subtocelltool], "jetalgHI_subtocelltool"))

#///////////////////////////////////////////////

    #setting subtraction 1 ( final )
    stdJetModifiers.update(
    subtr1 = JetModifier(
        "HIJetConstituentSubtractionTool",
        "HICS_HIEventShapeWeighted_iter1",
        Modulator=modulator1,
        EventShapeMapTool=theMapTool,
        Subtractor=subtractor1,
        EventShapeKey=iter1.OutputEventShapeKey,
        MomentName="JetSubtractedScaleMomentum",
        SetMomentOnly=False,
        ApplyOriginCorrection=True)
    )

    #Constituents subtraction for jets, tower-level
    cluster_key_final_deep=cluster_key_eGamma_deep+"_Cluster_deep"
    subtoclustertool = acc.popToolsAndMerge(
        ApplySubtractionToClustersCfg(
        flags,
        EventShapeKey=iter1.OutputEventShapeKey, 
        ClusterKey=cluster_key_eGamma_deep, 
        ClusterKey_out=cluster_key_final_deep, 
        modulator=modulator1, 
        map_tool=theMapTool, 
        CalculateMoments=False, 
        useClusters=True, 
        apply_origin_correction=False)
        )

    acc.merge(RunToolsCfg(flags,[subtoclustertool], "jetalgHI_subtoclustertool"))    

    stdJetModifiers.update(
    consmod = JetModifier(
        "HIJetConstituentModifierTool",
        "HIJetConstituentModifierTool_final",
        ClusterKey=cluster_key_final_deep,
        Subtractor=subtractor1,
        ApplyOriginCorrection=True)
    )


    #Configure final jets
    jetDefList_final = []
    jetNameList_final = []
    for i in range(len(jetRlist)):
        jetDefList_final.append(jetDef[i].clone())
        jetDefList_final[i].suffix = jetDef[i].suffix.replace("_Unsubtracted","")
        jetDefList_final[i].modifiers=["subtr1","consmod","HIJetCalib:{}___{}".format(calib_seq, JES_is_data),"Filter:{}".format(flags.HeavyIon.Jet.RecoOutputPtMin)]
        jetNameList_final.append(jetDefList_final[i].fullname())
        jetDefList_final[i] = solveDependencies(jetDefList_final[i])

    #adding final jets to CA
    for i in range(len(jetRlist)):
        acc.merge(JetCopyAlgCfg(flags,jetname[i], jetDefList_final[i]))

    if flags.HeavyIon.Jet.doTrackJetSeed :
        jetNameList_final.append(jetdef_trk.fullname())

    for jetcoll in jetNameList_final:
        output = ["xAOD::JetContainer#"+jetcoll,"xAOD::JetAuxContainer#"+jetcoll+"Aux.-PseudoJet"]
        acc.merge(addToESD(flags, output))
        acc.merge(addToAOD(flags, output))

    return acc


def HIClusterMakerCfg(flags, cell_key="AllCalo", cluster_key="", save = True) :
    """Function to equip HI cluster builder from towers and cells, adds to output AOD stream"""

    acc = ComponentAccumulator()

    #get towers
    from CaloRec.CaloRecoConfig import CaloRecoCfg
    acc.merge(CaloRecoCfg(flags))
    from CaloRec.CaloTowerMakerConfig import CaloTowerMakerCfg
    towerMaker = acc.getPrimaryAndMerge(CaloTowerMakerCfg(flags))
    tower_key = towerMaker.TowerContainerName

    if cluster_key == "" :
        cluster_key = "HICluster"

    HIClusterMaker = CompFactory.HIClusterMaker("HIClusterMaker",
                                InputTowerKey = tower_key,
                                CaloCellContainerKey = cell_key,
                                OutputContainerKey = cluster_key)

    if save is True :
        output = [ f"xAOD::CaloClusterContainer#{cluster_key}", f"xAOD::CaloClusterAuxContainer#{cluster_key}Aux."]
        acc.merge(addToESD(flags, output))
        acc.merge(addToAOD(flags, output))


    acc.addEventAlgo(HIClusterMaker, primary=True)

    return acc

def HIEventShapeCfg(flags, clustersKey= "HICluster", suffix="_Weighted", **kwargs) :

    acc = ComponentAccumulator()

    #get towers
    from CaloRec.CaloRecoConfig import CaloRecoCfg
    acc.merge(CaloRecoCfg(flags))
    from CaloRec.CaloTowerMakerConfig import CaloTowerMakerCfg
    towerMaker = acc.getPrimaryAndMerge(CaloTowerMakerCfg(flags))
    input_tower = towerMaker.TowerContainerName
   
    #map tool
    if 'map_tool' in kwargs.keys() : map_tool=kwargs['map_tool']
    else : map_tool = CompFactory.HIEventShapeMapTool()

    #Add weight tool to filler tool
    if flags.GeoModel.Run in [LHCPeriod.Run1, LHCPeriod.Run2]:
        HITowerWeightTool_inputFile='cluster.geo.DATA_PbPb_2018v2.root'
    else:
        HITowerWeightTool_inputFile='cluster.geo.DATA_PbPb_2022.root'

    TWTool = CompFactory.HITowerWeightTool("WeightTool",
        ApplyCorrection=flags.HeavyIon.Jet.ApplyTowerEtaPhiCorrection,
        ConfigDir='HIJetCorrection/',
        InputFile=HITowerWeightTool_inputFile)

    #Event Shape filler
    eventShapeTool = CompFactory.HIEventShapeFillerTool( 
            EventShapeMapTool = map_tool,
            TowerWeightTool = TWTool,
            UseClusters=True
    )
    
    #event shape maker
    shapeKey = "HIEventShape"+suffix
    eventShapeMakerAlg = CompFactory.HIEventShapeMaker("HIEventShapeMaker"+suffix,
                    InputTowerKey = clustersKey,
                    NaviTowerKey = input_tower,
                    OutputContainerKey = shapeKey,
                    HIEventShapeFillerTool = eventShapeTool)

    acc.addEventAlgo( eventShapeMakerAlg, primary=True )

    return acc

def defineHICaloJets(clustersKey=None,prefix='',suffix='',**kwargs):
    minpt = {
        2:  7000,
        3:  7000,
        4:  7000,
        6:  7000,
        10: 50000,
    }
    jetradius = kwargs['jetradius']
    actualradius = float(jetradius)/10

    constitMods = [] # modifiers
    jetConstit = []
    jetConstit = JetInputConstitSeq( "HICaloConstit",xAODType.CaloCluster, constitMods, inputname=clustersKey, outputname=clustersKey, label='HI')
    from JetRecConfig.StandardJetConstits import stdConstitDic
    stdConstitDic.setdefault(jetConstit.name, jetConstit)

    jetDef = JetDefinition( "AntiKt", actualradius, jetConstit, ptmin=minpt[jetradius], prefix=prefix, suffix=suffix)

    return jetDef

def defineHITrackJets(prefix='',suffix='',**kwargs):
    import JetRecTools.JetRecToolsConfig as jrtcfg

    jetradius = kwargs['jetradius']
    actualradius = float(jetradius)/10

    JetInputExternal("JetSelectedTracks",     
                  xAODType.TrackParticle,
                  prereqs = ["InDetTrackParticles"], # in std context, this is InDetTrackParticles (see StandardJetContext)
                  algoBuilder = lambda jdef, _ : jrtcfg.getTrackSelAlg(jdef.context, trackSelOpt=False )
                )
    jetConstit = JetInputConstit("Track", xAODType.TrackParticle,'JetSelectedTracks')

    jetDef = JetDefinition( "AntiKt", actualradius, jetConstit, ptmin=5000, prefix=prefix, suffix=suffix)

    return jetDef


def JetInputCfg(flags, clustersKey="HICluster"):
    # Create a PseudoJetAlgorithm
    acc = ComponentAccumulator()
    
    pjcs = CompFactory.PseudoJetAlgorithm("pjcs"+clustersKey,
        InputContainer = clustersKey,
        OutputContainer = "PseudoJet"+clustersKey,
        Label = "LCTopo",
        SkipNegativeEnergy=False,
        TreatNegativeEnergyAsGhost=True)

    acc.addEventAlgo(pjcs, primary=True)

    return acc


def JetRecAlgCfg(flags, jetdef, jetsName, pjcs_container):
    
    acc = ComponentAccumulator() 
    # Create the JetClusterer, set some standard options
    jclust = CompFactory.JetClusterer("builder",
        JetAlgorithm = jetdef.algorithm,
        JetRadius = jetdef.radius,
        PtMin = jetdef.ptmin, # MeV
        GhostArea = 0.0,
        InputPseudoJets = pjcs_container)
    # Create the JetRecAlg, configure it to use the builder

    mods = JetRecConfig.getJetModifierTools(jetdef)

    jra = CompFactory.JetRecAlg(
        "JRA_build"+jetsName,
        Provider = jclust, # Single ToolHandle
        Modifiers = mods, # ToolHandleArray
        OutputContainer = jetsName)

    acc.addEventAlgo(jra)

    return acc

def JetCopyAlgCfg(flags, jetsin_container, jetdef, decorations=[], shallowcopy=False, shallowIO=False):

    acc = ComponentAccumulator()
    # Create the JetClusterer, set some standard options
    jclust = CompFactory.JetCopier("builder_copy",
        InputJets = jetsin_container,
        DecorDeps = decorations,
        ShallowCopy = shallowcopy,
        ShallowIO = shallowIO)
    # Create the JetRecAlg, configure it to use the builder
    mods = JetRecConfig.getJetModifierTools(jetdef)
    
    jetsoutname = jetdef.fullname()
    jra = CompFactory.JetRecAlg(
        "JRA_build"+jetsoutname,
        Provider = jclust, # Single ToolHandle
        Modifiers = mods, # ToolHandleArray
        OutputContainer = jetsoutname)

    acc.addEventAlgo(jra)

    return acc

def AddIterationCfg(flags,seed_container,shape_name,clustersKey, **kwargs) :
    acc = ComponentAccumulator()

    useClusters = kwargs.pop('useClusters', True)
    harmicsforSub = flags.HeavyIon.Jet.HarmonicsForSubtraction 

    out_shape_name=shape_name
    if 'suffix' in kwargs.keys() : out_shape_name+='_' + kwargs['suffix']
    mod_shape_key=out_shape_name+'_Modulate'
    remodulate=True
    if remodulate :
        if 'modulator' in kwargs.keys() : mod_tool=kwargs['modulator']
        else :
            mod_tool=MakeModulatorTool(mod_shape_key,harmonics=harmicsforSub,**kwargs)

    if useClusters :
        if flags.GeoModel.Run in [LHCPeriod.Run1, LHCPeriod.Run2]:
            HIJetClusterSubtractorTool_inputFile='cluster.geo.DATA_PbPb_2018v2.root'
        else:
            HIJetClusterSubtractorTool_inputFile='cluster.geo.DATA_PbPb_2022.root'
        
        HIJetClusterSubtractorTool = CompFactory.HIJetClusterSubtractorTool
        sub_tool = HIJetClusterSubtractorTool("HIJetClusterSubtractor", 
                                              ConfigDir='HIJetCorrection/',
                                              InputFile=HIJetClusterSubtractorTool_inputFile,
                                              UseSamplings = False)
    else :
        HIJetCellSubtractorTool=CompFactory.HIJetCellSubtractorTool
        sub_tool = HIJetCellSubtractorTool("HIJetCellSubtractor")

    if 'map_tool' in kwargs.keys() : map_tool=kwargs['map_tool']
    else :
        from HIEventUtils.HIEventUtilsConf import HIEventShapeMapTool
        map_tool=HIEventShapeMapTool()

    if 'assoc_name' in kwargs.keys() : assoc_name=kwargs['assoc_name']
    else :
        from HIJetRec.HIJetRecConf import HIJetDRAssociationTool
        assoc=HIJetDRAssociationTool("HIJetDRAssociation")
        assoc.ContainerKey=clustersKey
        assoc.DeltaR=0.8
        assoc.AssociationName="%s_DR8Assoc" % (clustersKey)
        assoc_name=assoc.AssociationName


    HIEventShapeJetIteration=CompFactory.HIEventShapeJetIteration
    iter_tool=HIEventShapeJetIteration("HIJetIteration_" + out_shape_name)
    iter_tool.InputEventShapeKey = shape_name
    iter_tool.OutputEventShapeKey = out_shape_name
    iter_tool.AssociationKey = assoc_name
    iter_tool.CaloJetSeedContainerKey = seed_container
    iter_tool.ModulationScheme = 1
    iter_tool.RemodulateUE = True
    iter_tool.Modulator = mod_tool
    iter_tool.ShallowCopy = False
    iter_tool.ModulationEventShapeKey = mod_shape_key
    iter_tool.EventShapeMapTool = map_tool
    iter_tool.Subtractor = sub_tool
    if 'track_jet_seeds' in kwargs.keys() : iter_tool.TrackJetSeedContainerKey=kwargs['track_jet_seeds']

    acc.setPrivateTools(iter_tool)

    return acc

def MakeModulatorTool(mod_key, **kwargs):
    harmonics = kwargs.pop('harmonics', [2, 3, 4])
    tname = kwargs.pop('mod_name', 'Modulator_{}'.format(BuildHarmonicName(mod_key, harmonics=harmonics)))

    if 'suffix' in kwargs.keys():
        tname += '_' + kwargs['suffix']

    if len(harmonics) == 0:
        return GetNullModulator()

    HIUEModulatorTool=CompFactory.HIUEModulatorTool
    mod=HIUEModulatorTool(tname)
    mod.EventShapeKey=mod_key

    for n in [2,3,4] :
        val=(n in harmonics)
        attr_name='DoV%d' % n
        setattr(mod,attr_name,val)  
    
    if 'label' in kwargs.keys():
        label = kwargs['label']
        stdJetModifiers[label] = JetModifier(
            "HIUEModulatorTool",
            tname,
            DoV2 = getattr(mod, 'DoV2'),
            DoV3 = getattr(mod, 'DoV3'),
            DoV4 = getattr(mod, 'DoV4'),
            EventShapeKey=mod_key)

    return mod


def BuildHarmonicName(shape_key, **kwargs) :
    tname=shape_key
    if 'harmonics' in kwargs.keys() :
        for n in kwargs['harmonics'] :
            tname = str(tname) + str('_V%d' % n)
    return tname

def GetNullModulator() :
    tname='NullUEModulator'
    HIUEModulatorTool=CompFactory.HIUEModulatorTool
    mod=HIUEModulatorTool(tname)
    mod.EventShapeKey='NULL'
    for n in [2,3,4] : setattr(mod,'DoV%d' % n,False)
    return mod

def RunToolsCfg(flags,toollist, algoName):
    acc = ComponentAccumulator()

    #runner = CompFactory.JetToolRunner(toolName,
    #                      Tools=toollist)
    theAlg = CompFactory.JetAlgorithm(algoName)
    theAlg.Tools = toollist #[runner]
   
    acc.addEventAlgo(theAlg)

    return acc

def ApplySubtractionToClustersCfg(flags, **kwargs) :
    acc = ComponentAccumulator()

    useClusters = kwargs.pop('useClusters', True)

    EventShapeKey = kwargs.pop('EventShapeKey', 'EventShapeKey')#add flag for alternative
    ClusterKey = kwargs.pop('ClusterKey', 'HIClusters')#add flag for alternative
    ClusterKey_out = kwargs.pop('ClusterKey_out', 'ClusterKey_deep')#add flag for alternative
    update_only = kwargs.pop('update_only', False)#add flag for alternative
    apply_origin_correction = kwargs.pop('apply_origin_correction', True)#add flag for alternative
    CalculateMoments = kwargs.pop('CalculateMoments', False)#add flag for alternative

    if 'modulator' in kwargs.keys() : mod_tool=kwargs['modulator']
    else : mod_tool=GetNullModulator()

    if 'map_tool' in kwargs.keys() : map_tool=kwargs['map_tool']
    else :
        from HIEventUtils.HIEventUtilsConf import HIEventShapeMapTool
        map_tool=HIEventShapeMapTool()

    if useClusters :
        if flags.GeoModel.Run in [LHCPeriod.Run1, LHCPeriod.Run2]:
            HIJetClusterSubtractorTool_inputFile='cluster.geo.DATA_PbPb_2018v2.root'
        else:
            HIJetClusterSubtractorTool_inputFile='cluster.geo.DATA_PbPb_2022.root'

        HIJetClusterSubtractorTool = CompFactory.HIJetClusterSubtractorTool
        sub_tool = HIJetClusterSubtractorTool("HIJetClusterSubtractor",
                                              ConfigDir='HIJetCorrection/',
                                              InputFile=HIJetClusterSubtractorTool_inputFile,
                                              UseSamplings = False)
    else :
        HIJetCellSubtractorTool=CompFactory.HIJetCellSubtractorTool
        sub_tool = HIJetCellSubtractorTool("HIJetCellSubtractor")

    HIClusterSubtraction=CompFactory.HIClusterSubtraction
    toolName='HIClusterSubtraction'
    if 'name' in kwargs.keys() : toolName = kwargs['name']

    theAlg=HIClusterSubtraction(toolName)
    theAlg.ClusterKey=ClusterKey
    theAlg.OutClusterKey=ClusterKey_out
    theAlg.EventShapeKey=EventShapeKey
    theAlg.Subtractor=sub_tool
    theAlg.Modulator=mod_tool
    theAlg.UpdateOnly=update_only
    theAlg.SetMoments=CalculateMoments
    theAlg.ApplyOriginCorrection=apply_origin_correction
    theAlg.EventShapeMapTool=map_tool

    if CalculateMoments :
        CaloClusterMomentsMaker=CompFactory.CaloClusterMomentsMaker

        HIClusterMoments = CaloClusterMomentsMaker ("HIClusterMoments")
        HIClusterMoments.MinBadLArQuality = 4000
        HIClusterMoments.MomentsNames = ["CENTER_MAG",
                                         "LONGITUDINAL",
                                         "FIRST_ENG_DENS",
                                         "SECOND_ENG_DENS",
                                         "ENG_FRAC_EM",
                                         "ENG_FRAC_MAX",
                                         "ENG_FRAC_CORE",
                                         "ENG_BAD_CELLS",
                                         "N_BAD_CELLS",
                                         "N_BAD_CELLS_CORR",
                                         "BAD_CELLS_CORR_E",
                                         "BADLARQ_FRAC",
                                         "ENG_POS",
                                         "SIGNIFICANCE",
                                         "CELL_SIGNIFICANCE",
                                         "CELL_SIG_SAMPLING",
                                         "AVG_LAR_Q",
                                         "AVG_TILE_Q",
                                         "ENG_BAD_HV_CELLS",
                                         "N_BAD_HV_CELLS"]

        theAlg.ClusterCorrectionTools=[HIClusterMoments]
    acc.setPrivateTools(theAlg)
    
    return acc


def getTrackSelAlg(trkOpt="HI"):
    # tracking selection tool
    idtracksel = CompFactory.getComp("InDet::InDetTrackSelectionTool")(f"tracksel{trkOpt}",
                                                                       minNSiHits=7,
                                                                       maxAbsEta=2.5,
                                                                       maxNSiHoles=2,
                                                                       maxNPixelHoles=1,
                                                                       minPt=4000)

    # build the selection alg 
    trkSelAlg = CompFactory.JetTrackSelectionAlg( f"trackselalg_{trkOpt}",
                                                  TrackSelector = idtracksel,
                                                  InputContainer = "InDetTrackParticles",
                                                  OutputContainer = "HIJetTracks",
                                                  DecorDeps = []
                                                 )

    return trkSelAlg


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags.Input.Files = [defaultTestFiles.d + "/RecJobTransformTests/data18_hi.00367384.physics_HardProbes.daq.RAW._lb0145._SFO-8._0001.data"]
    flags.Exec.MaxEvents=5
    flags.Concurrency.NumThreads=1

    flags.fillFromArgs() # enable unit tests to switch only parts of reco: python -m HIRecConfig.HIRecConfig HeavyIon.doGlobal = 0 and so on
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)

    acc.merge(HIJetRecCfg(flags))

