# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator, ConfigurationError
from AthenaConfiguration.ComponentFactory import CompFactory


def TagInfoMgrCfg(flags, tagValuePairs={}):
    # Sanity check:
    if not isinstance(tagValuePairs, dict):
        raise ConfigurationError("Parameter extraTagValuePairs is supposed to be a dictionary")

    # Build project-version string for the TagInfoMgr
    from os import getenv
    project = getenv("AtlasProject", "Unknown")
    version = getenv("AtlasVersion", "Unknown")
    atlasRelease=project+"-"+version

    tagValuePairs.update({"AtlasRelease" : atlasRelease})

    from Campaigns.Utils import Campaign
    if flags.Input.isMC and flags.Input.MCCampaign is not Campaign.Unknown:
        tagValuePairs.update({"mc_campaign" : flags.Input.MCCampaign.value})

    result = ComponentAccumulator()
    result.addService(CompFactory.TagInfoMgr(ExtraTagValuePairs=tagValuePairs), primary=True)
    return result


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import ConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    ConfigFlags.Input.Files = defaultTestFiles.RDO_RUN2
    ConfigFlags.lock()

    acc = TagInfoMgrCfg( ConfigFlags, {"SomeKey": "SomeValue"} )
    acc2 = TagInfoMgrCfg( ConfigFlags, {"OtherKey": "OtherValue", "SomeKey": "SomeValue"} )
    acc.merge(acc2)

    assert "SomeKey" in acc.getService("TagInfoMgr").ExtraTagValuePairs
    assert "OtherKey" in acc.getService("TagInfoMgr").ExtraTagValuePairs
    acc.store( open( "test.pkl", "wb" ) )
    print("All OK")
