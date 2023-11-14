#!/usr/bin/env python
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import unittest

# always need to import this to initialize our semantics:
from AthenaConfiguration import AtlasSemantics
from AthenaServices.ItemListSemantics import OutputStreamItemListSemantics
from GaudiConfig2 import Configurables


class TestOutpputStreamItemListSemantics(unittest.TestCase):
   """Test OutputStream ItemList merging Semantics"""

   semantics = OutputStreamItemListSemantics("OutputStreamItemList")

   def test_semantics(self):
      stream = Configurables.AthenaOutputStream()
      self.assertIsInstance(stream._descriptors['ItemList'].semantics,
                            OutputStreamItemListSemantics)

   def test_multiple_selections_good(self):
      list = ['xAOD::AuxInfoBase!#EventShapeAux.Bla.Density',  'xAOD::AuxInfoBase!#EventShapeAux.Density']
      self.assertEqual(self.semantics.merge([], list), ["xAOD::AuxInfoBase!#EventShapeAux.Bla.Density"])

   def test_multiple_selections_withall(self):
      list = ['xAOD::AuxInfoBase!#EventShapeAux.',  'xAOD::AuxInfoBase!#EventShapeAux.Density']
      self.assertEqual(self.semantics.merge([], list), ["xAOD::AuxInfoBase!#EventShapeAux.*"])

   def test_doubledot_error(self):
      list = ['xAOD::AuxInfoBase!#EventShapeAux.',  'xAOD::AuxInfoBase!#EventShapeAux..Density']
      self.assertRaises(ValueError, self.semantics.merge, [], list );

   def test_enddot_error(self):
      list = ['xAOD::AuxInfoBase!#EventShapeAux.-Bla.']
      self.assertRaises(ValueError, self.semantics.merge, [], list );

   def test_middledot_error(self):
      list = ['xAOD::AuxInfoBase!#EventShapeAux.Bla..Blaa']
      self.assertRaises(ValueError, self.semantics.merge, [], list );

   def test_negative_positive_mix(self):
      list = ['xAOD::AuxInfoBase!#EventShapeAux.-Bla.Density',  'xAOD::AuxInfoBase!#EventShapeAux.Density']
      self.assertRaises(ValueError, self.semantics.merge, [], list );

   def test_negatives_merge(self):
      list = ['xAOD::AuxInfoBase!#EventShapeAux.-Bla',  'xAOD::AuxInfoBase!#EventShapeAux.-Density']
      self.assertRaises(ValueError, self.semantics.merge, [], list );

   def test_negatives_merge_all(self):
      list = ['xAOD::VertexAuxContainer#GSFConversionVerticesAux.',
              'xAOD::VertexAuxContainer#GSFConversionVerticesAux.-vxTrackAtVertex']
      self.assertRaises(ValueError, self.semantics.merge, [], list );
      
   def test_single_negative(self):
      list = ['xAOD::AuxInfoBase!#EventShapeAux.-Bla']
      self.assertEqual(self.semantics.merge([], list), list)

   def test_all_negative(self):
      list = ['xAOD::AuxInfoBase!#EventShapeAux.-']
      self.assertEqual(self.semantics.merge([], list), list)

   def test_all_nothing(self):
      list = ['xAOD::AuxInfoBase!#EventShapeAux.*',  'xAOD::AuxInfoBase!#EventShapeAux.-']
      self.assertRaises(ValueError, self.semantics.merge, [], list );

   def test_all_nothing2(self):
      list = [ 'xAOD::AuxInfoBase!#EventShapeAux.-.*' ] 
      self.assertRaises(ValueError, self.semantics.merge, [], list );

   def test_nothing_something(self):
      list = [ 'xAOD::AuxInfoBase!#EventShapeAux.-.Bla' ] 
      self.assertRaises(ValueError, self.semantics.merge, [], list );


if __name__ == "__main__":
    unittest.main()

