#! /usr/bin/env python3

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
## Simple script to split LHE files (XML, but not always correctly formatted)

import sys
import argparse
import math
import os

def main():
    # Grab and parse the arguments for the script
    parser = argparse.ArgumentParser(description="Split an LHE file into chunks")
    parser.add_argument('inputFile', help="Input file")
    parser.add_argument('--events', '-e', type=int, required=True,
                        help="Number of events per output file")
    parser.add_argument('--directory', '-d', help='Output directory (optional; default ".")')
    args = vars(parser.parse_args(sys.argv[1:]))

    # Pop open the input file
    if not os.access( args['inputFile'] , os.R_OK ):
        print(f'Error: cannot access input file {args["inputFile"][0]}')
        return 1
    in_file = open( args['inputFile'] , 'r' )

    # Do a little name parsing to figure out the name of the output file(s)
    last_dot = os.path.basename( args['inputFile'] ).rfind('.')
    out_stem = os.path.basename( args['inputFile'] )[ : last_dot ] 
    if 'directory' in args and args['directory'] is not None:
        out_stem = args['directory']+'/'+out_stem
    out_ext = '' if last_dot < 0 else os.path.basename( args['inputFile'] )[ last_dot : ]

    # Count the number of events in the file
    n_events = 0
    for line in in_file:
        if '<event>' in line or '<event ' in line:
            n_events+=1

    # Do a little error checking
    print(f'Attempting to split file of {n_events} events into files of {args["events"]} events each')
    if n_events < args['events']:
        print(f'Fewer events than requested {args["events"]} in input file. No action needed.')
        return 0
    if not (n_events/args['events']).is_integer():
        print(f'Warning: the final file will only have {n_events%args["events"]} events')

    # Reset to the beginning of the file
    in_file.seek(0,0)

    # Pop open all our output files
    n_files = math.ceil(n_events/args['events'])
    out_files = []
    for i in range(n_files):
        if os.access( f'{out_stem}_{i}{out_ext}' , os.R_OK ):
            print(f'Error: output file {out_stem}_{i}{out_ext} exists. Please cleanup and try again')
            sys.exit(1)
        out_files += [ open( f'{out_stem}_{i}{out_ext}', 'w' ) ]

    # Read back through the file, and let's parse!
    # Keep track of which event in the file we've reached
    this_event = -1
    for line in in_file:
        # See if we're in the header or the footer
        # If we are, it goes into ALL the files
        if (this_event==-1 and '<event>' not in line and '<event ' not in line) or this_event==n_events:
            for i in range(n_files):
                out_files[i].write(line)
            # Use a continue just to tidy up the next conditions
            continue

        # See if we're reading an event
        if '<event>' in line or '<event ' in line:
            this_event += 1

        # Which output file do we want?
        my_file = math.floor(this_event/args['events'])
        if my_file==n_files:
            print(f'Uh oh. {this_event} {args["events"]} {my_file}')
        out_files[my_file].write(line)

        # Special condition: we've reached the end
        if '</event>' in line and this_event==n_events-1:
            this_event += 1

    # Close up
    for i in range(n_files):
        out_files[i].close()
    in_file.close()

    # All done
    return 0

if __name__ == "__main__":
    sys.exit(main())
