# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
@brief Utility functions used by the MinBias monitoring
"""


def getMinBiasChains(monAccess, wildcard=''):
    """ Returns all MinBias chains passing wildcard as (chain name, monitoring level) tuple """

    chains = []
    chains.extend((x, 'Expert') for x in monAccess.monitoredChains(signatures='mbMon', monLevels='t0', wildcard=wildcard))
    chains.extend((x, 'Shifter') for x in monAccess.monitoredChains(signatures='mbMon', monLevels='shifter', wildcard=wildcard))

    return chains
