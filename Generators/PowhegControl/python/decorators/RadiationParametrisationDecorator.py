# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

## @PowhegControl RadiationParametrisationDecorator
#  Powheg runcard decorator for ISR/FSR phase space parameters
#
#  Authors: James Robinson  <james.robinson@cern.ch>

#! /usr/bin/env python

class RadiationParametrisationDecorator :

  def __init__( self, decorated ) :
    ## Attach decorations to Powheg configurable
    decorated.run_card_decorators.append( self )
    self.decorated = decorated

    self.decorated.fullphsp      = -1
    self.decorated.raisingscales = -1


  def append_to_run_card( self ) :
    ## Write decorations to runcard
    with open( self.decorated.runcard_path(), 'a' ) as f :
      f.write( 'fullphsp '+str(self.decorated.fullphsp)+'           ! use ISR/FSR phase space parametrization, default 0, do not\n' )
      f.write( 'raisingscales '+str(self.decorated.raisingscales)+' ! \n' )
