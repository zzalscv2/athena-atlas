# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from os import getenv

import re
versionRegex = re.compile(r'\(v\.(.*)\)$')

ignoredGenerators = ["Pythia8B", "Powheg"]


def legacyReleaseData():
    from pathlib import Path
    from PathResolver import PathResolver
    filePath = Path(PathResolver.FindCalibFile("GeneratorConfig/Legacy_AthGeneration_versions.txt"))
    data = {}
    with filePath.open() as f:
        header = f.readline().strip().split(",")
        generators = header[2:]
        for line in f:
            line = line.strip().split(",")
            item = {generator: version for generator, version in zip(generators, line[2:])}
            item['version'] = line[0]
            data[line[1]] = item
    return data

def generatorsGetInitialVersionedDictionary(generators):
    output = {}
    for generator in generators:
        env_variable = f"{generator.upper()}VER"
        output[generator] = getenv(env_variable, None)
    return output

def generatorsGetFromMetadata(metadataString):
    output = {}
    metadataStringSplit = metadataString.split("+")
    for s in metadataStringSplit:
        match = versionRegex.search(s)
        if match:
            s = s.replace(match.group(0), "")
            output[s] = match.group(1)
        else:
            output[s] = None
    return output

def generatorsVersionedStringList(generatorsDictionary):
    list = []
    for generator, version in generatorsDictionary.items():
        if version is not None:
            list.append(f"{generator}(v.{version})")
        else:
            list.append(generator)
    return list

def generatorsVersionedString(generatorsVersionedList):
    return "+".join(generatorsVersionedList)


def GeneratorVersioningFixCfg(flags):
    from AthenaCommon.Logging import logging
    log = logging.getLogger("GeneratorsInfo")
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

    generatorsData = flags.Input.GeneratorsInfo
    log.info(f"Generators data: {generatorsData}")

    missingVersion = False
    for k, v in generatorsData.items():
        if v is None and k not in ignoredGenerators:
            missingVersion = True

    if not missingVersion:
        return ComponentAccumulator()

    log.info("At least one MC generator is missing version information. Attempting to fix...")

    from PyUtils.AMITagHelperConfig import inputAMITags
    tags = inputAMITags(flags, fixBroken=True, silent=True)
    tag = None
    if tags and tags[0].startswith("e"):
        tag = tags[0]

    releaseDataDict = legacyReleaseData()
    if tag not in releaseDataDict:
        log.warning(f"Could not find release data for tag {tag}.")
        return ComponentAccumulator()

    releaseData = releaseDataDict[tag]
    for k, v in generatorsData.items():
        if v is None and k not in ignoredGenerators and k in releaseData:
            log.info(f"Setting version for {k} to {releaseData[k]}.")
            generatorsData[k] = releaseData[k]

    outputStringList = generatorsVersionedStringList(generatorsData)

    from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
    return TagInfoMgrCfg(flags, tagValuePairs={"generators": generatorsVersionedString(outputStringList)})
