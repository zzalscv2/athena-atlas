# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

import pickle
import ast
import logging
from typing import Dict, List, Set, Tuple, cast
import collections
import json
import sys
import re
import argparse

from AthenaConfiguration.iconfTool.models.element import (
    Element,
    GroupingElement,
)
from AthenaConfiguration.iconfTool.models.structure import ComponentsStructure

logger = logging.getLogger("confTool")
logger.setLevel(level=logging.INFO)
logger.addHandler(logging.FileHandler("confTool-last-run.log", mode='w'))

componentRenamingDict={}

baseParser = argparse.ArgumentParser()
baseParser.add_argument(
    "--includeComps",
    nargs="*",
    help="Report only component matching this string",
    action="append",
)
baseParser.add_argument(
    "--excludeComps",
    nargs="*",
    help="Exclude components matching this string",
    action="append",
)
baseParser.add_argument(
    "--ignoreIrrelevant",
    help="Ignore differences in e.g. outputlevel",
    action="store_true",
)
baseParser.add_argument(
    "--ignore",
    action="append",
    default= [
        "StoreGateSvc",
        "OutputLevel",
        "ExtraInputs",
        "ExtraOutputs",
        "DetStore",
        "EvtStore",
        "EventStore",
        "NeededResources",
        "GeoModelSvc",
        "MetaDataStore",
        "wallTimeOffset" # perfmon Svc property, is timestamp of stating the job, different by construction
    ],
    help="Ignore properties",
)
baseParser.add_argument(
    "--renameComps",
    nargs="*",
    help="Pass comps You want to rename as OldName=NewName.",
    action="append",
)
baseParser.add_argument(
    "--renameCompsFile",
    help="Pass the file containing remaps",
)

baseParser.add_argument(
    "--ignoreDefaults",
    help="Ignore values that are identical to the c++ defaults. Use it only when when the same release is setup as the one used to generate the config.",
    action="store_true"
)

baseParser.add_argument(
    "--ignoreDefaultNamedComps",
    help="""Ignores default handles that have full type specified. That is, if the setting is actually: Tool/A and the default value was just A, the Tool/A is assumed to be default and eliminated.
    Beware that there is a caveat, the ignored class name may be actually different from the default (there is no way to check that in python).""",
    action="store_true"
)


baseParser.add_argument(
    "--shortenDefaultComponents",
    help="Automatically shorten component names that have a default name i.e. ToolX/ToolX to ToolX. It helps comparing Run2 & Run3 configurations where these are handled differently",
    action="store_true",
)

baseParser.add_argument(
    "--skipProperties",
    help="Do not load properties other than those referring to other components",
    action="store_true",
)

baseParser.add_argument(
    "--follow",
    help="Follow to related components up to given recursion depth",
    type=int,
    default=0
)

baseParser.add_argument(
    "--debug",
    help="Enable tool debugging messages",
    action="store_true",
)

def __flatten_list(l):
    return [item for elem in l for item in elem] if l else []


def types_in_properties(value, dict_to_update):
    """Updates updates the dictionary with (potentially) component name -> component type"""
    parsable = False
    try:
        s = ast.literal_eval(str(value))
        parsable = True
        if isinstance(s, list):
            for el in s:
                types_in_properties(el, dict_to_update)
    except Exception:
        pass
    if isinstance(value,str) and "/" in value and not parsable:
        comp = value.split("/")
        if len(comp) == 2:
            dict_to_update[comp[1]] = comp[0]
            logger.debug("Found type of %s to be %s", comp[1], comp[0])
    if isinstance(value, dict):
        [ types_in_properties(v, dict_to_update) for v in value.values() ]


def collect_types(conf):
    name_to_type = {}
    for (comp_name, comp_settings) in conf.items():
        types_in_properties(comp_settings, name_to_type)
    return name_to_type


def excludeIncludeComps(dic, args, depth, compsToFollow=[]) -> Dict:
    conf = {}
    if depth == 0:
        return conf
    compsToReport = __flatten_list(args.includeComps)
    compsToExclude = __flatten_list(args.excludeComps)

    def eligible(component):
        exclude = any(re.match(s, component) for s in compsToExclude)
        if (component in compsToFollow or component.lstrip("ToolSvc.") in compsToFollow) and not (exclude or component in args.ignore):
            logger.debug("Considering this component: %s because some other one depends on it", component)
            return True
        include = any(re.match(s, component) for s in compsToReport)
        if args.includeComps and args.excludeComps:
            return include and not exclude
        elif args.includeComps:
            return include
        elif args.excludeComps:
            return not exclude

    for (comp_name, comp_attributes) in dic.items():
        if eligible(comp_name):
            conf[comp_name] = comp_attributes
            if depth > 0:
                types = {}
                types_in_properties(comp_attributes, types)
                logger.debug("Following up for types included in here %s whole set of components to follow %s ", types, compsToFollow)
                compsToFollow += types.keys()         
            logger.debug("Included component %s", comp_name)
        else:
            logger.debug("Ignored component %s", comp_name)
    if depth > 0:
        conf.update(excludeIncludeComps(dic, args, depth-1, compsToFollow))
    return conf

def ignoreIrrelevant(dic, args) -> Dict:
    def remove_irrelevant(val_dict):
        return (
            { key: val for key, val in val_dict.items() if key not in args.ignore }
            if isinstance(val_dict, dict)
            else val_dict
        )
    conf = {}
    for (key, value) in dic.items():
        conf[key] = remove_irrelevant(value)
    return conf

def renameComps(dic, args) -> Dict:
    compsToRename = __flatten_list(args.renameComps)
    if args.renameCompsFile:
        with open( args.renameCompsFile, "r") as refile:
            for line in refile:
                if not (line.startswith("#") or line.isspace() ):
                    compsToRename.append( line.rstrip('\n') )
    global componentRenamingDict
    componentRenamingDict.update({
        old_name: new_name
        for old_name, new_name in [
            [e.strip() for e in element.split("=")] for element in compsToRename
        ]
    })
    for f,t in componentRenamingDict.items():
        logger.info("Renaming from: %s to %s", f, t)

    def rename_comps(comp_name):
        """Renames component if it is in the dict or, when name fragment is in the dict
        The later is for cases like: ToolSvc.ToolA.X.Y is renamed to ToolSvc.ToolB.X.Y
        """
        logger.debug("Trying renaming on, %s", comp_name)
        for k,v in componentRenamingDict.items():
            if k == comp_name:
#                logger.debug("Renamed comp %s to %s", k, v)
                return v

            old = f".{k}."
            if old in comp_name:
                return comp_name.replace(old, f".{v}.")

            old = f"{k}."
            if comp_name.startswith(old):
                return comp_name.replace(old, f"{v}.")


            old = f".{k}"
            if comp_name.endswith(old):
                return comp_name.replace(old, f".{k}")
        return comp_name

    conf = {}
    for (key, value) in dic.items():        
        renamed = rename_comps(key)
        if renamed != key:
             logger.debug("Renamed comp %s to %s", key, renamed)
        conf[renamed] = value
    return conf

def ignoreDefaults(allconf, args, known) -> Dict:
    conf = {}
    def drop_defaults(component_name, val_dict):
        # try picking the name from the dict, if missing use last part of the name, if that fails use the component_name (heuristic)
        component_name_last_part = component_name.split(".")[-1]
        component_type = known.get(component_name, known.get(component_name_last_part, component_name_last_part))
        comp_cls = None
        try:
            from AthenaConfiguration.ComponentFactory import CompFactory
            comp_cls = CompFactory.getComp(component_type)
            logger.debug("Loaded the configuration class %s/%s for defaults elimination", component_type, component_name)
        except Exception:
            logger.debug("Could not find the configuration class %s/%s, no defaults for it can be eliminated", component_type, component_name)
            return val_dict
        c = {}

        for k,v in val_dict.items():
            if k not in comp_cls._descriptors: # property not in descriptors (for instance, removed from component now)
                c[k] = v
            else:    
                default = str(comp_cls._descriptors[k].default)
                sv = str(v)
                if default == sv or default.replace("StoreGateSvc+", "") == sv.replace("StoreGateSvc+", ""): 
                    logger.debug("Dropped default value %s of property %s in %s because the default is %s", sv, k, component_name, str(default))
                elif args.ignoreDefaultNamedComps and isinstance(v, str) and sv.endswith(f"/{default}"):
                    logger.debug("Dropped speculatively value %s of property %s in %s because the default it ends with %s", sv, k, component_name, str(default))
                else:
                    c[k] = v
                    logger.debug("Keep value %s of property %s in %s because it is different from default %s", str(v), str(k), component_name, str(comp_cls._descriptors[k].default))
        return c

     # collect types for all components (we look for A/B or lost of A/B strings)
    for (comp_name, comp_settings) in allconf.items():
        remaining = drop_defaults(comp_name, comp_settings)
        if len(remaining) != 0: # ignore components that have only default settings
            conf[comp_name] = remaining
    return conf

def shortenDefaultComponents(dic, args) -> Dict:
    conf = {}
    def shorten(val):
        value = val
        # the value can possibly be a serialized object (like a list)
        try:
            value = ast.literal_eval(str(value))
        except Exception:
            pass

        if isinstance(value, str):
            svalue = value.split("/")
            if len(svalue) == 2 and svalue[0] == svalue[1]:
                logger.debug("Shortened %s", svalue)
                return svalue[0]
        if isinstance(value, list):
            return [shorten(el) for el in value]
        if isinstance(value, dict):
            return shorten_defaults(value)

        return value

    def shorten_defaults(val_dict):
        if isinstance(val_dict, dict):
            return { key: shorten(val) for key,val in val_dict.items() }

    for (key, value) in dic.items():
        conf[key] = shorten_defaults(value)
    return conf

def isReference(value, compname, conf, svcCache={}) -> list:
    """Returns a list of (component,class) if value stores reference to other components
       value - the value to check
       compname - full component name
       conf - complete config dict
    """

    def _getSvcClass(instance):
        """Find instance in service lists to get class.
        Keeps a cache of the service classes in the svcCache default value.
        That's fine, unless we are dealing with more than one conf in the program.
        In that case, initialise svcCache to {} and specify in the caller."""
        if not svcCache:   # only scan ApplicationMgr once
            props = conf.get('ApplicationMgr',{"":None})
            if isinstance(props,dict):
                for prop,val in props.items():
                    if 'Svc' in prop:
                        try:
                            val = ast.literal_eval(str(val))
                        except Exception:
                            pass
                        if isinstance(val,list):
                            for v in val:
                                if isinstance(v,str):
                                    vv = v.split('/')
                                    if len(vv) == 2:
                                        if svcCache.setdefault(vv[1], vv[0]) != vv[0]:
                                            svcCache[vv[1]] = None # fail if same instance, different class
        return svcCache.get(instance)

    try:
        value = ast.literal_eval(str(value))
    except Exception:
        pass

    if isinstance(value, str):
        ctype_name = value.split('/')
        cls = ctype_name[0] if len(ctype_name) == 2 else None
        instance = ctype_name[-1]
        ref = None
        if instance:
            if compname and f"{compname}.{instance}" in conf: # private tool
                ref = f"{compname}.{instance}"
            elif f"ToolSvc.{instance}" in conf: # public tool
                ref = f"ToolSvc.{instance}"
            elif cls is not None or instance in conf: # service or other component
                ref = instance
                if cls is None:
                    cls = _getSvcClass(instance)
        if ref is not None:
                return [(ref, cls)]

    elif isinstance(value, list):
        refs = [isReference(el, compname, conf) for el in value]
        if any(refs):
            flattened = []
            [flattened.extend(el) for el in refs if el]
            return flattened
    return []


def skipProperties(conf, args) -> Dict:
    updated = {}
    for (name, properties) in conf.items():
        updated[name] = {}
        if not isinstance(properties, dict): # keep it
            updated[name] = properties
        else:
            for property_name, value in properties.items():
                if isReference( value, name, conf) or property_name == 'Members': # later for sequences structure
                    updated[name][property_name] = value
    return updated

def loadConfigFile(fname, args) -> Dict:
    """loads config file into a dictionary, supports several modifications of the input switched on via additional arguments
    Supports reading: Pickled file with the CA or properties & JSON
    """
    if args.debug:
        print("Debugging info from reading ", fname, " in ", logger.handlers[0].baseFilename)
        logger.setLevel(logging.DEBUG)

    conf = {}
    if fname.endswith(".pkl"):
        with open(fname, "rb") as input_file:
            # determine if there is a old or new configuration pickled
            cfg = pickle.load(input_file)
            logger.info("... Read %s from %s", cfg.__class__.__name__, fname)
            from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
            if isinstance(cfg, ComponentAccumulator):  # new configuration
                props = cfg.gatherProps()
                # to make json compatible with old configuration
                jos_props = props[2]
                to_json = {}
                for comp, name, value in jos_props:
                    to_json.setdefault(comp, {})[name] = value
                    to_json[comp][name] = value
                conf.update(to_json)
                conf.update(props[0])
                conf.update(props[1])

            elif isinstance(
                cfg, (collections.defaultdict, dict)
            ):  # old configuration
                conf.update(cfg)
                conf.update(pickle.load(input_file)) # special services
                # FIXME: there's a third pickle object with python components
            elif isinstance(cfg, (collections.Sequence)):
                for c in cfg:
                    conf.update(c)
        logger.info("... Read %d items from python pickle file: %s", len(conf), fname)

    elif fname.endswith(".json"):

        def __keepPlainStrings(element):
            if isinstance(element, str):
                return str(element)
            if isinstance(element, list):
                return [__keepPlainStrings(x) for x in element]
            if isinstance(element, dict):
                return {
                    __keepPlainStrings(key): __keepPlainStrings(value)
                    for key, value in element.items()
                }
            return element

        with open(fname, "r") as input_file:
            cfg = json.load(input_file, object_hook=__keepPlainStrings)
            for c in cfg:
                conf.update(cfg)

        # For compatibility with HLTJobOptions json, which produces structure:
        # {
        #   "filetype": "joboptions",
        #   "properties": { the thing we are interested in}
        # }
        if 'properties' in conf:
            conf = conf['properties']

        logger.info("... Read %d items from json file: %s", len(conf), fname)

    else:
        sys.exit("File format not supported.")

    if conf is None:
        sys.exit("Unable to load %s file" % fname)

    known_types = collect_types(conf)

    if args.includeComps or args.excludeComps:
        conf = excludeIncludeComps(conf, args, depth=args.follow)

    if args.ignoreIrrelevant:
        conf = ignoreIrrelevant(conf, args)

    if args.renameComps or args.renameCompsFile:
        conf = renameComps(conf, args)

    if args.ignoreDefaults:
        conf = ignoreDefaults(conf, args, known_types)

    if args.shortenDefaultComponents:
        conf = shortenDefaultComponents(conf, args)

    if args.skipProperties:
        conf = skipProperties(conf, args)
    return conf

class ComponentsFileLoader:
    def __init__(self, file_path: str, args, checked_elements=set()) -> None:
        self.file_path: str = file_path
        self.checked_elements: Set[str] = checked_elements
        self.args = args

    def _load_file_data(self) -> Dict:
        logger.info(f"Loading {self.file_path}")
        return loadConfigFile(self.file_path, self.args)


    def load_structure(self) -> ComponentsStructure:
        data = self._load_file_data()
        structure = ComponentsStructure(data, self.checked_elements)
        structure.generate()
        return structure

    def get_data(self) -> ComponentsStructure:
        return self.load_structure()


class ComponentsDiffFileLoader:
    def __init__(
        self,
        file_path: str,
        diff_file_path: str,
        checked_elements: Set[str],
    ) -> None:
        self.main_loader: ComponentsFileLoader = ComponentsFileLoader(
            file_path, checked_elements
        )
        self.diff_loader: ComponentsFileLoader = ComponentsFileLoader(
            diff_file_path, checked_elements
        )

    def get_data(self) -> Tuple[ComponentsStructure, ComponentsStructure]:
        structure = self.main_loader.load_structure()
        diff_structure = self.diff_loader.load_structure()
        self.mark_differences(structure.get_list(), diff_structure.get_list())
        return structure, diff_structure

    def equal(self, first: Element, second: Element) -> bool:
        return (
            first.get_name() == second.get_name()
            and first.x_pos == second.x_pos
            and type(first) == type(second)
        )

    def mark_differences(
        self, structure: List[Element], diff_structure: List[Element]
    ) -> None:
        i, j = 0, 0
        while i < len(structure) and j < len(diff_structure):
            if self.equal(structure[i], diff_structure[j]):
                if isinstance(structure[i], GroupingElement):
                    self.mark_differences(
                        cast(GroupingElement, structure[i]).children,
                        cast(GroupingElement, diff_structure[j]).children,
                    )
                i += 1
                j += 1
                continue

            # Find equal element in diff structure
            for tmp_j in range(j, len(diff_structure)):
                if self.equal(structure[i], diff_structure[tmp_j]):
                    for marking_j in range(j, tmp_j):
                        diff_structure[marking_j].mark()
                    j = tmp_j
                    break
            else:
                # Not found equal element in diff structure
                # Find equal element in first structure
                for tmp_i in range(i, len(structure)):
                    if self.equal(structure[tmp_i], diff_structure[j]):
                        for marking_i in range(i, tmp_i):
                            structure[marking_i].mark()
                        i = tmp_i
                        break
                else:
                    structure[i].mark()
                    diff_structure[j].mark()
                    i += 1
                    j += 1

        # Mark remaining elements in both structures
        while i < len(structure):
            structure[i].mark()
            i += 1

        while j < len(diff_structure):
            diff_structure[j].mark()
            j += 1


def loadDifferencesFile(fname) -> Dict:
    """
    Read differences file
    Format:
    full_component_name.property oldvalue=newvalue
    example:
    AlgX.ToolA.SubToolB.SettingZ 45=46
    It is possible to specify missing values, e.g:
    AlgX.ToolA.SubToolB.SettingZ 45=    means that now the old value should be ignored
    AlgX.ToolA.SubToolB.SettingZ =46    means that now the new value should be ignored
    AlgX.ToolA.SubToolB.SettingZ =      means that any change of the value should be ignored 

    """
    from collections import defaultdict
    differences = defaultdict(dict)
    count=0
    with open(fname, "r") as f:
        for line in f:
            if line[0] == "#" or line == "\n":
                continue
            line = line.strip()
            compAndProp, values = line.split(" ")           
            comp, prop = compAndProp.rsplit(".", 1)
            o,n = values.split("=")
            oldval,newval = o if o else None, n if n else None
                
            differences[comp][prop] = (oldval,newval)
            count+=1
    logger.info("... Read %d known differences from file: %s", count, fname)
    logger.info("..... %s", str(differences))

    return differences

