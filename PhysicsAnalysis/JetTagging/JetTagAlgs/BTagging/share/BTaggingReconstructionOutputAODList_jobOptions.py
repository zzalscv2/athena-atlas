include.block('BTagging/BTaggingReconstructionOutputAODList_jobOptions.py')
#**************   AOD list  ************************************************
from BTagging.BTagConfig import registerContainer

printfunc ("List of containers")
JetCollectionList = ['AntiKt4LCTopo', 'AntiKt4EMTopo', 'AntiKt4Track', 'AntiKt4EMPFlow', 'AntiKt2Track', 'AntiKt4HI']
from JetRec.JetRecFlags import jetFlags
BTaggingAODList = []
BTaggingESDList = []
for coll in JetCollectionList:
    if jetFlags.writeJetsToAOD():
        registerContainer(coll, BTaggingAODList)
    registerContainer(coll, BTaggingESDList)

printfunc ("#BTAG# ESD output container list: " + str(BTaggingESDList))
printfunc ("#BTAG# AOD output container list: " + str(BTaggingAODList))
