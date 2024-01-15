# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file D3PDMakerCoreComps/python/ContainedMultiAssociation.py
# @author scott snyder <snyder@bnl.gov>
# @date Aug, 2009
# @brief Helper for setting up an association to a set of contained objects.
#


from .D3PDObject import D3PDObject
from AthenaConfiguration.ComponentFactory import CompFactory

D3PD = CompFactory.D3PD


def ContainedMultiAssociation (parent,
                               assoctool,
                               prefix = '',
                               level = 0,
                               blockname = None,
                               **kw):
    """Helper for setting up an association to a set of contained objects.

    parent: The parent D3PDobject or block.
    assoctool: The class for the (single) association tool.
    prefix: Prefix to add to the contained variables, if any.
    level: Level of detail for the block.
    blockname: Name for the block.

    Extra arguments are passed to the association tool.
"""
    if blockname is None:
        blockname = assoctool.__name__

    def maker (name, prefix, object_name, **kw2):
        assoc = assoctool (name + 'Assoc', **kw2)
        return D3PD.ContainedMultiAssociationFillerTool \
               (name, Prefix = prefix, Associator = assoc)

    obj = D3PDObject (maker, prefix)
    parent.defineBlock (level, blockname, obj, **kw)
    return obj
