# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

# JetRecFlags.py
#
# These are flags for controlling the behavior of jet reconstruction
# in RecExCommon (which includes JetRec/JetRec_jobOptions.py) and in
# the example jet reconstruction options RunJetRec.py.
#
# Typical usage is
#   from JetRec.JetRecFlags import jetFlags
#   if jetFlags.UseTruth:
#     doSomethingWithTruth()
#
# Properties:
#   Enabled - Skip all jet reco if false (drop whe rec.doJets is added)
#   debug - JetRecTools are run a DEBUG level
#   useTruth - Truth jets and association are enabled (for MC)
#   useTopo  - Topocluster jets are enabled
#   useTracks - Track jets and association are enabled
#   useMuonSegmentss - Muon segemnt association is enabled
#   usePFlow - Pflow jets and associations are enabled
#   useInDetTrackSelection - The inner detector track selection
#     tool is used. This requires track propagator exist.
#   jetAODList - The list of jet collections to be written out

from AthenaCommon.JobProperties import JobProperty, JobPropertyContainer
from AthenaCommon.JobProperties import jobproperties



class JetRecFlags(JobPropertyContainer):
  """ The Jet making flag property container
  """
  pass

class Enabled(JobProperty):
  """ If false will prevent run of any Jet Alg
  """
  statusOn     = True     
  allowedTypes = ['bool']  # type
  StoredValue  = True      # default value

class debug(JobProperty):
  """ If > 0, debug (or higher) messages are written by jet tools.
  """
  statusOn     = True     
  allowedTypes = ['int']  # type
  StoredValue  = 0        # default value

class useTruth(JobProperty):
  """ If true, truth is present and used in jet reconstruction.
  This flag is special : it's off by default. In this state it just forwards
  the rec.doTruth() flags.
  When it's set it stays to the corresponding value.

  This behaviour avoids problem arising when JetRecFlags module is imported
  or used before useTruth = rec.doTruth() is set. (and setting
  StoredValue = rec.doTruth() isn't enough since the module can be imported
  *before* rec.doTruth is set to the correct value)
  """
  statusOn     = False     
  allowedTypes = ['bool']  # type
  StoredValue  = True # this irrelevant  

  def get_Value(self):
      if self.statusOn:
          return JobProperty.get_Value(self)
      else:
          from RecExConfig.RecFlags import rec
          return rec.doTruth()
  

class truthFlavorTags(JobProperty):
  """ List of flavor tags for truth tagging jets.
  """
  statusOn     = True     
  allowedTypes = ['array']  # type
  StoredValue  = ["BHadronsInitial", "BHadronsFinal", "BQuarksFinal",
                  "CHadronsInitial", "CHadronsFinal", "CQuarksFinal",
                  "TausFinal"]
  
class useTopo(JobProperty):
  """ If true, topoclusters are present and used in jet reconstruction.
  """
  statusOn     = True     
  allowedTypes = ['bool']  # type
  StoredValue  = True      # default value

class useTracks(JobProperty):
  """ If true, tracks and vertices are present and used in jet reconstruction.
  """
  statusOn     = True     
  allowedTypes = ['bool']  # type
  StoredValue  = True      # default value

class useMuonSegments(JobProperty):
  """ If true, muon segments are present and used in jet reconstruction.
  """
  statusOn     = True     
  allowedTypes = ['bool']  # type
  StoredValue  = True      # default value

class usePflow(JobProperty):
  """ If true, pflow objects are present and used in jet reconstruction.
  """
  statusOn     = True     
  allowedTypes = ['bool']  # type
  StoredValue  = True      # default value

class eventShapeTools(JobProperty):
  """ List of event shape tools that should be called to calculate rho.
      Allowed values are "em" and "lc".
  """
  statusOn     = True     
  allowedTypes = ['list']  # type
  StoredValue  = ['em', 'lc']        # default value

class useInDetTrackSelection(JobProperty):
  """ If true, the InDet track selection tool is used.
  """
  statusOn     = True     
  allowedTypes = ['bool']  # type
  StoredValue  = False     # default value

class useCells(JobProperty):
  """ If true, calo cells are accesible
  """
  statusOn     = True     
  allowedTypes = ['bool']  # type
  StoredValue  = False     # default value

class useCaloQualityTool(JobProperty):
  """ If true, the (slow) CaloQuality tool is used
  """
  statusOn     = True     
  allowedTypes = ['bool']  # type
  StoredValue  = True      # default value

class skipTools(JobProperty):
  """ List of modifier tools to exclude
  """
  statusOn     = True     
  allowedTypes = ['list']  # type
  StoredValue  = []        # default value

class additionalTopoGetters(JobProperty):
  """ List of PseudoJet getters to add for Topo jets.
      E.g. to tag jets with track jets
  """
  statusOn     = True     
  allowedTypes = ['list']  # type
  StoredValue  = []        # default value

class defaultCalibOpt(JobProperty):
  """ Calibration applied to topo jets during jet building. See JetRecCalibrationFinder.
  """
  statusOn     = True     
  allowedTypes = ['str']    # type
  StoredValue  = ""   # default value

class containerNamePrefix(JobProperty):
  """ Prefix for jet collection names
  """
  statusOn     = True     
  allowedTypes = ['str']    # type
  StoredValue  = ""         # default value

class separateJetAlgs(JobProperty):
  """ If true, find and build jet containers in separate alg. Used for debugging.
  """
  statusOn     = True     
  allowedTypes = ['bool']  # type
  StoredValue  = False      # default value

class timeJetToolRunner(JobProperty):
  """ Timing flag for JetToolRunner: 0 for no timing, 1 for some, 2 for detailed
  """
  statusOn     = True     
  allowedTypes = ['int']  # type
  StoredValue  = 0        # default value

class timeJetRecTool(JobProperty):
  """ Timing flag for JetRecTool: 0 for no timing, 1 for some, 2 for detailed
  """
  statusOn     = True     
  allowedTypes = ['int']  # type
  StoredValue  = 0        # default value

class jetAODList(JobProperty):
  """ The collections to be saved in (x)AOD files
  """
  statusOn = True
  allowedTypes = ['list']  
  StoredValue  = []   

jobproperties.add_Container(JetRecFlags)

jobproperties.JetRecFlags.add_JobProperty(Enabled)
jobproperties.JetRecFlags.add_JobProperty(debug)
jobproperties.JetRecFlags.add_JobProperty(useTruth)
jobproperties.JetRecFlags.add_JobProperty(truthFlavorTags)
jobproperties.JetRecFlags.add_JobProperty(useTopo)
jobproperties.JetRecFlags.add_JobProperty(useTracks)
jobproperties.JetRecFlags.add_JobProperty(useInDetTrackSelection)
jobproperties.JetRecFlags.add_JobProperty(useMuonSegments)
jobproperties.JetRecFlags.add_JobProperty(usePflow)
jobproperties.JetRecFlags.add_JobProperty(eventShapeTools)
jobproperties.JetRecFlags.add_JobProperty(jetAODList)
jobproperties.JetRecFlags.add_JobProperty(useCells)
jobproperties.JetRecFlags.add_JobProperty(useCaloQualityTool)
jobproperties.JetRecFlags.add_JobProperty(skipTools)
jobproperties.JetRecFlags.add_JobProperty(additionalTopoGetters)
jobproperties.JetRecFlags.add_JobProperty(defaultCalibOpt)
jobproperties.JetRecFlags.add_JobProperty(containerNamePrefix)
jobproperties.JetRecFlags.add_JobProperty(separateJetAlgs)
jobproperties.JetRecFlags.add_JobProperty(timeJetToolRunner)
jobproperties.JetRecFlags.add_JobProperty(timeJetRecTool)

jetFlags = jobproperties.JetRecFlags
