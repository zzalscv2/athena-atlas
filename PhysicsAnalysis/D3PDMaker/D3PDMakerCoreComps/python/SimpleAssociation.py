# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file D3PDMakerCoreComps/python/SimpleAssociation.py
# @author scott snyder <snyder@bnl.gov>
# @date Aug, 2009
# @brief Helper for setting up an association to a contained object.
#


from .D3PDObject import D3PDObject
from AthenaConfiguration.ComponentFactory import CompFactory

D3PD = CompFactory.D3PD


def SimpleAssociation (parent,
                       assoctool,
                       prefix = '',
                       matched = '',
                       level = 0,
                       blockname = None,
                       suffix = '',
                       **kw):
    """Helper for setting up an association to a contained object.

    parent: The parent D3PDobject or block.
    assoctool: The class for the (single) association tool.
    prefix: Prefix to add to the contained variables, if any.
    matched: If not null, a flag variable with this name will
             be created, set to true when the association succeeds.
    level: Level of detail for the block.
    blockname: Name for the block.
    suffix: Suffix to add to the contained variables, if any.

    Extra arguments are passed to the association tool.

    Here's an example.  Suppose we have a generic tool that tuples
    Perigee objects, producing variables z0, d0, etc.  Suppose that
    we also have an `electron' object that has a method to return
    a Perigee.  We want to use the Perigee filling tool to tuple
    these variables as a new block.

    We can do this by creating an associator tool that will go from
    an electron object to its contained Perigee object.  We then configure
    things by creating a ContainedAssociationFillerTool using this
    associator.  We then add this as a block to the electron object,
    and then add a Perigee block to the associator block.  The helper
    here reduces the amount of boilerplate configuration code
    we need to do this.

    Usage is something like this:

       ElectronObject = ...
       ElectronPerigee = SimpleAssocation (ElectronObject,
                                           electronPerigeeAssociator)
       ElectronPerigee.defineBlock (1, 'Perigee', PerigeeFillerTool)

    If the electron prefix is `el_', this would create variables
    `el_z0', etc.  With `prefix="track"', the names would be
    `el_trackz0', etc.

    If we add `matched="hastrack"', then this will add a boolean
    variable `el_hastrack', which is true if the association succeeded.
"""
    if blockname is None:
        blockname = assoctool.__name__

    def maker (name, prefix, object_name, **kw2):
        assoc = assoctool (name + 'Assoc', **kw2)
        return D3PD.ContainedAssociationFillerTool \
               (name, Prefix = prefix, Suffix = suffix,
                Associator = assoc, Matched = matched)

    obj = D3PDObject (maker, prefix)
    parent.defineBlock (level, blockname, obj, **kw)
    return obj


def IdentityAssociation (parent, **kw):
    """A SimpleAssociation configured with an identity association.

This can be useful to group blocks together in subblocks.
"""
    return SimpleAssociation (parent,
                              D3PD.IdentityAssociationTool,
                              **kw)
