# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from copy import copy, deepcopy
from difflib import get_close_matches
import importlib
from AthenaCommon.Logging import logging
from PyUtils.moduleExists import moduleExists

_msg = logging.getLogger('AthConfigFlags')

def isGaudiEnv():
    """Return whether or not this is a gaudi-based (athena) environment"""

    return moduleExists('Gaudi')

class CfgFlag(object):
    """The base flag object.

    A flag can be set to either a fixed value or a callable, which computes
    the value based on other flags.
    """

    __slots__ = ['_value', '_setDef', '_enum', '_help']

    def __init__(self, default, enum=None, help=None):
        """Initialise the flag with the default value.

        Optionally set an enum of allowed values.
        """
        if default is None:
            raise RuntimeError("Default value of a flag must not be None")
        self._enum = enum
        self._help = help
        self.set(default)
        return

    def set(self, value):
        """Set the value of the flag.

        Can be a constant value or a callable.
        """
        if callable(value):
            self._value=None
            self._setDef=value
        else:
            self._value=value
            self._setDef=None

            if not self._validateEnum(self._value):
                raise RuntimeError("Flag is of type '{}', but '{}' set.".format( self._enum, type(self._value) ))
        return

    def get(self, flagdict=None):
        """Get the value of the flag.

        If the currently set value is a callable, a dictionary of all available
        flags needs to be provided.
        """

        if self._value is not None:
            return deepcopy(self._value)

        # For cases where the value is intended to be None
        # i.e. _setDef applied this value, we should not progress
        if self._setDef is None:
            return None

        if not flagdict:
            raise RuntimeError("Flag is using a callable but all flags are not available.")

        #Have to call the method to obtain the default value, and then reuse it in all next accesses
        if flagdict.locked():
            # optimise future reads, drop possibility to update this flag ever
            self._value=self._setDef(flagdict)
            self._setDef=None
            if not self._validateEnum(self._value):
                raise RuntimeError("Flag is of type '{}', but '{}' set.".format( self._enum, type(self._value) ))
            return deepcopy(self._value)
        else:
            #use function for as long as the flags are not locked
            val=self._setDef(flagdict)
            if not self._validateEnum(val):
                raise RuntimeError("Flag is of type '{}', but '{}' set.".format( self._enum, type(val)))
            return deepcopy(val)

    def __repr__(self):
        if self._value is not None:
            return repr(self._value)
        else:
            return "[function]"

    def _validateEnum(self, value):
        if self._enum is None:
            return True

        if value is not None:
            try:
                return value in self._enum
            except TypeError:
                return False

class FlagAddress(object):
    def __init__(self, f, name):
        if isinstance(f, AthConfigFlags):
            self._flags = f
            self._name = name

        elif isinstance(f, FlagAddress):
            self._flags = f._flags
            self._name  = f._name+"."+name

    def __getattr__(self, name):
        return getattr(self._flags, self._name + "." + name)

    def __setattr__( self, name, value ):
        if name.startswith("_"):
            return object.__setattr__(self, name, value)
        merged = self._name + "." + name

        if not self._flags.hasFlag( merged ): # flag is misisng, try loading dynamic ones
            self._flags._loadDynaFlags( merged )

        if not self._flags.hasFlag( merged ):
            raise RuntimeError( "No such flag: {}  The name is likely incomplete.".format(merged) )
        return self._flags._set( merged, value )

    def __cmp__(self, other):
        raise RuntimeError( "No such flag: "+ self._name+".  The name is likely incomplete." )
    __eq__ = __cmp__
    __ne__ = __cmp__
    __lt__ = __cmp__
    __le__ = __cmp__
    __gt__ = __cmp__
    __ge__ = __cmp__

    def __bool__(self):
        raise RuntimeError( "No such flag: "+ self._name+".  The name is likely incomplete." )



class AthConfigFlags(object):

    def __init__(self):
        self._flagdict=dict()
        self._locked=False
        self._dynaflags = dict()
        self._loaded    = set() # dynamic dlags that were loaded
        self._hash = None
        self._parser = None
        self._args = None # user args from parser

    def athHash(self):
        if self._hash is None:
            raise RuntimeError("Cannot calculate hash of unlocked flag container")
        else:
            return self._hash

    def __hash__(self):
        raise DeprecationWarning("__hash__ method in AthConfigFlags is deprecated. Probably called from function decorator, use AccumulatorCache decorator instead.")

    def _calculateHash(self):
        return hash(str(self._flagdict.items()))

    def __getattr__(self, name):
        # First try to get an already loaded flag or category
        if self.hasFlag(name):
            return self._get(name)

        if self.hasCategory(name):
            return FlagAddress(self, name)

        # Reaching here means that we may need to load a dynamic flag
        self._loadDynaFlags(name)

        # Try again
        if self.hasFlag(name):
            return self._get(name)

        if self.hasCategory(name):
            return FlagAddress(self, name)

        # Reaching here means that it truly isn't something we know about
        raise AttributeError(f"No such flag: {name}")

    def __setattr__(self, name, value):
        if name.startswith("_"):
            return object.__setattr__(self, name, value)

        if name in self._flagdict:
            return self._set(name, value)
        raise RuntimeError( "No such flag: "+ name+". The name is likely incomplete." )


    def addFlag(self, name, setDef, enum=None, help=None):
        self._tryModify()
        if name in self._flagdict:
            raise KeyError("Duplicated flag name: {}".format( name ))
        self._flagdict[name]=CfgFlag(setDef, enum, help)
        return

    def addFlagsCategory(self, path, generator, prefix=False):
        """
        The path is the beginning of the flag name (e.g. "X" for flags generated with name "X.*").
        The generator is a function that returns a flags container, the flags have to start with the same path.
        When the prefix is True the flags created by the generator are prefixed by "path".

        Supported calls are then:
         addFlagsCategory("A", g) - where g is function creating flags  is f.addFlag("A.x", someValue)
         addFlagsCategory("A", g, True) - when flags are defined in g like this: f.addFalg("x", somevalue),
        The latter option allows to share one generator among flags that are later loaded in different paths.
        """
        self._tryModify()
        _msg.debug("Adding flag category %s", path)
        self._dynaflags[path] = (generator, prefix)

    def needFlagsCategory(self, name):
        """ public interface for _loadDynaFlags """
        self._loadDynaFlags( name )

    def _loadDynaFlags(self, name):
        """
        loads the flags of the form "A.B.C" first attempting the path "A" then "A.B" and then "A.B.C"
        """

        def __load_impl( flagBaseName ):
            if flagBaseName in self._loaded:
                _msg.debug("Flags %s already loaded",flagBaseName  )
                return
            if flagBaseName in self._dynaflags:
                _msg.debug("Dynamically loading the flags under %s", flagBaseName )
                # Retain locked status and hash
                isLocked = self._locked
                myHash = self._hash
                self._locked = False
                generator, prefix = self._dynaflags[flagBaseName]
                self.join( generator(), flagBaseName if prefix else "" )
                self._locked = isLocked
                self._hash = myHash
                del self._dynaflags[flagBaseName]
                self._loaded.add(flagBaseName)

        pathfrags = name.split('.')
        for maxf in range(1, len(pathfrags)+1):
            __load_impl( '.'.join(pathfrags[:maxf]) )

    def loadAllDynamicFlags(self):
        """Force load all the dynamic flags """
        while len(self._dynaflags) != 0:
            # Need to convert to a list since _loadDynaFlags may change the dict.
            for prefix in list(self._dynaflags.keys()):
                self._loadDynaFlags( prefix )

    def hasCategory(self, name):
        path = name+'.'
        for f in self._flagdict.keys():
            if f.startswith(path):
                return True
        for c in self._dynaflags.keys():
            if c.startswith(name):
                return True
        return False

    def hasFlag(self, name):
        return name in self._flagdict

    def _set(self,name,value):
        self._tryModify()
        if name in self._flagdict:
            self._flagdict[name].set(value)
            return
        errString="No flag with name \'{}\' found".format( name )
        closestMatch=get_close_matches(name,self._flagdict.keys(),1)
        if len(closestMatch)>0:
            errString+=". Did you mean \'{}\'?".format(  closestMatch[0] )
        raise KeyError(errString)

    def _get(self,name):
        if name in self._flagdict:
            return self._flagdict[name].get(self)

        errString="No flag with name \'{}\' found".format( name )
        closestMatch=get_close_matches(name,self._flagdict.keys(),1)
        if len(closestMatch)>0:
            errString+=". Did you mean \'{}\'?".format( closestMatch[0] )
        raise KeyError(errString)

    def __call__(self,name):
        return self._get(name)

    def lock(self):
        if not self._locked:
            # before locking, parse args if a parser was defined
            if self._args is None and self._parser is not None: self.fillFromArgs()
            self._locked = True
            self._hash = self._calculateHash()
        return

    def locked(self):
        return self._locked

    def _tryModify(self):
        if self._locked:
            raise RuntimeError("Attempt to modify locked flag container")
        else:
            # if unlocked then invalidate hash
            self._hash = None

    def clone(self):
        """Return an unlocked copy of self (dynamic flags are not loaded)"""
        cln = AthConfigFlags()
        cln._flagdict = deepcopy(self._flagdict)
        cln._dynaflags = copy(self._dynaflags)
        return cln


    def cloneAndReplace(self,subsetToReplace,replacementSubset):
        """
        This is to replace subsets of configuration flags like

        Example:
        newflags = flags.cloneAndReplace('Muon', 'Trigger.Offline.Muon')
        """

        def _copyFunction(obj):
            return obj if self.locked() else deepcopy(obj) # if flags are locked we can reuse containers, no need to deepcopy

        _msg.info("cloning flags and replacing %s by %s", subsetToReplace, replacementSubset)

        self._loadDynaFlags( subsetToReplace )
        self._loadDynaFlags( replacementSubset )

        if not subsetToReplace.endswith("."):
            subsetToReplace+="."
            pass
        if not replacementSubset.endswith("."):
            replacementSubset+="."
            pass

        #Sanity check: Don't replace a by a
        if (subsetToReplace == replacementSubset):
            raise RuntimeError("Can not replace flags {} with themselves".format(subsetToReplace))


        replacedNames=set()
        replacementNames=set()
        newFlagDict=dict()
        for (name,flag) in self._flagdict.items():
            if name.startswith(subsetToReplace):
                replacedNames.add(name[len(subsetToReplace):]) #Remember replaced flag for the check later
            elif name.startswith(replacementSubset):
                subName=name[len(replacementSubset):]
                replacementNames.add(subName) # remember replacement name
                #Move the flag to the new name:

                newFlagDict[subsetToReplace+subName] = _copyFunction(flag)
                pass
            else:
                newFlagDict[name] = _copyFunction(flag) #All other flags are simply copied
                pass
            #End loop over flags
            pass

        #Last sanity check: Make sure that the replaced section still contains the same names:
        if not replacementNames.issuperset(replacedNames):
            _msg.error(replacedNames)
            _msg.error(replacementNames)
            raise RuntimeError("Attempt to replace incompatible flags subsets: distinct flag are "
                               + repr(replacementNames - replacedNames))
        newFlags = AthConfigFlags()
        newFlags._flagdict = newFlagDict

        for k,v in self._dynaflags.items(): # cant just assign the dicts because then they are shared when loading
            newFlags._dynaflags[k] = _copyFunction(v)
        newFlags._hash = None

        if self._locked:
            newFlags.lock()
        return newFlags



    def join(self, other, prefix=''):
        """
        Merges two flag containers
        When the prefix is passed each flag from the "other" is prefixed by "prefix."
        """
        self._tryModify()

        for (name,flag) in other._flagdict.items():
            fullName = prefix+"."+name if prefix != "" else name
            if fullName in self._flagdict:
                raise KeyError("Duplicated flag name: {}".format( fullName ) )
            self._flagdict[fullName]=flag

        for (name,loader) in other._dynaflags.items():
            fullName = prefix+"."+name if prefix != "" else name
            if fullName in self._dynaflags:
                raise KeyError("Duplicated dynamic flags name: {}".format( fullName ) )
            _msg.debug("Joining dynamic flags with %s", fullName)
            self._dynaflags[fullName] = loader
        return

    def dump(self, pattern=".*", evaluate=False, formatStr="{:40} : {}", maxLength=None):
        import re
        compiled = re.compile(pattern)
        print(formatStr.format( "Flag Name","Value" ) )
        def truncate(s): return s[:maxLength] + ("..." if maxLength and len(s)>maxLength else "")
        for name in sorted(self._flagdict):
            if compiled.match(name):
                if evaluate:
                    try:
                        rep = repr(self._flagdict[name] )
                        val = repr(self._flagdict[name].get(self))
                        if val != rep:
                            print(formatStr.format(name,truncate("{} {}".format( val, rep )) ))
                        else:
                            print(formatStr.format( name, truncate("{}".format(val)) ) )
                    except Exception as e:
                        print(formatStr.format(name, truncate("Exception: {}".format( e )) ))
                else:
                    print(formatStr.format( name, truncate("{}".format(repr(self._flagdict[name] ) )) ))

        if len(self._dynaflags) == 0:
            return
        print("Flag categories that can be loaded dynamically")
        print("{:25} : {:>30} : {}".format( "Category","Generator name", "Defined in" ) )
        for name,gen_and_prefix in sorted(self._dynaflags.items()):
            if compiled.match(name):
                print("{:25} : {:>30} : {}".format( name, gen_and_prefix[0].__name__, '/'.join(gen_and_prefix[0].__code__.co_filename.split('/')[-2:]) ) )


    def initAll(self):
        """
        Mostly a self-test method
        """
        for n,f in list(self._flagdict.items()):
            f.get(self)
        return


    def getArgumentParser(self, **kwargs):
        """
        Scripts calling AthConfigFlags.fillFromArgs can extend this parser, and pass their version to fillFromArgs
        """
        import argparse
        parser= argparse.ArgumentParser(formatter_class = argparse.ArgumentDefaultsHelpFormatter, **kwargs )
        parser.add_argument("-d","--debug", default=None, help="attach debugger (gdb) before run, <stage>: init, exec, fini")
        parser.add_argument("-i","--interactive", default=None, help="Drop into interactive mode before <stage>: init or run")
        parser.add_argument("--evtMax", type=int, default=None, help="Max number of events to process")
        parser.add_argument("--skipEvents", type=int, default=None, help="Number of events to skip")
        parser.add_argument("--filesInput", default=None,nargs='+', help="Input file(s), supports * wildcard")
        parser.add_argument("-l", "--loglevel", default=None, help="logging level (ALL, VERBOSE, DEBUG,INFO, WARNING, ERROR, or FATAL")
        parser.add_argument("--config-only", type=str, default=None, help="Stop after configuration phase (may not be respected by all diver scripts)")
        parser.add_argument("--threads", type=int, default=None, help="Run with given number of threads (use 0 for serial execution)")
        parser.add_argument('--concurrent-events', type=int, default=None, help='number of concurrent events for AthenaMT')
        parser.add_argument("--nprocs", type=int, default=None, help="Run AthenaMP with given number of worker processes")
        parser.add_argument("---",dest="terminator",action='store_true', help=argparse.SUPPRESS) # special hidden option required to convert option terminator -- for --help calls
        parser.add_argument("--pmon", type=str, default=None, choices=['FastMonMT','FullMonMT'], help="Perfomance monitoring")

        return parser

    def parser(self):
        if self._parser is None: self._parser = self.getArgumentParser()
        return self._parser

    def args(self):
        return self._args


    def fillFromString(self, flag_string):
        """Fill the flags from a string of type key=value"""

        try:
            key, value = flag_string.split("=")
        except ValueError:
            raise ValueError(f"Cannot interpret argument {flag_string}, expected a key=value format")

        key = key.strip()
        value = value.strip()

        # also allow key+=value to append
        oper = "="
        if (key[-1]=="+"):
            oper = "+="
            key = key[:-1]

        if not self.hasFlag(key):
            self._loadDynaFlags( '.'.join(key.split('.')[:-1]) ) # for a flag A.B.C dymanic flags from category A.B
        if not self.hasFlag(key):
            raise KeyError(f"{key} is not a known configuration flag")

        enum = self._flagdict[key]._enum
        # Regular flag
        if enum is None:
            try:
                exec(f"type({value})")
            except (NameError, SyntaxError): #Can't determine type, assume we got an un-quoted string
                value=f"\"{value}\""
        # FlagEnum
        else:
            # import the module containing the FlagEnum class
            ENUM = importlib.import_module(enum.__module__)  # noqa: F841 (used in exec)
            value=f"ENUM.{value}"

        # Set the value
        exec(f"self.{key}{oper}{value}")


    # parser argument must be an ArgumentParser returned from getArgumentParser()
    def fillFromArgs(self, listOfArgs=None, parser=None):
        """
        Used to set flags from command-line parameters, like ConfigFlags.fillFromArgs(sys.argv[1:])
        """
        import sys

        self._tryModify()

        if parser is None:
            parser = self.parser()
        self._parser = parser # set our parser to given one
        argList = listOfArgs or sys.argv[1:]
        do_help = False
        # We will now do a pre-parse of the command line arguments to propagate these to the flags
        # the reason for this is so that we can use the help messaging to display the values of all
        # flags as they would be *after* any parsing takes place. This is nice to see e.g. the value
        # that any derived flag (functional flag) will take after, say, the filesInput are set
        import argparse
        unrequiredActions = []
        if "-h" in argList or "--help" in argList:
            do_help = True
            if "-h" in argList: argList.remove("-h")
            if "--help" in argList: argList.remove("--help")
            # need to unrequire any required arguments in order to do a "pre parse"
            for a in parser._actions:
                if a.required:
                    unrequiredActions.append(a)
                    a.required = False
        (args,leftover)=parser.parse_known_args(argList)
        for a in unrequiredActions: a.required=True

        # remove the leftovers from the argList ... for later use in the do_help
        argList = [a for a in argList if a not in leftover]

        #First, handle athena.py-like arguments:

        if args.debug is not None:
            from AthenaCommon.Debugging import DbgStage
            if args.debug not in DbgStage.allowed_values:
                raise ValueError("Unknown debug stage, allowed values {}".format(DbgStage.allowed_values))
            self.Exec.DebugStage=args.debug

        if args.evtMax is not None:
            self.Exec.MaxEvents=args.evtMax

        if args.interactive is not None:
            if args.interactive not in ("init","run"):
                raise ValueError("Unknown value for interactive, allowed values are 'init' and 'run'")
            self.Exec.Interactive=args.interactive

        if args.skipEvents is not None:
            self.Exec.SkipEvents=args.skipEvents

        if args.filesInput is not None:
            self.Input.Files = [] # remove generic
            for f in args.filesInput:
                for ffile in f.split(","):
                    if '*' in ffile: # handle wildcard
                        import glob
                        self.Input.Files += glob.glob(ffile)
                    else:
                        self.Input.Files += [ffile]

        if args.loglevel is not None:
            from AthenaCommon import Constants
            if hasattr(Constants,args.loglevel):
                self.Exec.OutputLevel=getattr(Constants,args.loglevel)
            else:
                raise ValueError("Unknown log-level, allowed values are ALL, VERBOSE, DEBUG,INFO, WARNING, ERROR, FATAL")

        if args.config_only is not None:
            from os import environ
            environ["PICKLECAFILE"] = args.config_only

        if args.threads is not None:
            self.Concurrency.NumThreads = args.threads

        if args.concurrent_events is not None:
            self.Concurrency.NumConcurrentEvents = args.concurrent_events

        if args.nprocs is not None:
            self.Concurrency.NumProcs = args.nprocs

        if args.pmon is not None:
            self._loadDynaFlags("PerfMon")
            self._set("PerfMon.do"+args.pmon,True)

        #All remaining arguments are assumed to be key=value pairs to set arbitrary flags:
        for arg in leftover:
            if arg=='--':
                argList += ["---"]
                continue # allows for multi-value arguments to be terminated by a " -- "
            if do_help and '=' not in arg:
                argList += arg.split(".") # put arg back back for help (but split by sub-categories)
                continue

            self.fillFromString(arg)

        if do_help:
            if parser.epilog is None: parser.epilog=""
            parser.epilog += "  Note: Specify additional flags in form <flagName>=<value>."
            subparsers = {"":[parser,parser.add_subparsers(help=argparse.SUPPRESS)]} # first is category's parser, second is subparsers (effectively the category's subcategories)
            # silence logging while evaluating flags
            logging.root.setLevel(logging.ERROR)
            def getParser(category): # get parser for a given category
                if category not in subparsers.keys():
                    cat1,cat2 = category.rsplit(".",1) if "." in category else ("",category)
                    p,subp = getParser(cat1)
                    if subp.help==argparse.SUPPRESS:
                        subp.help = "Flag subcategories:"
                    newp = subp.add_parser(cat2,help="{} flags".format(category),
                                           formatter_class = argparse.ArgumentDefaultsHelpFormatter,usage=argparse.SUPPRESS)
                    newp._positionals.title = "flags"
                    subparsers[category] = [newp,newp.add_subparsers(help=argparse.SUPPRESS)]
                return subparsers[category]
            self.loadAllDynamicFlags()
            for name in sorted(self._flagdict):
                category,flagName = name.rsplit(".",1) if "." in name else ("",name)
                try:
                    val = repr(self._flagdict[name].get(self))
                except Exception:
                    val = None
                if self._flagdict[name]._help != argparse.SUPPRESS:
                    getParser(category)[0].add_argument(name,nargs='?',default=val,help=": " + (self._flagdict[name]._help if self._flagdict[name]._help is not None else ""))

            parser._positionals.title = 'flags and positional arguments'
            parser.parse_known_args(argList + ["--help"])

        self._args = args

        return args



