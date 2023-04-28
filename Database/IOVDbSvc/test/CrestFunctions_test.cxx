/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/
/*
 */
/**
 * @file IOVDbSvc/test/CrestFunctions_test.cxx
 * @author Shaun Roe
 * @date July, 2019
 * @brief Some tests for CrestFunctions 
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE TEST_IOVDBSVC


#include <boost/test/unit_test.hpp>
//


#include "../src/CrestFunctions.h"
#include "CoolKernel/ChannelId.h"
#include <string>

using namespace IOVDbNamespace;

static const std::string metadata1{R"delim({"format":"TagMetaSetDto","resources":[{"tagName":"LARConfigurationFEBConfigPhysicsHECA-HEAD","description":"{\"dbname\":\"CONDBR2\",\"nodeFullpath\":\"/LAR/Configuration/FEBConfig/Physics/HECA\",\"schemaName\":\"COOLONL_LAR\"}","chansize":24,"colsize":265,"tagInfo":"{\"channel_list\":[{\"991559680\":\"endcap A 02L hc F1\"},{\"991592448\":\"endcap A 02L hc F2\"},{\"991625216\":\"endcap A 02L hc F3\"},{\"991657984\":\"endcap A 02L hc F4\"},{\"991690752\":\"endcap A 02L hc F5\"},{\"991723520\":\"endcap A 02L hc F6\"},{\"995229696\":\"endcap A 06L hc F1\"},{\"995262464\":\"endcap A 06L hc F2\"},{\"995295232\":\"endcap A 06L hc F3\"},{\"995328000\":\"endcap A 06L hc F4\"},{\"995360768\":\"endcap A 06L hc F5\"},{\"995393536\":\"endcap A 06L hc F6\"},{\"998375424\":\"endcap A 09L hc F1\"},{\"998408192\":\"endcap A 09L hc F2\"},{\"998440960\":\"endcap A 09L hc F3\"},{\"998473728\":\"endcap A 09L hc F4\"},{\"998506496\":\"endcap A 09L hc F5\"},{\"998539264\":\"endcap A 09L hc F6\"},{\"1001521152\":\"endcap A 12L hc F1\"},{\"1001553920\":\"endcap A 12L hc F2\"},{\"1001586688\":\"endcap A 12L hc F3\"},{\"1001619456\":\"endcap A 12L hc F4\"},{\"1001652224\":\"endcap A 12L hc F5\"},{\"1001684992\":\"endcap A 12L hc F6\"}],\"node_description\":\"<timeStamp>time</timeStamp><addrHeader><address_header service_type=\\\"71\\\" clid=\\\"1238547719\\\" /></addrHeader><typeName>CondAttrListCollection</typeName>\",\"payload_spec\":\"ID:UInt32,Name:String255,upper1:Int32,upper2:Int32,upper3:Int32,upper4:Int32,upper5:Int32,upper6:Int32,upper7:Int32,upper8:Int32,upper9:Int32,upper10:Int32,upper11:Int32,upper12:Int32,upper13:Int32,upper14:Int32,upper15:Int32,upper16:Int32,upper17:Int32,upper18:Int32,upper19:Int32,upper20:Int32,upper21:Int32,upper22:Int32,upper23:Int32,upper24:Int32,upper25:Int32,upper26:Int32,upper27:Int32,upper28:Int32,upper29:Int32,upper30:Int32,upper31:Int32,upper32:Int32,upper33:Int32,upper34:Int32,upper35:Int32,upper36:Int32,upper37:Int32,upper38:Int32,upper39:Int32,upper40:Int32,upper41:Int32,upper42:Int32,upper43:Int32,upper44:Int32,upper45:Int32,upper46:Int32,upper47:Int32,upper48:Int32,upper49:Int32,upper50:Int32,upper51:Int32,upper52:Int32,upper53:Int32,upper54:Int32,upper55:Int32,upper56:Int32,upper57:Int32,upper58:Int32,upper59:Int32,upper60:Int32,upper61:Int32,upper62:Int32,upper63:Int32,upper64:Int32,upper65:Int32,upper66:Int32,upper67:Int32,upper68:Int32,upper69:Int32,upper70:Int32,upper71:Int32,upper72:Int32,upper73:Int32,upper74:Int32,upper75:Int32,upper76:Int32,upper77:Int32,upper78:Int32,upper79:Int32,upper80:Int32,upper81:Int32,upper82:Int32,upper83:Int32,upper84:Int32,upper85:Int32,upper86:Int32,upper87:Int32,upper88:Int32,upper89:Int32,upper90:Int32,upper91:Int32,upper92:Int32,upper93:Int32,upper94:Int32,upper95:Int32,upper96:Int32,upper97:Int32,upper98:Int32,upper99:Int32,upper100:Int32,upper101:Int32,upper102:Int32,upper103:Int32,upper104:Int32,upper105:Int32,upper106:Int32,upper107:Int32,upper108:Int32,upper109:Int32,upper110:Int32,upper111:Int32,upper112:Int32,upper113:Int32,upper114:Int32,upper115:Int32,upper116:Int32,upper117:Int32,upper118:Int32,upper119:Int32,upper120:Int32,upper121:Int32,upper122:Int32,upper123:Int32,upper124:Int32,upper125:Int32,upper126:Int32,upper127:Int32,upper128:Int32,lower1:Int32,lower2:Int32,lower3:Int32,lower4:Int32,lower5:Int32,lower6:Int32,lower7:Int32,lower8:Int32,lower9:Int32,lower10:Int32,lower11:Int32,lower12:Int32,lower13:Int32,lower14:Int32,lower15:Int32,lower16:Int32,lower17:Int32,lower18:Int32,lower19:Int32,lower20:Int32,lower21:Int32,lower22:Int32,lower23:Int32,lower24:Int32,lower25:Int32,lower26:Int32,lower27:Int32,lower28:Int32,lower29:Int32,lower30:Int32,lower31:Int32,lower32:Int32,lower33:Int32,lower34:Int32,lower35:Int32,lower36:Int32,lower37:Int32,lower38:Int32,lower39:Int32,lower40:Int32,lower41:Int32,lower42:Int32,lower43:Int32,lower44:Int32,lower45:Int32,lower46:Int32,lower47:Int32,lower48:Int32,lower49:Int32,lower50:Int32,lower51:Int32,lower52:Int32,lower53:Int32,lower54:Int32,lower55:Int32,lower56:Int32,lower57:Int32,lower58:Int32,lower59:Int32,lower60:Int32,lower61:Int32,lower62:Int32,lower63:Int32,lower64:Int32,lower65:Int32,lower66:Int32,lower67:Int32,lower68:Int32,lower69:Int32,lower70:Int32,lower71:Int32,lower72:Int32,lower73:Int32,lower74:Int32,lower75:Int32,lower76:Int32,lower77:Int32,lower78:Int32,lower79:Int32,lower80:Int32,lower81:Int32,lower82:Int32,lower83:Int32,lower84:Int32,lower85:Int32,lower86:Int32,lower87:Int32,lower88:Int32,lower89:Int32,lower90:Int32,lower91:Int32,lower92:Int32,lower93:Int32,lower94:Int32,lower95:Int32,lower96:Int32,lower97:Int32,lower98:Int32,lower99:Int32,lower100:Int32,lower101:Int32,lower102:Int32,lower103:Int32,lower104:Int32,lower105:Int32,lower106:Int32,lower107:Int32,lower108:Int32,lower109:Int32,lower110:Int32,lower111:Int32,lower112:Int32,lower113:Int32,lower114:Int32,lower115:Int32,lower116:Int32,lower117:Int32,lower118:Int32,lower119:Int32,lower120:Int32,lower121:Int32,lower122:Int32,lower123:Int32,lower124:Int32,lower125:Int32,lower126:Int32,lower127:Int32,lower128:Int32,coarseDelay:Int32,fineDelay:Int32,ssw3:UInt32,ssw2:UInt32,ssw1:UInt32,ssw0:UInt32,Comment:String255\"}","insertionTime":"2022-07-08T15:48:11+0000"}],"size":1,"datatype":"tagmetas","format":null,"page":null,"filter":null})delim"};




//dont want unit test to depend on whether the CREST db is up or not
static const bool dontUseRealDatabase=true;

BOOST_AUTO_TEST_SUITE(CrestFunctionsTest)
  BOOST_AUTO_TEST_CASE(urlBase_test){
    auto baseUrl=urlBase();
    BOOST_TEST(baseUrl == "http://crest-02.cern.ch:9090");
  }
  
  BOOST_AUTO_TEST_CASE(extractHashFromJson_test){
    const std::string sampleJson{R"delim([{"tagName":"Indet_Align-description","since":0,"insertionTime":"2019-07-08T15:03:15.742+0000","payloadHash":"dd7a656e41d7b6426164fc411aa7ff51aa4bc047ab885a6e00b976b62291c18b"}])delim"};
    BOOST_TEST(extractHashFromJson(sampleJson) == "dd7a656e41d7b6426164fc411aa7ff51aa4bc047ab885a6e00b976b62291c18b");
    const std::string nonsense{"blubbyPinkBox333"};
    BOOST_TEST(extractHashFromJson(nonsense) == "");
  }
  
  BOOST_AUTO_TEST_CASE(getIovsForTag_test){
    const std::string tag{"LARIdentifierFebRodMap-RUN2-000"};
    const std::string referenceReply{"99331506eefbe6783a8d5d5bc8b9a44828a325adfcaac32f62af212e9642db71"};
    BOOST_TEST(getLastHashForTag(tag, dontUseRealDatabase) == referenceReply);
  }
  
  BOOST_AUTO_TEST_CASE(getPayloadForHash_test){
    const std::string hash{"b8c82f0d2c3443a8c0b72c69d79e57205ba11a55ab7107d3e666294a3607f09d"};
    const std::string referenceReply{R"delim({"data":{"0":["[DB=B2E3B2B6-B76C-DF11-A505-000423D5ADDA][CNT=CollectionTree(LArTTCell_P/LArTTCellMapAtlas)][CLID=DF8C509C-A91A-40B5-B76C-5B57EEE21EC3][TECH=00000202][OID=00000003-00000000]"]}})delim"};
    BOOST_TEST(getPayloadForHash(hash, dontUseRealDatabase) == referenceReply);
  }
  
  BOOST_AUTO_TEST_CASE(extractChannelListFromJson_test){
    const std::vector<cool::ChannelId> chans{991559680, 991592448, 991625216, 991657984, 991690752, 991723520, 995229696, 995262464, 995295232, 995328000, 995360768, 995393536, 998375424, 998408192, 998440960, 998473728, 998506496, 998539264, 1001521152, 1001553920, 1001586688, 1001619456, 1001652224, 1001684992};
    const std::vector<std::string> names{"endcap A 02L hc F1", "endcap A 02L hc F2", "endcap A 02L hc F3", "endcap A 02L hc F4", "endcap A 02L hc F5", "endcap A 02L hc F6", "endcap A 06L hc F1", "endcap A 06L hc F2", "endcap A 06L hc F3", "endcap A 06L hc F4", "endcap A 06L hc F5", "endcap A 06L hc F6", "endcap A 09L hc F1", "endcap A 09L hc F2", "endcap A 09L hc F3", "endcap A 09L hc F4", "endcap A 09L hc F5", "endcap A 09L hc F6", "endcap A 12L hc F1", "endcap A 12L hc F2", "endcap A 12L hc F3", "endcap A 12L hc F4", "endcap A 12L hc F5", "endcap A 12L hc F6"};
    const std::pair<std::vector<cool::ChannelId>, std::vector<std::string>> expectedReply{chans,names};
    BOOST_TEST((extractChannelListFromJson(metadata1) == expectedReply));
  }
  
  BOOST_AUTO_TEST_CASE(channelListForTag_test){
    const std::string folderTag{"LARBadChannelsOflBadChannels-RUN2-UPD4-21"};
    const std::pair<std::vector<cool::ChannelId>, std::vector<std::string>> expectedReply{{0, 1, 2, 3, 4, 5, 6, 7},{}};
    
    BOOST_TEST((channelListForTag(folderTag, dontUseRealDatabase) == expectedReply));
  }
  
  BOOST_AUTO_TEST_CASE(getPayloadForTag_test){
    const std::string tag("LARIdentifierLArTTCellMapAtlas-RUN2-HadFcalFix2");
    const std::string referenceReply{R"delim({"data":{"0":["[DB=B2E3B2B6-B76C-DF11-A505-000423D5ADDA][CNT=CollectionTree(LArTTCell_P/LArTTCellMapAtlas)][CLID=DF8C509C-A91A-40B5-B76C-5B57EEE21EC3][TECH=00000202][OID=00000003-00000000]"]}})delim"};
    BOOST_TEST(getPayloadForTag(tag, dontUseRealDatabase)  == referenceReply);
  }
  
  BOOST_AUTO_TEST_CASE(extractDescriptionFromJson_test){
    const std::string jsonDescr{R"delim({"format":"TagMetaSetDto","resources":[{"tagName":"LARElecCalibFlatPedestal-HEAD","description":"{\"dbname\":\"CONDBR2\",\"nodeFullpath\":\"/LAR/ElecCalibFlat/Pedestal\",\"schemaName\":\"COOLONL_LAR\"}","chansize":3,"colsize":3,"tagInfo":"{\"channel_list\":[{\"0\":\"\"},{\"1\":\"\"},{\"2\":\"\"}],\"node_description\":\"<timeStamp>run-lumi</timeStamp><addrHeader><address_header service_type=\\\"256\\\" clid=\\\"1238547719\\\" /></addrHeader><typeName>CondAttrListCollection</typeName><updateMode>UPD1</updateMode>\",\"payload_spec\":\"Pedestal:Blob16M,PedestalRMS:Blob16M,version:UInt32\"}","insertionTime":"2022-07-08T15:57:12+0000"}],"size":1,"datatype":"tagmetas","format":null,"page":null,"filter":null})delim"};
    const std::string expectedAnswer{R"delim(<timeStamp>run-lumi</timeStamp><addrHeader><address_header service_type="256" clid="1238547719" /></addrHeader><typeName>CondAttrListCollection</typeName><updateMode>UPD1</updateMode>)delim"};
    BOOST_TEST_MESSAGE(expectedAnswer);
    BOOST_TEST_MESSAGE(extractDescriptionFromJson(jsonDescr));
    BOOST_TEST(extractDescriptionFromJson(jsonDescr) == expectedAnswer);
  }
  
  BOOST_AUTO_TEST_CASE(folderDescriptionForTag_test){
    const std::string folderTag{"LARElecCalibFlatPedestal-HEAD"};
    const std::string expectedReply{R"delim(<timeStamp>run-lumi</timeStamp><addrHeader><address_header service_type="256" clid="1238547719" /></addrHeader><typeName>CondAttrListCollection</typeName><updateMode>UPD1</updateMode>)delim"};
    BOOST_TEST(folderDescriptionForTag(folderTag, dontUseRealDatabase) == expectedReply);
  }
  
  BOOST_AUTO_TEST_CASE(resolveCrestTag_test){
    const std::string globalTag{"CREST-RUN12-SDR-25-MC"};
    const std::string folderName{"/LAR/Align"};
    const std::string expectedReply{"LARAlign-RUN2-UPD4-03"};
    BOOST_TEST(resolveCrestTag(globalTag, folderName,"",dontUseRealDatabase) == expectedReply);
  }
  
BOOST_AUTO_TEST_SUITE_END()
