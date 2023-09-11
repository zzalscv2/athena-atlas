
import os
if os.environ["SHERPAVER"].startswith('3.'):

  genSeq.Sherpa_i.BaseFragment += """
HADRON_DECAYS:
  Model: Lund
PARJ(21): 0.36
PARJ(41): 0.3
PARJ(42): 0.6
"""

else:

  genSeq.Sherpa_i.Parameters += [
    "FRAGMENTATION=Lund",
    "DECAYMODEL=Lund",
    "PARJ(21)=0.36",
    "PARJ(41)=0.3",
    "PARJ(42)=0.6"
  ]
