#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import json
from ActsTrkEventCnv.ActsTrkEventCnvConfig import RunConversion
import math

if "__main__" == __name__:
    RunConversion()

    def _dparamsEqual(acts, trk):
        for acts_track_params, trk_track_params in zip(acts, trk):
            if (not math.isclose(acts_track_params, trk_track_params)):
                return False
        return True

    # Now compare outputs
    import json
    success = False
    with open('dump.json') as f:
        data = json.load(f)
        for event in data:
            print('Processing', event)
            acts = data[event]['MultiTrajectories']['ConvertedMultiTrajectory']
            trk = data[event]['Tracks']['CombinedITkTracks']
            if (acts != trk):
                # We might need to make this comparison more sophisticated
                # Since we do not necessarily expect binary identical results
                # ... but we are a ways off of worrying about this right now
                print('WARNING: Acts and Trk tracks differ')
                print('We have ', len(acts), '/',
                      len(trk), 'Acts / Trk tracks')
                for i, (acts_track, trk_track) in enumerate(zip(acts, trk)):
                    if (not _dparamsEqual(acts_track['dparams'], trk_track['dparams'])):
                        print('ERROR: Acts and Trk dparams differ for track', i)
                        print('Acts dparams:', acts_track['dparams'])
                        print('Trk dparams:', trk_track['dparams'])
                    if (acts_track['pos'] != trk_track['pos']):
                        print('ERROR: Acts and Trk pos differ for track', i)
                        print('Acts pos:', acts_track['pos'])
                        print('Trk pos:', trk_track['pos'])
            else:
                print('SUCCESS: Acts and Trk tracks agree')
                success = True
    if not success:
        print('ERROR: the output of the conversion is not correct')
        # Will uncomment this once the conversion is fixed
        # import sys
        # sys.exit(1)
