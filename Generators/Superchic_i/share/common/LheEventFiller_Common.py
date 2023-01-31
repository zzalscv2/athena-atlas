"""
Configuration for transfering SuperChic output from Lhe to ENVT.
"""

import Superchic_i.EventFiller as EF

ef = EF.LheEVNTFiller(runArgs.ecmEnergy)
genSeq += ef
