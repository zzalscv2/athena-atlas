# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Test for the OutputStreamAthenaPool.OutputStreamConfig.checkAuxAttributesInItemList function

from AthenaConfiguration.ComponentAccumulator import ConfigurationError
from AthenaServices.ItemListSemantics import OutputStreamItemListSemantics as semantics

Tests = [
    # first value indicates if the selection is good and should be accepted
 ( True, [ 'xAOD::AuxInfoBase!#EventShapeAux.Bla.Density',  'xAOD::AuxInfoBase!#EventShapeAux.Density' ] ) ,
 ( True, [ 'xAOD::AuxInfoBase!#EventShapeAux.',  'xAOD::AuxInfoBase!#EventShapeAux.Density' ] ) ,
 ( True, [ 'xAOD::AuxInfoBase!#EventShapeAux.',  'xAOD::AuxInfoBase!#EventShapeAux..Density' ] ) ,
 ( False, [ 'xAOD::AuxInfoBase!#EventShapeAux.-Bla.Density',  'xAOD::AuxInfoBase!#EventShapeAux.Density' ] ) ,
 ( False, [ 'xAOD::AuxInfoBase!#EventShapeAux.-Bla',  'xAOD::AuxInfoBase!#EventShapeAux.-Density' ] ) ,
 ( False, [ 'xAOD::VertexAuxContainer#GSFConversionVerticesAux.',
            'xAOD::VertexAuxContainer#GSFConversionVerticesAux.-vxTrackAtVertex' ] ) ,
 ( False, [ 'xAOD::AuxInfoBase!#EventShapeAux.-Bla.X' ] ) ,
 ( True, [ 'xAOD::AuxInfoBase!#EventShapeAux.-Bla.' ] ) ,
 ( True, [ 'xAOD::AuxInfoBase!#EventShapeAux..Bla.' ] ) ,
 ( True, [ 'xAOD::AuxInfoBase!#EventShapeAux.-' ] ) ,
 ( True, [ 'xAOD::AuxInfoBase!#EventShapeAux.-' ] ) ,
 ( False, [ 'xAOD::AuxInfoBase!#EventShapeAux.*', 'xAOD::AuxInfoBase!#EventShapeAux.-' ] ) ,
 ( True, [ 'xAOD::AuxInfoBase!#EventShapeAux.' ] ) ,
 ( False, [ 'xAOD::AuxInfoBase!#EventShapeAux.-.*' ] ) ,
 ( False, [ 'xAOD::AuxInfoBase!#EventShapeAux.-.Bla' ] )
]

allTestsOK = True
for test in Tests:
   print("Testing:", test[1])
   good = True
   try:
       newitemlist = semantics.checkAuxAttributes(None, test[1])
       for i in newitemlist:
          print(f"  {i}")
   except ConfigurationError as err:
       print("ERROR", err)
       good = False

   print("-"*120)
   if good == test[0]:
       print("Test succeeded")
   else:
       print("Test failed")
       allTestsOK = False
   print()

if not allTestsOK:
    print("Not all tests passes!")
    import sys
    sys.exit(-1)

