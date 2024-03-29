# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file EventCommonD3PDMaker/python/DRMultiAssociation.py
# @author scott snyder <snyder@bnl.gov>
# @author Ryan Reece <ryan.reece@cern.ch>
# @date Dec, 2009
# @brief Helper for setting up an association to objects within a DR cone,
#        represented by containment.
#


from D3PDMakerCoreComps.D3PDObject import D3PDObject
from AthenaConfiguration.ComponentFactory import CompFactory

D3PD = CompFactory.D3PD


def DRMultiAssociation (parent,
                   type_name,
                   default_sgkey,
                   default_drcut,
                   prefix = '',
                   level = 0,
                   blockname = None,
                   *args, **kw):
    """Helper for setting up an association to objects within a DR cone,
    represented by containment.
"""
    if blockname is None:
        blockname = prefix + 'DRMultiAssoc'

    def maker (name, prefix, object_name,
               sgkey = default_sgkey,
               getter = None,
               assoc = None,
               drcut = default_drcut):

        if not getter:
            getter = D3PD.SGDataVectorGetterTool \
                     (name + '_Getter',
                      TypeName = type_name,
                      SGKey = sgkey)
        if not assoc:
            assoc = D3PD.DRConeAssociationTool (name + 'Assoc',
                                                Getter = getter,
                                                DRCut = drcut)

        return D3PD.ContainedMultiAssociationFillerTool (name,
                                                         Prefix = prefix,
                                                         Associator = assoc)

    obj = D3PDObject (maker, prefix)
    parent.defineBlock (level, blockname, obj)
    return obj

    
