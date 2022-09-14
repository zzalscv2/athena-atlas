import os

makeDataDAODs=True
makeMCDAODs=True
makeTruthDAODs=True
makeTrains=True

formatList = ["PHYSVAL","PHYS","PHYSLITE",
              "LLP1",
              "JETM1","JETM3","JETM4","JETM5","JETM6","JETM8","JETM9","JETM10","JETM11","JETM12","JETM13","JETM14","JETM15",
              "IDTR2",
              "EGAM1", "EGAM2", "EGAM3", "EGAM4", "EGAM5", "EGAM7", "EGAM8", "EGAM9", "EGAM10",
              "FTAG1", "FTAG2",
              "BPHY1", "BPHY2", "BPHY4", "BPHY10", "BPHY12", "BPHY13", "BPHY15", "BPHY16", "BPHY21", "BPHY22"
]

truthFormatList = ["TRUTH0", "TRUTH1", "TRUTH3"]

trainList = [
              ["EGAM2","EGAM3","EGAM4","JETM12"],
              ["EGAM7","EGAM8","EGAM9","JETM3"],
              ["JETM10","JETM14"],
              ["EGAM1","JETM4","JETM11"],
              ["EGAM5","JETM5"],
              ["JETM1","JETM9"]
]


mcLabel = "mc20"
dataLabel = "data18"
truthLabel = "mc15"

mcFile = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3681_r13167/AOD.27162646._000001.pool.root.1"
dataFile = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/data18_13TeV.00357772.physics_Main.recon.AOD.r13286/AOD.27654050._000557.pool.root.1"
validFile = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.recon.AOD.e4993_s3834_r13556/AOD.28847225._00000*"
truthFile = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/DerivationFrameworkART/mc16_13TeV.410637.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad_2000_14000.merge.EVNT.e6685_e5984/EVNT.15803543._000001.pool.root.1"

preExec = "\'from AthenaCommon.DetFlags import DetFlags; DetFlags.detdescr.all_setOff(); DetFlags.BField_setOn(); DetFlags.digitize.all_setOff(); DetFlags.detdescr.Calo_setOn(); DetFlags.simulate.all_setOff(); DetFlags.pileup.all_setOff(); DetFlags.overlay.all_setOff(); DetFlags.detdescr.Muon_setOn();\'"
preExecLLP = preExec[:-1]+" DetFlags.detdescr.pixel_setOn(); DetFlags.detdescr.SCT_setOn(); from InDetRecExample.InDetJobProperties import InDetFlags; InDetFlags.doR3LargeD0.set_Value_and_Lock(True);\'"
postExec = "\'from DerivationFrameworkJetEtMiss.JetCommon import swapAlgsInSequence; swapAlgsInSequence(topSequence,\"jetalg_ConstitModCorrectPFOCSSKCHS_GPFlowCSSK\", \"UFOInfoAlgCSSK\" );\'"

def generateText(formatName,label,inputFile,isTruth,isMC,nEvents):
   outputFileName = "test_"+label+formatName+".sh"
   outputFile = open(outputFileName,"w")
   outputFile.write("#!/bin/sh"+"\n")
   outputFile.write("\n")
   outputFile.write("# art-include: master/AthDerivation"+"\n")
   outputFile.write("# art-include: master/Athena"+"\n")
   outputFile.write("# art-description: DAOD building "+formatName+" "+label+"\n")
   outputFile.write("# art-type: grid"+"\n")
   outputFile.write("# art-output: *.pool.root"+"\n")
   outputFile.write("# art-output: checkFile.txt"+"\n")
   outputFile.write("# art-output: checkxAOD.txt"+"\n")
   outputFile.write("# art-output: checkIndexRefs.txt"+"\n")
   outputFile.write("\n")
   outputFile.write("set -e"+"\n")
   outputFile.write("\n")
   if (not isTruth):
      outputFile.write('Reco_tf.py \\\n')
      outputFile.write('--inputAODFile '+inputFile+' \\\n')
      outputFile.write("--outputDAODFile art.pool.root \\\n")
      outputFile.write("--reductionConf "+formatName+" \\\n")
      outputFile.write("--maxEvents "+nEvents+" \\\n")
      if (formatName in ["LLP1","IDTR2"]):
         outputFile.write("--preExec "+preExecLLP+" \\\n")
      else:
         outputFile.write("--preExec "+preExec+" \\\n")
      outputFile.write("--postExec "+postExec+" \\\n")
      if (formatName not in ["PHYS","PHYSLITE"] and isMC):
         outputFile.write("--passThrough True\n")
   if isTruth: 
      outputFile.write("Derivation_tf.py \\\n")
      outputFile.write("--CA \\\n") 
      outputFile.write("--inputEVNTFile "+inputFile+" \\\n")
      outputFile.write("--outputDAODFile art.pool.root \\\n")
      outputFile.write("--formats "+formatName+" \\\n") 
      outputFile.write("--maxEvents "+nEvents+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $? reco\""+"\n")
   outputFile.write("\n")
   outputFile.write("checkFile.py DAOD_"+formatName+".art.pool.root > checkFile.txt"+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $?  checkfile\""+'\n')
   outputFile.write("\n")
   outputFile.write("checkxAOD.py DAOD_"+formatName+".art.pool.root > checkxAOD.txt"+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $?  checkxAOD\""+'\n')
   outputFile.write("\n")
   outputFile.write("checkIndexRefs.py DAOD_"+formatName+".art.pool.root > checkIndexRefs.txt"+"\n")
   outputFile.write("\n")
   outputFile.write("echo \"art-result: $?  checkIndexRefs\""+'\n')
   outputFile.close()
   os.system("chmod +x "+outputFileName)

def generateTrains(formatList,label,inputFile,isMC):
   outputFileName = "test_"+label+"_".join(formatList)+".sh"
   outputFile = open(outputFileName,"w")
   outputFile.write("#!/bin/sh"+"\n")
   outputFile.write("\n")
   outputFile.write("# art-include: master/AthDerivation"+"\n")
   outputFile.write("# art-include: master/Athena"+"\n")
   outputFile.write("# art-description: DAOD building "+" ".join(formatList)+" "+label+"\n")
   outputFile.write("# art-type: grid"+"\n")
   outputFile.write("# art-output: *.pool.root"+"\n")
   outputFile.write("\n")
   outputFile.write("set -e"+"\n")
   outputFile.write("\n")
   outputFile.write("Reco_tf.py \\\n")
   outputFile.write("--inputAODFile "+inputFile+" \\\n") 
   outputFile.write("--outputDAODFile art.pool.root \\\n") 
   outputFile.write("--reductionConf "+" ".join(formatList)+" \\\n") 
   outputFile.write("--maxEvents 500 \\\n")
   outputFile.write("--preExec "+preExec+" \\\n")
   outputFile.write("--postExec "+postExec+"\n")
   os.system("chmod +x "+outputFileName)

if (makeDataDAODs or makeMCDAODs):
   for formatName in formatList:
      if makeDataDAODs: 
         generateText(formatName,dataLabel,dataFile,False,False,"-1")
      if makeMCDAODs:
         if formatName == "PHYSVAL":
            generateText(formatName,"valid1",validFile,False,True,"-1")
         else:
            generateText(formatName,mcLabel,mcFile,False,True,"-1")
if makeTruthDAODs:
   for formatName in truthFormatList:
      generateText(formatName,truthLabel,truthFile,True,False,"1000")

if makeTrains:
   for train in trainList:
      generateTrains(train,dataLabel,dataFile,False)
      generateTrains(train,mcLabel,mcFile,True)
