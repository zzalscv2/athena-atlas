
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon.CFElements import seqAND
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory, isComponentAccumulatorCfg
from DecisionHandling.HLTSignatureHypoTools import MuTestHypoTool, ElTestHypoTool
from TriggerMenuMT.HLT.Config.MenuComponents import RecoFragmentsPool, MenuSequence
from TriggerMenuMT.HLT.Config.MenuComponents import MenuSequenceCA, SelectionCA, InEventRecoCA

import sys

HLTTest__TestHypoAlg=CompFactory.getComp("HLTTest::TestHypoAlg")
HLTTest__TestRecoAlg=CompFactory.getComp("HLTTest::TestRecoAlg")
HLTTest__TestInputMaker=CompFactory.getComp("HLTTest::TestInputMaker")

UseThisLinkName="initialRoI"


def InputMakerForInitialRoIAlg(name):
    return HLTTest__TestInputMaker(name, RoIsLink="initialRoI", LinkName="initialRoI")

def InputMakerForFeatureAlg(name):
    return HLTTest__TestInputMaker(name, RoIsLink="initialRoI", LinkName=UseThisLinkName)


#generalize


def makeSequence(flags, name,step, signature):
    """
    generate reco sequence for emulation chains
    """
    IM= InputMakerForFeatureAlg("IM"+signature+name+"Step"+step)
    IM.Output=name+signature+"IM"+step+"_out"
    if "el" in signature:
        Alg = CaloClustering("CaloClustering"+name+"Step"+step, FileName="emclusters.dat")
    elif "mu" in signature:
        Alg = muMSRecAlg("muMSRecAlg"+name+"Step"+step, FileName="msmu.dat")
    else:
        sys.exit("ERROR, in configuration of sequence "+name+step+signature)

    Alg.Output = name+signature+"Alg"+step+"_out"
    Alg.Input  = IM.Output
    
    if isComponentAccumulatorCfg():
        accAlg = ComponentAccumulator()
        accAlg.addEventAlgo(Alg)
        InEventReco = InEventRecoCA(name+signature+"SeqStep"+step,inputMaker=IM)
        InEventReco.mergeReco(accAlg)  
            
        return (InEventReco,IM, Alg.Output)
    else:
        Sequence   = seqAND(name+signature+"SeqStep"+step, [IM, Alg])
        return (Sequence, IM, Alg.Output)




# here define the sequences from the signatures
# signatures do this:
# - declare all the RecoAlg and the HypoAlg -> create the Sequence
# - creates the InputMaker, without the inputs



#### muon signatures
#####################


def muMSRecAlg(name, FileName="noreco.dat"):
    return HLTTest__TestRecoAlg(name=name, FileName=FileName)

def MuHypo(name):
    return HLTTest__TestHypoAlg(name=name, LinkName=UseThisLinkName)

def makeMuSequence(flags, name,step):
    return makeSequence(flags, name,step, "mu")


## ##### electron signatures
## ##########################

def CaloClustering(name,  FileName="noreco.dat"):
    return HLTTest__TestRecoAlg(name=name, FileName=FileName)

def ElGamHypo(name):
    return HLTTest__TestHypoAlg(name=name, LinkName=UseThisLinkName)

def makeElSequence(flags, name,step):
    return makeSequence(flags, name,step, "el")


def elMenuSequence(flags, step, reconame, hyponame):
    (Sequence, IM, seqOut) = RecoFragmentsPool.retrieve(makeElSequence,flags,name=reconame, step=step)
    elHypo = ElGamHypo(hyponame+"Step"+step+"ElHypo")
    elHypo.Input = seqOut
    if isComponentAccumulatorCfg():
        selAcc=SelectionCA(hyponame+"elStep"+step)        
        selAcc.mergeReco(Sequence) 
        selAcc.addHypoAlgo(elHypo)
        return MenuSequenceCA(flags, selAcc, HypoToolGen=ElTestHypoTool)
    else:
        return MenuSequence(flags, Maker=IM, Sequence=Sequence, Hypo=elHypo, HypoToolGen=ElTestHypoTool)
   

def gamMenuSequence(flags, step, reconame, hyponame):
    (Sequence, IM, seqOut) = RecoFragmentsPool.retrieve(makeElSequence,flags,name=reconame, step=step)
    elHypo = ElGamHypo(hyponame+"Step"+step+"GamHypo")
    elHypo.Input = seqOut
    if isComponentAccumulatorCfg():
        selAcc=SelectionCA(hyponame+"gamStep"+step+"Gam")        
        selAcc.mergeReco(Sequence) 
        selAcc.addHypoAlgo(elHypo)
        return MenuSequenceCA(flags,selAcc, HypoToolGen=ElTestHypoTool)
    else:
        return MenuSequence(flags, Maker=IM, Sequence=Sequence, Hypo=elHypo, HypoToolGen=ElTestHypoTool)
    


def muMenuSequence(flags, step, reconame, hyponame):
    (Sequence, IM, seqOut) = RecoFragmentsPool.retrieve(makeMuSequence,flags,name=reconame, step=step)
    muHypo = MuHypo(hyponame+"Step"+step+"MuHypo")
    muHypo.Input = seqOut
    if isComponentAccumulatorCfg():
        selAcc=SelectionCA(hyponame+"muStep"+step)        
        selAcc.mergeReco(Sequence) 
        selAcc.addHypoAlgo(muHypo)
        return MenuSequenceCA(flags, selAcc, HypoToolGen=MuTestHypoTool)
    else:
        return MenuSequence(flags, Maker=IM, Sequence=Sequence, Hypo=muHypo, HypoToolGen=MuTestHypoTool)
    
        
def genMenuSequence(flags, step, reconame, hyponame):
    (Sequence, IM, seqOut) = RecoFragmentsPool.retrieve(makeElSequence,flags,name=reconame, step=step)
    elHypo = ElGamHypo(hyponame+"Hypo")
    elHypo.Input = seqOut
    if isComponentAccumulatorCfg():
        selAcc=SelectionCA(hyponame+"elStep"+step)        
        selAcc.mergeReco(Sequence) 
        selAcc.addHypoAlgo(elHypo)
        return MenuSequenceCA(flags, selAcc, HypoToolGen=ElTestHypoTool)
    else:
        return MenuSequence(flags, Maker=IM, Sequence=Sequence, Hypo=elHypo, HypoToolGen=ElTestHypoTool)
