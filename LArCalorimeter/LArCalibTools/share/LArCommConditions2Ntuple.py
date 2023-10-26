import AthenaCommon.AtlasUnixGeneratorJob #use MC event selector
## get a handle to the default top-level algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence 
topSequence = AlgSequence()  


#Input Parameters:
# PoolFiles: sequence of pool files to read from though CondProxyProvider
# if not given, read from COOL
#
# RunNumber: Input to COOL IOV-DB if reading from 
#
# RootFile: root file for the ntuple
#
# Objects: List of objects written to ntuple (PEDESTAL OFC, RAMP, 

if not 'InputDB' in dir():
  InputDB="COOLONL_LAR/CONDBR2"

if not "OFCFolder" in dir():
  OFCFolder="5samples1phase"

if not 'RunNumber' in dir():
  RunNumber=2147483647

if not "RootFile" in dir():
  RootFile="LArConditions.root"

if not "Objects" in dir():
  Objects=["PEDESTAL","RAMP","OFC","MPHYSOVERMCAL"]
    
if not "SuperCells" in dir():
  SuperCells=False

# For shape MC-Data are different
if not "IsMC" in dir():
   IsMC=False

if not "IsFlat" in dir():
   IsFlat=True

if not "OffIdDump" in dir():
   OffIdDump=False

if not "HashDump" in dir():
   HashDump=False

if not "DBTag" in dir():
  if "OFLP" in InputDB:
     DBTag="OFLCOND-RUN1-SDR-06"
  else: 
     DBTag="LARCALIB-RUN2-00"

if not "TagSuffix" in dir():
  if SuperCells: # no linking to global tag yet
    TagSuffix="-000"

def doObj(objName):
  for o in Objects:
    if o.upper().find(objName.upper())!=-1:
      return True
  return False

def getDBFolderAndTag(folder):
  if ("TagSuffix" in globals()) and (TagSuffix != ""):
    tag="<tag>"+"".join(folder.split('/')) + TagSuffix+"</tag>"
  else:
    tag=""
  return "<db>"+InputDB+"</db>"+folder+tag

from AthenaCommon.GlobalFlags import  globalflags
if IsMC:
  globalflags.DataSource="geant4"
  globalflags.InputFormat="pool"
else:
  globalflags.DataSource="data"
  globalflags.InputFormat="bytestream"
  globalflags.DatabaseInstance="CONDBR2"

from AthenaCommon.JobProperties import jobproperties
jobproperties.Global.DetDescrVersion = "ATLAS-R2-2015-04-00-00"

from AthenaCommon.DetFlags import DetFlags
DetFlags.Calo_setOff()
DetFlags.ID_setOff()
DetFlags.Muon_setOff()
DetFlags.Truth_setOff()
DetFlags.LVL1_setOff()
DetFlags.digitize.all_setOff()

#Set up GeoModel (not really needed but crashes without)
from AtlasGeoModel import SetGeometryVersion
from AtlasGeoModel import GeoModelInit 

from LArConditionsCommon import LArAlignable #noqa F401

#Get identifier mapping (needed by LArConditionsContainer)
svcMgr.IOVDbSvc.GlobalTag=DBTag
if IsMC:
   include( "LArConditionsCommon/LArIdMap_MC_jobOptions.py" )
   conddb.addFolder("LAR_OFL","/LAR/BadChannels/BadChannels<tag>LArBadChannelsBadChannels-IOVDEP-06</tag>",className="CondAttrListCollection")
else:
   include( "LArConditionsCommon/LArIdMap_comm_jobOptions.py" )
   conddb.addFolder("LAR_OFL","/LAR/BadChannelsOfl/BadChannels<key>/LAR/BadChannels/BadChannels</key>",className="CondAttrListCollection")


from LArBadChannelTool.LArBadChannelToolConf import LArBadChannelCondAlg
theLArBadChannelCondAlg=LArBadChannelCondAlg()
theLArBadChannelCondAlg.ReadKey="/LAR/BadChannels/BadChannels"
condSeq+=theLArBadChannelCondAlg

theApp.EvtMax = 1
svcMgr.EventSelector.RunNumber = RunNumber

if SuperCells:   
   from LArCabling.LArCablingAccess import LArCalibIdMappingSC,LArOnOffIdMappingSC
   LArOnOffIdMappingSC()
   LArCalibIdMappingSC()

if 'PoolFiles' in dir():
  from AthenaCommon.ConfigurableDb import getConfigurable
  from AthenaCommon.AppMgr import ServiceMgr
  ServiceMgr.ProxyProviderSvc.ProviderNames += [ "CondProxyProvider" ]
  ServiceMgr += getConfigurable( "CondProxyProvider" )()
  svcMgr.CondProxyProvider.InputCollections=PoolFiles

if 'PoolCat' in dir():
  svcMgr.PoolSvc.ReadCatalog+=["xmlcatalog_file:"+PoolCat]

loadCastorCat=True
  

if doObj("PEDESTAL"):
  from LArCalibTools.LArCalibToolsConf import LArPedestals2Ntuple
  LArPedestals2Ntuple=LArPedestals2Ntuple("LArPedestals2Ntuple")
  LArPedestals2Ntuple.AddFEBTempInfo=False
  LArPedestals2Ntuple.OffId=OffIdDump
  if IsMC: 
    if SuperCells:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMCSC/Pedestal"))
      LArPedestals2Ntuple.ContainerKey="LArPedestalSC"
    else:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMC/Pedestal"))
      LArPedestals2Ntuple.ContainerKey="LArPedestal"
  elif IsFlat:    
    from AthenaCommon.AlgSequence import AthSequencer
    condSequence = AthSequencer("AthCondSeq")
    if SuperCells:
       from LArRecUtils.LArRecUtilsConf import LArFlatConditionsAlg_LArPedestalSC_ as LArPedCondAlg 
       folder="/LAR/ElecCalibFlatSC/Pedestal"
    else:
       from LArRecUtils.LArRecUtilsConf import LArFlatConditionsAlg_LArPedestalFlat_ as LArPedCondAlg 
       folder="/LAR/ElecCalibFlat/Pedestal"
    conddb.addFolder("",getDBFolderAndTag(folder),className = 'CondAttrListCollection')
    LArPedestals2Ntuple.ContainerKey="Pedestal"
    condSequence += LArPedCondAlg(ReadKey=folder, WriteKey='Pedestal')
  else:   
    conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/Pedestals/Pedestal"))
    LArPedestals2Ntuple.ContainerKey="Pedestal"
  LArPedestals2Ntuple.isSC=SuperCells 
  LArPedestals2Ntuple.isFlat=IsFlat 
  LArPedestals2Ntuple.OutputLevel=WARNING
  topSequence+=LArPedestals2Ntuple


if doObj("AUTOCORR"):
  from LArCalibTools.LArCalibToolsConf import LArAutoCorr2Ntuple
  LArAutoCorr2Ntuple=LArAutoCorr2Ntuple("LArAutoCorr2Ntuple")
  LArAutoCorr2Ntuple.AddFEBTempInfo=False
  LArAutoCorr2Ntuple.OffId=OffIdDump
  if IsMC: 
    if SuperCells:
       conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMCSC/AutoCorr"))
       LArAutoCorr2Ntuple.ContainerKey="LArAutoCorrSC"
    else:
       conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMC/AutoCorr"))
       LArAutoCorr2Ntuple.ContainerKey="LArAutoCorr"
  elif IsFlat:
       print( 'No Flat Autocorr exists !!!')
       import sys; sys.exit(-1) 
  else:
    if SuperCells:
       conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOflSC/AutoCorrs/AutoCorr"),className="LArAutoCorrComplete")
    else:   
       conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/AutoCorrs/AutoCorr"),className="LArAutoCorrComplete")
    loadCastorCat=True
    LArAutoCorr2Ntuple.ContainerKey="LArAutoCorr"
  LArAutoCorr2Ntuple.isSC=SuperCells
  LArAutoCorr2Ntuple.isFlat=IsFlat  
  topSequence+=LArAutoCorr2Ntuple

if doObj("PHYSAUTOCORR"):
  if SuperCells:
     conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOflSC/AutoCorrs/PhysicsAutoCorr"))
  else:   
     conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/AutoCorrs/PhysicsAutoCorr"))
  from LArCalibTools.LArCalibToolsConf import LArAutoCorr2Ntuple
  LArAutoCorr2Ntuple=LArAutoCorr2Ntuple("LArAutoCorr2Ntuple")
  LArAutoCorr2Ntuple.AddFEBTempInfo=False
  LArAutoCorr2Ntuple.isSC=SuperCells
  LArAutoCorr2Ntuple.isFlat=False  
  topSequence+=LArAutoCorr2Ntuple

if (doObj("OFPhase")):
  if SuperCells:
     conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOflSC/OFCBin/PhysShift"),className="LArOFCBinComplete")
     Ckey="LArSCOFCPhase"
  else: 
     conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/OFCBin/PhysWaveShifts"),className="LArOFCBinComplete")
     Ckey="LArPhysWaveShift"
  print("OFPhase key: ",Ckey)
  from LArCalibTools.LArCalibToolsConf import LArOFCBin2Ntuple
  LArOFCBin2Ntuple=LArOFCBin2Ntuple("LArOFCBin2Ntuple")
  LArOFCBin2Ntuple.ContainerKey=Ckey
  LArOFCBin2Ntuple.AddFEBTempInfo=False
  LArOFCBin2Ntuple.isSC = SuperCells
  LArOFCBin2Ntuple.OffId=OffIdDump
  topSequence+=LArOFCBin2Ntuple

if doObj("OFC"):
  from LArCalibTools.LArCalibToolsConf import LArOFC2Ntuple
  LArOFC2Ntuple = LArOFC2Ntuple("LArOFC2Ntuple")
  LArOFC2Ntuple.AddFEBTempInfo=False
  LArOFC2Ntuple.OffId=OffIdDump
  if IsMC: 
    if SuperCells:
       from LArRecUtils.LArOFCSCCondAlgDefault import LArOFCCondAlgDefault
       ofcAlg = LArOFCSCCondAlgDefault()
    else:   
       from LArRecUtils.LArOFCCondAlgDefault import LArOFCCondAlgDefault
       ofcAlg = LArOFCCondAlgDefault()
    LArOFC2Ntuple.ContainerKey = ofcAlg.LArOFCObjKey

  elif IsFlat:
    from LArRecUtils.LArRecUtilsConf import LArFlatConditionsAlg_LArOFCFlat_ as LArOFCCondAlg 
    from AthenaCommon.AlgSequence import AthSequencer
    condSequence = AthSequencer("AthCondSeq")
    if SuperCells:
       folder = '/LAR/ElecCalibFlatSC/OFC'
    else:   
       folder = '/LAR/ElecCalibFlat/OFC'
    conddb.addFolder('LAR_ONL', getDBFolderAndTag(folder), className = 'CondAttrListCollection')
    condSequence += LArOFCCondAlg  (ReadKey=folder, WriteKey='LArOFC')

  else:
    if SuperCells:
       conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOflSC/OFC/PhysWave/RTM/"+OFCFolder) + '<key>LArOFC</key>',
       #conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOflSC/OFC/CaliWave") + '<key>LArOFC</key>',
                     className='LArOFCComplete')
    else:   
       conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/OFC/PhysWave/RTM/"+OFCFolder) + '<key>LArOFC</key>',
                     className='LArOFCComplete')

  LArOFC2Ntuple.isSC=SuperCells
  LArOFC2Ntuple.isFlat=IsFlat  
  #LArOFC2Ntuple.OutputLevel=VERBOSE
  topSequence+=LArOFC2Ntuple

if (doObj("SHAPE")):
  from LArCalibTools.LArCalibToolsConf import LArShape2Ntuple
  LArShape2Ntuple = LArShape2Ntuple("LArShape2Ntuple")
  if SuperCells:  
     LArShape2Ntuple.ContainerKey = "LArShapeSC"
  else:   
     LArShape2Ntuple.ContainerKey = "LArShape"
  if IsMC: 
    if SuperCells:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMCSC/Shape"))
    else:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMC/Shape"))
  elif IsFlat: 
    conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibFlat/Shape"))
  else:  
    conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/Shape/RTM/"+OFCFolder))
    LArShape2Ntuple.isComplete=True
  pass  
  if SuperCells:  
     LArShape2Ntuple.ContainerKey = "LArShapeSC"
  else:   
     LArShape2Ntuple.ContainerKey = "LArShape"
  LArShape2Ntuple.AddFEBTempInfo=False
  LArShape2Ntuple.isSC = SuperCells
  LArShape2Ntuple.isFlat = IsFlat  
  LArShape2Ntuple.OffId=OffIdDump
  topSequence+=LArShape2Ntuple

if doObj("RAMP"):
  from LArCalibTools.LArCalibToolsConf import LArRamps2Ntuple
  LArRamps2Ntuple=LArRamps2Ntuple("LArRamps2Ntuple")
  LArRamps2Ntuple.NtupleName = "RAMPS"
  LArRamps2Ntuple.OffId=OffIdDump
  if IsMC: 
    if SuperCells:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMCSC/Ramp"))
      LArRamps2Ntuple.RampKey="LArRampSC"
    else:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMC/Ramp"))
  elif IsFlat: 
    conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibFlat/Ramp"))
  else:  
    if SuperCells:
       conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOflSC/Ramps/RampLinea"))
    else:   
       conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/Ramps/RampLinea"))
  LArRamps2Ntuple.RawRamp = False
  LArRamps2Ntuple.AddFEBTempInfo=False
  LArRamps2Ntuple.isSC = SuperCells
  LArRamps2Ntuple.isFlat = IsFlat  
  topSequence+=LArRamps2Ntuple

if (doObj("UA2MEV")):
  from LArCalibTools.LArCalibToolsConf import LAruA2MeV2Ntuple
  LAruA2MeV2Ntuple=LAruA2MeV2Ntuple("LAruA2MeV2Ntuple")
  LAruA2MeV2Ntuple.AddFEBTempInfo=False
  LAruA2MeV2Ntuple.isSC = SuperCells
  LAruA2MeV2Ntuple.isFlat = IsFlat
  LAruA2MeV2Ntuple.OffId=OffIdDump
  LAruA2MeV2Ntuple.AddHash=HashDump
  if IsMC: 
    if SuperCells:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMCSC/DAC2uA"))
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMCSC/uA2MeV"))
    else:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMC/DAC2uA"))
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMC/uA2MeV"))
  elif IsFlat: 
    conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibFlat/DAC2uA"))
    conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibFlat/uA2MeV"))
  else:  
    conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/uA2MeV/Symmetry"))
    LAruA2MeV2Ntuple.DAC2uAKey=""
  topSequence+=LAruA2MeV2Ntuple

if (doObj("MPHYSOVERMCAL")):
  if IsMC: 
    if SuperCells:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMCSC/MphysOverMcal"))
      print( 'Not MPHYSOVERMCAL fo SuperCells yet !!' )
      import sys; sys.exit(-2)
    else:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMC/MphysOverMcal"))
  elif IsFlat: 
    conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibFlat/MphysOverMcal"))
  else:  
    conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/MphysOverMcal/RTM"))
  from LArCalibTools.LArCalibToolsConf import LArMphysOverMcal2Ntuple
  LArMphysOverMcal2Ntuple=LArMphysOverMcal2Ntuple("LArMphysOverMcal2Ntuple")
  LArMphysOverMcal2Ntuple.AddFEBTempInfo=False
  LArMphysOverMcal2Ntuple.isSC = SuperCells
  LArMphysOverMcal2Ntuple.isFlat = IsFlat  
  LArMphysOverMcal2Ntuple.OffId=OffIdDump
  topSequence+=LArMphysOverMcal2Ntuple

if (doObj("CALIWAVE")):
  if IsMC: 
    if SuperCells:
      print( 'No CALIWAVE for SuperCells yet !!!')
      import sys; sys.exit(-3)
    else:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMC/CaliWave"))
  elif IsFlat: 
    print( 'No Flat CALIWAVE !!!')
    import sys; sys.exit(-3)
  else:  
    loadCastorCat=True
    #conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/CaliWaves/CaliWave"))
    conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/CaliWaves/CaliWaveXtalkCorr"))  
  from  LArCalibTools.LArCalibToolsConf import LArCaliWaves2Ntuple
  LArCaliWaves2Ntuple=LArCaliWaves2Ntuple("LArCaliWaves2Ntuple")
  LArCaliWaves2Ntuple.NtupleName   = "CALIWAVE"
  LArCaliWaves2Ntuple.KeyList      = [ 'LArCaliWave' ]
  LArCaliWaves2Ntuple.SaveDerivedInfo = True
  LArCaliWaves2Ntuple.SaveJitter      = True
  LArCaliWaves2Ntuple.AddFEBTempInfo=False
  LArCaliWaves2Ntuple.isSC = SuperCells
  LArCaliWaves2Ntuple.isFlat = IsFlat
  LArCaliWaves2Ntuple.OffId=OffIdDump
  topSequence+=LArCaliWaves2Ntuple

if (doObj("WFPARAMS")):
  loadCastorCat=True
  conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/DetCellParams/RTM"))
  conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/CaliPulseParams/RTM"))
  conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/Tdrift/Computed"))
  #conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/PhysCaliTdiff"))
  conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/OFCBin/PhysWaveShifts"))
  from  LArCalibTools.LArCalibToolsConf import LArWFParams2Ntuple
  LArWFParams2Ntuple=LArWFParams2Ntuple("LArWFParams2Ntuple")
  LArWFParams2Ntuple.DumpCaliPulseParams=True
  LArWFParams2Ntuple.DumpDetCellParams=True
  LArWFParams2Ntuple.DumpPhysCaliTdiff=False
  LArWFParams2Ntuple.DumpTdrift=True
  LArWFParams2Ntuple.DumpOFCBin=True
  LArWFParams2Ntuple.CaliPulseParamsKey="LArCaliPulseParams_RTM"
  LArWFParams2Ntuple.DetCellParamsKey="LArDetCellParams_RTM"
  LArWFParams2Ntuple.PhysCaliTDiffKey="LArPhysCaliTdiff"
  #LArWFParams2Ntuple.OFCBinKey="LArOFCPhase"
  LArWFParams2Ntuple.OFCBinKey="LArPhysWaveShift"
  #LArWFParams2Ntuple.DetStoreSuffix="_RTM"
  LArWFParams2Ntuple.OffId=OffIdDump
  #LArWFParams2Ntuple.OutputLevel=DEBUG
  topSequence+=LArWFParams2Ntuple

if (doObj("NOISE")):
  if IsMC: 
    if SuperCells:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMCSC/Noise"))
    else:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMC/Noise"))
  elif IsFlat: 
    #conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibFlat/Noise"))
    print( 'No Flat LArNoise yet !!!')
    import sys; sys.exit(-5) 
  else:  
    print( 'For Cell noise use the CaloNoise2Ntuple algo !')
    import sys; sys.exit(-5)
  pass
  from LArCalibTools.LArCalibToolsConf import LArNoise2Ntuple
  LArNoise2Ntuple=LArNoise2Ntuple("LArNoise2Ntuple")
  LArNoise2Ntuple.AddFEBTempInfo=False
  LArNoise2Ntuple.isSC = SuperCells
  LArNoise2Ntuple.isFlat = IsFlat  
  LArNoise2Ntuple.OffId=OffIdDump
  topSequence+=LArNoise2Ntuple

if (doObj("FSAMPL")):
  if IsMC: 
    if SuperCells:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMCSC/fSampl"))
    else:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMC/fSampl"))
  elif IsFlat: 
    #conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibFlat/fSampl"))
    print( 'No Flat LArfSampl yet !!!')
    import sys; sys.exit(-5) 
  else:  
    conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/fSampl/Symmetry"))
  from LArCalibTools.LArCalibToolsConf import LArfSampl2Ntuple
  LArfSampl2Ntuple=LArfSampl2Ntuple("LArfSampl2Ntuple")
  LArfSampl2Ntuple.AddFEBTempInfo=False
  LArfSampl2Ntuple.isSC = SuperCells
  LArfSampl2Ntuple.isFlat = IsFlat  
  LArfSampl2Ntuple.OffId=OffIdDump
  LArfSampl2Ntuple.AddHash=HashDump
  topSequence+=LArfSampl2Ntuple


if doObj("HVSCALE"):
  if IsMC: 
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMC/HVScaleCorr"))
  else: 
    if IsFlat:
      from AthenaCommon.AlgSequence import AthSequencer
      condSequence = AthSequencer("AthCondSeq")
      if SuperCells:
         folder="/LAR/ElecCalibFlatSC/HVScaleCorr"
         from LArRecUtils.LArRecUtilsConf import LArFlatConditionsAlg_LArHVScaleCorrSC_ as LArHVCondAlg 
      else:   
         folder="/LAR/ElecCalibFlat/HVScaleCorr"
         from LArRecUtils.LArRecUtilsConf import LArFlatConditionsAlg_LArHVScaleCorrFlat_ as LArHVCondAlg 
      conddb.addFolder("",getDBFolderAndTag(folder),className = 'CondAttrListCollection')
      condSequence += LArHVCondAlg(ReadKey=folder, WriteKey='HVScaleCorr')
    else:
      print( 'Only Flat HVSCALE !!!')
      import sys; sys.exit(-5) 


  from LArCalibTools.LArCalibToolsConf import LArHVScaleCorr2Ntuple
  theLArHVScaleCorr2Ntuple = LArHVScaleCorr2Ntuple("LArHVScaleCorr2Ntuple")
  theLArHVScaleCorr2Ntuple.AddFEBTempInfo = False
  theLArHVScaleCorr2Ntuple.isSC = SuperCells
  theLArHVScaleCorr2Ntuple.isFlat = IsFlat
  theLArHVScaleCorr2Ntuple.ContainerKey = 'HVScaleCorr'
  topSequence += theLArHVScaleCorr2Ntuple
    
if (doObj("MINBIAS")):
  if IsMC: 
    if SuperCells:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMCSC/MinBias"))
    else:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMC/MinBias"))
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMC/MinBiasAverage"))
  elif IsFlat: 
    #conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibFlat/Noise"))
    print( 'No Flat LArMinBias yet !!!')
    import sys; sys.exit(-5) 
  from LArCalibTools.LArCalibToolsConf import LArMinBias2Ntuple
  LArMinBias2Ntuple=LArMinBias2Ntuple("LArMinBias2Ntuple")
  LArMinBias2Ntuple.AddFEBTempInfo=False
  topSequence+=LArMinBias2Ntuple

if (doObj("PILEUP")):
  if IsMC: 
    if SuperCells:
      print( 'No LArPileup for SC yet !!!')
      import sys; sys.exit(-5) 
    else:
      conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibMC/LArPileupAverage"))
  elif IsFlat: 
    #conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibFlat/Noise"))
    print( 'No Flat LArPileup yet !!!')
    import sys; sys.exit(-5) 
  else:  
    conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/LArPileupAverage"))
  from LArCalibTools.LArCalibToolsConf import LArMinBias2Ntuple
  LArPileup2Ntuple=LArMinBias2Ntuple("LArPileup2Ntuple")
  LArPileup2Ntuple.AddFEBTempInfo=False
  LArPileup2Ntuple.ContainerKey="LArPileup"
  LArPileup2Ntuple.NtupleName="/NTUPLES/FILE1/PILEUP"
  topSequence+=LArPileup2Ntuple

if (doObj("RINJ")):
  conddb.addFolder("",getDBFolderAndTag("/LAR/ElecCalibOfl/HecPAMap"))
  from LArCalibTools.LArCalibToolsConf import LArRinj2Ntuple
  LArRinj2Ntuple=LArRinj2Ntuple("LArRinj2Ntuple")
  LArRinj2Ntuple.AddFEBTempInfo=False
  LArRinj2Ntuple.isSC = False
  LArRinj2Ntuple.OffId=OffIdDump
  topSequence+=LArRinj2Ntuple

if loadCastorCat:
  svcMgr.PoolSvc.ReadCatalog += ['xmlcatalog_file:'+'/afs/cern.ch/atlas/conditions/poolcond/catalogue/poolcond/PoolCat_comcond_castor.xml']


theApp.HistogramPersistency = "ROOT"
from GaudiSvc.GaudiSvcConf import NTupleSvc
svcMgr += NTupleSvc()
svcMgr.NTupleSvc.Output = [ "FILE1 DATAFILE='"+RootFile+"' OPT='NEW'" ]

#svcMgr.DetectorStore.Dump=True
#svcMgr.MessageSvc.OutputLevel = DEBUG
svcMgr.MessageSvc.Format = "% F%50W%C%8W%R%T %0W%M"
#svcMgr.MessageSvc.defaultLimit = 99999999

svcMgr.IOVDbSvc.DBInstance=""
