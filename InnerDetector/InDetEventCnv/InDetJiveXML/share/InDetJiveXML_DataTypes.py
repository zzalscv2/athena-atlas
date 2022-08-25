#Avoid multiple includes
include.block("InDetJiveXML/InDetJiveXML_DataTypes.py")

# Include the base options if the user didn't already do that
if not "theEventData2XML" in dir():
    include ("JiveXML/JiveXML_jobOptionBase.py")

theEventData2XML.DataTypes += ["JiveXML::SiSpacePointRetriever/SiSpacePointRetriever"]
theEventData2XML.DataTypes += ["JiveXML::SiClusterRetriever/SiClusterRetriever"]
theEventData2XML.DataTypes += ["JiveXML::PixelClusterRetriever/PixelClusterRetriever"]
theEventData2XML.DataTypes += ["JiveXML::TRTRetriever/TRTRetriever"]

from AthenaCommon.DetFlags import DetFlags
 
if DetFlags.makeRIO.SCT_on():
   theEventData2XML.DataTypes += ["JiveXML::SCTRDORetriever/SCTRDORetriever"]

if DetFlags.makeRIO.pixel_on():
   theEventData2XML.DataTypes += ["JiveXML::PixelRDORetriever/PixelRDORetriever"]

### new datatype, not yet understood by AtlantisJava:
#theEventData2XML.DataTypes += ["JiveXML::BeamSpotRetriever/BeamSpotRetriever"]

isMC = ConfigFlags.Input.isMC

from InDetJiveXML.InDetJiveXMLConf import JiveXML__PixelClusterRetriever
thePixelClusterRetriever = JiveXML__PixelClusterRetriever (name = "PixelClusterRetriever")
if not isMC:
    thePixelClusterRetriever.PixelTruthMap = ''
ToolSvc += thePixelClusterRetriever

from InDetJiveXML.InDetJiveXMLConf import JiveXML__SiClusterRetriever
theSiClusterRetriever = JiveXML__SiClusterRetriever (name = "SiClusterRetriever")
if not isMC:
    theSiClusterRetriever.SCT_TruthMap = ''
ToolSvc += theSiClusterRetriever

from InDetJiveXML.InDetJiveXMLConf import JiveXML__SiSpacePointRetriever
theSiSpacePointRetriever = JiveXML__SiSpacePointRetriever (name = "SiSpacePointRetriever")
if not isMC:
    theSiSpacePointRetriever.PRD_TruthPixel = ''
    theSiSpacePointRetriever.PRD_TruthSCT = ''
ToolSvc += theSiSpacePointRetriever

from InDetJiveXML.InDetJiveXMLConf import JiveXML__TRTRetriever
theTRTRetriever = JiveXML__TRTRetriever (name = "TRTRetriever")
if not isMC:
    theTRTRetriever.TRTTruthMap = ''
ToolSvc += theTRTRetriever

#
#from InDetJiveXML.InDetJiveXMLConf import JiveXML__PixelClusterRetriever
#thePixelClusterRetriever = JiveXML__PixelClusterRetriever (name = "PixelClusterRetriever")
#thePixelClusterRetriever.OutputLevel = DEBUG
#ToolSvc += thePixelClusterRetriever
