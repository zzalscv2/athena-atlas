# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Declare the package name:
atlas_subdir( SUSYTools )

# Extra dependencies based on the build environment:
set( extra_libs )
if( NOT XAOD_STANDALONE )
   set( extra_libs GaudiKernel AthAnalysisBaseCompsLib )
endif()

# External dependencies:
find_package( Boost )
find_package( ROOT COMPONENTS Core Tree MathCore Hist RIO )
find_package( GTest )

# Libraries in the package:
atlas_add_library( SUSYToolsLib
   Root/*.cxx
   PUBLIC_HEADERS SUSYTools src
   PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${BOOST_INCLUDE_DIRS}
   LINK_LIBRARIES AsgTools xAODCore xAODEgamma xAODEventInfo
   xAODJet xAODMissingET xAODMuon xAODTau xAODTracking xAODTruth
   AssociationUtilsLib BoostedJetTaggersLib GoodRunsListsLib PATInterfaces TrigDecisionToolLib
   MCTruthClassifierLib JetJvtEfficiencyLib JetSubStructureUtils
   ParticleJetToolsLib PMGAnalysisInterfacesLib AnaAlgorithmLib
   PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} ${BOOST_LIBRARIES}
   AthContainers EventPrimitives FourMomUtils xAODBTagging xAODBase
   xAODPrimitives xAODRootAccess
   EgammaAnalysisInterfacesLib
   EgammaAnalysisHelpersLib FTagAnalysisInterfacesLib IsolationSelectionLib
   IsolationCorrectionsLib JetAnalysisInterfacesLib JetSelectorToolsLib
   JetCPInterfaces JetCalibToolsLib JetInterface
   # JetResolutionLib
   JetUncertaintiesLib JetMomentToolsLib METInterface METUtilitiesLib
   PileupReweightingLib PathResolver TauAnalysisToolsLib
   TriggerAnalysisInterfaces
   TrigGlobalEfficiencyCorrectionLib TriggerMatchingToolLib TrigConfInterfaces TrigConfxAODLib
   xAODTrigMissingET xAODMetaData xAODCutFlow ${extra_libs} )

if( NOT XAOD_STANDALONE )
   atlas_add_component( SUSYTools
      src/*.cxx src/components/*.cxx
      NOCLID
      PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      PRIVATE_LINK_LIBRARIES ${ROOT_LIBRARIES} AthAnalysisBaseCompsLib SUSYToolsLib TauAnalysisToolsLib )
   atlas_add_dictionary( SUSYToolsDict
           SUSYTools/SUSYToolsDict.h
           SUSYTools/selection.xml
           LINK_LIBRARIES SUSYToolsLib )
else()
   atlas_add_library( SUSYTools src/SUSYToolsAlg.cxx src/SUSYToolsAlg.h
           NO_PUBLIC_HEADERS
           PRIVATE_INCLUDE_DIRS ${ROOT_INCLUDE_DIRS} ${BOOST_INCLUDE_DIRS}
           PRIVATE_LINK_LIBRARIES SUSYToolsLib TauAnalysisToolsLib
           )
   atlas_add_dictionary( SUSYToolsDict
           SUSYTools/SUSYToolsDict.h
           SUSYTools/selection.xml
           LINK_LIBRARIES SUSYToolsLib SUSYTools )
endif()



# Executable(s) in the package:
set( extra_libs )
if( NOT XAOD_STANDALONE )
   set( extra_libs POOLRootAccessLib )
endif()

if( XAOD_STANDALONE )
   atlas_add_executable( SUSYToolsTester
      util/SUSYToolsTester.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess xAODEventInfo xAODMuon
      xAODEgamma xAODTau xAODTruth xAODJet xAODCaloEvent xAODCore xAODMissingET
      FTagAnalysisInterfacesLib xAODBase TauAnalysisToolsLib GoodRunsListsLib
      PileupReweightingLib PATInterfaces PathResolver METUtilitiesLib xAODCutFlow
      SUSYToolsLib ${extra_libs} )

   atlas_add_executable( SUSYTruthTester
      util/SUSYTruthTester.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess xAODEventInfo
      xAODTruth xAODCore xAODBase PATInterfaces PathResolver xAODCutFlow
      SUSYToolsLib ${extra_libs} )

   atlas_add_executable( SUSYTools_check_xsections
      util/check_xsections.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} SUSYToolsLib )

   atlas_add_executable( SUSYTools_check_syst
      util/check_syst.cxx
      INCLUDE_DIRS ${ROOT_INCLUDE_DIRS}
      LINK_LIBRARIES ${ROOT_LIBRARIES} xAODRootAccess PATInterfaces
      SUSYToolsLib )
endif()

# Log ignore patterns
set (extra_patterns "  (Entire loop|Excluding first event|First event).*ms|cvmfs/atlas.*\.cern\.ch/repo|INFO|${ATLAS_PLATFORM}/data/SUSYTools|build(/Analysis.*)?/${ATLAS_PLATFORM}|mySTdefs_conf\.tmp|^Ran [0-9]+ tests in [0-9.]+s|create data set info.*|Booked|<HEADER>|Warning in <TClass::Init>|(start|done) processing event|data/JetMomentTools")

# Test(s) in the package:
if( XAOD_STANDALONE )
   atlas_add_test( ut_SUSYToolsTester_data
      SCRIPT SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/DAOD_PHYS.data18_13TeV.00356250_p5511.pool.root
             maxEvents=500 isData=1 isAtlfast=0 Debug=0
      LOG_IGNORE_PATTERN ${extra_patterns}
   )
   atlas_add_test( ut_SUSYToolsTester_data_Run3
      SCRIPT SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/DAOD_PHYS.data22_13p6TeV.00440543_p5514.pool.root
            maxEvents=1000 isData=1 isAtlfast=0 Debug=0
      LOG_IGNORE_PATTERN ${extra_patterns}
   )
   atlas_add_test( ut_SUSYToolsTester_mc
      SCRIPT SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/DAOD_PHYS.mc20_13TeV.410470.FS_mc20e_p5511.PHYS.pool.root
             maxEvents=100 isData=0 isAtlfast=0 Debug=0 NoSyst=0
             PRWFile=dev/PileupReweighting/mc16_13TeV/pileup_mc16e_dsid410470_FS.root
             ilumicalcFile=GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root
      LOG_IGNORE_PATTERN ${extra_patterns}
   )
   atlas_add_test( ut_SUSYToolsTester_mc_Run3
      SCRIPT SUSYToolsTester /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SUSYTools/DAOD_PHYS.mc21_13p6TeV.601229.FS_mc21a_p5511.PHYS.pool.root
             maxEvents=100 isData=0 isAtlfast=0 Debug=0 NoSyst=0
             PRWFile=dev/PileupReweighting/mc16_13TeV/pileup_mc21a_dsid601229_FS.root
             ilumicalcFile=GoodRunsLists/data22_13p6TeV/20230116/ilumicalc_histograms_None_431810-440613_OflLumi-Run3-002.root 
      LOG_IGNORE_PATTERN ${extra_patterns}
   )
else()
   atlas_add_test( ut_SUSYToolsAlg_data
      SCRIPT "athena SUSYTools/jobOptions.py --evtMax 500 - --testCampaign data18"
      PROPERTIES TIMEOUT 300
      LOG_IGNORE_PATTERN ${extra_patterns}
   )
   atlas_add_test( ut_SUSYToolsAlg_data_Run3
      SCRIPT "athena SUSYTools/jobOptions.py --evtMax 500 - --testCampaign data22"
      PROPERTIES TIMEOUT 300
      LOG_IGNORE_PATTERN ${extra_patterns}
   )
   atlas_add_test( ut_SUSYToolsAlg_mc
      SCRIPT "athena SUSYTools/jobOptions.py --evtMax 100 - --testCampaign mc20e --prwFiles dev/PileupReweighting/share/DSID410xxx/pileup_mc16e_dsid410470_FS.root --lumicalcFiles GoodRunsLists/data18_13TeV/20190318/ilumicalc_histograms_None_348885-364292_OflLumi-13TeV-010.root"
      PROPERTIES TIMEOUT 300
      LOG_IGNORE_PATTERN ${extra_patterns}
   )
   atlas_add_test( ut_SUSYToolsAlg_mc_Run3
      SCRIPT "athena SUSYTools/jobOptions.py --evtMax 100 - --testCampaign mc21a --prwFiles dev/PileupReweighting/share/DSID601xxx/pileup_mc21a_dsid601229_FS.root --lumicalcFiles GoodRunsLists/data22_13p6TeV/20230116/ilumicalc_histograms_None_431810-440613_OflLumi-Run3-002.root"
      PROPERTIES TIMEOUT 300
      LOG_IGNORE_PATTERN ${extra_patterns}
   )
endif()

# Install files from the package:
atlas_install_python_modules( python/*.py )
atlas_install_joboptions( share/*.py )
atlas_install_data( data/* )
atlas_install_scripts( scripts/TestSUSYToolsAlg.py )
