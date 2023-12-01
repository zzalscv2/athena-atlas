# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

####################################################################
# SlimmingHelper.py
# James.Catmore@cern.ch
# This is the main class implementing "smart slimming"
# The code has to do four things:
# (1) build a list of the items that are to be smart-slimmed
# Items look like this:
# CollectionName - for the main container
# CollectionNameAux.var1.var2.var3.varN - for the variable lists
# (2) process each of these into strings (for the main containers)
# and dictionaries (for the variable lists) and simultaneously
# expand the aux store into dynamic variables for all collections
# being slimmed - these steps (2) are done by the ContentHandler
# (3) take the resulting list and dictionary and ensure that each
# item appears once and only once
# (4) Add each item to the output stream
#
# The items come from the following sources:
# - for smart variables: from the branch lists. In this case the user
# input is just a collection name which is then used to grab the
# appropriate list: SmartCollections
# - for extra variables added by the user: directly from the user.
# In these cases the user provides the items directly in the form listed
# above: ExtraVariables
# - where the full list of variables is needed the aux item is just of
# the form CollectionNameAux., so this can be internally generated using
# the collection name from the user
# In addition to this the user may also add items directly to the output
# stream, but no expansion-to-dynamic is done, so it is important that
# this feature is only used for collections that haven't been processed
# as described above. If the user attempts to do this the output data
# may be unreadable.
####################################################################

from DerivationFrameworkCore.CompulsoryContent import CompulsoryContent, CompulsoryTriggerNavigation, CompulsoryDynamicContent
from DerivationFrameworkCore.ContentHandler import ContentHandler
from DerivationFrameworkCore.ContainersForExpansion import ContainersForExpansion
from DerivationFrameworkCore.ContainersOnTheFly import ContainersOnTheFly
from DerivationFrameworkCore.FullListOfSmartContainers import FullListOfSmartContainers
import PyUtils.Logging as L
msg = L.logging.getLogger('DerivationFramework__SlimmingHelper')
msg.setLevel(L.logging.INFO)

# This list base class allows the slimming helper to be locked after calling BuildFinalItemList
class lockable_list(list):
        def __init__(self,data=[]):
                list.__init__(self,data)
                self.__dict__["_locked"] = False
        def append(self,name):
                if self._locked is True:
                        msg.error("Attempting to Modify SlimmingHelper after BuildFinalItemList has Been Called")
                        raise RuntimeError("Late Modification to SlimmingHelper do not modify after calling BuildFinalItemList")
                else:
                        return list.append(self, name)
        def __setattr__(self,name,value):
                if self._locked is True:
                        msg.error("Attempting to Modify SlimmingHelper after BuildFinalItemList has Been Called")
                        raise RuntimeError("Late Modification to SlimmingHelper do not modify after calling BuildFinalItemList")
                else:
                        self.__dict__[name] = value
        def lock(self):
                self.__dict__["_locked"] = True

# Builds the "NamesAndTypes" map needed to set up the item list
def buildNamesAndTypes(*args):
        namesAndTypes = {}
        if len(args)==0:
                # 1st possibility: non-CA job, user didn't provide a list from ComponentAccumulator 
                from RecExConfig.InputFilePeeker import inputFileSummary
                if inputFileSummary['eventdata_items'] is not None:
                        for item in inputFileSummary['eventdata_items']:
                                namesAndTypes[item[1].strip('.')] = item[0]
                # 2nd possibility: CA job, user provided the list from ComponentAccumulator
                else:
                        from DerivationFrameworkCore.StaticNamesAndTypes import StaticNamesAndTypes
                        namesAndTypes = StaticNamesAndTypes
        else:
                for item in args[0]:
                        item = item.split('#')
                        namesAndTypes[item[1].strip('.')] = item[0] 
        return namesAndTypes      

class SlimmingHelper:
        def __init__(self,inputName,**kwargs):
                self.__dict__["_locked"] = False
                self.name = inputName
                self.FinalItemList = lockable_list() # The final item list that will be appended to the output stream
                self.StaticContent = lockable_list() # Content added explicitly via old-style content lists
                self.ExtraVariables = lockable_list() # Content added by users via variable names (dictionary type:[item1,item,..,N])
                # Smart slimming (only variables needed for CP + kinematics)
                self.SmartCollections = lockable_list()
                self.AllVariables = lockable_list() # Containers for which all branches should be kept
                self.AppendToDictionary = {}
                self.ConfigFlags = kwargs.pop("ConfigFlags", None) # Config flags to be set in CA configs
                self.IncludeTriggerNavigation = True
                self.IncludeAdditionalTriggerContent = False
                self.IncludeMuonTriggerContent = False
                self.IncludeEGammaTriggerContent = False
                self.IncludeJetTauEtMissTriggerContent = False
                self.IncludeJetTriggerContent = False
                self.IncludeTrackingTriggerContent = False
                self.IncludeTauTriggerContent = False
                self.IncludeEtMissTriggerContent = False
                self.IncludeBJetTriggerContent = False
                self.IncludeBPhysTriggerContent = False
                self.IncludeMinBiasTriggerContent = False
                self.OverrideJetTriggerContentWithTLAContent = False
                # Choice of whether user provided a typed container list or not (CA vs non-CA) 
                if "NamesAndTypes" in kwargs.keys(): self.NamesAndTypes = buildNamesAndTypes(kwargs["NamesAndTypes"])
                else: self.NamesAndTypes = buildNamesAndTypes()
                self.theHandler = ContentHandler(self.name+"Handler",self.NamesAndTypes)

        # This hack prevents any members from being modified after lock is set to true, this happens in AppendContentToStream
        def __setattr__(self,name,value):
                if self._locked is True:
                        msg.error("Attempting to Modify SlimmingHelper "+self.name+" After AppendContentToStream has Been Called")
                        raise RuntimeError("Late Modification to SlimmingHelper, do not modifiy after calling AppendContentToStream")
                elif type(value)==list:
                        self.__dict__[name] = lockable_list(value)
                else:
                        self.__dict__[name] = value

        # Function to check the configuration of the Smart Slimming List
        def CheckList(self,masterList):
                conflicted_items=[]
                for item in CompulsoryContent:
                        if item.endswith("#*"):
                                compare_str=item[:-2].replace("xAOD::","")
                                for m_item in masterList:
                                        if m_item.startswith(compare_str) and m_item.replace("Aux.","") not in CompulsoryDynamicContent:
                                                conflicted_items.append(m_item)
                if len(conflicted_items)!=0:
                        msg.error("Slimming list contains " +str(conflicted_items)+" which are already included in compulsory content: please remove these items from slimming list")
                        raise RuntimeError("Conflict in Slimming List and Compulsory Content")


        # Loops over final ItemList and appends each item to the stream
        # Used for jobs not set up in the component accumulator 
        def AppendContentToStream(self,Stream):
                # Check if the SlimmingHelper is locked. 
                # If it is, just loop over the items and append. 
                # If not, build the item list and then append.
                if self._locked is False:
                        self.BuildFinalItemList()
                for item in self.FinalItemList:
                        Stream.AddItem(item)

        # Returns the final item list. Used for component accumulator jobs
        def GetItemList(self):
                # Check if the SlimmingHelper is locked. 
                # If it is, just return the item list. 
                # If not, build the item list and then return it.
                if self._locked is False:
                        self.BuildFinalItemList()
                return(self.FinalItemList)

        def BuildFinalItemList(self):
                # Master item list: all items that must be passed to the ContentHandler for processing
                # This will now be filled
                masterItemList = []
                # All variables list: where all variables are requested, no variable lists are needed
                # This list ensures that variables are not added individually in such cases
                allVariablesList = []
                self.AllVariables += CompulsoryDynamicContent
                # Add all-variable collections
                if len(self.AllVariables)>0:
                        for item in self.AllVariables: masterItemList.extend(self.GetWholeContentItems(item))
                for item in masterItemList:
                        if "Aux." in item:
                                allVariablesList.append(item)

                # Trigger objects: add them by hand to the smart collection list (to keep the previous interface)
                triggerContent = False

                if (self.IncludeAdditionalTriggerContent is True):
                        triggerContent = True

                if (self.IncludeMuonTriggerContent is True):
                        triggerContent = True
                        self.SmartCollections.append("HLT_xAOD__MuonContainer_MuonEFInfo")

                if (self.IncludeEGammaTriggerContent is True):
                        triggerContent = True
                        self.SmartCollections.append("HLT_xAOD__PhotonContainer_egamma_Photons")

                if (self.IncludeJetTriggerContent is True):
                        triggerContent = True
                        self.SmartCollections.append("HLT_xAOD__JetContainer_a4tcemsubjesFS")
                        self.SmartCollections.append("HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf") # Run 3 jet collections
                        from DerivationFrameworkCore.JetTriggerFixContent import JetTriggerFixContent
                        for item in JetTriggerFixContent:
                                self.FinalItemList.append(item)

                if (self.IncludeTrackingTriggerContent is True):
                        triggerContent = True
                        self.SmartCollections.append("HLT_IDVertex_FS")
                        self.SmartCollections.append("HLT_IDTrack_FS_FTF")

                if (self.IncludeEtMissTriggerContent is True):
                        triggerContent = True
                        self.SmartCollections.append("HLT_xAOD__TrigMissingETContainer_TrigEFMissingET")
                        from DerivationFrameworkCore.EtMissTriggerFixContent import EtMissTriggerFixContent
                        for item in EtMissTriggerFixContent:
                                self.FinalItemList.append(item)

                if (self.IncludeTauTriggerContent is True):
                        triggerContent = True
                        self.SmartCollections.append("HLT_xAOD__TauJetContainer_TrigTauRecMerged")

                if (self.IncludeBJetTriggerContent is True):
                        triggerContent = True
                        self.SmartCollections.append("HLT_xAOD__BTaggingContainer_HLTBjetFex")

                if (self.IncludeBPhysTriggerContent is True):
                        triggerContent = True
                        self.SmartCollections.append("HLT_xAOD__TrigBphysContainer_EFBMuMuFex")

                if (self.IncludeMinBiasTriggerContent is True):
                        triggerContent = True
                        self.SmartCollections.append("HLT_xAOD__TrigVertexCountsContainer_vertexcounts")

                # Smart items
                if len(self.SmartCollections)>0:
                        for collection in self.SmartCollections:
                                masterItemList.extend(self.GetSmartItems(collection))

                # Run some basic tests to prevent clashes with CompulsoryContent content
                self.CheckList(masterItemList)

                # Add extra variables
                if len(self.ExtraVariables)>0:
                        for item in self.ExtraVariables:
                                masterItemList.extend(self.GetExtraItems(item))

                #Add on-the-fly containers to the dictionary
                for _cont,_type in ContainersOnTheFly(self.ConfigFlags):
                        if _cont not in self.AppendToDictionary:
                                self.AppendToDictionary[_cont]=_type

                # Process the master list...

                # Main containers (this is a simple list of lines, one per container X collection)
                mainEntries = []
                # Aux items (this is a dictionary: collection name and list of aux variables)
                auxEntries = {}
                self.theHandler.AppendToDictionary = self.AppendToDictionary
                mainEntries,auxEntries = self.theHandler.GetContent(masterItemList,allVariablesList)

                # Add processed items to the stream
                excludedAuxData = "-clusterAssociation.-PseudoJet"
                excludedAuxEntries= [entry.strip("-") for entry in excludedAuxData.split(".")]
                for item in mainEntries:
                        self.FinalItemList.append(item)
                for item in auxEntries.keys():
                        theDictionary = self.NamesAndTypes.copy()
                        theDictionary.update (self.AppendToDictionary)
                        if item in theDictionary.keys():
                                if (theDictionary[item]=='xAOD::JetAuxContainer'):
                                        entry = "xAOD::JetAuxContainer#"+item+"."
                                elif (theDictionary[item]=='xAOD::ShallowAuxContainer'):
                                        entry = "xAOD::ShallowAuxContainer#"+item+"."
                                elif (theDictionary[item]=='xAOD::MissingETAuxAssociationMap'):
                                        entry = "xAOD::MissingETAuxAssociationMap#"+item+"."
                                elif ("AuxInfo" in theDictionary[item]):
                                        entry = "xAOD::AuxInfoBase!#"+item+"."
                                # Next elif - remaining containers
                                # that still need to be expanded with AuxStoreWrapper
                                # In the run 3 trigger EDM there are no such containers so this is dead code
                                elif (theDictionary[item] in ContainersForExpansion):
                                        entry = "xAOD::AuxContainerBase#"+item+"."
                                else:
                                        entry = "xAOD::AuxContainerBase!#"+item+"."
                                for element in auxEntries[item]:
                                        #Skip anything that shouldn't be written out to a DAOD for tracks or jets
                                        if ('xAOD::TrackParticleContainer' in theDictionary[item]) and (element in excludedAuxEntries): continue
                                        if ('xAOD::JetAuxContainer' in theDictionary[item]) and (element in excludedAuxEntries): continue
                                        length = len(auxEntries[item])
                                        if (element==(auxEntries[item])[length-1]):
                                                entry += element
                                        else:
                                                entry += element+"."
                                if ('xAOD::TrackParticleContainer' in theDictionary[item] and auxEntries[item]==""):
                                        entry+=excludedAuxData
                                if ('xAOD::JetAuxContainer' in theDictionary[item] and auxEntries[item]==""):
                                        entry+=excludedAuxData
                                self.FinalItemList.append(entry)

                # Add compulsory items not covered by smart slimming (so no expansion)
                for item in CompulsoryContent:
                        self.FinalItemList.append(item)

                # Add trigger item (not covered by smart slimming so no expansion)
                # Old, will be removed (kept just to not break some deriavtions)
                if (self.IncludeJetTauEtMissTriggerContent is True):
                        from DerivationFrameworkCore.JetTauEtMissTriggerContent import JetTauEtMissTriggerContent
                        for item in JetTauEtMissTriggerContent:
                                self.FinalItemList.append(item)

                # non xAOD collections for MinBias
                if (self.IncludeMinBiasTriggerContent is True):
                        from DerivationFrameworkCore.MinBiasTrigger_nonxAOD_Content import MinBiasTrigger_nonxAOD_Content
                        for item in MinBiasTrigger_nonxAOD_Content:
                                self.FinalItemList.append(item)

                if (triggerContent and self.IncludeTriggerNavigation):
                        for item in CompulsoryTriggerNavigation:
                                self.FinalItemList.append(item)

                # Add non-xAOD and on-the-fly content (not covered by smart slimming so no expansion)
                badItemsWildcards = []
                badItemsXAOD = []
                for item in self.StaticContent:
                        if (self.ValidateStaticContent(item)=="OK"):
                                self.FinalItemList.append(item)
                        if (self.ValidateStaticContent(item)=="WILDCARD"):
                                badItemsWildcards.append(item)
                        if (self.ValidateStaticContent(item)=="XAOD"):
                                badItemsXAOD.append(item)
                if (len(badItemsWildcards)>0):
                        msg.error("These static items contain wildcards: not permitted")
                        print (badItemsWildcards)
                        raise RuntimeError("Static content list contains wildcards")
                if (len(badItemsXAOD)>0):
                        msg.error("These static items are xAOD collections: not permitted")
                        print (badItemsXAOD)
                        raise RuntimeError("Static content list contains xAOD collections")
                #Prevent any more modifications As they will be completely ignored, and hard to debug
                self.FinalItemList.lock()
                self.StaticContent.lock()
                self.ExtraVariables.lock()
                self.SmartCollections.lock()
                self.AllVariables.lock()
                self._locked=True

###################################################################################
###################################################################################

# Get full content (e.g. whole aux store) for this container
        def GetWholeContentItems(self,collection):
                items = [collection,collection+"Aux."]
                return items

        # Get all branches associated with all tools needed for this container
        def GetSmartItems(self,collectionName):
                # Look up what is needed for this container type
                items = []
                if collectionName not in FullListOfSmartContainers(self.ConfigFlags):
                        raise RuntimeError("Smart slimming container "+collectionName+" does not exist or does not have a smart slimming list")
                if collectionName=="EventInfo":
                        from DerivationFrameworkCore.EventInfoContent import EventInfoContent
                        items.extend(EventInfoContent)
                elif collectionName=="Electrons":
                        from DerivationFrameworkEGamma.ElectronsCPContent import ElectronsCPContent
                        items.extend(ElectronsCPContent)
                elif collectionName=="LRTElectrons":
                        from DerivationFrameworkEGamma.LargeD0ElectronsCPContent import LargeD0ElectronsCPContent
                        items.extend(LargeD0ElectronsCPContent)
                elif collectionName=="Photons":
                        from DerivationFrameworkEGamma.PhotonsCPContent import PhotonsCPContent
                        items.extend(PhotonsCPContent)
                elif collectionName=="Muons":
                        if not self.ConfigFlags:
                            raise RuntimeError("We're in the era of component accumulator. Please setup your job with CA if you want to have muons")
                        from DerivationFrameworkMuons.MuonsCommonConfig import MuonCPContentCfg
                        items.extend(MuonCPContentCfg(self.ConfigFlags))
                elif collectionName=="MuonsLRT":
                        if not self.ConfigFlags:
                            raise RuntimeError("We're in the era of component accumulator. Please setup your job with CA if you want to have muons")
                        from DerivationFrameworkMuons.MuonsCommonConfig import MuonCPContentLRTCfg
                        items.extend(MuonCPContentLRTCfg(self.ConfigFlags))                        
                elif collectionName=="TauJets":
                        from DerivationFrameworkTau.TauJetsCPContent import TauJetsCPContent
                        items.extend(TauJetsCPContent)
                elif collectionName=="TauMVATESJets":
                        from DerivationFrameworkTau.TauMVATESContent import TauMVATESContent
                        items.extend(TauMVATESContent)
                elif collectionName=="DiTauJets":
                        from DerivationFrameworkTau.DiTauJetsCPContent import DiTauJetsCPContent
                        items.extend(DiTauJetsCPContent)
                elif collectionName=="DiTauJetsLowPt":
                        from DerivationFrameworkTau.DiTauJetsLowPtCPContent import DiTauJetsLowPtCPContent
                        items.extend(DiTauJetsLowPtCPContent)
                elif collectionName=="TauJets_MuonRM":
                        from DerivationFrameworkTau.TauJets_MuonRMCPContent import TauJets_MuonRMCPContent
                        if "TauJets_MuonRM" not in self.AppendToDictionary:
                                self.AppendToDictionary["TauJets_MuonRM"]                          = 'xAOD::TauJetContainer'
                                self.AppendToDictionary["TauJets_MuonRMAux"]                       = 'xAOD::TauJetAuxContainer'
                                self.AppendToDictionary["TauTracks_MuonRM"]                        = 'xAOD::TauTrackContainer'
                                self.AppendToDictionary["TauTracks_MuonRMAux"]                     = 'xAOD::TauTrackAuxContainer'
                                self.AppendToDictionary["TauSecondaryVertices_MuonRM"]             = 'xAOD::VertexContainer'
                                self.AppendToDictionary["TauSecondaryVertices_MuonRMAux"]          = 'xAOD::VertexAuxContainer'
                                self.AppendToDictionary["TauNeutralParticleFlowObjects_MuonRM"]    = 'xAOD::PFOContainer'
                                self.AppendToDictionary["TauNeutralParticleFlowObjects_MuonRMAux"] = 'xAOD::PFOAuxContainer'
                        items.extend(TauJets_MuonRMCPContent)
                elif collectionName=="TauJets_EleRM":
                        from DerivationFrameworkTau.TauJets_EleRMCPContent import TauJets_EleRMCPContent
                        items.extend(TauJets_EleRMCPContent)
                elif collectionName=="MET_Baseline_AntiKt4EMTopo":
                        from DerivationFrameworkJetEtMiss.MET_Baseline_AntiKt4EMTopoCPContent import MET_Baseline_AntiKt4EMTopoCPContent
                        items.extend(MET_Baseline_AntiKt4EMTopoCPContent)
                elif collectionName=="MET_Baseline_AntiKt4EMPFlow":
                        from DerivationFrameworkJetEtMiss.MET_Baseline_AntiKt4EMPFlowCPContent import MET_Baseline_AntiKt4EMPFlowCPContent
                        items.extend(MET_Baseline_AntiKt4EMPFlowCPContent)
                elif collectionName=="AntiKt2TruthJets":
                        from DerivationFrameworkJetEtMiss.AntiKt2TruthJetsCPContent import AntiKt2TruthJetsCPContent
                        items.extend(AntiKt2TruthJetsCPContent)
                elif collectionName=="AntiKt4TruthJets":
                        from DerivationFrameworkJetEtMiss.AntiKt4TruthJetsCPContent import AntiKt4TruthJetsCPContent
                        items.extend(AntiKt4TruthJetsCPContent)
                elif collectionName=="AntiKt4TruthWZJets":
                        from DerivationFrameworkJetEtMiss.AntiKt4TruthWZJetsCPContent import AntiKt4TruthWZJetsCPContent
                        items.extend(AntiKt4TruthWZJetsCPContent)
                elif collectionName=="AntiKt4TruthDressedWZJets":
                        from DerivationFrameworkJetEtMiss.AntiKt4TruthDressedWZJetsCPContent import AntiKt4TruthDressedWZJetsCPContent
                        items.extend(AntiKt4TruthDressedWZJetsCPContent)
                elif collectionName=="AntiKt2LCTopoJets":
                        from DerivationFrameworkJetEtMiss.AntiKt2LCTopoJetsCPContent import AntiKt2LCTopoJetsCPContent
                        items.extend(AntiKt2LCTopoJetsCPContent)
                elif collectionName=="AntiKt4LCTopoJets":
                        from DerivationFrameworkJetEtMiss.AntiKt4LCTopoJetsCPContent import AntiKt4LCTopoJetsCPContent
                        items.extend(AntiKt4LCTopoJetsCPContent)
                elif collectionName=="AntiKt4EMTopoJets":
                        from DerivationFrameworkJetEtMiss.AntiKt4EMTopoJetsCPContent import AntiKt4EMTopoJetsCPContent
                        items.extend(AntiKt4EMTopoJetsCPContent)
                elif collectionName=="AntiKt4EMTopoLowPtJets":
                        from DerivationFrameworkJetEtMiss.AntiKt4EMTopoLowPtJetsCPContent import AntiKt4EMTopoLowPtJetsCPContent
                        items.extend(AntiKt4EMTopoLowPtJetsCPContent)
                elif collectionName=="AntiKt4EMTopoNoPtCutJets":
                        from DerivationFrameworkJetEtMiss.AntiKt4EMTopoNoPtCutJetsCPContent import AntiKt4EMTopoNoPtCutJetsCPContent
                        items.extend(AntiKt4EMTopoNoPtCutJetsCPContent)
                elif collectionName=="AntiKt4EMPFlowJets":
                        from DerivationFrameworkJetEtMiss.AntiKt4EMPFlowJetsCPContent import AntiKt4EMPFlowJetsCPContent
                        items.extend(AntiKt4EMPFlowJetsCPContent)
                elif collectionName=="AntiKt4EMPFlowLowPtJets":
                        from DerivationFrameworkJetEtMiss.AntiKt4EMPFlowLowPtJetsCPContent import AntiKt4EMPFlowLowPtJetsCPContent
                        items.extend(AntiKt4EMPFlowLowPtJetsCPContent)
                elif collectionName=="AntiKt4EMPFlowByVertexJets":
                        from DerivationFrameworkJetEtMiss.AntiKt4EMPFlowByVertexJetsCPContent import AntiKt4EMPFlowByVertexJetsCPContent
                        items.extend(AntiKt4EMPFlowByVertexJetsCPContent)
                elif collectionName=="AntiKt4UFOCSSKJets":
                        from DerivationFrameworkJetEtMiss.AntiKt4UFOCSSKJetsCPContent import AntiKt4UFOCSSKJetsCPContent
                        items.extend(AntiKt4UFOCSSKJetsCPContent)
                elif collectionName=="AntiKt10TruthJets":
                        from DerivationFrameworkJetEtMiss.AntiKt10TruthJetsCPContent import AntiKt10TruthJetsCPContent
                        items.extend(AntiKt10TruthJetsCPContent)
                elif collectionName=="AntiKt10TruthWZJets":
                        from DerivationFrameworkJetEtMiss.AntiKt10TruthWZJetsCPContent import AntiKt10TruthWZJetsCPContent
                        items.extend(AntiKt10TruthWZJetsCPContent)
                elif collectionName=="AntiKt10LCTopoJets":
                        from DerivationFrameworkJetEtMiss.AntiKt10LCTopoJetsCPContent import AntiKt10LCTopoJetsCPContent
                        items.extend(AntiKt10LCTopoJetsCPContent)
                elif collectionName=="AntiKt10TrackCaloClusterJets":
                        from DerivationFrameworkJetEtMiss.AntiKt10TrackCaloClusterJetsCPContent import AntiKt10TrackCaloClusterJetsCPContent
                        items.extend(AntiKt10TrackCaloClusterJetsCPContent)
                elif collectionName=="AntiKt10UFOCSSKJets":
                        from DerivationFrameworkJetEtMiss.AntiKt10UFOCSSKJetsCPContent import AntiKt10UFOCSSKJetsCPContent
                        items.extend(AntiKt10UFOCSSKJetsCPContent)
                elif collectionName=="AntiKt10UFOCHSJets":
                        from DerivationFrameworkJetEtMiss.AntiKt10UFOCHSJetsCPContent import AntiKt10UFOCHSJetsCPContent
                        items.extend(AntiKt10UFOCHSJetsCPContent)
                elif collectionName=="AntiKt10TruthTrimmedPtFrac5SmallR20Jets":
                        from DerivationFrameworkJetEtMiss.AntiKt10TruthTrimmedPtFrac5SmallR20JetsCPContent import AntiKt10TruthTrimmedPtFrac5SmallR20JetsCPContent
                        items.extend(AntiKt10TruthTrimmedPtFrac5SmallR20JetsCPContent)
                elif collectionName=="AntiKt10LCTopoTrimmedPtFrac5SmallR20Jets":
                        from DerivationFrameworkJetEtMiss.AntiKt10LCTopoTrimmedPtFrac5SmallR20JetsCPContent import AntiKt10LCTopoTrimmedPtFrac5SmallR20JetsCPContent
                        items.extend(AntiKt10LCTopoTrimmedPtFrac5SmallR20JetsCPContent)
                elif collectionName=="AntiKt10TrackCaloClusterTrimmedPtFrac5SmallR20Jets":
                        from DerivationFrameworkJetEtMiss.AntiKt10TrackCaloClusterTrimmedPtFrac5SmallR20JetsCPContent import AntiKt10TrackCaloClusterTrimmedPtFrac5SmallR20JetsCPContent
                        if "AntiKt10TrackCaloClusterTrimmedPtFrac5SmallR20Jets" not in self.AppendToDictionary:
                                self.AppendToDictionary["AntiKt10TrackCaloClusterTrimmedPtFrac5SmallR20Jets"]='xAOD::JetContainer'
                                self.AppendToDictionary["AntiKt10TrackCaloClusterTrimmedPtFrac5SmallR20JetsAux"]='xAOD::JetAuxContainer'
                        items.extend(AntiKt10TrackCaloClusterTrimmedPtFrac5SmallR20JetsCPContent)
                elif collectionName=="AntiKt10TruthSoftDropBeta100Zcut10Jets":
                        from DerivationFrameworkJetEtMiss.AntiKt10TruthSoftDropBeta100Zcut10JetsCPContent import AntiKt10TruthSoftDropBeta100Zcut10JetsCPContent
                        items.extend(AntiKt10TruthSoftDropBeta100Zcut10JetsCPContent)
                elif collectionName=="AntiKt10TruthDressedWZSoftDropBeta100Zcut10Jets":
                        from DerivationFrameworkJetEtMiss.AntiKt10TruthDressedWZSoftDropBeta100Zcut10JetsCPContent import AntiKt10TruthDressedWZSoftDropBeta100Zcut10JetsCPContent
                        items.extend(AntiKt10TruthDressedWZSoftDropBeta100Zcut10JetsCPContent)
                elif collectionName=="AntiKt10UFOCHSSoftDropBeta100Zcut10Jets":
                        from DerivationFrameworkJetEtMiss.AntiKt10UFOCHSSoftDropBeta100Zcut10JetsCPContent import AntiKt10UFOCHSSoftDropBeta100Zcut10JetsCPContent
                        items.extend(AntiKt10UFOCHSSoftDropBeta100Zcut10JetsCPContent)
                elif collectionName=="AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets":
                        from DerivationFrameworkJetEtMiss.AntiKt10UFOCSSKSoftDropBeta100Zcut10JetsCPContent import AntiKt10UFOCSSKSoftDropBeta100Zcut10JetsCPContent
                        items.extend(AntiKt10UFOCSSKSoftDropBeta100Zcut10JetsCPContent)
                elif collectionName=="AntiKtVR30Rmax4Rmin02PV0TrackJets":
                        from DerivationFrameworkJetEtMiss.AntiKtVR30Rmax4Rmin02PV0TrackJetsCPContent import AntiKtVR30Rmax4Rmin02PV0TrackJetsCPContent
                        items.extend(AntiKtVR30Rmax4Rmin02PV0TrackJetsCPContent)
                elif collectionName=="BTagging_AntiKt4EMPFlow":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingStandardContent
                        items.extend(BTaggingStandardContent("AntiKt4EMPFlowJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKt4EMPFlow_expert":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingExpertContent
                        items.extend(BTaggingExpertContent("AntiKt4EMPFlowJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKt4EMTopo":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingStandardContent
                        items.extend(BTaggingStandardContent("AntiKt4EMTopoJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKtVR30Rmax4Rmin02Track":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingStandardContent
                        items.extend(BTaggingStandardContent("AntiKtVR30Rmax4Rmin02PV0TrackJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKtVR30Rmax4Rmin02Track_expert":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingExpertContent
                        items.extend(BTaggingExpertContent("AntiKtVR30Rmax4Rmin02PV0TrackJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKt2Track":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingStandardContent
                        items.extend(BTaggingStandardContent("AntiKt2PV0TrackJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKt3Track":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingStandardContent
                        items.extend(BTaggingStandardContent("AntiKt3PV0TrackJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKt4Track":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingStandardContent
                        items.extend(BTaggingStandardContent("AntiKt4PV0TrackJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKt8EMTopoExKt2Sub":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingExpertContent
                        items.extend(BTaggingExpertContent("AntiKt8EMTopoExKt2SubJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKt8EMTopoExKt3Sub":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingExpertContent
                        items.extend(BTaggingExpertContent("AntiKt8EMTopoExKt3SubJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKt8EMTopoExCoM2Sub":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingExpertContent
                        items.extend(BTaggingExpertContent("AntiKt8EMTopoExCoM2SubJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKt8EMPFlowExKt2Sub":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingExpertContent
                        items.extend(BTaggingExpertContent("AntiKt8EMPFlowExKt2SubJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKt8EMPFlowExKt3Sub":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingExpertContent
                        items.extend(BTaggingExpertContent("AntiKt8EMPFlowExKt3SubJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKt8EMPFlowExKt2GASub":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingExpertContent
                        items.extend(BTaggingExpertContent("AntiKt8EMPFlowExKt2GASubJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKt8EMPFlowExKt3GASub":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingExpertContent
                        items.extend(BTaggingExpertContent("AntiKt8EMPFlowExKt3GASubJets", self.ConfigFlags))
                elif collectionName=="BTagging_DFAntiKt4HI":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingStandardContent
                        items.extend(BTaggingStandardContent("DFAntiKt4HIJets", self.ConfigFlags))
                elif collectionName=="BTagging_AntiKt4HI":
                        from DerivationFrameworkFlavourTag.BTaggingContent import BTaggingStandardContent
                        items.extend(BTaggingStandardContent("AntiKt4HIJets", self.ConfigFlags))
                elif collectionName=="InDetTrackParticles":
                        from DerivationFrameworkInDet.InDetTrackParticlesCPContent import InDetTrackParticlesCPContent
                        items.extend(InDetTrackParticlesCPContent)
                elif collectionName=="InDetPseudoTrackParticles":
                        from DerivationFrameworkInDet.InDetPseudoTrackParticlesCPContent import InDetPseudoTrackParticlesCPContent
                        items.extend(InDetPseudoTrackParticlesCPContent)
                elif collectionName=="InDetReplacedWithPseudoTrackParticles":
                        from DerivationFrameworkInDet.InDetReplacedWithPseudoTrackParticlesCPContent import InDetReplacedWithPseudoTrackParticlesCPContent
                        items.extend(InDetReplacedWithPseudoTrackParticlesCPContent)
                elif collectionName=="InDetReplacedWithPseudoFromBTrackParticles":
                        from DerivationFrameworkInDet.InDetReplacedWithPseudoFromBTrackParticlesCPContent import InDetReplacedWithPseudoFromBTrackParticlesCPContent
                        items.extend(InDetReplacedWithPseudoFromBTrackParticlesCPContent)
                elif collectionName=="InDetReplacedWithPseudoNotFromBTrackParticles":
                        from DerivationFrameworkInDet.InDetReplacedWithPseudoNotFromBTrackParticlesCPContent import InDetReplacedWithPseudoNotFromBTrackParticlesCPContent
                        items.extend(InDetReplacedWithPseudoNotFromBTrackParticlesCPContent)
                elif collectionName=="InDetPlusPseudoTrackParticles":
                        from DerivationFrameworkInDet.InDetPlusPseudoTrackParticlesCPContent import InDetPlusPseudoTrackParticlesCPContent
                        items.extend(InDetPlusPseudoTrackParticlesCPContent)
                elif collectionName=="InDetPlusPseudoFromBTrackParticles":
                        from DerivationFrameworkInDet.InDetPlusPseudoFromBTrackParticlesCPContent import InDetPlusPseudoFromBTrackParticlesCPContent
                        items.extend(InDetPlusPseudoFromBTrackParticlesCPContent)
                elif collectionName=="InDetPlusPseudoNotFromBTrackParticles":
                        from DerivationFrameworkInDet.InDetPlusPseudoNotFromBTrackParticlesCPContent import InDetPlusPseudoNotFromBTrackParticlesCPContent
                        items.extend(InDetPlusPseudoNotFromBTrackParticlesCPContent)
                elif collectionName=="InDetNoFakesTrackParticles":
                        from DerivationFrameworkInDet.InDetNoFakesTrackParticlesCPContent import InDetNoFakesTrackParticlesCPContent
                        items.extend(InDetNoFakesTrackParticlesCPContent)
                elif collectionName=="InDetNoFakesFromBTrackParticles":
                        from DerivationFrameworkInDet.InDetNoFakesFromBTrackParticlesCPContent import InDetNoFakesFromBTrackParticlesCPContent
                        items.extend(InDetNoFakesFromBTrackParticlesCPContent)
                elif collectionName=="InDetNoFakesNotFromBTrackParticles":
                        from DerivationFrameworkInDet.InDetNoFakesNotFromBTrackParticlesCPContent import InDetNoFakesNotFromBTrackParticlesCPContent
                        items.extend(InDetNoFakesNotFromBTrackParticlesCPContent)
                elif collectionName=="InDetSiSPSeededTracksParticles":
                        from DerivationFrameworkInDet.InDetSiSPSeededTracksParticlesCPContent import InDetSiSPSeededTracksParticlesCPContent
                        items.extend(InDetSiSPSeededTracksParticlesCPContent)
                elif collectionName=="InDetLargeD0TrackParticles":
                        from DerivationFrameworkInDet.InDetLargeD0TrackParticlesCPContent import InDetLargeD0TrackParticlesCPContent
                        items.extend(InDetLargeD0TrackParticlesCPContent)
                elif collectionName=="PrimaryVertices":
                        from DerivationFrameworkInDet.PrimaryVerticesCPContent import PrimaryVerticesCPContent
                        items.extend(PrimaryVerticesCPContent)
                elif self.IncludeAdditionalTriggerContent is True:
                        from DerivationFrameworkCore.AdditionalTriggerContent import AdditionalTriggerContent
                        items.extend(AdditionalTriggerContent)
                elif collectionName=="HLT_xAOD__MuonContainer_MuonEFInfo":
                        from DerivationFrameworkMuons.MuonTriggerContent import MuonTriggerContent
                        items.extend(MuonTriggerContent)
                elif collectionName=="HLT_xAOD__PhotonContainer_egamma_Photons":
                        from DerivationFrameworkCore.EGammaTriggerContent import EGammaTriggerContent
                        items.extend(EGammaTriggerContent)
                elif collectionName=="HLT_xAOD__JetContainer_a4tcemsubjesFS":
                        from DerivationFrameworkCore.JetTriggerContent import JetTriggerContent
                        items.extend(JetTriggerContent)
                elif collectionName=="HLT_IDVertex_FS":
                        from DerivationFrameworkCore.TrackingTriggerContent import TrackingTriggerContent
                        items.extend(TrackingTriggerContent)
                elif collectionName=="HLT_IDTrack_FS_FTF":
                        from DerivationFrameworkCore.TrackingTriggerContent import TrackingTriggerContent
                        items.extend(TrackingTriggerContent)
                elif collectionName=="HLT_xAOD__TrigMissingETContainer_TrigEFMissingET":
                        from DerivationFrameworkCore.EtMissTriggerContent import EtMissTriggerContent
                        items.extend(EtMissTriggerContent)
                elif collectionName=="HLT_xAOD__TauJetContainer_TrigTauRecMerged":
                        from DerivationFrameworkCore.TauTriggerContent import TauTriggerContent
                        items.extend(TauTriggerContent)
                elif collectionName=="HLT_xAOD__BTaggingContainer_HLTBjetFex":
                        from DerivationFrameworkFlavourTag.BJetTriggerContent import BJetTriggerContent
                        items.extend(BJetTriggerContent)
                elif collectionName=="HLT_xAOD__TrigBphysContainer_EFBMuMuFex":
                        from DerivationFrameworkCore.BPhysTriggerContent import BPhysTriggerContent
                        items.extend(BPhysTriggerContent)
                elif collectionName=="HLT_xAOD__TrigVertexCountsContainer_vertexcounts":
                        from DerivationFrameworkCore.MinBiasTriggerContent import MinBiasTriggerContent
                        items.extend(MinBiasTriggerContent)
                elif collectionName=="HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf":
                        from DerivationFrameworkCore.JetTriggerContentRun3 import JetTriggerContentRun3
                        from DerivationFrameworkCore.JetTriggerContentRun3TLA import JetTriggerContentRun3TLA
                        if not self.OverrideJetTriggerContentWithTLAContent:
                                items.extend(JetTriggerContentRun3)
                        else:
                                items.extend(JetTriggerContentRun3TLA)

                else:
                        raise RuntimeError("Smart slimming container "+collectionName+" does not exist or does not have a smart slimming list")
                return items

        # Kinematics content only
        def GetKinematicsItems(self,collectionName):
                # Content lines in the same style as is produced by the PrintStats
                kinematicsLine = collectionName+"Aux."+"pt.eta.phi.m"
                items = [collectionName,kinematicsLine]
                return items

        # Extra content, listed via container
        def GetExtraItems(self,userInput):
                # Build up a content list in the same style as is produced by the PrintStats
                splitup = userInput.split(".")
                auxContainerName = splitup[0]+"Aux"
                items = []
                items.append(splitup[0])
                auxLine = ""
                length = len(splitup)
                for string in splitup:
                        if string==splitup[0]:
                                auxLine = auxContainerName+"."
                                continue
                        if string==splitup[length-1]:
                                auxLine = auxLine+string
                        else:
                                auxLine = auxLine+string+"."
                items.append(auxLine)
                return items

        # Check that static content is legit
        def ValidateStaticContent(self,item):
                # No wildcards
                if ("*" in item):
                        return "WILDCARD"
                # No xAOD containers
                sep = item.split("#")
                if ("xAOD::" in item and sep[1] in self.NamesAndTypes.keys()):
                        return "XAOD"
                return "OK"
