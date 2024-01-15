# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file EventCommonD3PDMaker/python/DRIndexAssociation.py
# @author scott snyder <snyder@bnl.gov>
# @author Ryan Reece <ryan.reece@cern.ch>
# @date Dec, 2009
# @brief Helper for setting up a nearest-DR association, represented
#        by an index.
#


from D3PDMakerCoreComps.D3PDObject import D3PDObject
from AthenaConfiguration.ComponentFactory import CompFactory

D3PD = CompFactory.D3PD


def DRIndexAssociation (parent,
                   type_name,
                   default_sgkey,
                   default_drcut,
                   prefix = '',
                   target = '',
                   level = 0,
                   blockname = None,
                   *args, **kw):
    """Helper for setting up a nearest-DR association, represented by an index.
"""
    if blockname is None:
        blockname = prefix + 'DRIndexAssoc'

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
            assoc = D3PD.DRAssociationTool (name + 'Assoc',
                                            Getter = getter,
                                            DRCut = drcut)

        filler = D3PD.ContainedAssociationFillerTool \
                 (name,
                  Prefix = prefix,
                  Associator = assoc,
                  BlockName = blockname)
        indexer = D3PD.IndexFillerTool \
                  (name + 'Index',
                   Target = target)
        filler.BlockFillers += [indexer]
        return filler

    obj = D3PDObject (maker, prefix)
    parent.defineBlock (level, blockname, obj)
    return obj


