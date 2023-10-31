# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import GaudiConfig2.semantics
from GaudiKernel.GaudiHandles import PrivateToolHandleArray, PublicToolHandle, ServiceHandle
from GaudiKernel.DataHandle import DataHandle
import re
import copy

from collections import abc
Sequence = abc.Sequence

class AppendListSemantics(GaudiConfig2.semantics.SequenceSemantics):
    '''
    Extend the sequence-semantics with a merge-method that appends the lists
    Use 'appendList<T>' as fifth parameter of the Gaudi::Property<T> constructor 
    to invoke this merging method. The template parameter is important, also
    in the string that forms the fifth argument. 
    '''
    __handled_types__ = (re.compile(r"^appendList<.*>$"),)
    def __init__(self, cpp_type, name=None):
        super(AppendListSemantics, self).__init__(cpp_type, name)

    def merge(self,b,a):
        a.extend(b)
        return a

class MapMergeNoReplaceSemantics(GaudiConfig2.semantics.MappingSemantics):
    '''
    Extend the mapping-semantics with a merge-method that merges two mappings as long as they do not have different values for the same key
    Use 'mapMergeNoReplace<T>' as fifth parameter of the Gaudi::Property<T> constructor
    to invoke this merging method.
    '''
    __handled_types__ = (re.compile(r"^mapMergeNoReplace<.*>$"),)
    def __init__(self, cpp_type, name=None):
        super(MapMergeNoReplaceSemantics, self).__init__(cpp_type, name)

    def merge(self,a,b):
        for k in b.keys():
            if k in a and b[k] != a[k]:
                raise ValueError('conflicting values in map under key %r and %r %r' % (k, b[k], a[k]))
            a[k] = b[k]
        return a


class VarHandleKeySemantics(GaudiConfig2.semantics.PropertySemantics):
    '''
    Semantics for all data handle keys (Read, Write, Decor, Cond).
    '''
    __handled_types__ = (re.compile(r"SG::.*HandleKey<.*>$"),)

    def __init__(self, cpp_type, name=None):
        super().__init__(cpp_type, name)
        # Deduce actual handle type
        self._type = next(GaudiConfig2.semantics.extract_template_args(cpp_type))
        self._isCond = 'CondHandle' in cpp_type

        if cpp_type.startswith("SG::Read"):
            self._mode = "R"
        elif cpp_type.startswith("SG::Write"):
            self._mode = "W"
        else:
            raise TypeError(f"C++ type {cpp_type} not supported")

    def store(self, value):
        if isinstance(value, DataHandle):
            v = value.Path
        elif isinstance(value, str):
            v = value
        else:
            raise TypeError(f"cannot assign {value!r} ({type(value)}) to {self.name}"
                            ", expected string or DataHandle")
        return DataHandle(v, self._mode, self._type, self._isCond)


class VarHandleArraySematics(GaudiConfig2.semantics.SequenceSemantics):
    '''
    Treat VarHandleKeyArrays like arrays of strings
    '''
    __handled_types__ = ("SG::VarHandleKeyArray",)

    class _ItemSemantics(GaudiConfig2.semantics.StringSemantics):
        """Semantics for an item (DataHandle) in a VarHandleKeyArray converting to string"""

        def __init__(self, name=None):
            super().__init__("std::string", name)

        def store(self, value):
            if isinstance(value, DataHandle):
                return value.Path
            elif isinstance(value, str):
                return value
            else:
                raise TypeError(f"cannot assign {value!r} ({type(value)}) to {self.name}"
                                ", expected string or DataHandle")

    def __init__(self, cpp_type, name=None):
        super().__init__(cpp_type, name, valueSem = self._ItemSemantics())

    def merge(self,bb,aa):
        for b in bb:
            if b not in aa:
                aa.append(b)
        return aa


class ToolHandleSemantics(GaudiConfig2.semantics.PropertySemantics):
    '''
    Private alg-tools need recusive merging (de-duplication):
    ''' 
    __handled_types__ = ("PrivateToolHandle",)
    def __init__(self,cpp_type,name=None):
        super(ToolHandleSemantics, self).__init__(cpp_type,name)
        

    def merge(self,b,a):
        #Deal with 'None'
        if a is None or a=='': return b
        if b is None or b=='': return a
        return a.merge(b)

class PublicHandleSemantics(GaudiConfig2.semantics.PropertySemantics):
    '''
    ServiceHandles (and the deprecated PublicToolHandles) are assigned as strings
    '''
    __handled_types__ = ("PublicToolHandle","ServiceHandle")
       
    def __init__(self,cpp_type,name=None):
        super(PublicHandleSemantics, self).__init__(cpp_type,name)
        
    def default(self,value):
        return value.typeAndName

    def store(self,value):
        if isinstance(value,str): #Assume the string is correct and the tool/svc alreayd in the CA ... 
            return value
        
        if value is None: 
            return ""

        if not hasattr(value,"__component_type__"):
            raise TypeError("Got {}, expected Tool or Service in assignment to {}".format(type(value),self.name))

        if value.__component_type__ not in ('Service','AlgTool'):
            raise TypeError('{} expected, got {!r} in assignemnt to {}'.\
                            format(value.__component_type__,value, self.name))

        #It would be great if at this point we could verify that the service was
        #ineed added to the ComponentAccumulator. Latest, do that when bootstapping
        #the application

        return "{}/{}".format(value.__cpp_type__,value.name)

class PublicHandleArraySemantics(GaudiConfig2.semantics.PropertySemantics):
    '''
    Semantics for arrays of string-based pointers to components defined elsewhere
    '''
    __handled_types__ = ("PublicToolHandleArray","ServiceHandleArray")
    def __init__(self,cpp_type,name=None):
        super(PublicHandleArraySemantics, self).__init__(cpp_type,name)
        
    def store(self, value):
        if not isinstance(value,Sequence) and not isinstance(value,set):
            value=[value,]

        newValue=[]
        for v in value:
            if isinstance(v,GaudiConfig2._configurables.Configurable):
                if v.__component_type__ not in ('Service','AlgTool'):
                    raise TypeError('{} expected, got {!r} in assignemnt to {}'.\
                                    format(value.__component_type__,v, self.name))
                else:
                    newValue.append("{}/{}".format(v.__cpp_type__,v.name))

            elif isinstance(v,(PublicToolHandle,ServiceHandle)):
                newValue.append("{}/{}".format(v.getType(),v.getName()))

            elif isinstance(v,str):
                #Check if component is known ...
                newValue.append(v)
                pass
            else:
                raise TypeError('Configurable expected, got {!r} in assignment to {}'.\
                                format(v,self.name))
        return newValue
            
    def default(self, value):
        return copy.copy(value)
        

    def merge(self,bb,aa):
        for b in bb:
            if b not in aa:
                aa.append(b)
        return aa
        #union=set(a) | set(b)
        #return union


class ToolHandleArraySemantics(GaudiConfig2.semantics.PropertySemantics):
    '''
    Private alg-tools need recusive merging (de-duplication):
    ''' 
    __handled_types__ = ("PrivateToolHandleArray",)
    def __init__(self,cpp_type,name=None):
        super(ToolHandleArraySemantics, self).__init__(cpp_type,name)
    
    def default(self,value):
        return copy.copy(value)

    def store(self,value):
        if not isinstance(value,PrivateToolHandleArray):
            #try to convert the value to a PrivateToolHandleArray
            value=PrivateToolHandleArray(value)
        return value

    def merge(self,b,a):
        for bTool in b:
            try:
                #If a tool with that name exists in a, we'll merge it
                a.__getitem__(bTool.getName()).merge(bTool)
            except IndexError:
                #Tool does not exists in a, append it
                a.append(bTool)
        return a

class SubAlgoSemantics(GaudiConfig2.semantics.PropertySemantics):
    __handled_types__  = ("SubAlgorithm",)
    def __init__(self,cpp_type,name=None):
        super(SubAlgoSemantics, self).__init__(cpp_type,name)
        
    def store(self,value):
        if not isinstance(value,Sequence):
            value=[value,]
        
        for v in value:
            if v.__component_type__ != 'Algorithm':
                raise TypeError('Algorithm expected, got {!r} in assignemnt to {}'.\
                                format(value, self.name))
        return value


    #Without explicitly definig a default, calling .append or += will change the class-default, 
    #affecting all instances of the same class. 
    def default(self,value):
        return []


from AthenaServices.ItemListSemantics import OutputStreamItemListSemantics

GaudiConfig2.semantics.SEMANTICS.append(AppendListSemantics)
GaudiConfig2.semantics.SEMANTICS.append(VarHandleKeySemantics)
GaudiConfig2.semantics.SEMANTICS.append(VarHandleArraySematics)
GaudiConfig2.semantics.SEMANTICS.append(ToolHandleSemantics)
GaudiConfig2.semantics.SEMANTICS.append(ToolHandleArraySemantics)
GaudiConfig2.semantics.SEMANTICS.append(PublicHandleSemantics)
GaudiConfig2.semantics.SEMANTICS.append(PublicHandleArraySemantics)
GaudiConfig2.semantics.SEMANTICS.append(SubAlgoSemantics)
GaudiConfig2.semantics.SEMANTICS.append(MapMergeNoReplaceSemantics)
GaudiConfig2.semantics.SEMANTICS.append(OutputStreamItemListSemantics)
