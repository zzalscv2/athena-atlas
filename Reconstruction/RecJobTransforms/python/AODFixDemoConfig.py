# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from RecJobTransforms.AODFixHelper import releaseInRange


def AODFixDemoCfg(flags):

   #first check if we need to apply this AODFix
   #Let's assume the bug is in Athena-22.0.80 to Athena 22.0.89
   if not releaseInRange(flags,"Athena-22.0.80","Athena-22.0.89"): 
      return None

   result=ComponentAccumulator()

   #Use the AddressRemappingSvc to rename the input object
   from SGComps.AddressRemappingConfig import InputRenameCfg
   result.merge(InputRenameCfg("xAOD::ElectronContainer", "Electrons", "old_Electrons"))



   #Re-run the required algorithms or schedule and ad-hoc algorithm 
   #that creates a new container by correcting  the values in  the 
   #renamed input container
   result.addEventAlgo(CompFactory.electronRescaler(InputName="old_Electrons", 
                                                    OutputName="Electrons"))
                                                    
   return result


   
