#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import sys
import os
from os.path import isfile
from optparse import OptionParser
from random import randint
from math import sqrt

usage = "usage: %prog [options] input1 [input2...]"

parser = OptionParser(usage=usage, version="%prog v0.0.1 $Id: LArG4FSStartPointFilter.py 711210 2015-11-27 15:56:00Z jchapman $")

parser.add_option("-p","--particle", dest="part", type="choice", action="append", choices=["11","22","2112"], help="particle to be filtered out (default - no filter)")
parser.add_option("-t","--truncate", dest="numevents", type=int, help="Truncate the number of events (default - all)")
parser.add_option("-l","--outevents", dest="outevents", type=int, help="Truncate the number of output (default - all)")
parser.add_option("-o","--output", dest="outfile", help="Name of output file")

parser.set_defaults(part=[],outfile="genevents.ascii",numevents=0,outevents=0,draw=False,execute=False)

(options, args) = parser.parse_args()

print (options, args)

if len(args) == 0 :
    print ("ERROR: No input! Aborting")
    sys.exit(1)

stpoints = []
globallinever = ""
globallinehead = ""

#reading input
for infilename in args :
    #opening file
    print ("Opening file",infilename)
    infile = open(infilename)
    #first line is empty
    linever = infile.readline().rstrip("\n")
    if linever=="":
        linever = infile.readline().rstrip("\n")
    print(linever)
    if (globallinever == "") :
        globallinever = linever
    linehead = infile.readline().rstrip("\n")
    print(linehead)
    if (globallinehead == "") :
        globallinehead = linehead
    #must be FS SP header OR normal HepMC GenEvent header
    if (( not linever.startswith("HepMC::Version")) or ("START_EVENT_LISTING" not in linehead) or (linever != globallinever)) :
        print ("ERROR: Wrong input:",infilename,"omitting", file=sys.stderr)
        continue
    #starting the loop
    stpointsloc = []
    line = infile.readline()
    while ((line != "") and ( not line.startswith("HepMC::" ))) :
        line1 = infile.readline()
        line2 = infile.readline()
        line3 = infile.readline()
        line4 = infile.readline()

        stpoint = [line,line1,line2,line3]
        if "Ascii" in globallinehead:
            if (line[0] != "E") or (line1[0] != "U") or (line2[0] != "P") or (line3[0] != "V") or (line4[0] != "P"):
                print ("ERROR:",infilename,"is not a valid file!")
                sys.exit(1)
            stpoint.append(line4)
        else:
            #checking every event for being single particle event [HepMC2 style]
            if (line[0] != "E") or (line1[0] != "U") or (line2[0] != "V") or (line3[0] != "P") :
                #This event structure was default in HepMC2, 
                #but it results in empty pool files in HepMC3. 
                #The HepMC3 reader always require both input and output particle.
                print ("ERROR:",infilename,"is not a valid file!")
                sys.exit(1)

            #adding extra line according to HepMC3 style
            if line4[0] == "P":
                stpoint.append(line4)

            #uncommnent the following 3 lines if you want to read old HepMC2 files
            # if line4[0] == "E":
            #     line_dummy = "P 1 999 0.0000000000000000e+00 0.0000000000000000e+00 0.0000000000000000e+00 0.0000000000000000e+00 0.0000000000000000e+00 4 0 0 -1 0"
            #     stpoint.insert(-1,line_dummy)
 
        stpointsloc.append(stpoint)
        
        line = line4
        if line[0] != "E":
            line = infile.readline()

    infile.close()
    stpoints += stpointsloc
    if ("tfile" in dir()) :
        del tinfo
        tfile.close()

if len(stpoints) == 0 :
    print ("ERROR: no events found is not a valid file!")
    sys.exit(1)

#creating an output stream
if isfile(options.outfile) :
    print ("WARNING: File",options.outfile,"already exists.")
outdata = open(options.outfile,"w")
outdata.write(globallinever)
outdata.write("\n")
outdata.write(globallinehead)
outdata.write("\n")

stpsize = len(stpoints)
if (options.numevents > stpsize) :
    print ("WARNING: requested number of events is bigger then provided in input files")
    options.numevents = 0

if (options.numevents == 0) :
    options.numevents = stpsize #all events

i = 0
while i < options.numevents :
    if (stpsize == 0) :
        print ("INFO: We've run out of starting point.\nIt's okay if you didn't specify -t option, but may mean that you do not have enough otherwise.")
        break
    rand = randint(0,stpsize-1)
    stpoint = stpoints.pop(rand) #take a random event
    stpsize-=1
    parsed = stpoint[-1].split() #last line - the particle. PDG code and 4-vector is there.
    pid_filt = parsed[2]
    if "Ascii" in globallinehead:
        pid_filt = parsed[3]
    if (options.part == []) or (pid_filt in options.part) : #check PDG
        parsed = stpoint[0].split() #first line - event itself. The number of event is there
        parsed[1] = str(i) #Change the number of event...
        stpoint[0] = " ".join(parsed) #...and construct the line back
        if (options.draw) :
            parsed = stpoint[2].split() #third line - the vertex.
            x = float(parsed[3])
            y = float(parsed[4])
            z = float(parsed[5])
            r = sqrt(x*x + y*y)
            hist.Fill(z,r)
        if len(options.outfile) > 0 :
            for iline in stpoint:
                outdata.write(iline.rstrip("\n")+"\n")
        i += 1

outdata.write(globallinehead.replace("START","END"))
outdata.close()
print ("INFO: Written", i, options.numevents, "starting points")

if (options.outevents > i) :
    print ("WARNING: requested number of events is bigger then provided in input files")
    options.outevents = 0

if (options.outevents == 0) :
    options.outevents = i #all events                                                                                                                                             

#starting the filter
exec = __file__.replace("LArG4FSStartPointFilterLegacy.py","LArG4FSStartPointFilterBody.py")
#print('athena -c "options={:s}" {:s}'.format(str(options),exec))
os.system('athena -c "options={:s}" {:s}'.format(str(options),exec))

