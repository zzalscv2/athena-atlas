# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from DerivationFrameworkCore.JetTriggerContentRun3 import JetTriggerContentRun3


excludedTriggerContentForTLA = [
    # substrings listing excluded content
    "AntiKt10", # ALL large-R jets
]

additionalTriggerContentForTLA = []


def triggerContentIsExcludedForTLA(content:str, exclusionList:list)->bool:
    excluded = False
    for excludedStr in exclusionList:
        if excludedStr in content:
            excluded = True
            break
    return excluded

# create list for TLA jet trigger content
# excluding content configured for removal 
# in excludedTriggerContentForTLA
JetTriggerContentRun3TLA = [
    element for element in JetTriggerContentRun3
    if not triggerContentIsExcludedForTLA(element, excludedTriggerContentForTLA)
]

for additionalContent in additionalTriggerContentForTLA:
    # determine whether processing an Aux container or not
    if "Aux" not in additionalContent:
        JetTriggerContentRun3TLA.append(additionalContent)
    else:
        # processing an Aux container so different handling is needed
        # check if the Aux container exists and then compare Aux attributes
        # and use all unique elements
        matchedAuxElementIdx = -1
        for idx, element in enumerate(JetTriggerContentRun3TLA):
            if additionalContent.split(".")[0] == element.split(".")[0]:
                matchedAuxElementIdx = idx

        if matchedAuxElementIdx < 0:
            # Aux container doesn't already exist in TLA jet trigger content
            JetTriggerContentRun3TLA.append(additionalContent)
        else:
            # check whether we are taking the whole Aux container or just selected attributes
            if additionalContent.split(".")[-1] == "" and len(additionalContent.split(".")) == 2:
                # overwrite the existing entry with the updated content for TLA
                JetTriggerContentRun3TLA[matchedAuxElementIdx] = additionalContent
            else:
                # Aux container already exists, keep unique Aux attributes
                JetTriggerContentRun3TLA[matchedAuxElementIdx] = ".".join([additionalContent.split(".")[0]] + list(set(additionalContent.split(".")[1:] + JetTriggerContentRun3TLA[matchedAuxElementIdx].split(".")[1:])))
            
# remove any duplicates introduced by 
# adding non-Aux containers in additionalTriggerContentForTLA
JetTriggerContentRun3TLA = list(set(JetTriggerContentRun3TLA))


