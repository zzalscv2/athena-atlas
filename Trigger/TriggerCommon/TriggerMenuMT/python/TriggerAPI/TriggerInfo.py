# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__author__  = 'Javier Montejo'
__version__="$Revision: 2.0 $"
__doc__="Class containing all the information of an HLT chain"

import re
from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerType, TriggerPeriod
from collections import Counter
import six
from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

class TriggerInfo:
    ''' Object containing all the HLT information related to a given period.
        Stores a list of TriggerChain objects and the functions to skim them
    '''
    def __init__(self,period=0, customGRL=None, release=None, flags=None):
        self.triggerChains = []
        self.period = period
        self.totalLB = 0

        if not period: return
        from .TriggerDataAccess import getHLTlist
        HLTlist, totalLB = getHLTlist(period, customGRL, release, flags)
        self.totalLB = totalLB
        for hlt, l1, livefraction, activeLB, hasRerun in HLTlist:
            self.triggerChains.append( TriggerChain(hlt, l1, livefraction, activeLB, hasRerun))

    def toJSON(self):
        return dict(period= TriggerPeriod.toName(self.period), totalLB=self.totalLB,triggerChains=self.triggerChains)

    @classmethod
    def merge(cls,listofTI):
        from copy import deepcopy
        mergedTI = TriggerInfo()
        mergedHLTmap = {}
        for ti in listofTI:
            mergedTI.period |= ti.period
            mergedTI.totalLB += ti.totalLB
            for tc in ti.triggerChains:
                if tc.name not in mergedHLTmap: mergedHLTmap[tc.name] = deepcopy(tc)
                else: mergedHLTmap[tc.name].activeLB += tc.activeLB
        for tc in six.itervalues (mergedHLTmap):
            tc.livefraction = tc.activeLB/float(mergedTI.totalLB)
        mergedTI.triggerChains = list(mergedHLTmap.values())
        return mergedTI
        

    @classmethod
    def testCustomGRL(cls, grl):
        from TriggerMenuMT.TriggerAPI.TriggerPeriodData import TriggerPeriodData
        return TriggerPeriodData.testCustomGRL(grl)

    def reparse(self):
        self.triggerChains = [ TriggerChain(t.name, t.l1seed, t.livefraction, t.activeLB) for t in self.triggerChains ]

    def _getUnprescaled(self,triggerType, additionalTriggerType, matchPattern, livefraction=1.0):
        return [x.name for x in self.triggerChains if x.isUnprescaled(livefraction) and x.passType(triggerType, additionalTriggerType) and re.search(matchPattern, x.name)]

    def _getLowestUnprescaled(self,triggerType, additionalTriggerType, matchPattern, livefraction=1.0):
        chainList = [ x for x in self.triggerChains if x.isUnprescaled(livefraction) and x.passType(triggerType, additionalTriggerType) and re.search(matchPattern, x.name)]
        typeMap = {}
        for chain in chainList:
            if chain.triggerType not in typeMap:
                typeMap[chain.triggerType] = [chain]
                continue
            append = False
            for other in typeMap[chain.triggerType][:]:
                comp = chain.isLowerThan(other,self.period)
                if comp ==  0: 
                    append = False
                    break
                append = True
                if comp ==  1:    typeMap[chain.triggerType].remove(other)
            if append:
                typeMap[chain.triggerType].append(chain)
        return [x.name for t in six.itervalues (typeMap) for x in t ]


    def _getAllHLT(self,triggerType, additionalTriggerType, matchPattern, livefraction):
        return {x.name: x.livefraction for x in self.triggerChains if x.passType(triggerType, additionalTriggerType) and re.search(matchPattern, x.name) and x.isUnprescaled(livefraction)}

    def _getActive(self,triggerType, additionalTriggerType, matchPattern, livefraction):
        return [x.name for x in self.triggerChains if x.isActive(livefraction) and x.passType(triggerType, additionalTriggerType) and re.search(matchPattern, x.name)]
    def _getInactive(self,triggerType, additionalTriggerType, matchPattern, livefraction):
        return [x.name for x in self.triggerChains if x.isInactive(livefraction) and x.passType(triggerType, additionalTriggerType) and re.search(matchPattern, x.name)]

    def _checkPeriodConsistency(self,triggerType, additionalTriggerType, matchPattern):
        inconsistent = set()
        for i in range(len(self.triggerChains)):
            probe1 = self.triggerChains[i]
            if not (probe1.passType(triggerType, additionalTriggerType) and re.search(matchPattern, probe1.name)): continue
            for j in range(i+1,len(self.triggerChains)):
                probe2 = self.triggerChains[j]
                if not (probe2.passType(triggerType, additionalTriggerType) and re.search(matchPattern, probe2.name)): continue
                if probe1.isUnprescaled() and not probe2.isUnprescaled() and probe1.isLowerThan(probe2,self.period)==1:
                    log.error(f"Found {probe2.name} that is tighter than Primary {probe1.name}")
                    inconsistent.add(probe2.name)
                if probe2.isUnprescaled() and not probe1.isUnprescaled() and probe2.isLowerThan(probe1,self.period)==1:
                    log.error(f"Found {probe1.name} that is tighter than Primary {probe2.name}")
                    inconsistent.add(probe1.name)
                
        return inconsistent


class TriggerLeg:
    types          = ('e','j','mu','tau','xe','g','ht')
    legpattern     = re.compile('([0-9]*)(%s)([0-9]+)(noL1)?' % '|'.join(types))
    detailpattern  = re.compile(r'(?:-?\d+)|(?:[^0-9 -]+)') #split into text-only vs number-only
    bjetpattern    = re.compile('bmv|bhmv|btight|bmedium|bloose|bld1')
    bphyspattern   = re.compile('b[A-Z]')
    exoticspattern = re.compile('llp|LLP|muvtx|hiptrt|LATE|NOMATCH')
    afppattern     = re.compile('afp|AFP')

    def __init__(self,legname, chainseed, chainname=None):
        self.legname = legname
        self.l1seed = ""
        details = []
        chainseed= chainseed.replace("L1_","")
        blocks = legname.split("_L1")

        for token in blocks[0].split("_"):
            m = self.legpattern.match(token)
            if m:
                count,legtype,thr,noL1 = m.groups()
                self.count = int(count) if count else 1
                self.thr = int(thr)
                if legtype == 'e':
                    if self.count > 1: self.legtype = TriggerType.el_multi
                    else:              self.legtype = TriggerType.el_single
                elif legtype == 'mu':
                    if self.count > 1: self.legtype = TriggerType.mu_multi
                    else:              self.legtype = TriggerType.mu_single
                elif legtype == 'j':
                    if self.count > 1: self.legtype = TriggerType.j_multi
                    else:              self.legtype = TriggerType.j_single
                elif legtype == 'tau':
                    if self.count > 1: self.legtype = TriggerType.tau_multi
                    else:              self.legtype = TriggerType.tau_single
                elif legtype == 'g':
                    if self.count > 1: self.legtype = TriggerType.g_multi
                    else:              self.legtype = TriggerType.g_single
                elif legtype == 'xe':
                    self.legtype = TriggerType.xe
                elif legtype == 'ht':
                    self.legtype = TriggerType.ht
                else:
                    log.info("Unknown trigger type:",legtype)
                if noL1: details.append(noL1)
            else:
                if self.bjetpattern.match(token):
                    if self.legtype == TriggerType.j_single: self.legtype = TriggerType.bj_single
                    if self.legtype == TriggerType.j_multi:  self.legtype = TriggerType.bj_multi
                if self.bphyspattern.match(token):
                    self.legtype = TriggerType.mu_bphys
                if self.exoticspattern.search(token):
                    self.legtype = TriggerType.exotics
                if self.afppattern.search(token):
                    self.legtype = TriggerType.afp
                details.append(token)

        for l1seed in blocks[1:]:
            if self.exoticspattern.search(l1seed):
                self.legtype = TriggerType.exotics
            if self.afppattern.search(l1seed):
                self.legtype = TriggerType.afp
            if l1seed == chainseed: continue
            else: 
                assert self.l1seed=="", (self.l1seed, chainseed, chainname, blocks[1:])
                self.l1seed = l1seed
        if not self.l1seed: self.l1seed = chainseed
        self.details = tuple(details)

    def __eq__(self,other):
        return (self.l1seed == other.l1seed and self.count == other.count and self.thr == other.thr and self.legtype == other.legtype and self.details == other.details)

    def __hash__(self):
        return hash((self.l1seed,self.count,self.thr,self.legtype,self.details))

    def __repr__(self):
        return "{0} {1} {2} {3} {4} {5:b}".format(self.legname,self.l1seed,self.count,self.thr,self.details,self.legtype)

    def isLegLowerThan(self, other, is2015, debug=False):
        ''' Returns -9 if none of them is lower than the other (e.g. different met flavour).
            Returns -1 if identical
            Returns  0 if other is lower than self.
            Returns  1 if self  is lower than other.
        '''
        if debug:
            log.info("DEBUG LEGS --------")
            log.info(self.legname, other.legname)
            log.info(self.legtype, other.legtype)
            log.info(self.l1seed, other.l1seed)
            log.info(self.details, other.details)
            log.info(self.thr, other.thr)
            log.info(self.compareDetails(other, is2015, debug=True))
            log.info(self.details == other.details)
            log.info("DEBUG LEGS END --------")

        if self.legtype != other.legtype: return -9
        if self.compareDetails(other, is2015) == -1:
            if self.thr < other.thr: return 1
            if self.thr > other.thr: return 0
            else: return -1

        if self.compareDetails(other, is2015) == 1 and self.thr  <= other.thr: return 1
        if self.compareDetails(other, is2015) == 0 and other.thr <= self.thr:  return 0
        return -9

    def compareDetails(self, other, is2015, debug=False):
        ''' Returns -9 if none of them is lower than the other (e.g. different met flavour).
            Returns -1 if identical
            Returns  0 if other is lower than self.
            Returns  1 if self  is lower than other.
        '''
        from copy import deepcopy

        if debug: log.info("compareDetails:",len(self.details), len(other.details),(self.l1seed == other.l1seed),(self.details == other.details) )
        if len(self.details) != len(other.details): 
            if not is2015 and any([x.startswith("noL1") for x in self.details]):
                cloneself = deepcopy(self)
                cloneself.details = [ x for x in self.details if not x.startswith("noL1")]
                compno = cloneself.compareDetails(other,is2015,debug)
                if compno ==1 or compno == -1: 
                    return 1
            if not is2015 and any([x.startswith("noL1") for x in other.details]):
                cloneother = deepcopy(other)
                cloneother.details = [ x for x in other.details if not x.startswith("noL1")]
                compno = self.compareDetails(cloneother,is2015,debug)
                if compno ==0 or compno == -1:
                    return 0
            if not is2015 and any([x.startswith("nod0") for x in self.details]):
                cloneself = deepcopy(self)
                cloneself.details = [ x for x in self.details if not x.startswith("nod0")]
                compno = cloneself.compareDetails(other,is2015,debug)
                if compno ==1 or compno == -1: 
                    return 1
            if not is2015 and any([x.startswith("nod0") for x in other.details]):
                cloneother = deepcopy(other)
                cloneother.details = [ x for x in other.details if not x.startswith("nod0")]
                compno = self.compareDetails(cloneother,is2015,debug)
                if compno ==0 or compno == -1:
                    return 0
            if any([x.startswith("cut") for x in self.details]):
                cloneself = deepcopy(self)
                cloneself.details = [ x for x in self.details if not x.startswith("cut")]
                compno = cloneself.compareDetails(other,is2015,debug)
                if compno ==0 or compno == -1: 
                    return 0
            if any([x.startswith("cut") for x in other.details]):
                cloneother = deepcopy(other)
                cloneother.details = [ x for x in other.details if not x.startswith("cut")]
                compno = self.compareDetails(cloneother,is2015,debug)
                if compno ==1 or compno == -1:
                    return 1
            return -9
        compl1seed  = self.compareTags(self.l1seed, other.l1seed, stringSubset=True, debug=debug)
        compdetails = self.compareTags(" ".join(self.details), " ".join(other.details), debug=debug )
        if self.l1seed == other.l1seed:
            if self.details == other.details: return -1
            if debug: log.info("compareTags 1:",compdetails)
            return compdetails

        if self.details == other.details:
            if debug: log.info("compareTags 2:",compl1seed)
            return compl1seed

        if compl1seed == compdetails:
            return compl1seed
        return -9

    def compareTags(self, tag1, tag2, stringSubset=False,debug=False):
        def mycomp(x,y):
            ''' Return -9 for different strings, 
                -1 for identical strings/nums, 
                0/1 for high/low numbers or string subsets
            '''
            try: 
                x,y = int(x), int(y)
                if x < y: return 1
                elif x > y: return 0
                else: return -1
            except ValueError: 
                if x==y: return -1
                if x == y.replace("vloose","loose"): return 0
                if x == y.replace("vloose","loose").replace("loose","medium"): return 0
                if x == y.replace("vloose","loose").replace("loose","medium").replace("medium","tight"): return 0
                if y == x.replace("vloose","loose"): return 1
                if y == x.replace("vloose","loose").replace("loose","medium"): return 1
                if y == x.replace("vloose","loose").replace("loose","medium").replace("medium","tight"): return 1
                if stringSubset:
                    if x in y: return 1
                    if y in x: return 0
                return -9

        if tag1 == tag2: return -1
        #lower mv2 and deltaR/deltaZ/deltaPhi values are tighter, put a minus sign to trick it
        inverseCuts = ("mv2c","dr","dz","dphi","dl1d","dl1r","gn1")
        for cut in inverseCuts:
            tag1 = tag1.replace(cut,cut+"-")
            tag2 = tag2.replace(cut,cut+"-")
        #only make a statement on the numerical values, with everything else identical
        reself  = self.detailpattern.findall(tag1)
        reother = self.detailpattern.findall(tag2)

        if len(reself) != len(reother): return -9
        thecomp = [mycomp(a,b) for a,b in zip(reself,reother)]
        if debug: log.info("thecomp:",thecomp,reself,reother)
        if any([x == -9 for x in thecomp]): return -9
        if all([x !=0 for x in thecomp]) and any([x == 1 for x in thecomp]): return 1
        if all([x !=1 for x in thecomp]) and any([x == 0 for x in thecomp]): return 0
        return -9

    @classmethod
    def parse_legs(cls,name,l1seed,chainname):
        legsname = []
        name = name.replace("HLT_","")
        for token in name.split("_"):
            m = cls.legpattern.match(token)
            if m:
                legsname.append(token)
            elif legsname: 
                legsname[-1] += "_"+token
            else: #first token doesn't match
                #log.info("parse_legs: Problem parsing",name)
                return []
        return [TriggerLeg(l,l1seed,chainname) for l in legsname]

class TriggerChain:
    l1types        = ('EM','J','MU','TAU','XE','XS','HT')
    l1pattern      = re.compile('([0-9]*)(%s)([0-9]+)' % '|'.join(l1types))

    def __init__(self,name,l1seed,livefraction,activeLB=1,hasRerun=False):
        self.name = name
        self.l1seed = l1seed
        tmplegs = TriggerLeg.parse_legs(name,l1seed,name)
        self.legs = self.splitAndOrderLegs(tmplegs)
        self.livefraction = livefraction
        self.activeLB = activeLB
        self.hasRerun = hasRerun
        self.triggerType = self.getTriggerType(self.legs, l1seed)

    def toJSON(self):
        return dict(name=self.name, l1seed=self.l1seed, livefraction=self.livefraction, activeLB=self.activeLB, hasRerun=self.hasRerun)

    def splitAndOrderLegs(self, legs):
        from copy import deepcopy
        newLegs = []
        for triggerType in TriggerType:
            for l in legs:
                if not l.legtype == triggerType: continue
                for i in range(l.count): #split into N single legs
                    tmp = deepcopy(l)
                    tmp.count = 1
                    if tmp.legtype & TriggerType.el_multi:
                        tmp.legtype |=  TriggerType.el_single
                        tmp.legtype &= ~TriggerType.el_multi
                    elif tmp.legtype & TriggerType.mu_multi:
                        tmp.legtype |=  TriggerType.mu_single
                        tmp.legtype &= ~TriggerType.mu_multi
                    elif tmp.legtype & TriggerType.tau_multi:
                        tmp.legtype |=  TriggerType.tau_single
                        tmp.legtype &= ~TriggerType.tau_multi
                    elif tmp.legtype & TriggerType.j_multi:
                        tmp.legtype |=  TriggerType.j_single
                        tmp.legtype &= ~TriggerType.j_multi
                    elif tmp.legtype & TriggerType.bj_multi:
                        tmp.legtype |=  TriggerType.bj_single
                        tmp.legtype &= ~TriggerType.bj_multi
                    elif tmp.legtype & TriggerType.g_multi:
                        tmp.legtype |=  TriggerType.g_single
                        tmp.legtype &= ~TriggerType.g_multi
                    newLegs.append(tmp)
        return newLegs

    def getTriggerType(self, legs, l1seed):
        mtype = TriggerType.UNDEFINED
        for l in legs:
            if mtype & TriggerType.el and l.legtype & TriggerType.el:
                mtype |=  TriggerType.el_multi
                mtype &= ~TriggerType.el_single
            elif mtype & TriggerType.mu and l.legtype & TriggerType.mu:
                mtype |=  TriggerType.mu_multi
                mtype &= ~TriggerType.mu_single
            elif mtype & TriggerType.tau and l.legtype & TriggerType.tau:
                mtype |=  TriggerType.tau_multi
                mtype &= ~TriggerType.tau_single
            elif mtype & TriggerType.j and l.legtype & TriggerType.j:
                mtype |=  TriggerType.j_multi
                mtype &= ~TriggerType.j_single
            elif mtype & TriggerType.bj and l.legtype & TriggerType.bj:
                mtype |=  TriggerType.bj_multi
                mtype &= ~TriggerType.bj_single
            elif mtype & TriggerType.g and l.legtype & TriggerType.g:
                mtype |=  TriggerType.g_multi
                mtype &= ~TriggerType.g_single
            elif l.legtype & TriggerType.mu_bphys:
                mtype |=  TriggerType.mu_bphys
                mtype &= ~(TriggerType.mu_single | TriggerType.mu_multi)
            elif l.legtype & TriggerType.exotics:
                mtype |=  TriggerType.exotics
            elif l.legtype & TriggerType.afp:
                mtype  =  TriggerType.afp #on purpose not OR-ed
            else:
                mtype |= l.legtype

        l1seed= l1seed.replace("L1_","")
        if mtype & TriggerType.exotics or mtype & TriggerType.afp:
            return mtype
        for token in l1seed.split("_"):
            m = self.l1pattern.match(token)
            if m:
                count,legtype,thr = m.groups()
                count = int(count) if count else 1
                if legtype == 'EM' or legtype == 'TAU':
                    pass
                elif legtype == 'MU':
                    if not mtype & TriggerType.mu_bphys:
                        if count > 1: mtype |= TriggerType.mu_multi
                        elif not mtype & TriggerType.mu_multi: mtype |= TriggerType.mu_single
                elif legtype == 'J':
                    if not mtype & TriggerType.bj and not mtype & TriggerType.j and not mtype & TriggerType.tau and not mtype & TriggerType.ht:
                        if count > 1: mtype |= TriggerType.j_multi
                        elif not mtype & TriggerType.j_multi: mtype |= TriggerType.j_single
                elif legtype == 'XE' or legtype == 'XS':
                    mtype |= TriggerType.xe
                elif legtype == 'HT':
                    mtype |= TriggerType.ht
                else:
                    log.info("Unknown trigger type:",(legtype, mtype, token, self.name))
        return mtype

    def isActive(self, livefraction=1e-99):
        return self.livefraction > livefraction or self.hasRerun
    def isInactive(self, livefraction=1e-99):
        return not self.isActive(livefraction)

    def isUnprescaled(self, livefraction=1.0):
        return self.livefraction >= livefraction

    def getType(self):
        return self.triggerType

    def passType(self, triggerType, additionalTriggerType):
        if self.triggerType == TriggerType.UNDEFINED: return False
        if self.triggerType == TriggerType.ALL:       return True
        match   = (self.triggerType & triggerType)
        if not match: return False
        tmpType = self.triggerType & ~triggerType

        try: 
            for t in additionalTriggerType:
                match   = (tmpType & t)
                if not match: return False
                tmpType = tmpType & ~t
        except TypeError: #Not iterable
            if additionalTriggerType!=TriggerType.UNDEFINED:
                match   = (tmpType & additionalTriggerType)
                if not match: return False
                tmpType = tmpType & ~additionalTriggerType

        return tmpType == TriggerType.UNDEFINED #After matches nothing remains

    def __repr__(self):
        log.info(self.name, self.legs, "{0:b}".format(self.triggerType), self.livefraction, self.activeLB)
        return ""

    def isSubsetOf(self, other):
        ''' Returns -1 if none of them is a strict subset of the other
            Returns  0 if the legs in other are a subset of self.
            Returns  1 if the legs in self  are a subset of other.
        '''
        if not self.legs or not other.legs: return -1 #problems with AFP
        selfcounter = Counter(self.legs)
        othercounter = Counter(other.legs)
        for leg, count in selfcounter.items():
            if leg not in othercounter or count > othercounter[leg]: break
        else: return 1
        for leg, count in othercounter.items():
            if leg not in selfcounter or count > selfcounter[leg]: break
        else: return 0
        return -1

    def isLowerThan(self, other,period=TriggerPeriod.future):
        ''' Returns -1 if none of them is lower than the other (e.g. asymmetric dilepton).
            Returns  0 if other is lower than self.
            Returns  1 if self  is lower than other.
        '''
        is2015  = period & TriggerPeriod.y2015 and not TriggerPeriod.isRunNumber(period)
        is2015 |= period <= 284484             and     TriggerPeriod.isRunNumber(period)
        if self.triggerType != other.triggerType: return -1
        if len(self.legs) != len(other.legs): return -1
        comp = -1
        debug = False
        #if re.search("HLT_j55_gsc75_bmv2c1040_split_3j55_gsc75_boffperf_split", self.name): debug = True
        if debug: log.info("DEBUG:",self.name,other.name)
        for selfleg, otherleg in zip(self.legs, other.legs):
            legcomp = selfleg.isLegLowerThan(otherleg, is2015, debug)
            if debug: log.info("DEBUG LEG return:", legcomp)
            if legcomp == -9: return -1
            elif legcomp == -1: continue
            elif legcomp == 0 and comp == 1: return -1
            elif legcomp == 1 and comp == 0: return -1
            elif legcomp == 0 : comp = 0
            elif legcomp == 1 : comp = 1
        if debug: log.info("DEBUG FINAL:",comp)
        return comp


def test():
    a = TriggerChain("HLT_j50_gsc65_bmv2c1040_split_3j50_gsc65_boffperf_split", "L1J100",1)
    log.info(a)
    log.info(bin(a.getType()))
    log.info(a.passType(TriggerType.j_multi, TriggerType.UNDEFINED))
    log.info(a.passType(TriggerType.j_multi | TriggerType.bj_single, TriggerType.UNDEFINED))
    log.info(a.isUnprescaled())

if __name__ == "__main__":
    import sys
    sys.exit(test())
