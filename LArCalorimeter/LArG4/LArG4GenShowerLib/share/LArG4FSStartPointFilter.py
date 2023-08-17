#!/usr/bin/env python
# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

import sys
import os
from os.path import isfile
from optparse import OptionParser
from random import randint
from math import sqrt
from pyHepMC3 import HepMC3 as HepMC

usage = "usage: %prog [options] input1 [input2...]"

parser = OptionParser(usage=usage, version="%prog v0.0.1 $Id: LArG4FSStartPointFilter.py 711210 2015-11-27 15:56:00Z jchapman $")

parser.add_option("-p","--particle", dest="part", type="choice", action="append", choices=["11","22","2112"], help="particle to be filtered out (default - no filter)")
parser.add_option("-t","--truncate", dest="numevents", type=int, help="Truncate the number of events (default - all)")
parser.add_option("-l","--outevents", dest="outevents", type=int, help="Truncate the number of output (default - all)")
parser.add_option("-o","--output", dest="outfile", help="Name of output file")

parser.set_defaults(part=[],outfile="genevents.ascii",numevents=0,outevents=0)

(options, args) = parser.parse_args()

print (options, args)

if len(args) == 0 :
    print ("ERROR: No input! Aborting")
    sys.exit(1)

stpoints = []
#reading input
for infilename in args :
    #opening file
    infile = HepMC.ReaderAscii(infilename)

    if infile.failed():
        print("Wrong input. Exit.\n")
        sys.exit(1)

    while not infile.failed():
        evt = HepMC.GenEvent()
        infile.read_event(evt)
        
        if infile.failed():
            print("End of file reached.\n")
            break

        stpoints.append(evt)

    infile.close()

if len(stpoints) == 0 :
    print ("ERROR: no events found is not a valid file!")
    sys.exit(1)


#creating an output stream
if isfile(options.outfile) :
    print ("WARNING: File",options.outfile,"already exists.")

outdata = HepMC.WriterAscii(options.outfile)
if outdata.failed():
    print("Bad output. Exit.\n")
    sys.exit(1)

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

    particles = stpoint.particles()
    pid_filt = str(particles[-1].pdg_id())

    if (options.part == []) or (pid_filt in options.part) : #check PID
        stpoint.set_event_number(i)        
        outdata.write_event(stpoint)
        i += 1

outdata.close()
print ("INFO: Written", i, options.numevents, "starting points")

if (options.outevents > i) :
    print ("WARNING: requested number of events is bigger then provided in input files")
    options.outevents = 0

if (options.outevents == 0) :
    options.outevents = i #all events                                                                                                                                             

#starting the filter
exec = __file__.replace("LArG4FSStartPointFilter.py","LArG4FSStartPointFilterBody.py")
#print('athena -c "options={:s}" {:s}'.format(str(options),exec))
os.system('athena -c "options={:s}" {:s}'.format(str(options),exec))

