#!/usr/bin/env python
#
# RunLumiTime.py
#
# Simple utility script to take a range of run numbers and output a text list of times associated
# with the luminosity blocks in those runs.
#
# This is also a useful example of how to use the CoolLumiUtilities tools
#
import sys
from argparse import ArgumentParser

# Get our global DB handler object
from CoolLumiUtilities.CoolDataReader import CoolDataReader
from CoolLumiUtilities.LumiDBHandler import LumiDBHandler

class RunLumiTime:

    def __init__(self):

        # online luminosity database
        self.onlLumiDB = 'COOLONL_TRIGGER/CONDBR2'

        # folder with LB -> time conversion
        self.onlLBLBFolder = '/TRIGGER/LUMI/LBLB'

        # the LumiDBHandler
        self.dbHandler = None

        # parse command-line input
        args = self.parseArgs()

        # output level
        self.verbose = args.verbose

        # output file (default stdout)
        self.outFile = args.outfile

        # List of (integer) run numbers specified on the command line
        self.runList = args.runlist

        print(f"Finished parsing run list: {', '.join([str(run) for run in self.runList])}")

    def __enter__(self):
        # Instantiate the LumiDBHandler, so we can cleanup all COOL connections at exit
        self.dbHandler = LumiDBHandler()
        return self

    def __exit__(self, exc_type, exc_value, exc_traceback):
        self.closeDb()

    def __del__(self):
        self.closeDb()

    def closeDb(self):
        if self.dbHandler is not None:
            self.dbHandler.closeAllDB()

    def parseArgs(self):
        parser = ArgumentParser()

        parser.add_argument("-v", "--verbose",
                            action="store_true", dest="verbose",
                            help="turn on verbose output")

        parser.add_argument("-r", "--run", nargs='*', required=True, type=int,
                            dest="runlist", metavar="RUN",
                            help="show specific run(s)")

        parser.add_argument('-o', '--output',
                            dest='outfile', metavar = "FILE", default=None, type=str,
                            help="write results to output file (default stdout). If filename ends in csv or json those formats are used.")
        
        return parser.parse_args()

    def execute(self):
        # Instantiate the LumiDBHandler, so we can cleanup all COOL connections in the destructor
        if self.dbHandler is None:
            self.dbHandler = LumiDBHandler()

        # Open outfile if desired
        fp = None
        format = "stdout"
        if self.outFile is not None:
            fp = open(self.outFile, 'w')
            format = "txt"
            if self.outFile.endswith(".json"):
                format = "json"
                output = {}
            if self.outFile.endswith(".csv"):
                format = "csv"


        # Get our COOL folder
        lblb = CoolDataReader(self.onlLumiDB, self.onlLBLBFolder)
        
        # Load data for each run specified
        for run in self.runList:

            lblb.setIOVRangeFromRun(run)
            if not lblb.readData():
                print(f'RunLumiTime - No LBLB data found for run {run}!')
                continue

            for obj in lblb.data:
                # IOV is equal to (Run << 32) + LB number.
                run = obj.since() >> 32
                lb = obj.since() & 0xFFFFFFFF
                # Time is UTC nanoseconds
                startTime = obj.payload()['StartTime']
                endTime = obj.payload()['EndTime']
                if format == "json":
                    if not run in output:
                        output[run] = []
                    output[run] += [{
                        "lb":lb,
                        "begin": startTime/1.e9,
                        "end": endTime/1.e9
                    }]
                else:
                    entry = (run, lb, startTime/1.e9, endTime/1.e9)
                    separator = ',' if format=='csv' else ' '
                    print(separator.join([str(v) for v in entry]), file=fp)

        if format=="json":
            import json
            json.dump(output, fp, indent=4)
        
        # close the file
        if fp is not None:
            fp.close()
            print(f"Wrote file {self.outFile}")

# Executed from the command line
if __name__ == '__main__':
    # rlt = RunLumiTime()
    # sys.exit(rlt.execute())

    with RunLumiTime() as rlt:
        rlt.execute()
      
