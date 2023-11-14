# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from GaudiConfig2.semantics import getSemanticsFor, SequenceSemantics
from collections import defaultdict

from AthenaCommon.Logging import logging
msg = logging.getLogger("OutputStreamItemListAuxCheck")


class OutputStreamItemListSemantics(SequenceSemantics):
    __handled_types__ = ( "OutputStreamItemList", )
    __AUX_ext__  = "Aux."
    __AUX_len__  = len(__AUX_ext__)

    def __init__(self, cpp_type):
        valueSem = getSemanticsFor("std::string") if cpp_type in self.__handled_types__ else None
        super(OutputStreamItemListSemantics, self).__init__(cpp_type, valueSem=valueSem)

    def merge(self, bb, aa):
        for b in bb:
            if b not in aa:
                aa.append(b)
        scrubbed = self.checkAuxAttributes( aa )
        aa.clear()
        aa.extend( scrubbed )
        return aa

    def checkAuxAttributes(self, itemList):
        """
        Checks dynamic Aux attribute selection in the ItemList for duplicates and conflicts

        From Event/xAOD/xAODCore/Root/AuxSelection.cxx
        The formalism for attribute selection is the following:
         - An empty set, or a set containing "*" will select all the dynamic
             attributes passed to the object.
         - A single "-" attribute will not select any of the dynamic attributes.
         - A set of variables (without "-" as the first character of the
             variable names) will select just the variables listed.
         - A set of variable names, each prefixed by "-", will select all
             variables but the ones listed.
        """
        newitemlist=[]
        auxitems = defaultdict(set)
        for item in itemList:
            auxpos = item.find(self.__AUX_ext__)
            if auxpos > 0:
                # Aux store item
                itemname  = item[ : auxpos+self.__AUX_len__]
                selection = item[auxpos+self.__AUX_len__ : ]
                # collect attributes selection for this item in a set
                # empty selection means 'everything'
                auxitems[itemname].add( selection )
            else:
                newitemlist.append(item)

        newauxlist=[]
        for k,sel in auxitems.items():
            allsel = set()    # gather all selection items in this set
            for line in sel:
                allsel.update( line.split('.') )
                if ".." in line or line.startswith(".") or line.endswith('.'):
                    raise ValueError(f"ItemList AuxAttribute selection syntax error - extra dot in '{line}'")
            negsel = [s for s in allsel if s[:1]=='-']
            if negsel and len(negsel) != len(allsel):
                raise ValueError(f"Mixing up negative and positive Aux selections is not supported: {k} : {str(sel)}")
            if len(sel) == 1:
                # single selection, just pass it on (cleaned)
                newauxlist.append( k + ".".join(sorted(allsel)))
                continue
            # multiple selections fun
            if negsel:
                # multiple different negative selections are usually logically incompatible, report error
                raise ValueError(f"Multiiple (different) negative selection are not supported: for {k} : {str(sel)}")
            if '' in sel or '*' in sel:
                msg.info(f"Multiple Aux attribute selections for {k} - will write all attributes." +
                         f" Original selection was: {str(sel)}")
                newauxlist.append( k + '*')
                continue
            # 2 or more positive selections - merge them into one
            newitem =  k + ".".join(sorted(allsel))
            newauxlist.append(newitem)
            msg.info(f"Multiple attribute selections for {k} - will write combined selection. Found {len(sel)} selections: {str(sel)}")
            msg.info(f"  New selection: {newitem}")

        return newitemlist + newauxlist

