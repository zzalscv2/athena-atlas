# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""Test multi-threaded xAOD::FileMetaData writing

Read default test HITS and configure to write output stream with
xAOD::FileMetaData object. Set up to process 4 concurrent events. Inspect the
produced file for xAOD::FileMetaData metadata items.

    Typical usage example
        python -m xAODMetaDataCnv.TestWriteFileMetaDataHITS |tee log
"""
import sys
from AthenaConfiguration import AllConfigFlags, Enums, TestDefaults, MainServicesConfig
from AthenaCommon import Constants, Logging
from OutputStreamAthenaPool import OutputStreamConfig
from PyUtils import MetaReader
from xAODEventInfoCnv import xAODEventInfoCnvConfig
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg


def writeFileMetaData(flags):
    """set up an output stream and xAOD::EventInfo"""
    accumulator = OutputStreamConfig.OutputStreamCfg(flags, streamName="HITS")
    accumulator.merge(SetupMetaDataForStreamCfg(flags, streamName="HITS"))

    accumulator.merge(
        xAODEventInfoCnvConfig.EventInfoCnvAlgCfg(
            flags=flags, inputKey="", disableBeamSpot=flags.Common.Project is not Enums.Project.Athena
        )
    )

    return accumulator


def testMetaData(file_name):
    """Check that file metadata is in output"""
    meta = MetaReader.read_metadata([file_name])[file_name]["metadata_items"]
    file_info_items = [
        v for _, v in meta.items() if "FileMetaData" in v
    ]
    print("found file metadata objects:", file_info_items)
    return bool(file_info_items)


def main():
    """Run a job writing a file with FileMetaData"""
    msg = Logging.logging.getLogger("TestFileMetaData")

    flags = AllConfigFlags.initConfigFlags()
    flags.Exec.OutputLevel = Constants.DEBUG
    flags.Input.Files = TestDefaults.defaultTestFiles.HITS_RUN2
    flags.Output.HITSFileName = "test.pool.root"
    flags.Concurrency.NumThreads = 4
    flags.Concurrency.NumConcurrentEvents = 4
    flags.lock()

    write = MainServicesConfig.MainServicesCfg(flags)
    write.merge(PoolReadCfg(flags))
    write.merge(writeFileMetaData(flags))
    write.run(100)

    try:
        if testMetaData(flags.Output.HITSFileName):
            msg.info("File contains xAOD::FileMetaData")
            return 0
        msg.error("File does not contain xAOD::FileMetaData")
    except ReferenceError:
        msg.error("Failed to produce output file")
    except KeyError:
        msg.error("Failed to get metadata item list from file")
    return 1


if __name__ == "__main__":
    sys.exit(main())
