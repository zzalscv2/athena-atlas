#!/usr/bin/env python3
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import argparse
import re

parser = argparse.ArgumentParser(
    description='Postprocessing of HLT jobOptions json comparison'
)
parser.add_argument('input', type=str, help='Input text file to sanitise')
parser.add_argument('output', type=str, help='Output sanitised text file')
parser.add_argument('--max-comps', type=int, help='Max components to print, for debugging')
args = parser.parse_args()

component_re = re.compile(r'Component ([\w\.]*) may differ')
step_sequence_re = re.compile(r'Step\d+_.*')

skip_components = [
    'HLTAllSteps',
]

with open(args.input) as fin:
    with open(args.output,'w') as fout:
        # For cleaning up diffs that should be suppressed, we collect
        # sets of lines pertaining to one component
        # Clear if the diff ends with the suppression message
        these_lines = []
        i = 0
        skip_comp = False
        for l in fin.readlines():
            match = component_re.match(l)
            if match:
                # Print anything in the list, then reset
                for ll in these_lines:
                    print(ll,file=fout)
                comp_name = match.group(1)
                if comp_name in skip_components or step_sequence_re.match(comp_name):
                    skip_comp = True
                else:
                    skip_comp = False
                    these_lines = [l.strip()]
                    i+=1
            elif 'all are suppressed by' in l:
                these_lines = []
            elif not skip_comp:
                # This includes the preamble
                these_lines.append(l.strip())
            if i==args.max_comps:
                break


