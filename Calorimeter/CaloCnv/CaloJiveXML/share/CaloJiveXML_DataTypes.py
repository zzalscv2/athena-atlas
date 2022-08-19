#Avoid multiple includes
include.block("CaloJiveXML/CaloJiveXML_DataTypes.py")

## Include the base options if the user didn't already do that
if not "theEventData2XML" in dir():
    include ("JiveXML/JiveXML_jobOptionBase.py")

from LArRecUtils.LArADC2MeVCondAlgDefault import LArADC2MeVCondAlgDefault
LArADC2MeVCondAlgDefault()

tileDigitsContainer = ""
tileRawChannelContainer = ""

from AthenaConfiguration.Enums import Format
if ConfigFlags.Input.Format is Format.BS:
   tileDigitsContainer = "TileDigitsCnt"

   if ConfigFlags.Tile.doOpt2:
       tileRawChannelContainer = 'TileRawChannelOpt2'
   elif ConfigFlags.Tile.doOptATLAS:
       tileRawChannelContainer = 'TileRawChannelFixed'
   elif ConfigFlags.Tile.doFitCOOL:
       tileRawChannelContainer = 'TileRawChannelFitCool'
   elif ConfigFlags.Tile.doFit:
       tileRawChannelContainer = 'TileRawChannelFit'
   else:
       tileRawChannelContainer = 'TileRawChannelCnt'

else:
   if "TileDigitsCnt" in ConfigFlags.Input.Collections:
       tileDigitsContainer = "TileDigitsCnt"
   elif "TileDigitsFlt" in ConfigFlags.Input.Collections:
       tileDigitsContainer = "TileDigitsFlt"

   if "TileRawChannelOpt2" in ConfigFlags.Input.Collections:
       tileRawChannelContainer = 'TileRawChannelOpt2'
   elif "TileRawChannelFitCool" in ConfigFlags.Input.Collections:
       tileRawChannelContainer = 'TileRawChannelFitCool'
   elif "TileRawChannelFit" in ConfigFlags.Input.Collections:
       tileRawChannelContainer = 'TileRawChannelFit'
   elif "TileRawChannelCnt" in ConfigFlags.Input.Collections:
       tileRawChannelContainer = 'TileRawChannelCnt'


## example how to switch on writting-out of HLT collections
## and select favourite and other collections 

from CaloJiveXML.CaloJiveXMLConf import JiveXML__CaloClusterRetriever
theCaloClusterRetriever = JiveXML__CaloClusterRetriever (name = "CaloClusterRetriever")

## Note: if 'Other Collection' list is given, this flag is ignored
#theCaloClusterRetriever.DoWriteHLT = True
## Default collection (most Electron have elementLink to this one):

theCaloClusterRetriever.FavouriteClusterCollection="egammaTopoClusters"

## example how to set other collection. when commented out: all other, non-HLT
##
theCaloClusterRetriever.OtherClusterCollections=["CombinedCluster","MuonClusterCollection","CaloTopoClusters"]

# See M5 jOs for further commissioning options
#
from CaloJiveXML.CaloJiveXMLConf import JiveXML__CaloTileRetriever
theCaloTileRetriever = JiveXML__CaloTileRetriever (name = "CaloTileRetriever")
theCaloTileRetriever.TileDigitsContainer = tileDigitsContainer
theCaloTileRetriever.TileRawChannelContainer = tileRawChannelContainer
theCaloTileRetriever.DoTileCellDetails = False
theCaloTileRetriever.DoTileDigit = False
theCaloTileRetriever.DoBadTile = False

from CaloJiveXML.CaloJiveXMLConf import JiveXML__CaloMBTSRetriever
theCaloMBTSRetriever = JiveXML__CaloMBTSRetriever (name = "CaloMBTSRetriever")
theCaloMBTSRetriever.TileDigitsContainer = tileDigitsContainer
theCaloMBTSRetriever.TileRawChannelContainer = tileRawChannelContainer
theCaloMBTSRetriever.DoMBTSDigits = False

from CaloJiveXML.CaloJiveXMLConf import JiveXML__CaloFCalRetriever
theCaloFCalRetriever = JiveXML__CaloFCalRetriever (name = "CaloFCalRetriever")
theCaloFCalRetriever.DoFCalCellDetails = False
theCaloFCalRetriever.DoBadFCal = False

from CaloJiveXML.CaloJiveXMLConf import JiveXML__CaloLArRetriever
theCaloLArRetriever = JiveXML__CaloLArRetriever (name = "CaloLArRetriever")
theCaloLArRetriever.DoLArCellDetails = False
theCaloLArRetriever.DoBadLAr = False

from CaloJiveXML.CaloJiveXMLConf import JiveXML__CaloHECRetriever
theCaloHECRetriever = JiveXML__CaloHECRetriever (name = "CaloHECRetriever")
theCaloHECRetriever.DoHECCellDetails = False
theCaloHECRetriever.DoBadHEC = False

from CaloJiveXML.CaloJiveXMLConf import JiveXML__LArDigitRetriever
theLArDigitRetriever = JiveXML__LArDigitRetriever (name = "LArDigitRetriever")
theLArDigitRetriever.DoLArDigit = False
theLArDigitRetriever.DoHECDigit = False
theLArDigitRetriever.DoFCalDigit = False 
    
if (theLArDigitRetriever.DoLArDigit or theLArDigitRetriever.DoHECDigit or theLArDigitRetriever.DoFCalDigit):
    theEventData2XML.DataTypes += ["JiveXML::LArDigitRetriever/LArDigitRetriever"]
else:    
    theEventData2XML.DataTypes += ["JiveXML::CaloFCalRetriever/CaloFCalRetriever"]
    theEventData2XML.DataTypes += ["JiveXML::CaloLArRetriever/CaloLArRetriever"]
    theEventData2XML.DataTypes += ["JiveXML::CaloHECRetriever/CaloHECRetriever"]

theEventData2XML.DataTypes += ["JiveXML::CaloMBTSRetriever/CaloMBTSRetriever"]
theEventData2XML.DataTypes += ["JiveXML::CaloTileRetriever/CaloTileRetriever"]
theEventData2XML.DataTypes += ["JiveXML::CaloClusterRetriever/CaloClusterRetriever"]


ToolSvc += theCaloClusterRetriever
ToolSvc += theCaloTileRetriever
ToolSvc += theCaloMBTSRetriever
ToolSvc += theCaloFCalRetriever
ToolSvc += theCaloLArRetriever
ToolSvc += theCaloHECRetriever
ToolSvc += theLArDigitRetriever

### flags to write calo bad cell details (code from Nikolina Ilic):
# theCaloTileRetriever.DoBadTile = True
# theCaloFCalRetriever.DoBadFCal = True
# theCaloLArRetriever.DoBadLAr = True
# theCaloHECRetriever.DoBadHEC = True


