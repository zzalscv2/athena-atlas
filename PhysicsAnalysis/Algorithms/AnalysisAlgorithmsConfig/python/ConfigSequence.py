# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

class ConfigSequence:
    """a sequence of ConfigBlock objects

    This could in principle just be a simple python list, and maybe we
    change it to that at some point (10 Mar 22).  Having it as its own
    class allows to implement some helper functions.

    This implements an interface similar to ConfigBlock, but it
    doesn't derive from it, as ConfigBlock will likely gain
    functionality in the future that wouldn't work for a sequence (or
    wouldn't work in the same way).

    """

    def __init__ (self) :
        self._blocks = []


    def append (self, block) :
        """append a configuration block to the sequence"""

        self._blocks.append (block)


    def makeAlgs (self, config) :
        """call makeAlgs() on all blocks

        This will create the actual algorithm configurables based on
        how the blocks are configured right now.
        """
        for block in self._blocks:
            block.makeAlgs (config)


    def fullConfigure (self, config) :
        """do the full configuration on this sequence

        This sequence needs to be the only sequence, i.e. it needs to
        contain all the blocks that will be configured, as it will
        perform all configuration steps at once.
        """
        self.makeAlgs (config)
        config.nextPass ()
        self.makeAlgs (config)


    def setOptionValue (self, name, value, **kwargs) :
        """set the given option on the sequence

        The name should generally be of the form
        "groupName.optionName" to identify what group the option
        belongs to.

        For simplicity I also allow a ".optionName" here, which will
        then set the property in the last group added.  That makes it
        fairly straightforward to add new blocks, set options on them,
        and then move on to the next blocks.  Please note that this
        mechanism ought to be viewed as strictly as a temporary
        convenience, and this short cut may go away once better
        alternatives are available.

        WARNING: The backend to option handling is slated to be
        replaced at some point.  This particular function may change
        behavior, interface or be removed/replaced entirely.

        """

        nameSplit = name.split ('.')
        groupName = '.'.join (nameSplit[0:-1])
        optionName = nameSplit[-1]

        # option names of the form ".option" are used to set the
        # option on the last set group of blocks configured
        if groupName == '' and len(nameSplit)==2 :
            groupName = self._blocks[-1].groupName()

        used = False
        for block in self._blocks :
            if block.groupName() == groupName and \
               block.hasOption (optionName):
                block.setOptionValue (optionName, value, **kwargs)
                used = True
        if not used :
            raise KeyError ('unknown option: ' + name)


    def __iadd__( self, sequence, index = None ):
        """Add another sequence to this one

        This function is used to add another sequence to this sequence
        using the '+=' operator.
        """

        # Check that the received object is of the right type:
        if not isinstance( sequence, ConfigSequence ):
            raise TypeError( 'The received object is not of type ConfigSequence' )

        for block in sequence._blocks :
            self._blocks.append (block)

        # Return the modified object:
        return self

    def __iter__( self ):
        """Create an iterator over all the configurations in this sequence

        This is to allow for a Python-like iteration over all
        configuration blocks that are part of the sequence.

        """

        # Create the iterator to process the internal list of algorithms:
        return self._blocks.__iter__()
