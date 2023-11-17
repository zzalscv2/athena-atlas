# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

## @package PyJobTransforms.trfJobOptions
#
# @brief Contains functions related Athena Job Options files 
# @details Generates runArgs JobOptions and interfaces with skeleton
# @author atlas-comp-transforms-dev@cern.ch
#

import os
import time

import logging
msg = logging.getLogger(__name__)

import PyJobTransforms.trfArgClasses as trfArgClasses
import PyJobTransforms.trfExceptions as trfExceptions
from PyJobTransforms.trfExeStepTools import getExecutorStepEventCounts
from PyJobTransforms.trfExitCodes import trfExit

from PyJobTransforms.trfUtils import findFile


## @class RunArguments
## @brief Hold run arguments as name-value pairs
class RunArguments(object):
    """Dynamic class that holds the run arguments as named members with values."""
    def __str__(self):
        myself = 'RunArguments:'
        for arg in dir(self):
            if not arg.startswith('__'):
                myself += '%s   %s = %s' % (os.linesep, arg, repr(getattr(self, arg)))
        return myself


## @class JobOptionsTemplate
#  @brief Class that generates the job options (AKA runargs) python file for an athena executor
class JobOptionsTemplate(object):
    """For generating runArgs JobOptions file """

    ## @brief Initialise the job options template class
    #  @param exe Associated athena executor
    #  @param version Optional version string    
    #  @param runArgsName Name of runtime argument class
    #  @note Almost all useful parameters for this class are part of the executor itself
    def __init__(self, exe, version=None, runArgsName='runArgs'):
        self._exe = exe
        self._version = version
        self._runArgsName = runArgsName
        self._runArgsFile = 'runargs.' + self._exe.name + '.py'


    ## @brief Write the runArgs Job Options file
    def writeRunArgs(self, input = dict(), output = dict()):
        msg.info('Writing runArgs to file \"%s\"', self._runArgsFile)

        ## Check consistency btw --CA flag and provided skeletons:
        if self._exe._isCAEnabled():
            if self._exe._skeletonCA is None:
                errMsg = "Got the --CA option but this transform doesn't supply a ComponentAccumulator-based skeleton file"
                msg.error(errMsg)
                raise  trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_EXEC_RUNARGS_ERROR'),errMsg)
        else: # not self._exe._isCAEnabled():
            if self._exe._skeleton is None:
                errMsg = "No --CA option given, but this transform doesn't supply old-style skeleton file"
                msg.error(errMsg)
                raise  trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_EXEC_RUNARGS_ERROR'),errMsg)
        
        with open(self._runArgsFile, 'w') as runargsFile:
            try:
                if self._exe._isCAEnabled():
                    # Write a shebang to identify CA files.
                    print("#!/usr/bin/env athena.py --CA", file=runargsFile)
                # First write a little header
                print(os.linesep.join(("# Run arguments file auto-generated on {0} by:".format(time.asctime()),
                                                     "# JobTransform: {0}".format(self._exe.name),
                                                     "# Version: {0}".format(self._version)
                                                     )), file=runargsFile)
                
                # Now make sure we import the runArgs class for out job options
                print(os.linesep.join(("# Import runArgs class",
                                                      "from PyJobTransforms.trfJobOptions import RunArguments",
                                                      "{0} = RunArguments()".format(self._runArgsName)
                                                      )), file=runargsFile)
                
                # Handy to write the substep name here as it can be used as (part of) a random seed
                # in some cases
                print('{0}.trfSubstepName = {1!r}'.format(self._runArgsName, self._exe.name), os.linesep, file=runargsFile)
    
                # Now loop over the core argdict and see what needs to be given as a runArg
                declaredRunargs = []
                for k, v in self._exe.conf.argdict.items():
                    # Check if this arg is supposed to be in runArgs
                    if isinstance(v, trfArgClasses.argument) and v.isRunarg:
                        # Files handled later
                        if isinstance(v, trfArgClasses.argFile):
                            continue
                        
                        msg.debug('Argument {0} is a runarg, will be added to JO file (value {1})'.format(k, v.value))
                        
                        ## @note Substep type arguments are rather special, they apply to only named
                        #  executors or substeps. We use the returnMyValue() method to sort out what
                        #  specific value applies to us
                        if isinstance(v, trfArgClasses.argSubstep):
                            myValue = v.returnMyValue(exe = self._exe)
                            if myValue is not None:
                                print("{0}.{1!s} = {2!r}".format(self._runArgsName, k, myValue), file=runargsFile)
                                msg.debug('Added substep type argument {0} as: {1}'.format(k, myValue))
                                declaredRunargs.append(k)
                        else:
                            print("{0}.{1!s} = {2!r}".format(self._runArgsName, k, v.value), file=runargsFile)
                            declaredRunargs.append(k)
                    else:
                        msg.debug('Argument {0} is not a runarg - ignored'.format(k))
                
                # Now make sure that if we did not add maxEvents  then we set this to -1, which
                # avoids some strange defaults that only allow 5 events to be processed
                if 'maxEvents' not in declaredRunargs:
                    print(os.linesep.join(("", "# Explicitly added to process all events in this step",
                                                          "{0}.maxEvents = -1".format(self._runArgsName),
                                                          )), file=runargsFile)
            
                # Now deal with our input and output files
                print(os.linesep, "# Input data", file=runargsFile)
                for dataType, dataArg in input.items():
                    if isinstance(dataArg, list) and dataArg:
                        dataArgStep = dataArg[self._exe.conf.executorStep]
                        print('{0}.input{1}File = {2!r}'.format(self._runArgsName, dataType, dataArgStep.value), file=runargsFile)
                        print('{0}.input{1}FileType = {2!r}'.format(self._runArgsName, dataType, dataArgStep.type), file=runargsFile)
                        # Add the input event count, if we know it
                        if dataArgStep.isCached(metadataKeys = ['nentries']):
                            print('{0}.input{1}FileNentries = {2!r}'.format(self._runArgsName, dataType, dataArgStep.nentries), file=runargsFile)
                        print("{0}.{1}FileIO = {2!r}".format(self._runArgsName, dataType, dataArgStep.io), file=runargsFile)
                    else:
                        print('{0}.input{1}File = {2!r}'.format(self._runArgsName, dataType, dataArg.value), file=runargsFile)
                        print('{0}.input{1}FileType = {2!r}'.format(self._runArgsName, dataType, dataArg.type), file=runargsFile)
                        # Add the input event count, if we know it
                        if dataArg.isCached(metadataKeys = ['nentries']):
                            print('{0}.input{1}FileNentries = {2!r}'.format(self._runArgsName, dataType, dataArg.nentries), file=runargsFile)
                        print("{0}.{1}FileIO = {2!r}".format(self._runArgsName, dataType, dataArg.io), file=runargsFile)
                
                print(os.linesep, "# Output data", file=runargsFile)
                for dataType, dataArg in output.items():
                    # Need to be careful to convert _output_ filename as a strings, not a list
                    print('{0}.output{1}File = {2!r}'.format(self._runArgsName, dataType, dataArg.value[0]), file=runargsFile)
                    print('{0}.output{1}FileType = {2!r}'.format(self._runArgsName, dataType, dataArg.type), file=runargsFile)
    
                    
                # Process all of the tweaky special runtime arguments
                print(os.linesep, "# Extra runargs", file=runargsFile)
                ## @note extraRunargs are passed using repr, i.e., they should be constants
                for k, v in self._exe._extraRunargs.items():
                    ## @note: What to do if this is a CLI argument as well, in particular
                    #  for arguments like preExec we want to add to the list, not replace it 
                    if k in declaredRunargs:
                        if isinstance(self._exe.conf.argdict[k].value, list):
                            msg.debug('Extending runarg {0!s}={1!r}'.format(k, v))
                            print('{0}.{1!s}.extend({2!r})'.format(self._runArgsName, k, v), file=runargsFile)
                    else:
                        msg.debug('Adding runarg {0!s}={1!r}'.format(k, v))
                        print('{0}.{1!s} = {2!r}'.format(self._runArgsName, k, v), file=runargsFile)
    
                ## @note runtime runargs are passed as strings, i.e., they can be evaluated
                print(os.linesep, '# Extra runtime runargs', file=runargsFile)
                for k, v in self._exe._runtimeRunargs.items():
                    # These options are string converted, not repred, so they can write an option
                    # which is evaluated at runtime
                    # Protect this with try: except: for the Embedding use case
                    msg.debug('Adding runarg {0!s}={1!r}'.format(k, v))
                    if self._exe._isCAEnabled():
                        print(os.linesep.join(('try:',
                                               '    {0}.{1!s} = {2!s}'.format(self._runArgsName, k, v),
                                               'except AttributeError:',
                                               '    print ("WARNING - AttributeError for {0}")'.format(k))), file=runargsFile)
                    else:
                        print(os.linesep.join(('try:',
                                               '    {0}.{1!s} = {2!s}'.format(self._runArgsName, k, v),
                                               'except AttributeError:',
                                               '    printfunc ("WARNING - AttributeError for {0}")'.format(k))), file=runargsFile)
                ## @note Now write the literals into the runargs file
                if self._exe._literalRunargs is not None:
                    print(os.linesep, '# Literal runargs snippets', file=runargsFile)
                    for line in self._exe._literalRunargs:
                        print(line, file=runargsFile)
                        
                ## Another special option - dataArgs are always written to the runargs file
                for dataType in self._exe._dataArgs:
                    print(os.linesep, '# Forced data value arguments', file=runargsFile)
                    if dataType in self._exe.conf.dataDictionary:
                        print('{0}.data{1}arg = {2!r}'.format(self._runArgsName, dataType, 
                                                                          self._exe.conf.dataDictionary[dataType].value), file=runargsFile)
                    else:
                        print('# Warning: data type "{0}" is not part of this transform'.format(dataType), file=runargsFile)
    
                # This adds the correct JO fragment for AthenaMP job, where we need to ask
                # the FileMgr to produce the requested log and report files
                # Also, aggregating the workers' logfiles into the mother's makes life
                # easier for debugging
                if self._exe._athenaMP:
                    print(os.linesep, '# AthenaMP Options. nprocs = %d' % self._exe._athenaMP, file=runargsFile)
                    # Proxy for both options
                    if self._exe._isCAEnabled():
                        # do not edit flags directly in this case
                        print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'athenaMPWorkerTopDir', self._exe._athenaMPWorkerTopDir), file=runargsFile)
                        print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'athenaMPOutputReportFile', self._exe._athenaMPFileReport), file=runargsFile)
                        print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'athenaMPEventOrdersFile', self._exe._athenaMPEventOrdersFile), file=runargsFile)
                        print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'athenaMPCollectSubprocessLogs', True), file=runargsFile)
                    else:
                        print(os.linesep.join((os.linesep,
                                               'from AthenaMP.AthenaMPFlags import jobproperties as AthenaMPJobProps',
                                               'AthenaMPJobProps.AthenaMPFlags.WorkerTopDir="{0}"'.format(self._exe._athenaMPWorkerTopDir),
                                               'AthenaMPJobProps.AthenaMPFlags.OutputReportFile="{0}"'.format(self._exe._athenaMPFileReport),
                                               'AthenaMPJobProps.AthenaMPFlags.EventOrdersFile="{0}"'.format(self._exe._athenaMPEventOrdersFile),
                                               'AthenaMPJobProps.AthenaMPFlags.CollectSubprocessLogs=True'
                                              )), file=runargsFile)
                    if self._exe._athenaMPStrategy:
                        if self._exe._isCAEnabled():
                            print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'athenaMPStrategy', self._exe._athenaMPStrategy), file=runargsFile)
                        else:
                            # Beware of clobbering a non default value (a feature used by EventService)
                            print('if AthenaMPJobProps.AthenaMPFlags.Strategy.isDefault():', file=runargsFile)
                            print('\tAthenaMPJobProps.AthenaMPFlags.Strategy="{0}"'.format(self._exe._athenaMPStrategy), file=runargsFile)
                    if self._exe._athenaMPReadEventOrders:
                        if os.path.isfile(self._exe._athenaMPEventOrdersFile):
                            if self._exe._isCAEnabled():
                                print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'athenaMPReadEventOrders', True), file=runargsFile)
                            else:
                                print('AthenaMPJobProps.AthenaMPFlags.ReadEventOrders=True', file=runargsFile)
                        else:
                            raise trfExceptions.TransformExecutionException(trfExit.nameToCode("TRF_EXEC_RUNARGS_ERROR"), "Failed to find file: {0} required by athenaMP option: --athenaMPUseEventOrders true".format(self._exe._athenaMPEventOrdersFile))
                    if 'athenaMPEventsBeforeFork' in self._exe.conf.argdict:
                        if self._exe._isCAEnabled():
                            print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'athenaMPEventsBeforeFork', self._exe.conf.argdict['athenaMPEventsBeforeFork'].value), file=runargsFile)
                        else:
                            print('AthenaMPJobProps.AthenaMPFlags.EventsBeforeFork={0}'.format(self._exe.conf.argdict['athenaMPEventsBeforeFork'].value), file=runargsFile)
                    if 'sharedWriter' in self._exe.conf.argdict:
                        if self._exe._isCAEnabled():
                            print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'sharedWriter', self._exe.conf.argdict['sharedWriter'].value), file=runargsFile)
                        else:
                            print(f"AthenaMPJobProps.AthenaMPFlags.UseSharedWriter={self._exe.conf.argdict['sharedWriter'].value}", file=runargsFile)
                    if 'parallelCompression' in self._exe.conf.argdict:
                        if self._exe._isCAEnabled():
                            print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'parallelCompression', self._exe.conf.argdict['parallelCompression'].value), file=runargsFile)
                        else:
                            print(f"AthenaMPJobProps.AthenaMPFlags.UseParallelCompression={self._exe.conf.argdict['parallelCompression'].value}", file=runargsFile)

                # Executor substeps
                print(os.linesep, '# Executor flags', file=runargsFile)
                msg.debug('Adding runarg {0!s}={1!r}'.format('totalExecutorSteps', self._exe.conf.totalExecutorSteps))
                print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'totalExecutorSteps', self._exe.conf.totalExecutorSteps), file=runargsFile)
                if self._exe.conf.executorStep >= 0:
                    msg.debug('Adding runarg {0!s}={1!r}'.format('executorStep', self._exe.conf.executorStep))
                    print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'executorStep', self._exe.conf.executorStep), file=runargsFile)
                    executorEventCounts, executorEventSkips = getExecutorStepEventCounts(self._exe)
                    msg.debug('Adding runarg {0!s}={1!r}'.format('executorEventCounts', executorEventCounts))
                    print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'executorEventCounts', executorEventCounts), file=runargsFile)
                    msg.debug('Adding runarg {0!s}={1!r}'.format('executorEventSkips', executorEventSkips))
                    print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'executorEventSkips', executorEventSkips), file=runargsFile)

                # CA
                if self._exe._isCAEnabled():
                    print(os.linesep, '# Threading flags', file=runargsFile)
                    # Pass the number of threads and processes
                    nprocs = self._exe._athenaMP
                    threads = self._exe._athenaMT
                    concurrentEvents = self._exe._athenaConcurrentEvents
                    msg.debug('Adding runarg {0!s}={1!r}'.format('nprocs', nprocs))
                    print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'nprocs', nprocs), file=runargsFile)
                    msg.debug('Adding runarg {0!s}={1!r}'.format('threads', threads))
                    print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'threads', threads), file=runargsFile)
                    msg.debug('Adding runarg {0!s}={1!r}'.format('concurrentEvents', concurrentEvents))
                    print('{0}.{1!s} = {2!r}'.format(self._runArgsName, 'concurrentEvents', concurrentEvents), file=runargsFile)
                    #ComponentAccumulator based config, import skeleton here:
                    print(os.linesep, '# Import skeleton and execute it', file=runargsFile)
                    print('from {0} import fromRunArgs'.format(self._exe._skeletonCA),file=runargsFile)
                    print('fromRunArgs({0})'.format(self._runArgsName),file=runargsFile)

                msg.info('Successfully wrote runargs file {0}'.format(self._runArgsFile))

            except (IOError, OSError) as e:
                errMsg = 'Got an error when writing JO template {0}: {1}'.format(self._runArgsFile, e)
                msg.error(errMsg)
                raise trfExceptions.TransformExecutionException(trfExit.nameToCode('TRF_EXEC_RUNARGS_ERROR'), errMsg)



    ## @brief Make sure skeleton file is available
    def ensureJobOptions(self):
        # Check the runArgs:
        if self._runArgsFile is None:
            msg.warning('No runArgs available')

        if not findFile(os.environ["JOBOPTSEARCHPATH"], self._runArgsFile):
            msg.warning('Could not find runArgs file %s', self._runArgsFile)

        # Check the skeleton(s):
        if  self._exe._skeleton:
            for skeleton in self._exe._skeleton:
                if not findFile(os.environ["JOBOPTSEARCHPATH"], skeleton):
                    msg.warning('Could not find job options skeleton file %s', skeleton)

  
    ## @brief Get the runArgs and skeleton joboptions, Master function
    #  @param input Input file list
    #  @param output Output file list
    #  @return List of runargs and skeletons to be processed by athena
    def getTopOptions(self, input = dict(), output = dict()):
        # Update the output name
        self._runArgsFile = 'runargs.' + self._exe.name + '.py'
        # First Make the runArgs file:
        self.writeRunArgs(input = input, output = output)
        # Make sure runArgs and skeleton are valid
        self.ensureJobOptions()
        if self._exe._isCAEnabled():
            #ComponentAccumulator based config, use only runargs file
            return [ self._runArgsFile ]
        else:
            #Traditional athena: runargs + skeleton
            return [ self._runArgsFile ] + self._exe._skeleton

