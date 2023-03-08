# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
Utilities to get the online configuration.

Code modernised and ported from
Reconstruction/RecExample/RecExOnline/python/OnlineISConfiguration.py

    Example usage from as CLI:
        python -m PyUtils.OnlineISConfig

    Example usage in python:
        from PyUtils.OnlineISConfig import GetRunType
        print(GetRunType()[1])

"""
import os
import sys

import ipc
from AthenaCommon.Logging import logging
from ispy import IPCPartition, ISInfoDynAny, ISObject

log = logging.getLogger("OnlineISConfig")


def GetAtlasReady():
    try:
        r4p = ISObject(IPCPartition("ATLAS"), "RunParams.Ready4Physics", "RunParams")
        r4p.checkout()
        return r4p.ready4physics
    except Exception:
        log.error("#### Failed to determine if we are ready for physics")
        raise


def GetRunType():
    """Get the run type by reading the run-type setting in the partition from IS"""

    # Try to get the partition name
    try:
        partition = os.environ["TDAQ_PARTITION"]
        if partition == "EventDisplays":
            partition = "ATLAS"
    except KeyError:
        partition = "ATLAS"
        log.warning(
            f"TDAQ_PARTITION not defined in environment, using {partition} as default"
        )

    # now try and read the information from IS
    try:
        ipcPart = ipc.IPCPartition(partition)
        if not ipcPart.isValid():
            raise UserWarning(f"{partition=} invalid - cannot access run type settings")
        runparams = ISObject(ipcPart, "RunParams.RunParams", "RunParams")
        runparams.checkout()
        beamEnergy = runparams.beam_energy
        projectTag = runparams.T0_project_tag
    except UserWarning as err:
        log.error(err)
        beamEnergy = None
        projectTag = None

    log.info(f"Setting {projectTag=}")
    return (
        None,
        beamEnergy,
        projectTag,
    )  # the BeamType in the IS RunParams is not useful for auto-configuration


def GetBFields():
    # BFields are read from initial partition
    partition = "initial"
    log.debug(f"Trying to read magnetic field configuration from {partition=}")

    # now try and read the information from IS
    try:
        ipcPart = ipc.IPCPartition(partition)
        if not ipcPart.isValid():
            raise UserWarning(
                f"{partition=} invalid - cannot access magnetic field setting "
            )

        torCurrent = ISInfoDynAny(ipcPart, "DdcFloatInfo")
        solCurrent = ISInfoDynAny(ipcPart, "DdcFloatInfo")
        torInvalid = ISInfoDynAny(ipcPart, "DdcIntInfo")
        solInvalid = ISInfoDynAny(ipcPart, "DdcIntInfo")

        torInvalid.value = 1
        solInvalid.value = 1

        log.info(f"toroidCurrent = {torCurrent.value}")
        log.info(f"toroidInvalid = {torInvalid.value}")
        log.info(f"solenoidCurrent = {solCurrent.value}")
        log.info(f"solenoidInvalid = {solInvalid.value}")

        # And calculate the flags
        solOn = (solCurrent.value > 1000.0) and (solInvalid.value == 0)
        torOn = (torCurrent.value > 1000.0) and (torInvalid.value == 0)
    except UserWarning as err:
        log.error(err)
        # Should always be able to access initial parititon
        log.fatal("Failed to read magnetic field configuration from IS, aborting")

        sys.exit(1)

    # print the result
    log.info(f"Magnetic field in solenoid is {((solOn and 'ON') or 'OFF')}")
    log.info(f"Magnetic field in toroid is {((torOn and 'ON') or 'OFF')}")

    # finally return our values
    return (solCurrent, torCurrent)


if __name__ == "__main__":
    runType = GetRunType()
    print(f"(BeamType, BeamEnergy, ProjectTag): {runType}")
    bFields = GetBFields()
    print(f"(SolCurrent, TorCurrent): ({bFields[0].value, bFields[1].value})")
