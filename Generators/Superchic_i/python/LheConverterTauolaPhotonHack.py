# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

import os
import re
import math
import xml.dom.minidom

from GeneratorModules.EvgenAlg import EvgenAlg
from AthenaPython.PyAthena import StatusCode


class LheConverterTauolaPhotonHack(EvgenAlg):
    '''
    Class for converting output LHE file from SuperChic to LHE recognised by Tauola
    This is a hack recommended by the Tauola authors, intended for the gamma gamma -> tau+ tau- process
    The LHE conversion is the same as Generators/Superchic_i/LheConverter.py, with the additional hack
    that changes the PDG ID of the initial state particles, from photons to electrons
    '''

    def __init__(self, name='LheConverterTauolaPhotonHack'):
        super(LheConverterTauolaPhotonHack, self).__init__(name=name)

    inFileName = 'evrecs/evrecout.dat'
    outFileName = 'events.lhe'
    done = False
    leptons = ['11', '-11', '13', '-13', '15', '-15']  # e, µ, τ

    def initialize(self):
        if(os.path.isfile(self.inFileName)):
            print(self.fileName)

            return self.convert()
        else:
            return StatusCode.Failure

    def convert(self):
        '''
        Converts `evrecs/evrecout.dat` input file to `events.lhe` recognised by the Pythia:
            - Replaces init block
            - Changes photon px and py to zero and energy to pz
            - Changes the PDG ID of initial state from (22 22) to (11 -11) and (-11 11) for every other event
            - Recalculates energy scales as an average of lepton pair transverse momentum
        '''

        if not self.done:
            DOMTree = xml.dom.minidom.parse(self.inFileName)
            collection = DOMTree.documentElement

            # Replace init block
            init = collection.getElementsByTagName('init')
            init_repl = r'''
 13  -13  2.510000e+03  2.510000e+03  0  0  0  0  3  1
 1.000000e+00  0.000000e+00  1.000000e+00   9999
 '''
            init[0].firstChild.data = init_repl

            # The comment line below indicates which part of the regex grabs the parton-level information that's to be modified. The index in list(header.groups()) is also shown
            #                                        energy scale [1]
            event_header = r'^(\s*\S*\s*\S*\s*\S*\s*)(\S*)(.*)$'
      
            # The comment line below indicates which part of the regex grabs the parton-level information that's to be modified. The index in list(particle) is also shown
            #                          pdgid [1]                                                       px [3]    py [5]    pz [7]    e [9]
            event_particle = r'^(\s*)([0-9-]+)(\s+[0-9-]+\s+[0-9-]+\s+[0-9-]+\s+[0-9-]+\s+[0-9-]+\s+)(\S+)(\s+)(\S+)(\s+)(\S+)(\s+)(\S+)(.*)$'

            events = collection.getElementsByTagName('event')
            for i, event in enumerate(events):
                new_particles = []
                lepton_mom = []

                particles = re.findall(event_particle, event.firstChild.data, re.MULTILINE)
                for particle in particles:
                    particle = list(particle)
                    if particle[1] == '22':  # photon
                        # Zero photon transverse momentum, change energy to be equal pz
                        particle[3] = '0.'
                        particle[5] = '0.'
                        particle[9] = particle[7]
                        if i%2 == 0 : 
                            particle[1] = '11' if float(particle[7]) > 0.0 else '-11'
                        else:
                            particle[1] = '-11' if float(particle[7]) > 0.0 else '11'
                        new_particles.append(''.join(particle))
                    elif particle[1] in self.leptons:
                        # Read leptons px and py
                        lepton_mom.append(float(particle[3]))
                        lepton_mom.append(float(particle[5]))
                        new_particles.append(''.join(particle))
                    else:
                        new_particles.append(''.join(particle))

                # Calculate new energy scale
                l1_px, l1_py, l2_px, l2_py = lepton_mom
                l1_pt = math.sqrt(l1_px**2 + l1_py**2)
                l2_pt = math.sqrt(l2_px**2 + l2_py**2)
                energy_scale = f'{(l1_pt + l2_pt) / 2.:.9E}'

                header = re.search(event_header, event.firstChild.data, re.MULTILINE)
                header = list(header.groups())
                header[1] = energy_scale

                event.firstChild.data = '\n'.join([''.join(header)] + new_particles) + '\n '

            with open(self.outFileName, 'w') as output:
                output.write(DOMTree.toxml().replace('<?xml version="1.0" ?>', ' '))

            self.done = True

        return StatusCode.Success

