#!/usr/bin/env python3

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaPython.PyAthena import Alg, py_svc, StatusCode
from AthenaCommon.Logging import logging

# A basic algorithm that reads the DataHeader to print some information
class ReadDataHeaderAlg(Alg):

    def __init__(self,name='ReadDataHeaderAlg', totalEvents=0):
        super(ReadDataHeaderAlg,self).__init__(name=name)
        self.totalEvents = totalEvents
        self.nevt = 0
        return

    def initialize(self):
        self.sg = py_svc('StoreGateSvc')
        return StatusCode.Success

    def execute(self):
        if self.sg.contains('DataHeader', 'EventSelector'):
            dh = self.sg.retrieve('DataHeader', 'EventSelector')
            sz = dh.size()
            self.nevt += 1
            if self.nevt % 100 == 0:
                logging.info(f'DataHeader Size {sz} in event #{self.nevt}')
            if sz > 0:
                it = dh.begin()
                for idx in range(sz):
                    logging.debug(f'  >>  Key {it.getKey()} : Token {it.getToken().toString()}')
                    it+=1
                else:
                    logging.debug('  >> Successfully read through the DataHeader')
            else:
                logging.error(f'  >> Negative DataHeader size ({sz})!')
                return StatusCode.Failure
        else:
            logging.error('Could NOT find the DataHeader!')
            return StatusCode.Failure

        return StatusCode.Success

    def finalize(self):
        # Now this hack is needed because some errors, e.g. those in loading proxies,
        # don't cause a job failure because most loop managers seem to ignore them.
        # The problem is that even though we fail here, the job still "succeeds"
        if self.nevt != self.totalEvents:
            logging.error(f'Expected {self.totalEvents} events but processed {self.nevt}!')
            logging.error('This most likely means we could not process some events successfully!')
            return StatusCode.Failure

        return StatusCode.Success

# A minimal job that reads the DataHeader
if __name__ == '__main__':
    """
    Example usage: readDataHeader.py --filesInput=DAOD_PHYS.pool.root --evtMax=10 --loglevel=DEBUG
    """

    # Import the common flags/services
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    # Set the necessary configuration flags and lock them
    # This is filled from the command line arguments as shown above
    flags = initConfigFlags()
    flags.fillFromArgs()
    flags.lock()

    # Figure out how many events we expect
    if flags.Exec.MaxEvents < 0:
        totalEvents = 0
        from AthenaConfiguration.AutoConfigFlags import GetFileMD
        for filename in flags.Input.Files:
            totalEvents += GetFileMD(filename).get('nentries', 0)
    else:
        totalEvents = flags.Exec.MaxEvents

    # Set up the configuration and add the relevant services
    cfg = MainServicesCfg(flags)

    # Input reading
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # Schedule the Custom Algorithm
    cfg.addEventAlgo(ReadDataHeaderAlg('ReadDataHeaderAlg', totalEvents=totalEvents), sequenceName = 'AthAllAlgSeq')

    # Now run the job and exit accordingly
    sc = cfg.run()
    import sys
    sys.exit(not sc.isSuccess())
