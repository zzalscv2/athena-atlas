# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from __future__ import print_function

import logging

__author__  = 'Radist Morse radist.morse@gmail.com'


class FourVector() :
    def __init__(self) :
        self.x = 0.0
        self.y = 0.0
        self.z = 0.0
        self.e = 0.0

class StoredTestShower() :
    def __init__(self):
        self.vertex = FourVector()
        self.momentum = FourVector()
        self.rsize = 0.0
        self.zsize = 0.0
        self.shower = [] #hits, list of four vectors (which are actually 5-vector in this case)

class StoredEnergyShower() :
    def __init__(self):
        self.shower = [] #hits, list of four vectors (which are actually 5-vector in this case)
        self.egen = 0.0
        self.rsize = 0.0
        self.zsize = 0.0
    def __hash__(self): #needed for making a set
        return hash(self.egen)
    def __eq__(self,other): #same here
        return self.egen == other.egen

class TestShowerLib() :
    def __init__(self) :
        self.library = [] # list of StoredTestShower objs
        self.detector= ""
        self.particle= ""
        self.release= ""
        self.geometry= ""
        self.geant= ""
        self.phys= ""
        self.comment= ""
    def fromLibs(self,libs) :
        for lib in libs :
            if not isinstance(lib,self.__class__):
                print ("ERROR: Different types of libs")
                return False
        self.detector = libs[0].detector
        self.particle = libs[0].particle
        self.release = libs[0].release
        self.geometry = libs[0].geometry
        self.geant = libs[0].geant
        self.phys = libs[0].phys
        for lib in libs :
            if (    self.detector != lib.detector or
                self.particle != lib.particle or
                self.release != lib.release or
                self.geometry != lib.geometry or
                self.geant != lib.geant or
                self.phys != lib.phys ) :
                print ("ERROR: DIFFERENT LIBS!!!")
                return False
        from datetime import datetime
        self.comment = "ADDED "+str(datetime.now())
        for lib in libs :
            self.library += lib.library
        return True
    def readFromFile(self,filename) :
        from ROOT import TFile
        tfile = TFile(filename)
        try :
            ver = int(tfile.Get("version").GetVal())
        except Exception:
            print ("Not a TestShowerLib: Broken file")
            tfile.Close()
            return False
        if (ver != 10) : #<<<<<<<<<<<<<<<<<<<<<<-------------- lib ver
            print ("Not a TestShowerLib")
            tfile.Close()
            return False
        meta = tfile.Get("meta")
        libr = tfile.Get("library")
        for event in meta :
            self.detector=str(event.detector)
            self.particle=str(event.particle)
            self.release=str(event.release)
            self.geometry=str(event.geometry)
            self.geant=str(event.geantVersion)
            self.phys=str(event.physicsList)
            self.comment=str(event.comment)

        state = 0

        #state == 0 : first line of shower header
        #state == 1 : second line of sower header
        #state == 2 : hit

        for event in libr : #this is quite unclear, but easy to implement
            if   (state == 0) : #first line
                curShower = StoredTestShower()
                curShower.vertex.x = event.x
                curShower.vertex.y = event.y
                curShower.vertex.z = event.z
                curShower.zsize = event.time
                hitsInCurShower = event.e
                state = 1
            elif (state == 1) : #shower header
                curShower.momentum.x = event.x
                curShower.momentum.y = event.y
                curShower.momentum.z = event.z
                curShower.momentum.e = event.e
                curShower.rsize = event.time
                if (hitsInCurShower > 0) :
                    state = 2
                else :
                    self.library.append(curShower)
                    state = 0
            elif (state == 2) :
                hit = FourVector()
                hit.e = event.e
                hit.x = event.x
                hit.y = event.y
                hit.z = event.z
                hit.time = event.time
                curShower.shower.append(hit)
                hitsInCurShower -= 1
                if (hitsInCurShower == 0) : #last hit
                    self.library.append(curShower)
                    state = 0
        tfile.Close()
        if (state != 0) :
            print ("FILE CORRUPTED!!")
            return False
        return True
    def writeToFile(self,filename) :
        from ROOT import TFile,TTree,TParameter
        from ROOT import gROOT, addressof
        gROOT.ProcessLine(
            "struct MyMetaStruct {\
            Char_t   detector[40];\
            Char_t   release[40];\
            Char_t   geometry[40];\
            Char_t   geant[40];\
            Char_t   phys[40];\
            Char_t   comment[400];\
            Int_t    particle;\
        };" )
        from ROOT import MyMetaStruct
        gROOT.ProcessLine(
            "struct MyStruct {\
            Float_t   x;\
            Float_t   y;\
            Float_t   z;\
            Float_t   e;\
            Float_t   time;\
        };" )
        from ROOT import MyStruct

        tfile = TFile(filename,"RECREATE")

        ver = TParameter(int)("version",10) #<<<<<<<<<<<<<<<<<<<<<<-------------- lib ver
        ver.Write("version")

        meta = TTree()
        libr = TTree()

        mmstruct = MyMetaStruct()

        mmstruct.detector = "%s" % (str(self.detector))
        mmstruct.particle = int(self.particle)
        mmstruct.release = "%s" % (str(self.release))
        mmstruct.geometry = "%s" % (str(self.geometry))
        mmstruct.geant = "%s" % (str(self.geant))
        mmstruct.phys = "%s" % (str(self.phys))
        mmstruct.comment = "%s" % (str(self.comment))

        meta.Branch("detector",addressof(mmstruct,"detector"),"detector/C")
        meta.Branch("particle",addressof(mmstruct,"particle"),"particle/I")
        meta.Branch("release",addressof(mmstruct,"release"),"release/C")
        meta.Branch("geometry",addressof(mmstruct,"geometry"),"geometry/C")
        meta.Branch("geantVersion",addressof(mmstruct,"geant"),"geantVersion/C")
        meta.Branch("physicsList",addressof(mmstruct,"phys"),"physicsList/C")
        meta.Branch("comment",addressof(mmstruct,"comment"),"physicsList/C")

        meta.Fill()

        mstruct = MyStruct()

        libr.Branch("x",addressof(mstruct,"x"),"x/F")
        libr.Branch("y",addressof(mstruct,"y"),"y/F")
        libr.Branch("z",addressof(mstruct,"z"),"z/F")
        libr.Branch("e",addressof(mstruct,"e"),"e/F")
        libr.Branch("time",addressof(mstruct,"time"),"time/F")

        for storedShower in self.library :
            mstruct.x = storedShower.vertex.x
            mstruct.y = storedShower.vertex.y
            mstruct.z = storedShower.vertex.z
            mstruct.time = storedShower.zsize
            mstruct.e = len(storedShower.shower)
            libr.Fill()
            mstruct.x = storedShower.momentum.x
            mstruct.y = storedShower.momentum.y
            mstruct.z = storedShower.momentum.z
            mstruct.e = storedShower.momentum.e
            mstruct.time = storedShower.rsize
            libr.Fill()
            for hit in storedShower.shower:
                mstruct.e = hit.e
                mstruct.x = hit.x
                mstruct.y = hit.y
                mstruct.z = hit.z
                mstruct.time = hit.time
                libr.Fill()
        meta.Write("meta")
        libr.Write("library")
        tfile.Close()
    def printInfo(self) :
        pass
    def drawHits(self):
        from ROOT import TH3F
        from math import sqrt,copysign,log10
        hits = TH3F("HITS","Hits Distrib",50,1,1000,101,-300,300,100,0,500)
        containmentZ = TH3F("CONTZ","ContZ Distrib",50,1,1000,101,-300,300,100,0,500)
        containmentR = TH3F("CONTR","ContR Distrib",50,1,1000,101,-300,300,100,0,500)
        for storedShower in self.library :
            containmentR.Fill(log10(storedShower.momentum.e)*333,storedShower.rsize,storedShower.zsize/2,10)
            containmentR.Fill(log10(storedShower.momentum.e)*333,-storedShower.rsize,storedShower.zsize/2,10)
            containmentZ.Fill(log10(storedShower.momentum.e)*333,0,storedShower.zsize,10)
            for hit in storedShower.shower :
                hits.Fill(log10(storedShower.momentum.e)*333,copysign(sqrt(hit.x*hit.x + hit.y*hit.y),hit.x),hit.z)
        return hits,containmentZ,containmentR

class EtaEnergyShowerLib() :
    def __init__(self) :
        self.library = {} # key (float) - eta, value (list) - list of StoredEnergyShower objs
        self.detector= ""
        self.particle= ""
        self.release= ""
        self.geometry= ""
        self.geant= ""
        self.phys= ""
        self.comment= ""
    def scaleEnergy(self,scalefactor) :
        for etabin in self.library.values():
            for storedShower in etabin :
                for hit in storedShower.shower :
                    hit.e *= scalefactor
        self.comment += " SCALED: "+str(scalefactor)
    def truncate(self,truncate) :
        showers = []
        for eta,etabin in self.library.items():
            for storedShower in etabin :
                showers += [(eta,storedShower)]
        if len(showers) <= truncate :
            print ("WARNING: Size of the library is already less:",truncate,"<",len(showers))
            return
        from random import randint
        while (len(showers) > truncate) :
            rand = randint(0,len(showers)-1)
            self.library[showers[rand][0]].remove(showers[rand][1])
            del showers[rand]
        return
    def fromLibs(self,libs) :
        for lib in libs :
            if not isinstance(lib,self.__class__):
                print ("ERROR: Different types of libs")
                return False
        self.detector = libs[0].detector
        self.particle = libs[0].particle
        self.release = libs[0].release
        self.geometry = libs[0].geometry
        self.geant = libs[0].geant
        self.phys = libs[0].phys
        self.mineta = libs[0].mineta
        self.maxeta = libs[0].maxeta
        self.comment = libs[0].comment
        etas = set(libs[0].library.keys())
        for lib in libs :
            if (    self.detector != lib.detector or
                self.particle != lib.particle or
                self.release != lib.release or
                self.geometry != lib.geometry or
                self.geant != lib.geant or
                self.phys != lib.phys or
                self.mineta != lib.mineta or
                self.maxeta != lib.maxeta or
                etas != set(lib.library.keys()) ) :
                print ("ERROR: DIFFERENT LIBS!!!")
                return False
        for lib in libs :
            for k,v in lib.library.items():
                self.library.setdefault(k,set()).update(v)
        for k,v in self.library.items():
            self.library[k] = list(v)
        return True
    def moveEta(self,oldEta,newEta) :
        if not (oldEta in self.library.keys()) :
            return False
        self.library[newEta] = self.library.pop(oldEta)
        return True
    def removeEta(self,eta) :
        if not (eta in self.library.keys()) :
            return False
        self.library.pop(eta)
        return True
    def readFromFile(self,filename) :
        from ROOT import TFile
        #from sets import Set
        tfile = TFile(filename)
        try:
            ver = int(tfile.Get("version").GetVal())
        except Exception:
            print ("Not an EtaEnergyLib: Broken file")
            tfile.Close()
            return False

        if (ver != 1) : #<<<<<<<<<<<<<<<<<<<<<<-------------- lib ver
            print ("Not an EtaEnergyLib")
            tfile.Close()
            return False
        meta = tfile.Get("meta")
        libr = tfile.Get("library")

        for event in meta :
            self.detector=str(event.detector)
            self.particle=str(event.particle)
            self.release=str(event.release)
            self.geometry=str(event.geometry)
            self.geant=str(event.geantVersion)
            self.phys=str(event.physicsList)
            self.comment=str(event.comment)

        state = 0
        lastShower = False

        for event in libr : #this is quite unclear, but easy to implement
            if   (state == 0) : #eta bin header
                showersInCurEta = event.x
                curEta = round(event.y,4)
                self.mineta = event.z
                self.maxeta = event.e
                self.library[curEta] = []
                if (showersInCurEta > 0) :
                    state = 1 #go to shower header
            elif (state == 1) : #shower header
                hitsInCurShower = event.x
                rSize = event.y
                zSize = event.z
                genEnergy = event.e
                showersInCurEta -= 1
                if (showersInCurEta == 0) : #last shower
                    lastShower = True
                curShower = StoredEnergyShower()
                curShower.egen = genEnergy
                curShower.rsize = rSize
                curShower.zsize = zSize
                #curShower["hits"] = []
                if (hitsInCurShower > 0) :
                    state = 2 #go to hits
                else : #empty shower
                    self.library[curEta].append(curShower)
                    if (lastShower) : #special case of last shower in bin being the empty one
                        lastShower = False
                        state = 0 #next bin
            elif (state == 2) :
                hit = FourVector()
                hit.e = event.e
                hit.x = event.x
                hit.y = event.y
                hit.z = event.z
                hit.time = event.time
                curShower.shower.append(hit)
                hitsInCurShower -= 1
                if (hitsInCurShower == 0) : #last hit
                    self.library[curEta].append(curShower)
                    if (lastShower) : # end of eta bin
                        lastShower = False
                        state = 0
                    else : #not yet
                        state = 1
        tfile.Close()
        if (state != 0) :
            print ("FILE CORRUPTED!!")
            return False
        return True
    def writeToFile(self,filename) :
        from ROOT import TFile,TTree,TParameter
        from ROOT import gROOT, addressof
        gROOT.ProcessLine(
            "struct MyMetaStruct {\
            Char_t   detector[40];\
            Char_t   release[40];\
            Char_t   geometry[40];\
            Char_t   geant[40];\
            Char_t   phys[40];\
            Char_t   comment[400];\
            Int_t    particle;\
        };" )
        from ROOT import MyMetaStruct
        gROOT.ProcessLine(
            "struct MyStruct {\
            Float_t   x;\
            Float_t   y;\
            Float_t   z;\
            Float_t   e;\
            Float_t   time;\
        };" )
        from ROOT import MyStruct

        tfile = TFile(filename,"RECREATE")

        ver = TParameter(int)("version",1) #<<<<<<<<<<<<<<<<<<<<<<-------------- lib ver
        ver.Write("version")

        meta = TTree()
        libr = TTree()

        mmstruct = MyMetaStruct()

        mmstruct.detector = "%s" % (str(self.detector))
        mmstruct.particle = int(self.particle)
        mmstruct.release = "%s" % (str(self.release))
        mmstruct.geometry = "%s" % (str(self.geometry))
        mmstruct.geant = "%s" % (str(self.geant))
        mmstruct.phys = "%s" % (str(self.phys))
        mmstruct.comment = "%s" % (str(self.comment))

        meta.Branch("detector",addressof(mmstruct,"detector"),"detector/C")
        meta.Branch("particle",addressof(mmstruct,"particle"),"particle/I")
        meta.Branch("release",addressof(mmstruct,"release"),"release/C")
        meta.Branch("geometry",addressof(mmstruct,"geometry"),"geometry/C")
        meta.Branch("geantVersion",addressof(mmstruct,"geant"),"geantVersion/C")
        meta.Branch("physicsList",addressof(mmstruct,"phys"),"physicsList/C")
        meta.Branch("comment",addressof(mmstruct,"comment"),"physicsList/C")

        meta.Fill()

        mstruct = MyStruct()

        libr.Branch("x",addressof(mstruct,"x"),"x/F")
        libr.Branch("y",addressof(mstruct,"y"),"y/F")
        libr.Branch("z",addressof(mstruct,"z"),"z/F")
        libr.Branch("e",addressof(mstruct,"e"),"e/F")
        libr.Branch("time",addressof(mstruct,"time"),"time/F")

        etas = self.library.keys()

        for eta in sorted(etas) :
            mstruct.x = len(self.library[eta])
            mstruct.y = eta
            mstruct.z = self.mineta
            mstruct.e = self.maxeta
            mstruct.time = 0
            libr.Fill()
            self.library[eta].sort(key=lambda x: x.egen)
            for storedShower in self.library[eta] :
                mstruct.x = len(storedShower.shower)
                mstruct.y = storedShower.rsize
                mstruct.z = storedShower.zsize
                mstruct.e = storedShower.egen
                mstruct.time = 0
                libr.Fill()
                for hit in storedShower.shower:
                    mstruct.e = hit.e
                    mstruct.x = hit.x
                    mstruct.y = hit.y
                    mstruct.z = hit.z
                    mstruct.time = hit.time
                    libr.Fill()
        meta.Write("meta")
        libr.Write("library")
        tfile.Close()
    def printInfo(self) :
        print ("VERSION: EtaEnergyLib","PARTICLE:",self.particle,"DETECTOR:",self.detector)
        print (self.release, self.geometry, self.geant, self.phys)
        print (self.comment)
        ebins = [1,2,5,10,20,50,100,200,500,1000]
        etas = sorted(self.library.keys())
        print ("Number of etabins:",str(len(etas)))
        print ("MinEta:",self.mineta,"MaxEta:",self.maxeta)
        fstot = 0
        for eta in etas :
            fstot +=len(self.library[eta])
        print ("Number of showers:",str(fstot))
        print ("-"*(12+len(ebins)*8)) #horizontal line
        infostr = "|etas|ebins|"
        for ebin in ebins : #header for energy bins
            infostr += ("<%d" %ebin).rjust(7)                         #str(ebin).rjust(7)
            infostr += "|"
        print (infostr)
        print ("-"*(12+len(ebins)*8)) #horizontal line
        for etalow,etahigh in zip(etas,(etas[1:] + [self.maxeta])) : #looping over eta bins
            prevebin = 0
            erec = {}
            egen = {}
            hits = {}
            count = {}
            for ebin in ebins : # for all energy bins
                count[ebin] = 0
                erec[ebin] = 0.
                egen[ebin] = 0.
                hits[ebin] = 0.
                for shower in self.library[etalow] :
                    if (shower.egen <= ebin) and (shower.egen > prevebin) :
                        count[ebin] += 1
                        egenshow = shower.egen
                        erecshow = 0
                        for hit in shower.shower :
                            erecshow += hit.e
                        erec[ebin] += erecshow
                        egen[ebin] += egenshow
                        hits[ebin] += len(shower.shower)
                if (count[ebin] > 0) :
                    hits[ebin] /= count[ebin]
                prevebin = ebin
            infostr = "|#"                # |
            infostr+= str(round(etalow,5)).rjust(9) # | eta header
            infostr+= "|"                # |\
            infostr2 = "|Hits"
            infostr2+= str(round(etahigh,3)).rjust(6) # | eta header
            infostr2+= "|"                # |
            infostr3 = "|ErecEgen"
            infostr3+= " ".rjust(2) # | eta header
            infostr3+= "|"                # |
            for ebin in ebins :
                infostr+= str(count[ebin]).rjust(7) #print the number of showers
                if (egen[ebin] > 0) :
                    infostr2+= ("%.2f" %(hits[ebin])).rjust(7)
                    infostr3+= ("%.5f" %(erec[ebin]/egen[ebin])).rjust(7)
                else :
                    infostr2+= ("%.2f" %(hits[ebin])).rjust(7)
                    infostr3+= "0.0".rjust(7) #else print "xxx"
                infostr+="|"
                infostr2+="|"
                infostr3+="|"
            print (infostr)
            print (infostr2)
            print (infostr3)
            print ("-"*(12+len(ebins)*8)) #horizontal line
    def drawHits(self):
        from ROOT import TH3F
        from math import sqrt,copysign,log10
        hits = TH3F("HITS","Hits Distrib",50,1,1000,101,-300,300,100,0,500)
        containmentZ = TH3F("CONTZ","ContZ Distrib",50,1,1000,101,-300,300,100,0,500)
        containmentR = TH3F("CONTR","ContR Distrib",50,1,1000,101,-300,300,100,0,500)
        for etabin in self.library.values():
            for storedShower in etabin :
                containmentR.Fill(log10(storedShower.egen)*333,storedShower.rsize,storedShower.zsize,10)
                containmentR.Fill(log10(storedShower.egen)*333,-storedShower.rsize,storedShower.zsize,10)
                containmentZ.Fill(log10(storedShower.egen)*333,0,storedShower.zsize,10)
                for hit in storedShower.shower :
                    hits.Fill(log10(storedShower.egen)*333,copysign(sqrt(hit.x*hit.x + hit.y*hit.y),hit.x),hit.z)
        return hits,containmentZ,containmentR

class FCALDistShowerLib() :
    def __init__(self) :
        self.library = {} # key (float) - dist, value (list) - list of StoredEnergyShower objs
        self.detector= ""
        self.particle= ""
        self.release= ""
        self.geometry= ""
        self.geant= ""
        self.phys= ""
        self.comment= ""
        self.xrod_cent = 0.0
        self.yrod_cent = 0.0
        self.step = 0.0
    def scaleEnergy(self,scalefactor) :
        for distbin in self.library.values():
            for storedShower in distbin :
                for hit in storedShower.shower :
                    hit.e *= scalefactor
        self.comment += " SCALED: "+str(scalefactor)
    def truncate(self,truncate) :
        showers = []
        for dist,distbin in self.library.items():
            for storedShower in distbin :
                showers += [(dist,storedShower)]
        if len(showers) <= truncate :
            print ("WARNING: Size of the library is already less:",truncate,"<",len(showers))
            return
        from random import randint
        while (len(showers) > truncate) :
            rand = randint(0,len(showers)-1)
            self.library[showers[rand][0]].remove(showers[rand][1])
            del showers[rand]
        return
    def moveDist(self,oldDist,newDist) :
        if not (oldDist in self.library.keys()) :
            return False
        self.library[newDist] = self.library.pop(oldDist)
        return True
    def removeDist(self,dist) :
        if not (dist in self.library.keys()) :
            return False
        self.library.pop(dist)
        return True
    def fromLibs(self,libs) :
        for lib in libs :
            if not isinstance(lib,self.__class__):
                print ("ERROR: Different types of libs")
                return False
        self.detector = libs[0].detector
        self.particle = libs[0].particle
        self.release = libs[0].release
        self.geometry = libs[0].geometry
        self.geant = libs[0].geant
        self.phys = libs[0].phys
        self.comment = libs[0].comment
        self.xrod_cent = libs[0].xrod_cent
        self.yrod_cent = libs[0].yrod_cent
        self.step = libs[0].step
        dists = set(libs[0].library.keys())
        for lib in libs :
            if (    self.detector != lib.detector or
                self.particle != lib.particle or
                self.release != lib.release or
                self.geometry != lib.geometry or
                self.geant != lib.geant or
                self.phys != lib.phys or
                self.xrod_cent != lib.xrod_cent or
                self.yrod_cent != lib.yrod_cent or
                self.step != lib.step or
                dists != set(lib.library.keys()) ) :
                print ("ERROR: DIFFERENT LIBS!!!")
                return False
        for lib in libs :
            for k,v in lib.library.items():
                self.library.setdefault(k,set()).update(v)
        for k,v in self.library.items():
            self.library[k] = list(v)
        return True
    def readFromFile(self,filename) :
        from ROOT import TFile
        #from sets import Set
        tfile = TFile(filename)
        try:
            ver = int(tfile.Get("version").GetVal())
        except Exception:
            print ("Not an FCALDistEnergyLib: Broken file")
            tfile.Close()
            return False
        if (ver != 4) : #<<<<<<<<<<<<<<<<<<<<<<-------------- lib ver
            print ("Not an FCALDistEnergyLib")
            tfile.Close()
            return False
        meta = tfile.Get("meta")
        libr = tfile.Get("library")

        for event in meta :
            self.detector=str(event.detector)
            self.particle=str(event.particle)
            self.release=str(event.release)
            self.geometry=str(event.geometry)
            self.geant=str(event.geantVersion)
            self.phys=str(event.physicsList)
            self.comment=str(event.comment)

        state = -1
        lastShower = False

        for event in libr : #this is quite unclear, but easy to implement
            if   (state == -1) : #library header (calculator parameters)
                self.xrod_cent = event.x
                self.yrod_cent = event.y
                self.step = event.z
                state = 0
            elif (state == 0) : #eta bin header
                showersInCurDist = event.x
                curDist = round(event.y,4)
                self.library[curDist] = []
                if (showersInCurDist > 0) :
                    state = 1 #go to shower header
            elif (state == 1) : #shower header
                hitsInCurShower = event.x
                rSize = event.y
                zSize = event.z
                genEnergy = event.e
                showersInCurDist -= 1
                if (showersInCurDist == 0) : #last shower
                    lastShower = True
                curShower = StoredEnergyShower()
                curShower.egen = genEnergy
                curShower.rsize = rSize
                curShower.zsize = zSize
                #curShower["hits"] = []
                if (hitsInCurShower > 0) :
                    state = 2 #go to hits
                else : #empty shower
                    self.library[curDist].append(curShower)
                    if (lastShower) : #special case of last shower in bin being the empty one
                        lastShower = False
                        state = 0 #next bin
            elif (state == 2) :
                hit = FourVector()
                hit.e = event.e
                hit.x = event.x
                hit.y = event.y
                hit.z = event.z
                hit.time = event.time
                curShower.shower.append(hit)
                hitsInCurShower -= 1
                if (hitsInCurShower == 0) : #last hit
                    self.library[curDist].append(curShower)
                    if (lastShower) : # end of eta bin
                        lastShower = False
                        state = 0
                    else : #not yet
                        state = 1
        tfile.Close()
        if (state != 0) :
            print ("FILE CORRUPTED!!")
            return False
        return True
    def writeToFile(self,filename) :
        from ROOT import TFile,TTree,TParameter
        from ROOT import gROOT, addressof
        gROOT.ProcessLine(
            "struct MyMetaStruct {\
            Char_t   detector[40];\
            Char_t   release[40];\
            Char_t   geometry[40];\
            Char_t   geant[40];\
            Char_t   phys[40];\
            Char_t   comment[400];\
            Int_t    particle;\
        };" )
        from ROOT import MyMetaStruct
        gROOT.ProcessLine(
            "struct MyStruct {\
            Float_t   x;\
            Float_t   y;\
            Float_t   z;\
            Float_t   e;\
            Float_t   time;\
        };" )
        from ROOT import MyStruct

        tfile = TFile(filename,"RECREATE")

        ver = TParameter(int)("version",4) #<<<<<<<<<<<<<<<<<<<<<<-------------- lib ver
        ver.Write("version")

        meta = TTree()
        libr = TTree()

        mmstruct = MyMetaStruct()

        mmstruct.detector = "%s" % (str(self.detector))
        mmstruct.particle = int(self.particle)
        mmstruct.release = "%s" % (str(self.release))
        mmstruct.geometry = "%s" % (str(self.geometry))
        mmstruct.geant = "%s" % (str(self.geant))
        mmstruct.phys = "%s" % (str(self.phys))
        mmstruct.comment = "%s" % (str(self.comment))

        meta.Branch("detector",addressof(mmstruct,"detector"),"detector/C")
        meta.Branch("particle",addressof(mmstruct,"particle"),"particle/I")
        meta.Branch("release",addressof(mmstruct,"release"),"release/C")
        meta.Branch("geometry",addressof(mmstruct,"geometry"),"geometry/C")
        meta.Branch("geantVersion",addressof(mmstruct,"geant"),"geantVersion/C")
        meta.Branch("physicsList",addressof(mmstruct,"phys"),"physicsList/C")
        meta.Branch("comment",addressof(mmstruct,"comment"),"physicsList/C")

        meta.Fill()

        mstruct = MyStruct()

        libr.Branch("x",addressof(mstruct,"x"),"x/F")
        libr.Branch("y",addressof(mstruct,"y"),"y/F")
        libr.Branch("z",addressof(mstruct,"z"),"z/F")
        libr.Branch("e",addressof(mstruct,"e"),"e/F")
        libr.Branch("time",addressof(mstruct,"time"),"time/F")

        mstruct.x = self.xrod_cent
        mstruct.y = self.yrod_cent
        mstruct.z = self.step
        mstruct.e = 0
        mstruct.time = 0
        libr.Fill()

        dists = sorted(self.library.keys())

        for dist in dists :
            mstruct.x = len(self.library[dist])
            mstruct.y = dist
            mstruct.z = 0
            mstruct.e = 0
            mstruct.time = 0
            libr.Fill()
            self.library[dist].sort(key=lambda x: x.egen)
            for storedShower in self.library[dist] :
                mstruct.x = len(storedShower.shower)
                mstruct.y = storedShower.rsize
                mstruct.z = storedShower.zsize
                mstruct.e = storedShower.egen
                mstruct.time = 0
                libr.Fill()
                for hit in storedShower.shower:
                    mstruct.e = hit.e
                    mstruct.x = hit.x
                    mstruct.y = hit.y
                    mstruct.z = hit.z
                    mstruct.time = hit.time
                    libr.Fill()
        meta.Write("meta")
        libr.Write("library")
        tfile.Close()
    def printInfo(self) :
        print ("VERSION: FCALDistEnergyLib","PARTICLE:",self.particle,"DETECTOR:",self.detector)
        print (self.release, self.geometry, self.geant, self.phys)
        print ("xrodcent:",self.xrod_cent,"yrodcent:",self.yrod_cent,"step:",self.step)
        print (self.comment)
        ebins = [1,2,5,10,20,50,100,200,500,1000]
        dists = sorted(self.library.keys())
        print ("Number of etabins:",str(len(dists)))
        fstot = 0
        for dist in dists :
            fstot +=len(self.library[dist])
        print ("Number of showers:",str(fstot))
        print ("-"*(13+len(ebins)*8)) #horizontal line
        infostr = "|dists|ebins|"
        for ebin in ebins : #header for energy bins
            infostr += ("<%d" %ebin).rjust(7)                         #str(ebin).rjust(7)
            infostr += "|"
        print (infostr)
        print ("-"*(13+len(ebins)*8)) #horizontal line
        for distlow,disthigh in zip(dists,(dists[1:] + [4.5])) : #looping over eta bins
            prevebin = 0
            erec = {}
            egen = {}
            hits = {}
            count = {}
            for ebin in ebins : # for all energy bins
                count[ebin] = 0
                erec[ebin] = 0.
                egen[ebin] = 0.
                hits[ebin] = 0.
                for shower in self.library[distlow] :
                    if (shower.egen <= ebin) and (shower.egen > prevebin) :
                        count[ebin] += 1
                        egenshow = shower.egen
                        erecshow = 0
                        for hit in shower.shower :
                            erecshow += hit.e
                        erec[ebin] += erecshow
                        egen[ebin] += egenshow
                        hits[ebin] += len(shower.shower)
                if (count[ebin] > 0) :
                    hits[ebin] /= count[ebin]
                prevebin = ebin
            infostr = "|#"                # |
            infostr+= str(round(distlow,5)).rjust(10) # | eta header
            infostr+= "|"                # |\
            infostr2 = "|Hits"
            infostr2+= str(round(disthigh,3)).rjust(7) # | eta header
            infostr2+= "|"                # |
            infostr3 = "|ErecEgen"
            infostr3+= " ".rjust(3) # | eta header
            infostr3+= "|"                # |
            for ebin in ebins :
                infostr+= str(count[ebin]).rjust(7) #print the number of showers
                if (egen[ebin] > 0) :
                    infostr2+= ("%.2f" %(hits[ebin])).rjust(7)
                    infostr3+= ("%.5f" %(erec[ebin]/egen[ebin])).rjust(7)
                else :
                    infostr2+= ("%.2f" %(hits[ebin])).rjust(7)
                    infostr3+= "0.0".rjust(7) #else print "xxx"
                infostr+="|"
                infostr2+="|"
                infostr3+="|"
            print (infostr)
            print (infostr2)
            print (infostr3)
            print ("-"*(12+len(ebins)*8)) #horizontal line
    def drawHits(self):
        from ROOT import TH3F
        from math import sqrt,copysign,log10
        hits = TH3F("HITS","Hits Distrib",50,1,1000,101,-300,300,100,0,500)
        containmentZ = TH3F("CONTZ","ContZ Distrib",50,1,1000,101,-300,300,100,0,500)
        containmentR = TH3F("CONTR","ContR Distrib",50,1,1000,101,-300,300,100,0,500)
        for distbin in self.library.values():
            for storedShower in distbin :
                containmentR.Fill(log10(storedShower.egen)*333,storedShower.rsize,storedShower.zsize,10)
                containmentR.Fill(log10(storedShower.egen)*333,-storedShower.rsize,storedShower.zsize,10)
                containmentZ.Fill(log10(storedShower.egen)*333,0,storedShower.zsize,10)
                for hit in storedShower.shower :
                    hits.Fill(log10(storedShower.egen)*333,copysign(sqrt(hit.x*hit.x + hit.y*hit.y),hit.x),hit.z)
        return hits,containmentZ,containmentR

class FCALDistEtaShowerLib() :
        
    def __init__(self) :
        self.library = {} # key (float) - eta, value (dict), key - dist, value - (list) list of StoredEnergyShower objs
        self.detector= ""
        self.particle= ""
        self.release= ""
        self.geometry= ""
        self.geant= ""
        self.phys= ""
        self.comment= ""
        self.xrod_cent = 0.0
        self.yrod_cent = 0.0
        self.step = 0.0
    def scaleEnergy(self,scalefactor) :
        for etabin in self.library.values():
            for distbin in etabin.values():
                for storedShower in distbin :
                    for hit in storedShower.shower :
                        hit.e *= scalefactor
        self.comment += " SCALED: "+str(scalefactor)
    def truncate(self,truncate,nShowersMin=0) :
        log = logging.getLogger("FCALDistEtaShowerLib::truncate()")
        showers = []
        for eta,etabin in self.library.items():
            for dist,distbin in etabin.items():
                log.info("Number of showers in %s %s is %d",str(eta),str(dist),len(distbin))
                for storedShower in distbin :
                    showers += [(eta, dist, storedShower)]
        log.info("total number of showers: %d", len(showers))
        if nShowersMin:
            log.info("will not remove from eta-dist bins with less then %d showers", nShowersMin)
        if len(showers) <= truncate :
            log.warning("Size of the library is already less: %d < %d",truncate,len(showers))
            return
        from random import randint
        while (len(showers) > truncate) :
            rand = randint(0,len(showers)-1)
            if len(self.library[showers[rand][0]][showers[rand][1]]) < nShowersMin:
                continue
            self.library[showers[rand][0]][showers[rand][1]].remove(showers[rand][2])
            del showers[rand]
        return
    def moveDist(self,oldDist,newDist) :
        rez = False
        for eta,etabin in self.library.items():
            if (oldDist in etabin.keys()) :
                    etabin[newDist] = etabin.pop(oldDist)
                    rez=True
        return rez
    def moveEta(self,oldEta,newEta) :
        if not (oldEta in self.library.keys()) :
            return False
        self.library[newEta] = self.library.pop(oldEta)
        return True
    def removeDist(self,dist) :
        rez = False
        for eta,etabin in self.library.items():
            if (dist in etabin.keys()) :
                    self.library.pop(dist)
                    rez=True
        return rez
    def removeEta(self,eta) :
        if not (eta in self.library.keys()) :
            return False
        self.library.pop(eta)
        return True
    def fromLibs(self,libs) :
        for lib in libs :
            if not isinstance(lib,self.__class__):
                print ("ERROR: Different types of libs")
                return False
        self.detector = libs[0].detector
        self.particle = libs[0].particle
        self.release = libs[0].release
        self.geometry = libs[0].geometry
        self.geant = libs[0].geant
        self.phys = libs[0].phys
        self.comment = libs[0].comment
        self.xrod_cent = libs[0].xrod_cent
        self.yrod_cent = libs[0].yrod_cent
        self.step = libs[0].step
        etas = set(libs[0].library.keys())
        for lib in libs :
            if (    self.detector != lib.detector or
                self.particle != lib.particle or
                self.release != lib.release or
                self.geometry != lib.geometry or
                self.geant != lib.geant or
                self.phys != lib.phys or
                self.xrod_cent != lib.xrod_cent or
                self.yrod_cent != lib.yrod_cent or
                self.step != lib.step or
                etas != set(lib.library.keys()) ) :
                print ("ERROR: DIFFERENT LIBS!!!")
                return False
            for eta in libs[0].library.keys() :
                if (set(libs[0].library[eta].keys()) != set(lib.library[eta].keys())) :
                    print ("ERROR: DIFFERENT LIBS!!!")
                    return False
        for lib in libs :
            for k,v in lib.library.items():
                    for ki,vi in v.items():
                        self.library.setdefault(k,dict()).setdefault(ki,set()).update(vi)
        for k,v in self.library.items():
                for ki,vi in v.items():
                    self.library[k][ki] = list(vi)
        return True
    def readFromFile(self,filename) :
        log = logging.getLogger("FCALDistEtaShowerLib::readFromFile()")
        from ROOT import TFile
        #from sets import Set
        tfile = TFile(filename)
        try:
            ver = int(tfile.Get("version").GetVal())
        except Exception:
            print ("Not an FCALDistEtaEnergyLib: Broken file")
            tfile.Close()
            return False

        if (ver != 5) : #<<<<<<<<<<<<<<<<<<<<<<-------------- lib ver
            print ("Not an FCALDistEtaEnergyLib")
            tfile.Close()
            return False
        meta = tfile.Get("meta")
        libr = tfile.Get("library")

        for event in meta :
            self.detector=str(event.detector)
            self.particle=str(event.particle)
            self.release=str(event.release)
            self.geometry=str(event.geometry)
            self.geant=str(event.geantVersion)
            self.phys=str(event.physicsList)
            self.comment=str(event.comment)

        state = -1
        lastShower = False
        lastEta = False

        log.debug("dector: %s", str(event.detector))
        log.debug("particle: %s", str(event.particle))
        log.debug("release: %s", str(event.release))
        log.debug("geometry: %s", str(event.geometry))
        log.debug("geant ver: %s", str(event.geantVersion))
        log.debug("physList: %s", str(event.physicsList))
        log.debug("comment: %s", str(event.comment))

        for event in libr : #this is quite unclear, but easy to implement, we change the "state" depending on what we are reading
            log.debug("-------")
            log.debug("x=%f, y=%f, z=%f, e=%f",event.x,event.y,event.z,event.e)
            log.debug("beginnnig ev loop. lastShower: %s",str(lastShower))
            log.debug("beginnnig ev loop. state: %s",str(state))

            if   (state == -1) : #library header (calculator parameters)
                log.debug("in state=-1")

                self.xrod_cent = event.x
                self.yrod_cent = event.y
                self.step = event.z
                state = 0
            elif (state == 0) : #eta bin header
                log.debug("in state=0")
                log.debug("x=distsInCurEta, y=curEta")

                distsInCurEta = event.x
                curEta = round(event.y,4)
                self.library[curEta] = {}
                if (distsInCurEta > 0) :
                    state = 1 #go to dist header
            elif (state == 1) :
                log.debug("in state=1")
                log.debug("x=showersInCurDist, y=curDist")

                showersInCurDist = event.x
                curDist = round(event.y,4)
                self.library[curEta][curDist] = []
                distsInCurEta -= 1
                if (distsInCurEta == 0) :
                    lastEta = True
                if (showersInCurDist > 0) :
                    state = 2 #go to shower header
                else : #empty dist bin
                    if (lastEta) : #special case of last eta bin being the empty one
                        lastEta = False
                        state = 0
            elif (state == 2) :
                # writing shower info
                log.debug("in state=2")
                log.debug("x=hitsInCurShower, y=curShower.rSize, z=curShower.zSize, e=curShower.genEnergy")

                hitsInCurShower = event.x
                rSize = event.y
                zSize = event.z
                genEnergy = event.e
                showersInCurDist -= 1
                if (showersInCurDist == 0) : #last shower
                    lastShower = True
                curShower = StoredEnergyShower()
                curShower.egen = genEnergy
                curShower.rsize = rSize
                curShower.zsize = zSize
                #curShower["hits"] = []
                if (hitsInCurShower > 0) :
                    state = 3 #go to hits
                else : #empty shower
                    log.debug("Appending shower to lib pos %s %s",curEta,curDist)

                    self.library[curEta][curDist].append(curShower)
                    if (lastShower) : #special case of last shower in bin being the empty one
                        lastShower = False
                        if (lastEta) : #double special case: last shower in last eta bin is empty
                            lastEta = False
                            state = 0 #next eta bin
                        else :
                            state = 1 #next dist bin
            elif (state == 3) :
                ## writing hits info
                log.debug("in state=3")
                log.debug("x=hit.x, y=hit.y, z=hit.z, e=hit.e")

                hit = FourVector()
                hit.e = event.e
                hit.x = event.x
                hit.y = event.y
                hit.z = event.z
                hit.time = event.time
                curShower.shower.append(hit)
                hitsInCurShower -= 1
                if (hitsInCurShower == 0) : #last hit
                    log.debug("Appending shower+hit to lib pos %s %s",curEta,curDist)

                    self.library[curEta][curDist].append(curShower)
                    if (lastShower) : # end of dist bin
                        lastShower = False
                        if (lastEta) : #end of eta bin as well
                            lastEta = False
                            state = 0 #next eta bin
                        else :
                            state = 1 #next dist bin
                    else : #not yet
                        state = 2

            log.debug("ending ev loop. lastShower: %s", lastShower)
            log.debug("ending ev loop. state %s", state)
            if log.root.level == logging.DEBUG:
                input("Continue? Press Enter.")

        tfile.Close()
        if (state != 0) : #the last entry should be the last hit of the last shower in the last bin. if not - file is corrupted
            print ("FILE CORRUPTED!!")
            return False
        return True
    def writeToFile(self,filename) :
        from ROOT import TFile,TTree,TParameter
        from ROOT import gROOT, addressof
        gROOT.ProcessLine(
            "struct MyMetaStruct {\
            Char_t   detector[40];\
            Char_t   release[40];\
            Char_t   geometry[40];\
            Char_t   geant[40];\
            Char_t   phys[40];\
            Char_t   comment[400];\
            Int_t    particle;\
        };" )
        from ROOT import MyMetaStruct
        gROOT.ProcessLine(
            "struct MyStruct {\
            Float_t   x;\
            Float_t   y;\
            Float_t   z;\
            Float_t   e;\
            Float_t   time;\
        };" )
        from ROOT import MyStruct

        tfile = TFile(filename,"RECREATE")

        ver = TParameter(int)("version",5) #<<<<<<<<<<<<<<<<<<<<<<-------------- lib ver
        ver.Write("version")

        meta = TTree()
        libr = TTree()

        mmstruct = MyMetaStruct()

        mmstruct.detector = "%s" % (str(self.detector))
        mmstruct.particle = int(self.particle)
        mmstruct.release = "%s" % (str(self.release))
        mmstruct.geometry = "%s" % (str(self.geometry))
        mmstruct.geant = "%s" % (str(self.geant))
        mmstruct.phys = "%s" % (str(self.phys))
        mmstruct.comment = "%s" % (str(self.comment))

        meta.Branch("detector",addressof(mmstruct,"detector"),"detector/C")
        meta.Branch("particle",addressof(mmstruct,"particle"),"particle/I")
        meta.Branch("release",addressof(mmstruct,"release"),"release/C")
        meta.Branch("geometry",addressof(mmstruct,"geometry"),"geometry/C")
        meta.Branch("geantVersion",addressof(mmstruct,"geant"),"geantVersion/C")
        meta.Branch("physicsList",addressof(mmstruct,"phys"),"physicsList/C")
        meta.Branch("comment",addressof(mmstruct,"comment"),"physicsList/C")

        meta.Fill()

        mstruct = MyStruct()

        libr.Branch("x",addressof(mstruct,"x"),"x/F")
        libr.Branch("y",addressof(mstruct,"y"),"y/F")
        libr.Branch("z",addressof(mstruct,"z"),"z/F")
        libr.Branch("e",addressof(mstruct,"e"),"e/F")
        libr.Branch("time",addressof(mstruct,"time"),"time/F")

        etas = sorted(self.library.keys())

        mstruct.x = self.xrod_cent
        mstruct.y = self.yrod_cent
        mstruct.z = self.step
        mstruct.e = 0
        mstruct.time = 0
        libr.Fill()

        for eta in etas :
            dists = sorted(self.library[eta].keys())
            mstruct.x = len(self.library[eta])
            mstruct.y = eta
            mstruct.z = 0
            mstruct.e = 0
            mstruct.time = 0
            libr.Fill()
            for dist in dists :
                mstruct.x = len(self.library[eta][dist])
                mstruct.y = dist
                mstruct.z = 0
                mstruct.e = 0
                mstruct.time = 0
                libr.Fill()
                self.library[eta][dist].sort(key=lambda x: x.egen)
                for storedShower in self.library[eta][dist] :
                    mstruct.x = len(storedShower.shower)
                    mstruct.y = storedShower.rsize
                    mstruct.z = storedShower.zsize
                    mstruct.e = storedShower.egen
                    mstruct.time = 0
                    libr.Fill()
                    for hit in storedShower.shower:
                        mstruct.e = hit.e
                        mstruct.x = hit.x
                        mstruct.y = hit.y
                        mstruct.z = hit.z
                        mstruct.time = hit.time
                        libr.Fill()
        meta.Write("meta")
        libr.Write("library")
        tfile.Close()
    def printInfo(self) :
        print ("VERSION: FCALDistEtaEnergyLib","PARTICLE:",self.particle,"DETECTOR:",self.detector)
        print (self.release, self.geometry, self.geant, self.phys)
        print ("xrodcent:",self.xrod_cent,"yrodcent:",self.yrod_cent,"step:",self.step)
        print (self.comment)
        ebins = [1,2,3,4,5,10,20,50,100,200,500,1000]
        etas = sorted(self.library.keys())
        print ("Number of etabins:",str(len(etas)))
        fstot = 0
        for etabin in self.library.values():
            for distbin in etabin.values():
                fstot +=len(distbin)
        print ("Number of showers:",str(fstot))
        print ("-"*(13+len(ebins)*8)) #horizontal line
        infostr = "|dists|ebins|"
        for ebin in ebins : #header for energy bins
            infostr += ("<%d" %ebin).rjust(7)                         #str(ebin).rjust(7)
            infostr += "|"
        print (infostr)
        print ("-"*(13+len(ebins)*8)) #horizontal line
        for eta in etas :
            dists = sorted(self.library[eta].keys())
            for distlow,disthigh in zip(dists,(dists[1:] + [4.5])) : #looping over eta bins
                prevebin = 0
                erec = {}
                egen = {}
                hits = {}
                count = {}
                for ebin in ebins : # for all energy bins
                    count[ebin] = 0
                    erec[ebin] = 0.
                    egen[ebin] = 0.
                    hits[ebin] = 0.
                    for shower in self.library[eta][distlow] :
                        if (shower.egen <= ebin) and (shower.egen > prevebin) :
                            count[ebin] += 1
                            egenshow = shower.egen
                            erecshow = 0
                            for hit in shower.shower :
                                erecshow += hit.e
                            erec[ebin] += erecshow
                            egen[ebin] += egenshow
                            hits[ebin] += len(shower.shower)
                    if (count[ebin] > 0) :
                        hits[ebin] /= count[ebin]
                    prevebin = ebin
                infostr = "|#"                # |
                infostr+= str(eta).rjust(10)  # | eta header
                infostr+= "|"                 # |\
                infostr2 = "|Hits"
                infostr2+= str(round(distlow,5)).rjust(7) # | eta header
                infostr2+= "|"                # |
                infostr3 = "|E/E"
                infostr3+= str(round(disthigh,3)).rjust(8) # | eta header
                infostr3+= "|"                # |
                for ebin in ebins :
                    infostr+= str(count[ebin]).rjust(7) #print the number of showers
                    if (egen[ebin] > 0) :
                        infostr2+= ("%.2f" %(hits[ebin])).rjust(7)
                        infostr3+= ("%.5f" %(erec[ebin]/egen[ebin])).rjust(7)
                    else :
                        infostr2+= ("%.2f" %(hits[ebin])).rjust(7)
                        infostr3+= "0.0".rjust(7) #else print "xxx"
                    infostr+="|"
                    infostr2+="|"
                    infostr3+="|"
                print (infostr)
                print (infostr2)
                print (infostr3)
                print ("-"*(12+len(ebins)*8)) #horizontal line
    def drawHits(self):
        from ROOT import TH3F
        from math import sqrt,copysign,log10
        hits = TH3F("HITS","Hits Distrib",50,1,1000,101,-300,300,100,0,500)
        containmentZ = TH3F("CONTZ","ContZ Distrib",50,1,1000,101,-300,300,100,0,500)
        containmentR = TH3F("CONTR","ContR Distrib",50,1,1000,101,-300,300,100,0,500)

        etas = sorted(self.library.keys())
        for eta in etas :
            dists = sorted(self.library[eta].keys())
            for distlow,disthigh in zip(dists,(dists[1:] + [4.5])) : #looping over eta bins
                for storedShower in self.library[eta][distlow] :
                    containmentR.Fill(log10(storedShower.egen)*333,storedShower.rsize,storedShower.zsize,10)
                    containmentR.Fill(log10(storedShower.egen)*333,-storedShower.rsize,storedShower.zsize,10)
                    containmentZ.Fill(log10(storedShower.egen)*333,0,storedShower.zsize,10)
                    for hit in storedShower.shower :
                        hits.Fill(log10(storedShower.egen)*333,copysign(sqrt(hit.x*hit.x + hit.y*hit.y),hit.x),hit.z)

        return hits,containmentZ,containmentR
