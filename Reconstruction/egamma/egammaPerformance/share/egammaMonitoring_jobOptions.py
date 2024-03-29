if  DQMonFlags.monManEnvironment() in ('tier0','tier0ESD','online', 'AOD'):
                                       
    from AthenaCommon.AlgSequence import AlgSequence
    topSequence = AlgSequence()
 
    from AthenaMonitoring.AthenaMonitoringConf import AthenaMonManager
    from AthenaMonitoring.DQMonFlags import DQMonFlags

    try:

        monManEgamma = AthenaMonManager(name="EgammaMonManager",
                                        FileKey             = DQMonFlags.monManFileKey(),
                                        Environment         = DQMonFlags.monManEnvironment(),
                                        ManualDataTypeSetup = DQMonFlags.monManManualDataTypeSetup(),
                                        DataType            = DQMonFlags.monManDataType())
        topSequence += monManEgamma

        from AthenaMonitoring.BadLBFilterTool import GetLArBadLBFilterTool
        from AthenaMonitoring.FilledBunchFilterTool import GetFilledBunchFilterTool
        
        egammaMonOutputLevel = INFO
        #egammaMonOutputLevel = VERBOSE
        #egammaMonOutputLevel = DEBUG
        ### Setup which objects will be monitored

        if not ('egammaMonitorPhotons' in dir()):
            egammaMonitorPhotons  = True
            
        if not ('egammaMonitorElectrons' in dir()):
            egammaMonitorElectrons  = True

        if not ('egammaMonitorFwdEg' in dir()):
            egammaMonitorFwdEg  = True

        if not ('egammaMonitorZee' in dir()):
            egammaMonitorZee  = True

        if not ('egammaMonitorJPsi' in dir()):
            egammaMonitorJPsi  = True

        if not ('egammaMonitorUpsilon1S' in dir()):
            egammaMonitorUpsilon1S  = True

        if not ('egammaMonitorUpsilon2S' in dir()):
            egammaMonitorUpsilon2S  = True

        if not ('egammaMonitorWenu' in dir()):
            egammaMonitorWenu  = True
            
        if not ('egammaMonitorTop' in dir()):
            egammaMonitorTop  = True

        ## Commenting in/out Mon tools for now
        egammaMonitorWenu  = False
        egammaMonitorTop = False
        egammaMonitorUpsilon1S  = False
        egammaMonitorUpsilon2S  = False
        # egammaMonitorJPsi  = False
        # egammaMonitorZee  = False
        # egammaMonitorFwdEg  = False
        # egammaMonitorPhotons  = False
        # egammaMonitorElectrons  = False

        photonTrigItems = []
        electronTrigItems = []
        mySingleElectronTrigger = []
        myDiElectronTrigger = [] 
        ZeeTrigItems = []
        JPsiTrigItems = []
        UpsilonTrigItems = []
        FrwdETrigItems = []
        MyTrigDecisionTool = ""

        ## Trigger not supported anymore in legacy DQ
        MyDoTrigger = False
        BypassMyTrigDecisionTool = None

        if(egammaMonitorPhotons):
            from egammaPerformance.egammaPerformanceConf import photonMonTool
            phMonTool = photonMonTool(name= "phMonTool",
                                      EgTrigDecisionTool = MyTrigDecisionTool,
                                      EgUseTrigger = MyDoTrigger,
                                      EgTrigger = photonTrigItems,
                                      EgGroupExtension = "",
                                      PhotonContainer = "Photons",
                                      OutputLevel = egammaMonOutputLevel,
                                      )
            
            phMonTool.FilterTools += [ GetLArBadLBFilterTool() ]

            if jobproperties.Beam.beamType()=='collisions':
                phMonTool.FilterTools += [GetFilledBunchFilterTool()]
            monManEgamma.AthenaMonTools += [ phMonTool ]


        if(egammaMonitorElectrons):
            from egammaPerformance.egammaPerformanceConf import electronMonTool
            elMonTool = electronMonTool(name= "elMonTool",
                                        EgTrigDecisionTool = MyTrigDecisionTool,
                                        EgUseTrigger = MyDoTrigger,
                                        EgTrigger = mySingleElectronTrigger,
                                        EgGroupExtension = "",
                                        ElectronContainer = "Electrons",
                                        OutputLevel = egammaMonOutputLevel,
                                        )
            elMonTool.FilterTools += [ GetLArBadLBFilterTool() ]
            if jobproperties.Beam.beamType()=='collisions':
                elMonTool.FilterTools += [GetFilledBunchFilterTool()]
            monManEgamma.AthenaMonTools += [ elMonTool ]
            print(elMonTool)


        if(egammaMonitorFwdEg):
            from egammaPerformance.egammaPerformanceConf import forwardElectronMonTool
            fwdMonTool = forwardElectronMonTool(name= "fwdMonTool",
                                                EgTrigDecisionTool = MyTrigDecisionTool,
                                                EgUseTrigger = MyDoTrigger,
                                                EgTrigger = FrwdETrigItems,
                                                ForwardElectronContainer ="ForwardElectrons",
                                                OutputLevel = egammaMonOutputLevel,
                                              )
            
            fwdMonTool.FilterTools += [ GetLArBadLBFilterTool() ]
            
            if jobproperties.Beam.beamType()=='collisions':
                fwdMonTool.FilterTools += [GetFilledBunchFilterTool()]
            monManEgamma.AthenaMonTools += [ fwdMonTool ]    

        if(egammaMonitorZee):
            from egammaPerformance.egammaPerformanceConf import ZeeTaPMonTool
            ZeeMonTool = ZeeTaPMonTool(name= "ZeeMonTool",
                                       EgTrigDecisionTool = MyTrigDecisionTool,
                                       EgUseTrigger = MyDoTrigger,
                                       EgTrigger = ZeeTrigItems,
                                       EgGroupExtension="Z",
                                       ElectronContainer  ="Electrons",
                                       massPeak = 91188,
                                       electronEtCut = 20000,
                                       massLowerCut = 60000,
                                       massUpperCut = 120000,
                                       #PhiBinning = 64,
                                       OutputLevel = egammaMonOutputLevel,
                                       )
            
            if jobproperties.Beam.beamType()=='collisions':
                ZeeMonTool.FilterTools += [GetFilledBunchFilterTool()]
            monManEgamma.AthenaMonTools += [ ZeeMonTool ]

        if(egammaMonitorJPsi):
            from egammaPerformance.egammaPerformanceConf import ZeeTaPMonTool
            JPsiMonTool = ZeeTaPMonTool(name= "JPsiMonTool",
                                        EgTrigDecisionTool = MyTrigDecisionTool,
                                        EgUseTrigger = MyDoTrigger,
                                        EgTrigger = JPsiTrigItems,
                                        EgGroupExtension="JPsi",
                                        ElectronContainer="Electrons",
                                        massPeak = 3097,
                                        electronEtCut = 3000,
                                        massLowerCut = 2000,
                                        massUpperCut = 5000,
                                        #PhiBinning = 40,
                                        OutputLevel = egammaMonOutputLevel,
                                        )
            if jobproperties.Beam.beamType()=='collisions':
                JPsiMonTool.FilterTools += [GetFilledBunchFilterTool()]
            monManEgamma.AthenaMonTools += [ JPsiMonTool ]

        if(egammaMonitorUpsilon1S):
            from egammaPerformance.egammaPerformanceConf import physicsMonTool
            Upsilon1SMonTool = physicsMonTool(name= "Upsilon1SMonTool",
                                              ElectronContainer="Electrons",
                                              Trigger_Items = UpsilonTrigItems,
                                              ProcessName = "Upsilon1See",
                                              Selection_Items = ["all"],
                                              massShift = 9460,
                                              massElectronClusterEtCut = 1000 ,
                                              massLowerCut = 5000,
                                              massUpperCut = 15000,
                                              #PhiBinning = 64,
                                              OutputLevel = egammaMonOutputLevel,
                                              TrigDecisionTool = MyTrigDecisionTool,
                                              UseTrigger = MyDoTrigger)
            
            monManEgamma.AthenaMonTools += [ Upsilon1SMonTool ]


        if(egammaMonitorUpsilon2S):
            from egammaPerformance.egammaPerformanceConf import physicsMonTool
            Upsilon2SMonTool = physicsMonTool(name= "Upsilon2SMonTool",
                                              ElectronContainer="Electrons",
                                              Trigger_Items = UpsilonTrigItems,
                                              ProcessName = "Upsilon2See",
                                              Selection_Items = ["all"],
                                              massShift = 10023,
                                              massElectronClusterEtCut = 1000 ,
                                              massLowerCut = 5000,
                                              massUpperCut = 15000,
                                              #PhiBinning = 64,
                                              OutputLevel = egammaMonOutputLevel,
                                              TrigDecisionTool = MyTrigDecisionTool,
                                              UseTrigger = MyDoTrigger)
            
            monManEgamma.AthenaMonTools += [ Upsilon2SMonTool ]

        if(egammaMonitorWenu):
            from egammaPerformance.egammaPerformanceConf import ephysicsMonTool
            WenuMonTool = ephysicsMonTool(name= "WenuMonTool",
                                          ElectronContainer="Electrons",
                                          #JetContainer="Cone7TowerJets",
                                          JetContainer="AntiKt4TopoEMJets",
                                          metName = "MET_RefFinal",
                                          ProcessName = "Wenu",
                                          triggerXselection = WenutriggerXselection,
                                          massShift = 9460,
                                          massElectronClusterEtCut = 1000 ,
                                          massLowerCut = 0,
                                          massUpperCut = 200,
                                          #PhiBinning = 64,
                                          LeadingElectronClusterPtCut = 20000,
                                          MissingParticleEtCut = 25000,
                                          JetEnergyCut = 1000000,
                                          DeltaRCut = 10,
                                          OutputLevel = egammaMonOutputLevel,
                                          TrigDecisionTool = MyTrigDecisionTool,
                                          UseTrigger = MyDoTrigger)
            
            monManEgamma.AthenaMonTools += [ WenuMonTool ]

        if(egammaMonitorTop):
            from egammaPerformance.egammaPerformanceConf import TopphysicsMonTool
            TopMonTool = TopphysicsMonTool(name= "TopMonTool",
                                          ElectronContainer="Electrons",
                                          JetContainer="AntiKt4TopoEMJets",
                                          metName = "MET_RefFinal",
                                          ProcessName = "Topww",
                                          triggerXselection = WenutriggerXselection,
                                          Selection_Items = ["all","loose","tight","medium"],
                                          massShift = 9460,
                                          massElectronClusterEtCut = 1000 ,
                                          massLowerCut = 0,
                                          massUpperCut = 200,
                                          #PhiBinning = 64,
                                          LeadingElectronClusterPtCut = 25000,
                                          MissingParticleEtCut = 40000,
                                          JetEnergyCut = 1000000,
                                          DeltaRCut = 10,
                                          OutputLevel = egammaMonOutputLevel,
                                          TrigDecisionTool = MyTrigDecisionTool,
                                          UseTrigger = MyDoTrigger)

            monManEgamma.AthenaMonTools += [ TopMonTool ]
        
            

    except Exception:
        from AthenaCommon.Resilience import treatException
        treatException("egammaMonitoring_jobOptions.py: exception when setting up Egamma monitoring")
 
