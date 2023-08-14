import os

makeDataDAODs=True
makeMCDAODs=True
makeTruthDAODs=True
makeTrains=True

formatList = ['PHYSVAL','PHYS',
              'TOPQ1', 'TOPQ2', 'TOPQ4', 'TOPQ5',
              'HDBS1', 'HDBS3', 'HDBS4',
              'HIGG1D1', 'HIGG1D2',
              'HIGG2D1', 'HIGG2D4', 'HDBS2',
              'HIGG3D1', 'HIGG3D3',
              'HIGG4D1', 'HIGG4D2', 'HIGG4D3', 'HIGG4D4', 'HIGG4D5','HIGG4D6',
              'HIGG5D1', 'HIGG5D2', 'HIGG5D3',
              'HIGG6D1', 'HIGG6D2',
              'HIGG8D1',
              'STDM2', 'STDM3', 'STDM4', 'STDM5', 'STDM6', 'STDM7', 'STDM8', 'STDM9','STDM11','STDM12','STDM13','STDM14','STDM15',
              'TAUP1', 'TAUP2', 'TAUP3', 'TAUP4', 'TAUP5',
              'SUSY1', 'SUSY2', 'SUSY3', 'SUSY4', 'SUSY5', 'SUSY6', 'SUSY7', 'SUSY8', 'SUSY9', 'SUSY10', 'SUSY11', 'SUSY12', 'SUSY15', 'SUSY16', 'SUSY17', 'SUSY18','SUSY19','SUSY20',
              'EXOT0', 'EXOT2', 'EXOT3', 'EXOT4', 'EXOT5', 'EXOT6', 'EXOT7', 'EXOT8', 'EXOT9', 'EXOT10', 'EXOT12', 'EXOT13', 'EXOT15', 'EXOT17', 'EXOT19', 'EXOT20', 'EXOT21', 'EXOT22', 'EXOT23', 'EXOT27', 'EXOT28', 'EXOT29',
              'JETM1', 'JETM2', 'JETM3', 'JETM4', 'JETM5', 'JETM6', 'JETM7', 'JETM8', 'JETM9', 'JETM10', 'JETM11', 'JETM12', 'JETM13','JETM14','JETM15',
              'IDTR1',
              'EGAM1', 'EGAM2', 'EGAM3', 'EGAM4', 'EGAM5', 'EGAM6', 'EGAM7', 'EGAM8', 'EGAM9',
              'FTAG1', 'FTAG2', 'FTAG3', 'FTAG4', 'FTAG5',
              'BPHY1', 'BPHY2', 'BPHY3', 'BPHY4', 'BPHY5', 'BPHY6', 'BPHY7', 'BPHY8', 'BPHY9', 'BPHY10', 'BPHY11', 'BPHY12', 'BPHY13', 'BPHY14','BPHY15','BPHY16', 'BPHY17', 'BPHY18','BPHY19','BPHY20','BPHY21','BPHY22','BPHY23', 'BPHY24',
              'MUON0', 'MUON1', 'MUON2', 'MUON3', 'MUON4',
              'TCAL1',
              'HION3','HION4','HION5','HION7','HION8'
              #'HION1', 'HION2', 'HION3', 'HION4', 'HION5', 'HION6', 'HION7', 'HION8', 'HION9', 'HION10'
]

truthFormatList = ['TRUTH0', 'TRUTH1', 'TRUTH3']

trainList = [ 
              ["TCAL1","EXOT10","HDBS2"],
              ["JETM12","EGAM3","JETM10"],
              ["STDM5","EGAM4","EGAM2","EXOT12","SUSY9","EXOT9"],
              ["MUON2","HIGG4D4","JETM7","BPHY7","EXOT17","BPHY5","EGAM7","HIGG1D2"],
              ["EGAM9","EXOT20","SUSY11","EXOT6","SUSY2","HIGG4D1","BPHY1","BPHY4"],
              ["SUSY12","STDM3","EXOT15","JETM3","EXOT19","HIGG4D6","HIGG6D1","HIGG1D1"],
              ["TAUP1","HIGG4D5","JETM4","TOPQ2","TOPQ5","HIGG4D3","SUSY16","EXOT7"],
              ["STDM2","SUSY18","EXOT3","EGAM1","EGAM5","EXOT2","SUSY3","EXOT5","HIGG6D2"],
              ["EXOT22","SUSY4","JETM11","EXOT21","SUSY1","STDM7","SUSY8","SUSY10"],
              ["EXOT13","SUSY5","SUSY7","EXOT8","EXOT4","HIGG4D2"],
              ["HIGG8D1","JETM6","MUON1","SUSY6","HIGG2D1","JETM1","MUON0","TOPQ1","TAUP3"],
              ["JETM9","STDM4","TOPQ4","FTAG4"],
              # All FTAG run as singletons
              # Special streams (BLS/ZB) not run as trains
            ]

mcLabel = "mc16"
dataLabel = "data18"
truthLabel = "mc15"
delayedStreamLabel = "data16DELAYED"
blsStreamLabel = "data17BPHYSLS"
mcFileBPHY24 = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/mc16_13TeV.801756.P8BEG_A4_CTEQ6L1_Bd_K0_mu4mu4.recon.AOD.e8392_a875_r10724/AOD.29384833._000009.pool.root.1"
mcFileBPHY23 = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.27332925._000069.pool.root.1"
mcFileBPHY22 = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.12399377._000199.pool.root.1"
mcFileBPHY21 = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.16767830._000007.pool.root.1"
mcFileBPHY18 = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.16278878._000048.pool.root.1"
mcFileBPHY20 = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.16471215._000010.pool.root.1"
mcFileBPHY8 = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.11705353._000001.pool.root.1"
mcFileBPHY19 = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.15110756._002435.pool.root.1"
mcFileBPHY13 = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.14763670._000001.pool.root.1"
mcFileBPHY14 = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.13151497._000097.pool.root.1"
mcFileRPVLL = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.14859811._000014.pool.root.1"
mcFileEXOT23 = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/DAOD_RPVLL.15268048._000134.pool.root.1"
mcFile = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.14795494._005958.pool.root.1"
dataFile = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/data18_13TeV.00364292.physics_Main.merge.AOD.f1002_m2037._lb0163._0006.1"
dataFileRPVLL = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/DAOD_RPVLL.13725673._000089.pool.root.1"
dataFileDelayed = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.11270451._000007.pool.root.1"
dataFileBLS = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/data17_13TeV.00337491.physics_BphysLS.merge.AOD.f873_m1885._lb0100._0001.1"
dataFileZeroBias = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/data17_13TeV.00339070.physics_ZeroBias.merge.AOD.f887_m1892._lb0998-lb1007._0001.1"
heavyIonFile = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/AOD.17356768._000726.pool.root.1"#AOD.15763788._016518.pool.root.1"
heavyIon4File = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/data18_hi.00365602.physics_UPC.merge.AOD.f1022_m2037._lb0203._0001.1"#data15_hi.00287866.physics_UPC.merge.AOD.f984_m2025._lb0257._0001.1"
heavyIonHPFile = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/data18_hi.00365602.physics_HardProbes.merge.AOD.f1021_m2037._lb0203._0001.1"
truthFile = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/EVNT.05192704._020091.pool.root.1"
dataPreExec = " --preExec \'rec.doApplyAODFix.set_Value_and_Lock(True); from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = \"BTagCalibRUN12Onl-08-49\"; from AthenaCommon.AlgSequence import AlgSequence; topSequence = AlgSequence(); topSequence += CfgMgr.xAODMaker__DynVarFixerAlg( \"InDetTrackParticlesFixer\", Containers = [ \"InDetTrackParticlesAux.\" ] )\' "
dataPreExecHION = " --preExec \'rec.doApplyAODFix.set_Value_and_Lock(True); from BTagging.BTaggingFlags import BTaggingFlags; BTaggingFlags.CalibrationTag = \"BTagCalibRUN12Onl-08-49\"; from AthenaCommon.AlgSequence import AlgSequence; topSequence = AlgSequence(); topSequence += CfgMgr.xAODMaker__DynVarFixerAlg( \"BTaggingFixer\", Containers=[\"BTagging_AntiKt4HIAux.\"] ); topSequence += CfgMgr.xAODMaker__DynVarFixerAlg( \"InDetTrackParticlesFixer\", Containers = [ \"InDetTrackParticlesAux.\" ] )\' "
dataPreExecHION4 = " --preExec \'rec.doApplyAODFix.set_Value_and_Lock(True); from BTagging.BTaggingFlags import BTaggingFlags; BTaggingFlags.CalibrationTag = \"BTagCalibRUN12Onl-08-49\"; from AthenaCommon.AlgSequence import AlgSequence; topSequence = AlgSequence(); topSequence += CfgMgr.xAODMaker__DynVarFixerAlg( \"BTaggingFixer\", Containers=[\"BTagging_AntiKt4HIAux.\", \"BTagging_AntiKt4EMTopoAux.\"] )\' "
dataPreExecHION5 = " --preExec \'rec.doApplyAODFix.set_Value_and_Lock(True); from HIRecExample.HIRecExampleFlags import jobproperties; jobproperties.HIRecExampleFlags.doHIAODFix.set_Value_and_Lock(True); from BTagging.BTaggingFlags import BTaggingFlags; BTaggingFlags.CalibrationTag = \"BTagCalibRUN12Onl-08-49\"; from AthenaCommon.AlgSequence import AlgSequence; topSequence = AlgSequence(); topSequence += CfgMgr.xAODMaker__DynVarFixerAlg( \"BTaggingFixer\", Containers=[\"BTagging_AntiKt4HIAux.\"] )\' "
dataPreExecHION7 = " --preExec \'rec.doApplyAODFix.set_Value_and_Lock(True); from HIRecExample.HIRecExampleFlags import jobproperties; jobproperties.HIRecExampleFlags.doHIAODFix.set_Value_and_Lock(True); from BTagging.BTaggingFlags import BTaggingFlags; BTaggingFlags.CalibrationTag = \"BTagCalibRUN12Onl-08-49\"; from AthenaCommon.AlgSequence import AlgSequence; topSequence = AlgSequence(); topSequence += CfgMgr.xAODMaker__DynVarFixerAlg( \"BTaggingFixer\", Containers=[\"BTagging_AntiKt4HIAux.\"] ); from HIJetRec.HIJetRecFlags import HIJetFlags; HIJetFlags.DoHIBTagging.set_Value_and_Lock(True)\' "
dataMergePreExec = " --preExec \'rec.doHeavyIon.set_Value_and_Lock(False)\' "
mcPreExec = " --preExec \'rec.doApplyAODFix.set_Value_and_Lock(True);from BTagging.BTaggingFlags import BTaggingFlags;BTaggingFlags.CalibrationTag = \"BTagCalibRUN12-08-49\" \' "

def generateText(formatName,label,inputFile,isTruth,isMC,nEvents):
   outputFileName = "test_"+label+formatName+".sh"
   outputFile = open(outputFileName,"w")
   outputFile.write("#!/bin/sh"+"\n")
   outputFile.write("\n")
   outputFile.write("# art-include: 21.2/AthDerivation"+"\n")
   outputFile.write("# art-description: DAOD building "+formatName+" "+label+"\n")
   outputFile.write("# art-type: grid"+"\n")
   outputFile.write("# art-output: *.pool.root"+"\n")
   outputFile.write("# art-output: checkFile.txt"+"\n")
   outputFile.write("# art-output: checkxAOD.txt"+"\n")
   outputFile.write("\n")
   outputFile.write("set -e"+"\n")
   outputFile.write("\n")
   if ((isTruth==False) and (isMC==False) ): 
      if formatName[0:4] == "HION":# and not formatName == "HION3":
         if formatName == "HION4":
            outputFile.write("Reco_tf.py --inputAODFile "+inputFile+" --outputDAODFile art.pool.root --reductionConf "+formatName+" --maxEvents "+nEvents+dataPreExecHION4+"\n")
         elif formatName in ["HION5"]:
            outputFile.write("Reco_tf.py --inputAODFile "+inputFile+" --outputDAODFile art.pool.root --reductionConf "+formatName+" --maxEvents "+nEvents+dataPreExecHION5+"\n")
         elif formatName in ["HION7","HION8"]:
            outputFile.write("Reco_tf.py --inputAODFile "+inputFile+" --outputDAODFile art.pool.root --reductionConf "+formatName+" --maxEvents "+nEvents+dataPreExecHION7+"\n")
         else:
            outputFile.write("Reco_tf.py --inputAODFile "+inputFile+" --outputDAODFile art.pool.root --reductionConf "+formatName+" --maxEvents "+nEvents+dataPreExecHION+"\n")
      else:
         outputFile.write("Reco_tf.py --inputAODFile "+inputFile+" --outputDAODFile art.pool.root --reductionConf "+formatName+" --maxEvents "+nEvents+dataPreExec+"\n")
   if ((isTruth==False) and (isMC==True) ): outputFile.write("Reco_tf.py --inputAODFile "+inputFile+" --outputDAODFile art.pool.root --reductionConf "+formatName+" --maxEvents "+nEvents+mcPreExec+"\n")
   if (isTruth==True): outputFile.write("Reco_tf.py --inputEVNTFile "+inputFile+" --outputDAODFile art.pool.root --reductionConf "+formatName+" --maxEvents "+nEvents+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $? reco\""+"\n")
   outputFile.write("\n")
   if (isTruth==False): 
      if formatName[0:4] == "HION" and not formatName == "HION3":
         outputFile.write("DAODMerge_tf.py --inputDAOD_"+formatName+"File DAOD_"+formatName+".art.pool.root --outputDAOD_"+formatName+"_MRGFile art_merged.pool.root"+dataMergePreExec+"\n")
      else:
         outputFile.write("DAODMerge_tf.py --inputDAOD_"+formatName+"File DAOD_"+formatName+".art.pool.root --outputDAOD_"+formatName+"_MRGFile art_merged.pool.root"+"\n")
   if (isTruth==True): outputFile.write("DAODMerge_tf.py --inputDAOD_"+formatName+"File DAOD_"+formatName+".art.pool.root --outputDAOD_"+formatName+"_MRGFile art_merged.pool.root"+" --autoConfiguration ProjectName RealOrSim BeamType ConditionsTag DoTruth InputType BeamEnergy LumiFlags TriggerStream --athenaopts=\"-s\" "+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $? merge\""+'\n')
   outputFile.write("\n")
   outputFile.write("checkFile.py DAOD_"+formatName+".art.pool.root > checkFile.txt"+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $?  checkfile\""+'\n')
   outputFile.write("\n")
   outputFile.write("checkxAOD.py DAOD_"+formatName+".art.pool.root > checkxAOD.txt"+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $?  checkxAOD\""+'\n')
   outputFile.close()
   os.system("chmod +x "+outputFileName)

def generateTrains(formatList,label,inputFile,isMC):
   if (isMC == True) and ("BPHY8" in formatList): formatList.remove("BPHY8")
   if (isMC == True) and ("BPHY19" in formatList): formatList.remove("BPHY19")
   if (isMC == True) and ("TOPQ2" in formatList): formatList.remove("TOPQ2")
   outputFileName = "test_"+label+"_".join(formatList)+".sh"
   outputFile = open(outputFileName,"w")
   outputFile.write("#!/bin/sh"+"\n")
   outputFile.write("\n")
   outputFile.write("# art-include: 21.2/AthDerivation"+"\n")
   #outputFile.write("# art-include"+"\n")
   outputFile.write("# art-description: DAOD building "+" ".join(formatList)+" "+label+"\n")
   outputFile.write("# art-type: grid"+"\n")
   outputFile.write("# art-output: *.pool.root"+"\n")
   outputFile.write("\n")
   outputFile.write("set -e"+"\n")
   outputFile.write("\n")
   if (isMC == False): outputFile.write("Reco_tf.py --inputAODFile "+inputFile+" --outputDAODFile art.pool.root --reductionConf "+" ".join(formatList)+" --maxEvents 500 "+dataPreExec+" --passThrough True "+"\n")
   if (isMC == True): outputFile.write("Reco_tf.py --inputAODFile "+inputFile+" --outputDAODFile art.pool.root --reductionConf "+" ".join(formatList)+" --maxEvents 500 "+mcPreExec+" --passThrough True "+"\n")
   os.system("chmod +x "+outputFileName)


if (makeDataDAODs or makeMCDAODs):
   for formatName in formatList:
      if (makeDataDAODs): 
         if formatName in ["EXOT23","SUSY15","SUSY6","EXOT15","EXOT29"]:
            generateText(formatName,dataLabel+"RPVLL",dataFileRPVLL,False,False,"-1")
            if formatName == "SUSY6": generateText(formatName,dataLabel,dataFile,False,False,"-1") 
         elif formatName == "BPHY3":
            generateText(formatName,dataLabel,dataFile,False,False,"500")
         elif formatName in ['BPHY7']:
            generateText(formatName,dataLabel,dataFile,False,False,"-1")
            generateText(formatName,delayedStreamLabel,dataFileDelayed,False,False,"-1")
            generateText(formatName,blsStreamLabel,dataFileBLS,False,False,"500")
         elif formatName in ['BPHY8']:
            generateText(formatName,dataLabel,dataFile,False,False,"-1")
            generateText(formatName,delayedStreamLabel,dataFileDelayed,False,False,"-1")
            generateText(formatName,blsStreamLabel,dataFileBLS,False,False,"1000") 
         elif formatName in ['BPHY13', 'BPHY18','BPHY21','BPHY22','BPHY23','BPHY24']:
            generateText(formatName,dataLabel,dataFile,False,False,"-1")
            generateText(formatName,blsStreamLabel,dataFileBLS,False,False,"1000") 
         elif formatName in ['BPHY10','BPHY19']:
            generateText(formatName,dataLabel,dataFile,False,False,"-1")
            generateText(formatName,delayedStreamLabel,dataFileDelayed,False,False,"-1")
            generateText(formatName,blsStreamLabel,dataFileBLS,False,False,"-1")
         elif formatName in ['STDM12','STDM7']:
            generateText(formatName,dataLabel,dataFile,False,False,"-1")
            generateText(formatName,blsStreamLabel,dataFileBLS,False,False,"-1")
         elif formatName=='BPHY20':
            generateText(formatName,blsStreamLabel,dataFileBLS,False,False,"-1")
         elif formatName=='JETM5':
            generateText(formatName,dataLabel,dataFileZeroBias,False,False,"-1")
         elif formatName=='HION4':
            generateText(formatName,dataLabel,heavyIon4File,False,False,"-1")
         elif formatName in ['HION5','HION7','HION8']:
            generateText(formatName,dataLabel,heavyIonHPFile,False,False,"-1")
         elif formatName[0:4]=='HION':
            generateText(formatName,dataLabel,heavyIonFile,False,False,"-1")
         else: generateText(formatName,dataLabel,dataFile,False,False,"-1")
      if (makeMCDAODs):
         if formatName == "TOPQ2": continue # only for data
         if formatName[0:4]=='HION' and not formatName=='HION3': continue # only HION3 runs on MC
         if formatName in ["SUSY15","SUSY6","EXOT15","EXOT29"]:
            generateText(formatName,mcLabel+"RPVLL",mcFileRPVLL,False,True,"-1")
            if formatName == "SUSY6":generateText(formatName,mcLabel,mcFile,False,True,"-1") 
         elif formatName == "EXOT23":
            generateText(formatName,mcLabel+"RPVLL",mcFileEXOT23,False,True,"-1")
         elif formatName=="BPHY3":
            generateText(formatName,mcLabel,mcFile,False,True,"500")
         elif formatName=="BPHY8":
            generateText(formatName,mcLabel,mcFileBPHY8,False,True,"-1")
         elif formatName=="BPHY19":
            generateText(formatName,mcLabel,mcFileBPHY19,False,True,"5000")
         elif formatName=="BPHY13":
            generateText(formatName,mcLabel,mcFileBPHY13,False,True,"-1")
         elif formatName=="BPHY14":
            generateText(formatName,mcLabel,mcFileBPHY14,False,True,"-1")
         elif formatName=="BPHY18":
            generateText(formatName,mcLabel,mcFileBPHY18,False,True,"5000")
         elif formatName=="BPHY20":
            generateText(formatName,mcLabel,mcFileBPHY20,False,True,"5000")
         elif formatName=="BPHY21":
            generateText(formatName,mcLabel,mcFileBPHY21,False,True,"5000")
         elif formatName=="BPHY22":
            generateText(formatName,mcLabel,mcFileBPHY22,False,True,"5000")
         elif formatName=="BPHY23":
            generateText(formatName,mcLabel,mcFileBPHY23,False,True,"-1")
         elif formatName=="BPHY24":
            generateText(formatName,mcLabel,mcFileBPHY24,False,True,"5000")
         else: generateText(formatName,mcLabel,mcFile,False,True,"-1")

if (makeTruthDAODs):
   for formatName in truthFormatList:
      generateText(formatName,truthLabel,truthFile,True,False,"1000")

if (makeTrains):
   for train in trainList:
      generateTrains(train,dataLabel,dataFile,False)
      generateTrains(train,mcLabel,mcFile,True)
   generateTrains(['TOPQ1','TOPQ4','TOPQ5'],mcLabel,mcFile,True) # special train, not run in production but needed for testing purposes
   generateTrains(["SUSY15","EXOT23","EXOT15","SUSY6","EXOT29"],mcLabel,mcFileEXOT23,True)
   generateTrains(["SUSY15","EXOT23","EXOT15","SUSY6","EXOT29"],dataLabel,dataFileRPVLL,False)
