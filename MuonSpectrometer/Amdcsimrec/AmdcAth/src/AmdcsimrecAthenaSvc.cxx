/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "StoreGate/StoreGateSvc.h"

//----------------------------------------------------------------//
#include "StoreGate/DataHandle.h"

#include "PathResolver/PathResolver.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

//----------------------------------------------------------------//
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "AtlasDetDescr/AtlasDetectorID.h"

/////////////////////////////////////////////////////////
#include "AmdcAth/AmdcsimrecAthenaSvc.h"

#include "AmdcStand/AmdcsimrecStand.h"
 
#include "AmdcCore/cppbigdump.h"

//----------------------------------------------------------------//
#include "AmdcCore/AmdcAlineStore.h"
#include "AmdcCore/AmdcBlineStore.h"
#include "AmdcCore/AmdcIlineStore.h"

#include "AmdcStand/controlaline.h"
#include "AmdcStand/setablines.h"
#include "AmdcStand/fgetamdcagdd.h"
#include "AmdcStand/amdcread.h"
#include "AmdcStand/bigamdcdump.h"
#include "AmdcStand/loadamdcagddfromstring.h"

#include <fstream>

/// Standard Constructor
AmdcsimrecAthenaSvc::AmdcsimrecAthenaSvc(const std::string& name,ISvcLocator* svc):
  AthService(name,svc),
  p_Amdcsimrec(nullptr),
  p_IGeoModelSvc(nullptr),
  p_detStore(nullptr),
  p_MuonDetectorManager(nullptr),
  p_AtlasDetectorID(nullptr)
{

//*Set Default values
   p_AmdcAlineStoreInternal = new AmdcAlineStore;
   p_AmdcBlineStoreInternal = new AmdcBlineStore;
   p_AmdcIlineStoreInternal = new AmdcIlineStore ;

   p_AmdcAlineStoreExternal = new AmdcAlineStore;
   p_AmdcBlineStoreExternal = new AmdcBlineStore;
   p_AmdcIlineStoreExternal = new AmdcIlineStore ;

   m_AmdcABlinesStamp = 1;
   
   m_AGDD2GeoSwitchesStamp = -1 ;
   m_AGDD2GeoSwitches.clear();
   
//*Declare the properties
   declareProperty("NameOfTheSource"        , m_NameOfTheSource        = "GEOMODEL" ) ;
   declareProperty("NameOfTheAmdbFile"      , m_NameOfTheFile          = ""         ) ;
   declareProperty("OracleNodedetectorKey"  , m_OracleNodedetectorKey  = ""         ) ;
   declareProperty("OracleNodedetectorNode" , m_OracleNodedetectorNode = ""         ) ;

   declareProperty("CtrlDumpCo"        , m_CtrlDumpCo         = 0          ) ;
   declareProperty("CtrlFileStrings"   , m_CtrlFileStrings    = 0          ) ;
   declareProperty("CtrlFileOracle"    , m_CtrlFileOracle     = 0          ) ;
   declareProperty("EmergencyOut"      , m_EmergencyOut       = 0          ) ;
   declareProperty("ControlALine"      , m_ControlALine       = 111111     ) ;
   declareProperty("ControlILine"      , m_ControlILine       = 111111     ) ;
   declareProperty("TruncateALine"     , m_TruncateALine      = 0          ) ;
   declareProperty("TruncatePLine"     , m_TruncatePLine      = 0          ) ;
   declareProperty("HardSoftCheck"     , m_HardSoftCheck      = 0          );
   declareProperty("XtomoCheck"        , m_XtomoCheck         = 0          );

//-----------------------------------------------------------------
// Alignment Corrections :
// AlignmentSource  2: Get them from Geomodel
//                  3: Get them from Amdc string from Ascii file or from Oracle
// AlignmentCorr    0 : off 
//                  1: A 
//                  2: B 
//                  3: A&B

   declareProperty( "AlignmentSource"                   , m_AlignmentSource                   = 3 ) ;
   declareProperty( "AlignmentCorr"                     , m_AlignmentCorr                     = 1 ) ;

   declareProperty( "SortAlignContainer"                , m_SortAlignContainer                = 0 );
   declareProperty( "RmAlignCSC"                        , m_RmAlignCSC                        = 0 );
   declareProperty( "ModifyALineContainer"              , m_ModifyALineContainer              = 2 );
   
   declareProperty( "CompareInternalExternalALineContainer" , m_CompareInternalExternalALineContainer      = 0 );
  
   declareProperty( "UseMuonDetectorManagerForInternal", m_UseMuonDetectorManagerForInternal              = 0 );
   
   declareProperty( "DontSetAmdcABlineFromCool", m_DontSetAmdcABlineFromCool              = 0 );
   
   declareProperty( "PrintLevel", m_PrintLevel = 0 );
}
 
/// Standard Destructor
AmdcsimrecAthenaSvc::~AmdcsimrecAthenaSvc()  {
  delete p_AmdcAlineStoreInternal ;
  delete p_AmdcBlineStoreInternal ;
  delete p_AmdcIlineStoreInternal ;
  delete p_AmdcAlineStoreExternal ;
  delete p_AmdcBlineStoreExternal ;
  delete p_AmdcIlineStoreExternal ;
  delete p_Amdcsimrec ;
}
 
// Service initialisation
StatusCode AmdcsimrecAthenaSvc::initialize() {

  ATH_MSG_DEBUG( "Initialisation started     " ) ;

  ATH_CHECK(AthService::initialize());

  ATH_MSG_DEBUG( "================================" ) ;
  ATH_MSG_DEBUG( "=Proprieties are         " ) ;
  ATH_MSG_DEBUG( "= NameOfTheSource        " << m_NameOfTheSource      ) ;
  ATH_MSG_DEBUG( "= NameOfTheAmdbFile      " << m_NameOfTheFile        ) ;
  ATH_MSG_DEBUG( "= OracleNodedetectorKey  " << m_OracleNodedetectorKey  ) ;
  ATH_MSG_DEBUG( "= OracleNodedetectorNode " << m_OracleNodedetectorNode ) ;
  ATH_MSG_DEBUG( "= CtrlDumpCo             " << m_CtrlDumpCo           ) ;
  ATH_MSG_DEBUG( "= CtrlFileStrings        " << m_CtrlFileStrings      ) ;
  ATH_MSG_DEBUG( "= CtrlFileOracle         " << m_CtrlFileOracle     ) ;
  ATH_MSG_DEBUG( "= EmergencyOut           " << m_EmergencyOut         ) ;
  ATH_MSG_DEBUG( "= ControlALine           " << m_ControlALine         ) ;
  ATH_MSG_DEBUG( "= ControlILine           " << m_ControlILine         ) ;
  ATH_MSG_DEBUG( "= TruncateALine          " << m_TruncateALine         ) ;
  ATH_MSG_DEBUG( "= TruncatePLine          " << m_TruncatePLine         ) ;
  ATH_MSG_DEBUG( "= AlignmentSource        " << m_AlignmentSource ) ;
  ATH_MSG_DEBUG( "= AlignmentCorr          " << m_AlignmentCorr ) ;
  ATH_MSG_DEBUG( "= HardSoftCheck          " << m_HardSoftCheck         ) ;
  ATH_MSG_DEBUG( "= XtomoCheck             " << m_XtomoCheck         ) ;
  ATH_MSG_DEBUG( "= SortAlignContainer                    " << m_SortAlignContainer         ) ;
  ATH_MSG_DEBUG( "= RmAlignCSC                            " << m_RmAlignCSC                 ) ;
  ATH_MSG_DEBUG( "= ModifyALineContainer                  " << m_ModifyALineContainer                 ) ;
  ATH_MSG_DEBUG( "= CompareInternalExternalALineContainer " << m_CompareInternalExternalALineContainer                 ) ;
  ATH_MSG_DEBUG( "= UseMuonDetectorManagerForInternal     " << m_UseMuonDetectorManagerForInternal                 ) ;
  ATH_MSG_DEBUG( "= DontSetAmdcABlineFromCool             " << m_DontSetAmdcABlineFromCool                 ) ;
  ATH_MSG_DEBUG( "================================" ) ;


//Set pointer on DetectorStore 
  ATH_CHECK(service("DetectorStore",p_detStore));
  ATH_MSG_DEBUG( "Found DetectorStore ") ;

//Set pointer on GeoModelSvc
  if ( m_AlignmentSource == 2 
  || m_NameOfTheSource=="POOL" 
  || m_NameOfTheSource=="GEOMODEL" ) {
    ATH_CHECK(service ("GeoModelSvc",p_IGeoModelSvc));
    ATH_MSG_DEBUG( "Found GeoModelSvc ") ;
  }

//Set pointer on Muondetector Manager
  if ( m_UseMuonDetectorManagerForInternal == 1 ) {
    ATH_CHECK(p_detStore->retrieve(p_MuonDetectorManager));
    ATH_MSG_DEBUG( "Found MuonDetectorManager ") ;
  }


//Check geometry related proprieties
  if (  m_NameOfTheSource!="ASCII"
     && m_NameOfTheSource!="GEOMODEL" 
     && m_NameOfTheSource!="ORACLENODE" 
     && m_NameOfTheSource!="POOL" 
     && m_NameOfTheSource!="POOLHARD") {
    ATH_MSG_FATAL( "Selected source " << m_NameOfTheSource << " unknown " ) ;
    return StatusCode::FAILURE;
  }
  if ( m_AlignmentCorr != 0 && m_AlignmentCorr != 1 && m_AlignmentCorr != 2 && m_AlignmentCorr != 3 ) {
    ATH_MSG_FATAL( " AlignmentCorr is " << m_AlignmentCorr ) ;
    ATH_MSG_FATAL( "  while it should be 0, 1, 2 or 3 " ) ;
    return StatusCode::FAILURE;
  }
  if ( m_AlignmentCorr != 0 ) {
    if ( m_AlignmentSource != 2 && m_AlignmentSource != 3){
      ATH_MSG_FATAL( " AlignmentSource is " << m_AlignmentSource ) ;
      ATH_MSG_FATAL( "  while it should be 2: Get them from Geomodel ") ;
      ATH_MSG_FATAL( "  or                 3: Get them from Amdc string from Ascii file or from Oracle ") ;
      return StatusCode::FAILURE;
    }
  }


//Strings come Ascii file 
  if (m_NameOfTheSource=="ASCII"){

    if (m_AlignmentSource == 3 ){
      ATH_MSG_DEBUG( "=>Strings come from Ascii file and A/B line stores as well<=" ) ;
    }
    else{
      ATH_MSG_DEBUG( "=>Strings come from Ascii file and A/B line stores from cool<=" ) ;
    }

    ATH_CHECK(initializeAscii());
    ATH_MSG_DEBUG( "Done: initializeAscii " ) ;
    
    if (m_AlignmentSource == 2 ){
      ATH_CHECK(SetAmdcABlineFromCool());
      ATH_MSG_DEBUG( "Done: SetAmdcABlineFromCool " ) ;
    }

  }

//Strings come from Geomodel and A/B line stores as well
  if ( (m_NameOfTheSource=="POOL" || m_NameOfTheSource=="GEOMODEL" ) && m_AlignmentSource == 3 ){
    ATH_MSG_DEBUG( "=>Strings come from Geomodel and A/B line stores as well<=" ) ;
    
    ATH_CHECK(initializeFromGeomodel());
    ATH_MSG_DEBUG( "Done: initializeFromGeomodel " ) ;
  }
  
//Strings come from Geomodel and A/B line stores from cool
   if ( (m_NameOfTheSource=="POOL" || m_NameOfTheSource=="GEOMODEL" ) && m_AlignmentSource == 2 ){
    ATH_MSG_DEBUG( "=>Strings come from Geomodel and A/B line stores from cool<=" ) ;

    ATH_CHECK(initializeFromGeomodel());
    ATH_MSG_DEBUG( "Done: initializeFromGeomodel " ) ;
    
    ATH_CHECK(SetAmdcABlineFromCool());
    ATH_MSG_DEBUG( "Done: SetAmdcABlineFromCool " ) ;

  }
  

//Strings come from Oracle 
  if (m_NameOfTheSource=="POOLHARD" || m_NameOfTheSource=="ORACLENODE" ){

    if (m_AlignmentSource == 3 ){
      ATH_MSG_DEBUG( "=>Strings come from Oracle and A/B line stores as well<=" ) ;
    }
    else{
      ATH_MSG_DEBUG( "=>Strings come from Oracle and A/B line stores from cool<=" ) ;
    }

    ATH_CHECK(initializeFromOracleNode());
    ATH_MSG_DEBUG( "Done: initializeFromOracleNode " ) ;
    
    if (m_AlignmentSource == 2 ){
      ATH_CHECK(SetAmdcABlineFromCool());
      ATH_MSG_DEBUG( "Done: SetAmdcABlineFromCool " ) ;
    }
  }

//Set pointer on Muondetector Manager
  if ( m_UseMuonDetectorManagerForInternal == 1 ) {
    ATH_MSG_DEBUG( "=>A/B line in internal stores come from  MuonDetectorManager<=" ) ;
  }

  ATH_MSG_DEBUG( "Initialisation ended     " ) ;
  if ( m_EmergencyOut == 1 ) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}

StatusCode AmdcsimrecAthenaSvc::queryInterface( const InterfaceID& riid, void** ppvInterface ) {
  if ( IID_IAmdcsimrecAthenaSvc == riid )    {
    *ppvInterface = (AmdcsimrecAthenaSvc*)this;
  }else{
    return Service::queryInterface(riid, ppvInterface);
  }
  return StatusCode::SUCCESS;
}

Amdcsimrec* AmdcsimrecAthenaSvc::GetAmdcsimrec(){return p_Amdcsimrec;}

std::string AmdcsimrecAthenaSvc::GetNameOfTheSource(){return m_NameOfTheSource;}

std::string AmdcsimrecAthenaSvc::GetNameOfTheFile(){return m_NameOfTheFile;}
std::string AmdcsimrecAthenaSvc::GetLocationOfTheFile(){return m_LocationOfTheFile;}

std::string AmdcsimrecAthenaSvc::GetAmdcString(){return m_AmdcString;}
std::string AmdcsimrecAthenaSvc::GetAgddString(){return m_AgddString;}

/// Service initialisation
StatusCode AmdcsimrecAthenaSvc::initializeAscii()
{

  ATH_MSG_DEBUG("----> initializeAscii is called" ) ; 

  ATH_CHECK(SetLocation(m_NameOfTheFile,m_LocationOfTheFile));
  ATH_MSG_DEBUG( "Done: SetLocation " ) ;

  ATH_MSG_DEBUG( "File to be read " << m_NameOfTheFile ) ;
  ATH_MSG_DEBUG( " found as       " << m_LocationOfTheFile ) ;

//Initialize Muon Spectrometer Geometry
  int  SizeName = m_LocationOfTheFile.size();
  char* FileName = new char[SizeName];
  int  Istate = 0;
  for (int i= 0; i <SizeName; i++){FileName[i]=m_LocationOfTheFile[i];}
  int IFLAG = 100 + m_PrintLevel ;
  amdcreadnn_(FileName,SizeName,Istate,IFLAG);
  delete [] FileName ;
  if (Istate == 0) {
    ATH_MSG_FATAL( "amdcreadn failed " ) ;
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG( "Done: amdcreadn " ) ;

//Post Geometry Loading Sequence
  ATH_CHECK(PostGeometryLoadingSequence());
  ATH_MSG_DEBUG( "Done: PostGeometryLoadingSequence " ) ;

  return StatusCode::SUCCESS;

}

StatusCode AmdcsimrecAthenaSvc::initializeFromGeomodel()
{

  ATH_MSG_DEBUG("----> initializeFromGeomodel is called" ) ; 

  std::string AtlasVersion = p_IGeoModelSvc->atlasVersion();
  std::string MuonVersion  = p_IGeoModelSvc->muonVersionOverride();

  m_detectorKey  = MuonVersion.empty() ? AtlasVersion : MuonVersion;
  m_detectorNode = MuonVersion.empty() ? "ATLAS" : "MuonSpectrometer";
  if ( MuonVersion == "CUSTOM"){
    m_detectorKey  = AtlasVersion ;
    m_detectorNode = "ATLAS"  ;
  } 
  ATH_CHECK(initializeFromOracle());
  ATH_MSG_DEBUG( "Done: initializeFromOracle" ) ;

  return StatusCode::SUCCESS;

}

StatusCode AmdcsimrecAthenaSvc::initializeFromOracleNode() {

  ATH_MSG_DEBUG("----> initializeFromOracleNode is called" ) ; 

  m_detectorKey  = m_OracleNodedetectorKey  ;
  m_detectorNode = m_OracleNodedetectorNode ;
  
  ATH_CHECK(initializeFromOracle());
  ATH_MSG_DEBUG( "Done: initializeFromOracle" ) ;

  return StatusCode::SUCCESS;

}

StatusCode AmdcsimrecAthenaSvc::initializeFromOracle()
{

  ATH_MSG_DEBUG("----> initializeFromOracle is called" ) ; 

  IRDBAccessSvc* pIRDBAccessSvc=nullptr;
  ATH_CHECK(service("RDBAccessSvc",pIRDBAccessSvc));

  ATH_MSG_DEBUG( "      Keys are  (key) "  << m_detectorKey  
      << " (node) " << m_detectorNode 
      ) ;

  IRDBRecordset_ptr recordsetAMDC = pIRDBAccessSvc->getRecordsetPtr("AMDC",m_detectorKey,m_detectorNode);
  if (recordsetAMDC->size()==0){
    ATH_MSG_FATAL( "recordsetAMDC->size() is 0" ) ;
    return StatusCode::FAILURE;
  }
  const IRDBRecord *recordAMDC =  (*recordsetAMDC)[0];
  std::string AmdcString = recordAMDC->getString("DATA");
  ATH_MSG_DEBUG( "        AMDC::VNAME " << recordAMDC->getString("VNAME") ) ;

  IRDBRecordset_ptr recordsetAGDD = pIRDBAccessSvc->getRecordsetPtr("AGDD",m_detectorKey,m_detectorNode);
  if (recordsetAGDD->size()==0){
    ATH_MSG_FATAL( "        recordsetAGDD->size() is 0" ) ;
    return StatusCode::FAILURE;
  }
  const IRDBRecord *recordAGDD =  (*recordsetAGDD)[0];
  std::string AgddString = recordAGDD->getString("DATA");
  ATH_MSG_DEBUG( "        AGDD::VNAME " << recordAGDD->getString("VNAME") ) ;


//Retrieve AGDD2GeoSwitches data
  std::string agdd2geoVersion = pIRDBAccessSvc->getChildTag("AGDD2GeoSwitches",m_detectorKey,m_detectorNode);
  m_AGDD2GeoSwitches.clear();
  if(!agdd2geoVersion.empty()) {
    m_AGDD2GeoSwitchesStamp = m_AGDD2GeoSwitchesStamp + 1;
    std::string TheKEYNAME;
    int TheKEYVALUE; 
    IRDBRecordset_ptr pIRDBRecordset = pIRDBAccessSvc->getRecordsetPtr("AGDD2GeoSwitches",m_detectorKey,m_detectorNode);
    for(unsigned int i=0; i<pIRDBRecordset->size(); i++) {
      const IRDBRecord* record = (*pIRDBRecordset)[i];
      TheKEYNAME = record->getString("KEYNAME");
      TheKEYVALUE = record->getInt("KEYVALUE");
      if ( TheKEYVALUE == 1 ){
        ATH_MSG_DEBUG( "        Add to m_AGDD2GeoSwitches " << TheKEYNAME ) ;
        m_AGDD2GeoSwitches.push_back(TheKEYNAME);
      }
    }
  }else{
    ATH_MSG_DEBUG( "      agdd2geoVersion is empty " ) ;
  }
  ATH_MSG_DEBUG( "      AGDD2GeoSwitches data collected " ) ;

  if (m_CtrlFileOracle==1){
    std::ofstream  GeneratedFile;
    GeneratedFile.open("Generated_amdb_simrec_Oracle");
    GeneratedFile << AmdcString << std::endl;
    GeneratedFile << AgddString << std::endl;
    GeneratedFile.close();
  }

//Set Muon Spectrometer Geometry
  ATH_CHECK(SetFromString(AmdcString,AgddString));
  ATH_MSG_DEBUG( "Done: SetFromString" ) ;

//Post Geometry Loading Sequence
  ATH_CHECK(PostGeometryLoadingSequence());
  ATH_MSG_DEBUG( "Done: PostGeometryLoadingSequence" ) ;

  return StatusCode::SUCCESS;

}

StatusCode AmdcsimrecAthenaSvc::PostGeometryLoadingSequence() 
{

  ATH_MSG_DEBUG("----> PostGeometryLoadingSequence is called" ) ; 

//Produce strings
  ATH_CHECK(ProduceString());
  ATH_MSG_DEBUG( "Done: ProduceString" ) ;

//Possibly modify A line
  controlaline_(m_ControlALine);

//Possibly modify I line
  controliline_(m_ControlILine);

//Possibly truncate A line rotation parameters
  if (m_TruncateALine == 1 ) truncatealine_();

//Possibly truncate P line rotation parameters
  if (m_TruncatePLine == 1 ) truncatepline_();

//Delete p_Amdcsimrec and Set it
  delete p_Amdcsimrec ;
  p_Amdcsimrec = new AmdcsimrecStand;

//Set Internal Stores
  ATH_CHECK(SetAliStoreInternal());
  ATH_MSG_DEBUG( "Done: SetAliStoreInternal" ) ;

//Possibly Check Hard/Soft schemes
  if (m_HardSoftCheck == 1 ) {
    TestHardSoftStuff() ;
  }

//Possibly Xtomo stuff
  if (m_XtomoCheck == 1 ) {
    TestXtomoStuff() ;
  }

//Do Dumps
  if (m_CtrlDumpCo==1){
    cppbigdump(p_Amdcsimrec);
    bigamdcdump2_();
  }

  return StatusCode::SUCCESS;

}

StatusCode AmdcsimrecAthenaSvc::ProduceString()
{

  ATH_MSG_DEBUG("----> ProduceString is called" ) ; 

  m_AmdcString = "";
  int  Namdc = 0;
  fgetamdccharnumber_(Namdc);
  if (Namdc!=0){
    char* Camdc = new char[Namdc];
    fgetamdccharstring_(Namdc,Camdc);
    m_AmdcString.resize(Namdc);
    for (int i= 0; i <Namdc; i++){m_AmdcString[i]=Camdc[i];}
    delete [] Camdc ;
  }

  m_AgddString = "";
  int  Nagdd = 0;
  fgetagddcharnumber_(Nagdd);
  if (Nagdd!=0){
    char* Cagdd = new char[Nagdd];
    fgetagddcharstring_(Nagdd,Cagdd);
    m_AgddString.resize(Nagdd);
    for (int i= 0; i <Nagdd; i++){m_AgddString[i]=Cagdd[i];}
    delete [] Cagdd ;
  } 

  if (m_CtrlFileStrings==1){
    std::ofstream  GeneratedFile;
    GeneratedFile.open("Generated_amdb_simrec_String");
    GeneratedFile << m_AmdcString << std::endl;
    GeneratedFile << m_AgddString << std::endl;
    GeneratedFile.close();
  }

  return StatusCode::SUCCESS;

} 

StatusCode AmdcsimrecAthenaSvc::SetFromString(const std::string& AmdcString,
                                              const std::string& AgddString)
{

  ATH_MSG_DEBUG("----> SetFromString is called" ) ; 

  int   NAmdc = AmdcString.size();
  char* CAmdc = new char[NAmdc];
  for (int i= 0; i <NAmdc; i++){CAmdc[i]=AmdcString[i];}
  int   NAgdd = AgddString.size();
  char* CAgdd = new char[NAgdd];
  for (int i= 0; i <NAgdd; i++){CAgdd[i]=AgddString[i];}
  int IFLAG = 100 + m_PrintLevel ;
  loadamdcagddfromstringn_(NAmdc,CAmdc,NAgdd,CAgdd,IFLAG);
  delete [] CAmdc ;
  delete [] CAgdd ;

  return StatusCode::SUCCESS;

} 

StatusCode AmdcsimrecAthenaSvc::GetAmdcAliStores(
                               int&  AmdcABlinesStamp,
                               const AmdcAlineStore*& pAmdcAlineStore ,
                               const AmdcBlineStore*& pAmdcBlineStore ,
                               const AmdcIlineStore*& pAmdcIlineStore
){

  pAmdcAlineStore = nullptr ;
  pAmdcBlineStore = nullptr ;
  pAmdcIlineStore = nullptr ;
  
  if ( m_AlignmentCorr != 0 ) {
    if ( m_AlignmentSource == 2 ) {
      pAmdcAlineStore = p_AmdcAlineStoreExternal ;
      pAmdcBlineStore = p_AmdcBlineStoreExternal ;
      pAmdcIlineStore = p_AmdcIlineStoreExternal ;
      AmdcABlinesStamp = m_AmdcABlinesStamp ;
      return StatusCode::SUCCESS;
    } 
  }
  pAmdcAlineStore = p_AmdcAlineStoreInternal ;
  pAmdcBlineStore = p_AmdcBlineStoreInternal ;
  pAmdcIlineStore = p_AmdcIlineStoreInternal ;
  AmdcABlinesStamp = m_AmdcABlinesStamp ;
  return StatusCode::SUCCESS; 

}
StatusCode AmdcsimrecAthenaSvc::SetAliStoreInternal()
{

  ATH_MSG_DEBUG("----> SetAliStoreInternal is called" ) ; 
  
  p_AmdcAlineStoreInternal->Reset();
  p_AmdcBlineStoreInternal->Reset();  
  p_AmdcIlineStoreInternal->Reset();  
  
  if ( m_UseMuonDetectorManagerForInternal == 0 ){
    ATH_MSG_DEBUG( "      UseMuonDetectorManagerForInternal == 0 " ) ;
    int DB_Jadjust_Max = p_Amdcsimrec->NBadjust();
    for (int DB_Jadjust=1 ; DB_Jadjust<=DB_Jadjust_Max ; DB_Jadjust++){
      int    DB_JTYP ;
      int    DB_JFF  ;
      int    DB_JZZ  ;
      int    DB_JOB  ;
      double Amdc_TraS ;
      double Amdc_TraZ ;
      double Amdc_TraT ;
      double Amdc_RotS ;
      double Amdc_RotZ ;
      double Amdc_RotT ;
      p_Amdcsimrec->GetStationDisplacementFromAMDCJadjust(
                                                          DB_Jadjust,
                                                          DB_JTYP,
                                                          DB_JFF, 
                                                          DB_JZZ,
                                                          DB_JOB,
                                                          Amdc_TraS, 
                                                          Amdc_TraZ, 
                                                          Amdc_TraT,
                                                          Amdc_RotS, 
                                                          Amdc_RotZ, 
                                                          Amdc_RotT 
                                                         );
      if ( m_ModifyALineContainer == 2) 
      reformataline_( 
                     Amdc_TraS , 
                     Amdc_TraZ , 
                     Amdc_TraT , 
                     Amdc_RotS , 
                     Amdc_RotZ , 
                     Amdc_RotT 
                    );

      AmdcAline aAmdcAline;
      aAmdcAline.SetStationType(p_Amdcsimrec->StationName(DB_JTYP));
      aAmdcAline.Setjff  ( DB_JFF    );
      aAmdcAline.Setjzz  ( DB_JZZ    );
      aAmdcAline.Setjob  ( DB_JOB    );
      aAmdcAline.Sets    ( Amdc_TraS );
      aAmdcAline.Setz    ( Amdc_TraZ );
      aAmdcAline.Sett    ( Amdc_TraT );
      aAmdcAline.SetrotS ( Amdc_RotS );
      aAmdcAline.SetrotZ ( Amdc_RotZ );
      aAmdcAline.SetrotT ( Amdc_RotT );
      int StoreIt = 1 ;
      if ( p_Amdcsimrec->StationName(DB_JTYP) == "CSS" && m_RmAlignCSC == 1 ) StoreIt = 0 ;
      if ( p_Amdcsimrec->StationName(DB_JTYP) == "CSL" && m_RmAlignCSC == 1 ) StoreIt = 0 ;
      if ( StoreIt == 1 ) {
        p_AmdcAlineStoreInternal->Add(aAmdcAline);
      }
    }
    
    if ( m_SortAlignContainer == 1 ) p_AmdcAlineStoreInternal->Sort() ;
    
    int DB_Jdeform_Max = p_Amdcsimrec->NBdeform();
    for (int DB_Jdeform=1 ; DB_Jdeform<=DB_Jdeform_Max ; DB_Jdeform++){

      int    DB_JTYP ;
      int    DB_JFF  ;
      int    DB_JZZ  ;
      int    DB_JOB  ;
      double Amdc_bz ;
      double Amdc_bp ;
      double Amdc_bn ;
      double Amdc_sp ;
      double Amdc_sn ;
      double Amdc_tw ;
      double Amdc_pg ;
      double Amdc_tr ;
      double Amdc_eg ;
      double Amdc_ep ;
      double Amdc_en ;
      p_Amdcsimrec->GetStationDeformationFromAMDCJdeform(
                                                         DB_Jdeform,
                                                         DB_JTYP,
                                                         DB_JFF, 
                                                         DB_JZZ,
                                                         DB_JOB,
                                                         Amdc_bz,
                                                         Amdc_bp,
                                                         Amdc_bn,
                                                         Amdc_sp,
                                                         Amdc_sn,
                                                         Amdc_tw,
                                                         Amdc_pg,
                                                         Amdc_tr,
                                                         Amdc_eg,
                                                         Amdc_ep,
                                                         Amdc_en  
                                                        );
      AmdcBline aAmdcBline;
      aAmdcBline.SetStationType(p_Amdcsimrec->StationName(DB_JTYP));
      aAmdcBline.Setjff ( DB_JFF  );
      aAmdcBline.Setjzz ( DB_JZZ  );
      aAmdcBline.Setjob ( DB_JOB  );
      aAmdcBline.Setbz  ( Amdc_bz );
      aAmdcBline.Setbp  ( Amdc_bp );
      aAmdcBline.Setbn  ( Amdc_bn );
      aAmdcBline.Setsp  ( Amdc_sp );
      aAmdcBline.Setsn  ( Amdc_sn );
      aAmdcBline.Settw  ( Amdc_tw );
      aAmdcBline.Setpg  ( Amdc_pg );
      aAmdcBline.Settr  ( Amdc_tr );
      aAmdcBline.Seteg  ( Amdc_eg );
      aAmdcBline.Setep  ( Amdc_ep );
      aAmdcBline.Seten  ( Amdc_en );
      int StoreIt = 1 ;
      if ( p_Amdcsimrec->StationName(DB_JTYP) == "CSS" && m_RmAlignCSC == 1 ) StoreIt = 0 ;
      if ( p_Amdcsimrec->StationName(DB_JTYP) == "CSL" && m_RmAlignCSC == 1 ) StoreIt = 0 ;
      if ( StoreIt == 1 ) {
        p_AmdcBlineStoreInternal->Add(aAmdcBline);
      }
    }
    
    int DB_Jdwnlay_Max = p_Amdcsimrec->NBdwnlay();
    for (int DB_Jdwnlay=1 ; DB_Jdwnlay<=DB_Jdwnlay_Max ; DB_Jdwnlay++){
      int    DB_JTYP ;
      int    DB_JFF  ;
      int    DB_JZZ  ;
      int    DB_JOB  ;
      int    DB_JLAY ;
      double Amdc_TraS ;
      double Amdc_TraZ ;
      double Amdc_TraT ;
      double Amdc_RotS ;
      double Amdc_RotZ ;
      double Amdc_RotT ;
      p_Amdcsimrec->GetStationInternalAlignmentFromAMDCJdwnlay(
                                                          DB_Jdwnlay,
                                                          DB_JTYP,
                                                          DB_JFF, 
                                                          DB_JZZ,
                                                          DB_JOB,
                                                          DB_JLAY,
                                                          Amdc_TraS, 
                                                          Amdc_TraZ, 
                                                          Amdc_TraT,
                                                          Amdc_RotS, 
                                                          Amdc_RotZ, 
                                                          Amdc_RotT 
                                                         );
      AmdcIline aAmdcIline;
      aAmdcIline.SetStationType(p_Amdcsimrec->StationName(DB_JTYP));
      aAmdcIline.Setjff        ( DB_JFF       );
      aAmdcIline.Setjzz        ( DB_JZZ       );
      aAmdcIline.Setjob        ( DB_JOB       );
      aAmdcIline.Setjlay       ( DB_JLAY      );
      aAmdcIline.Sets          ( Amdc_TraS    );
      aAmdcIline.Setz          ( Amdc_TraZ    );
      aAmdcIline.Sett          ( Amdc_TraT    );
      aAmdcIline.SetrotS       ( Amdc_RotS    );
      aAmdcIline.SetrotZ       ( Amdc_RotZ    );
      aAmdcIline.SetrotT       ( Amdc_RotT    );
      p_AmdcIlineStoreInternal->Add(aAmdcIline);
    }
  }else{
    ATH_MSG_DEBUG( "      UseMuonDetectorManagerForInternal != 0 " ) ;
    int LoadIer = 0 ;
    ATH_CHECK(SetAmdcAlineStoreFromExternal(p_AmdcAlineStoreInternal,LoadIer));
    ATH_MSG_DEBUG( "Done: SetAmdcAlineStoreFromExternal" ) ;
    LoadIer = 0 ;
    ATH_CHECK(SetAmdcBlineStoreFromExternal(p_AmdcBlineStoreInternal,LoadIer));
    ATH_MSG_DEBUG( "Done: SetAmdcBlineStoreFromExternal" ) ;
    LoadIer = 0 ;
    ATH_CHECK(SetAmdcIlineStoreFromExternal(p_AmdcIlineStoreInternal,LoadIer));
    ATH_MSG_DEBUG( "Done: SetAmdcIlineStoreFromExternal" ) ;
  }

  p_AmdcAlineStoreExternal->Reset() ;
  int SizeAInternal = p_AmdcAlineStoreInternal->NberOfObjects();
  for (int Item=0 ; Item<SizeAInternal ; Item++){
    const AmdcAline* pAmdcAlineInternal = p_AmdcAlineStoreInternal->GetAmdcAline( Item ) ;
    AmdcAline aAmdcAline;
    aAmdcAline.SetStationType( pAmdcAlineInternal->GetStationType() );
    aAmdcAline.Setjff        ( pAmdcAlineInternal->Getjff        () );
    aAmdcAline.Setjzz        ( pAmdcAlineInternal->Getjzz        () );
    aAmdcAline.Setjob        ( pAmdcAlineInternal->Getjob        () );
    aAmdcAline.Sets          ( pAmdcAlineInternal->Gets          () );
    aAmdcAline.Setz          ( pAmdcAlineInternal->Getz          () );
    aAmdcAline.Sett          ( pAmdcAlineInternal->Gett          () );
    aAmdcAline.SetrotS       ( pAmdcAlineInternal->GetrotS       () );
    aAmdcAline.SetrotZ       ( pAmdcAlineInternal->GetrotZ       () );
    aAmdcAline.SetrotT       ( pAmdcAlineInternal->GetrotT       () );
    p_AmdcAlineStoreExternal->Add(aAmdcAline);
  }
  
  p_AmdcBlineStoreExternal->Reset() ;
  int SizeBInternal = p_AmdcBlineStoreInternal->NberOfObjects();
  for (int Item=0 ; Item<SizeBInternal ; Item++){
    const AmdcBline* pAmdcBlineInternal = p_AmdcBlineStoreInternal->GetAmdcBline(Item);
    AmdcBline aAmdcBline;
    aAmdcBline.SetStationType( pAmdcBlineInternal->GetStationType() );
    aAmdcBline.Setjff        ( pAmdcBlineInternal->Getjff        () );
    aAmdcBline.Setjzz        ( pAmdcBlineInternal->Getjzz        () );
    aAmdcBline.Setjob        ( pAmdcBlineInternal->Getjob        () );
    aAmdcBline.Setbz         ( pAmdcBlineInternal->Getbz         () );
    aAmdcBline.Setbp         ( pAmdcBlineInternal->Getbp         () );
    aAmdcBline.Setbn         ( pAmdcBlineInternal->Getbn         () );
    aAmdcBline.Setsp         ( pAmdcBlineInternal->Getsp         () );
    aAmdcBline.Setsn         ( pAmdcBlineInternal->Getsn         () );
    aAmdcBline.Settw         ( pAmdcBlineInternal->Gettw         () );
    aAmdcBline.Setpg         ( pAmdcBlineInternal->Getpg         () );
    aAmdcBline.Settr         ( pAmdcBlineInternal->Gettr         () );
    aAmdcBline.Seteg         ( pAmdcBlineInternal->Geteg         () );
    aAmdcBline.Setep         ( pAmdcBlineInternal->Getep         () );
    aAmdcBline.Seten         ( pAmdcBlineInternal->Geten         () );
    p_AmdcBlineStoreExternal->Add(aAmdcBline);
  }

  p_AmdcIlineStoreExternal->Reset() ;
  int SizeIInternal = p_AmdcIlineStoreInternal->NberOfObjects();
  for (int Item=0 ; Item<SizeIInternal ; Item++){
    const AmdcIline* pAmdcIlineInternal = p_AmdcIlineStoreInternal->GetAmdcIlineForUpdate( Item ) ;
    AmdcIline aAmdcIline;
    aAmdcIline.SetStationType( pAmdcIlineInternal->GetStationType() );
    aAmdcIline.Setjff        ( pAmdcIlineInternal->Getjff        () );
    aAmdcIline.Setjzz        ( pAmdcIlineInternal->Getjzz        () );
    aAmdcIline.Setjob        ( pAmdcIlineInternal->Getjob        () );
    aAmdcIline.Setjlay       ( pAmdcIlineInternal->Getjlay       () );
    aAmdcIline.Sets          ( pAmdcIlineInternal->Gets          () );
    aAmdcIline.Setz          ( pAmdcIlineInternal->Getz          () );
    aAmdcIline.Sett          ( pAmdcIlineInternal->Gett          () );
    aAmdcIline.SetrotS       ( pAmdcIlineInternal->GetrotS       () );
    aAmdcIline.SetrotZ       ( pAmdcIlineInternal->GetrotZ       () );
    aAmdcIline.SetrotT       ( pAmdcIlineInternal->GetrotT       () );
    p_AmdcIlineStoreExternal->Add(aAmdcIline);
  }

  ATH_MSG_DEBUG( "      p_AmdcAlineStoreInternal->NberOfObjects()" << p_AmdcAlineStoreInternal->NberOfObjects() ) ;
  ATH_MSG_DEBUG( "      p_AmdcAlineStoreExternal->NberOfObjects()" << p_AmdcAlineStoreExternal->NberOfObjects() ) ;
  ATH_MSG_DEBUG( "      p_AmdcBlineStoreInternal->NberOfObjects()" << p_AmdcBlineStoreInternal->NberOfObjects() ) ;
  ATH_MSG_DEBUG( "      p_AmdcBlineStoreExternal->NberOfObjects()" << p_AmdcBlineStoreExternal->NberOfObjects() ) ;
  ATH_MSG_DEBUG( "      p_AmdcIlineStoreInternal->NberOfObjects()" << p_AmdcIlineStoreInternal->NberOfObjects() ) ;
  ATH_MSG_DEBUG( "      p_AmdcIlineStoreExternal->NberOfObjects()" << p_AmdcIlineStoreExternal->NberOfObjects() ) ;

  return StatusCode::SUCCESS;

}

StatusCode AmdcsimrecAthenaSvc::SetAmdcABlineFromCool()
{

 ATH_MSG_DEBUG( "----> SetAmdcABlineFromCool is called " ) ;
 ATH_CHECK(p_detStore->retrieve(p_MuonDetectorManager));
 
 if ( m_DontSetAmdcABlineFromCool == 1 ){
   ATH_MSG_DEBUG( "DontSetAmdcABlineFromCool == 1; SetAmdcABlineFromCool will not change the containers " ) ; 
   return StatusCode::SUCCESS;
 }

 if ( m_AlignmentCorr == 1  || m_AlignmentCorr == 3 ){
   ATH_CHECK(SetAmdcAlineStoreExternal());
   ATH_MSG_DEBUG( "Done: SetAmdcAlineStoreExternal" ) ;
 }
 
 if ( m_AlignmentCorr == 2 || m_AlignmentCorr == 3 ){
   ATH_CHECK(SetAmdcBlineStoreExternal());
   ATH_MSG_DEBUG( "Done: SetAmdcBlineStoreExternal" ) ;
 }

 ATH_CHECK(SetAmdcIlineStoreExternal());
 ATH_MSG_DEBUG( "Done: SetAmdcIlineStoreExternal" ) ;

 return StatusCode::SUCCESS;
  
}

StatusCode AmdcsimrecAthenaSvc::SetAmdcAlineStoreExternal()
{

  ATH_MSG_DEBUG( "----> SetAmdcAlineStoreExternal is called " ) ;

  int LoadIer = 0 ;
  ATH_CHECK(SetAmdcAlineStoreFromExternal(p_AmdcAlineStoreExternal,LoadIer));
  if (LoadIer==1){
    ATH_MSG_DEBUG( "SetAmdcAlineStoreFromExternal did not changed the store " ) ; 
    ATH_MSG_DEBUG( "=> will not change the container " ) ; 
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG( "Done: SetAmdcAlineStoreFromExternal" ) ;

  m_AmdcABlinesStamp = m_AmdcABlinesStamp + 1 ;

  if ( m_ModifyALineContainer == 1) {
    int SizeInternal = p_AmdcAlineStoreInternal->NberOfObjects();
    for (int Item=0 ; Item<SizeInternal ; Item++){
      const AmdcAline* pAmdcAlineInternal = p_AmdcAlineStoreInternal->GetAmdcAline( Item ) ;
      int JTYP_Internal = pAmdcAlineInternal->Getjtyp() ;
      int JFF_Internal  = pAmdcAlineInternal->Getjff()  ;
      int JZZ_Internal  = pAmdcAlineInternal->Getjzz()  ;
      int JOB_Internal  = pAmdcAlineInternal->Getjob()  ;
      int Item_External = p_AmdcAlineStoreExternal->getData(JTYP_Internal, JFF_Internal ,JZZ_Internal, JOB_Internal);
      if ( Item_External != -1) {
        double The_s    = pAmdcAlineInternal->Gets   () ;
        double The_z    = pAmdcAlineInternal->Getz   () ;
        double The_t    = pAmdcAlineInternal->Gett   () ;
        double The_rotS = pAmdcAlineInternal->GetrotS() ;
        double The_rotZ = pAmdcAlineInternal->GetrotZ() ;
        double The_rotT = pAmdcAlineInternal->GetrotT() ;
        AmdcAline* pAmdcAlineExternal = p_AmdcAlineStoreExternal->GetAmdcAlineForUpdate( Item_External ) ;
        pAmdcAlineExternal->Sets    ( The_s    );
        pAmdcAlineExternal->Setz    ( The_z    );
        pAmdcAlineExternal->Sett    ( The_t    );
        pAmdcAlineExternal->SetrotS ( The_rotS );
        pAmdcAlineExternal->SetrotZ ( The_rotZ );
        pAmdcAlineExternal->SetrotT ( The_rotT );
      }
    }
  }
         
  ATH_MSG_DEBUG( "      p_AmdcAlineStoreExternal->NberOfObjects()" << p_AmdcAlineStoreExternal->NberOfObjects() ) ;

  if ( m_CompareInternalExternalALineContainer == 1) {
  
    int SizeExternal = p_AmdcAlineStoreExternal->NberOfObjects();
    int SizeInternal = p_AmdcAlineStoreInternal->NberOfObjects();

    ATH_MSG_DEBUG(" AmdcsimrecAthenaSvc::SetAmdcAlineStoreExternal Compare "); 
    if ( SizeExternal != SizeInternal ){
      ATH_MSG_DEBUG("=> SizeExternal != SizeInternal " << " " << SizeExternal << " " << SizeInternal);
    }
    double MaxDiffGets    = 0. ;
    double MaxDiffGetz    = 0. ;
    double MaxDiffGett    = 0. ;
    double MaxDiffGetrotS = 0. ;
    double MaxDiffGetrotZ = 0. ;
    double MaxDiffGetrotT = 0. ;
    for (int Item=0 ; Item<SizeInternal ; Item++){
      const AmdcAline* pAmdcAlineInternal = p_AmdcAlineStoreInternal->GetAmdcAline( Item ) ;
      pAmdcAlineInternal->SuperPrint();
      int JTYP_Internal = pAmdcAlineInternal->Getjtyp() ;
      int JFF_Internal  = pAmdcAlineInternal->Getjff()  ;
      int JZZ_Internal  = pAmdcAlineInternal->Getjzz()  ;
      int JOB_Internal  = pAmdcAlineInternal->Getjob()  ;
      int Item_External = p_AmdcAlineStoreExternal->getData(JTYP_Internal, JFF_Internal ,JZZ_Internal, JOB_Internal);
      if ( Item_External != -1){
        const AmdcAline* pAmdcAlineExternal = p_AmdcAlineStoreExternal->GetAmdcAline( Item_External ) ;
        pAmdcAlineExternal->SuperPrint();
        double DiffGets    = pAmdcAlineExternal->Gets    () - pAmdcAlineInternal->Gets    () ;
        double DiffGetz    = pAmdcAlineExternal->Getz    () - pAmdcAlineInternal->Getz    () ;
        double DiffGett    = pAmdcAlineExternal->Gett    () - pAmdcAlineInternal->Gett    () ;
        double DiffGetrotS = pAmdcAlineExternal->GetrotS () - pAmdcAlineInternal->GetrotS () ;
        double DiffGetrotZ = pAmdcAlineExternal->GetrotZ () - pAmdcAlineInternal->GetrotZ () ;
        double DiffGetrotT = pAmdcAlineExternal->GetrotT () - pAmdcAlineInternal->GetrotT () ;
        ATH_MSG_DEBUG(setiosflags(std::ios::fixed)
            <<  std::setw(3)                        << pAmdcAlineExternal->GetStationType() 
            <<  std::setw(4)                        << pAmdcAlineExternal->Getjff        () 
            <<  std::setw(4)                        << pAmdcAlineExternal->Getjzz        () 
            <<  std::setw(4)                        << pAmdcAlineExternal->Getjob        ()
            <<  std::setw(13)<<std::setprecision(6) << DiffGets    
            <<  std::setw(13)<<std::setprecision(6) << DiffGetz    
            <<  std::setw(13)<<std::setprecision(6) << DiffGett    
            <<  std::setw(16)<<std::setprecision(12)<< DiffGetrotS 
            <<  std::setw(16)<<std::setprecision(12)<< DiffGetrotZ 
            <<  std::setw(16)<<std::setprecision(12)<< DiffGetrotT);
        getAbsMax(MaxDiffGets, DiffGets);
        getAbsMax(MaxDiffGetz, DiffGetz);
        getAbsMax(MaxDiffGett, DiffGett);
        getAbsMax(MaxDiffGetrotS, DiffGetrotS);
        getAbsMax(MaxDiffGetrotZ, DiffGetrotZ);
        getAbsMax(MaxDiffGetrotT, DiffGetrotT);
      }
    }

    ATH_MSG_DEBUG(std::setw(3)                  << "Max" 
        <<  std::setw(4)                        << "Diff" 
        <<  std::setw(4)                        << "    " 
        <<  std::setw(4)                        << "    "
        <<  std::setw(13)<<std::setprecision(6) << MaxDiffGets    
        <<  std::setw(13)<<std::setprecision(6) << MaxDiffGetz    
        <<  std::setw(13)<<std::setprecision(6) << MaxDiffGett    
        <<  std::setw(16)<<std::setprecision(12)<< MaxDiffGetrotS 
        <<  std::setw(16)<<std::setprecision(12)<< MaxDiffGetrotZ 
        <<  std::setw(16)<<std::setprecision(12)<< MaxDiffGetrotT); 

    ATH_MSG_DEBUG(" In Internal and not in External"); 
    for (int Item=0 ; Item<SizeInternal ; Item++){
      const AmdcAline* pAmdcAlineInternal = p_AmdcAlineStoreInternal->GetAmdcAline( Item ) ;
      int JTYP_Internal = pAmdcAlineInternal->Getjtyp() ;
      int JFF_Internal  = pAmdcAlineInternal->Getjff()  ;
      int JZZ_Internal  = pAmdcAlineInternal->Getjzz()  ;
      int JOB_Internal  = pAmdcAlineInternal->Getjob()  ;
      int Item_External = p_AmdcAlineStoreExternal->getData(JTYP_Internal, JFF_Internal ,JZZ_Internal, JOB_Internal);
      if ( Item_External == -1) pAmdcAlineInternal->SuperPrint();
    }

    ATH_MSG_DEBUG(" In External and not in Internal"); 
    for (int Item=0 ; Item<SizeExternal ; Item++){
      const AmdcAline* pAmdcAlineExternal = p_AmdcAlineStoreExternal->GetAmdcAline( Item ) ;
      int JTYP_External = pAmdcAlineExternal->Getjtyp() ;
      int JFF_External  = pAmdcAlineExternal->Getjff()  ;
      int JZZ_External  = pAmdcAlineExternal->Getjzz()  ;
      int JOB_External  = pAmdcAlineExternal->Getjob()  ;
      int Item_Internal = p_AmdcAlineStoreInternal->getData(JTYP_External, JFF_External ,JZZ_External, JOB_External);
      if ( Item_Internal == -1) pAmdcAlineExternal->SuperPrint();
    }
    
  }
  
  ATH_CHECK(LoadAlineStoreExternal());
  ATH_MSG_DEBUG( "Done: LoadAlineStoreExternal" ) ; 

  return StatusCode::SUCCESS;
   
}
StatusCode AmdcsimrecAthenaSvc::SetAmdcBlineStoreExternal()
{

  ATH_MSG_DEBUG( "----> SetAmdcBlineStoreExternal is called " ) ;

  int LoadIer = 0 ;
  ATH_CHECK(SetAmdcBlineStoreFromExternal(p_AmdcBlineStoreExternal,LoadIer));
  if (LoadIer==1){
    ATH_MSG_DEBUG( "SetAmdcBlineStoreFromExternal did not changed the store " ) ; 
    ATH_MSG_DEBUG( "=> will not change the container " ) ; 
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG( "Done: SetAmdcBlineStoreFromExternal" ) ; 

  m_AmdcABlinesStamp = m_AmdcABlinesStamp + 1 ;
   
  ATH_MSG_DEBUG( "      p_AmdcBlineStoreExternal->NberOfObjects()" << p_AmdcBlineStoreExternal->NberOfObjects() ) ;

  if ( m_CompareInternalExternalALineContainer == 1) {
    int SizeExternal = p_AmdcBlineStoreExternal->NberOfObjects();
    int SizeInternal = p_AmdcBlineStoreInternal->NberOfObjects();

    ATH_MSG_DEBUG(" AmdcsimrecAthenaSvc::SetAmdcBlineStoreExternal Compare "); 
    if ( SizeExternal != SizeInternal ){
      ATH_MSG_DEBUG("=> SizeExternal != SizeInternal " << " " << SizeExternal << " " << SizeInternal);
    }
    double MaxDiffGetbz = 0. ;
    double MaxDiffGetbp = 0. ;
    double MaxDiffGetbn = 0. ;
    double MaxDiffGetsp = 0. ;
    double MaxDiffGetsn = 0. ;
    double MaxDiffGettw = 0. ;
    double MaxDiffGetpg = 0. ;
    double MaxDiffGettr = 0. ;
    double MaxDiffGeteg = 0. ;
    double MaxDiffGetep = 0. ;
    double MaxDiffGeten = 0. ;
    for (int Item=0 ; Item<SizeInternal ; Item++){
      const AmdcBline* pAmdcBlineInternal = p_AmdcBlineStoreInternal->GetAmdcBline( Item ) ;
      pAmdcBlineInternal->SuperPrint();
      int JTYP_Internal = pAmdcBlineInternal->Getjtyp() ;
      int JFF_Internal  = pAmdcBlineInternal->Getjff()  ;
      int JZZ_Internal  = pAmdcBlineInternal->Getjzz()  ;
      int JOB_Internal  = pAmdcBlineInternal->Getjob()  ;
      int Item_External = p_AmdcBlineStoreExternal->getData(JTYP_Internal, JFF_Internal ,JZZ_Internal, JOB_Internal);
      if ( Item_External != -1){
        const AmdcBline* pAmdcBlineExternal = p_AmdcBlineStoreExternal->GetAmdcBline( Item_External ) ;
        pAmdcBlineExternal->SuperPrint();
        double DiffGetbz = pAmdcBlineExternal->Getbz () - pAmdcBlineInternal->Getbz () ;
        double DiffGetbp = pAmdcBlineExternal->Getbp () - pAmdcBlineInternal->Getbp () ;
        double DiffGetbn = pAmdcBlineExternal->Getbn () - pAmdcBlineInternal->Getbn () ;
        double DiffGetsp = pAmdcBlineExternal->Getsp () - pAmdcBlineInternal->Getsp () ;
        double DiffGetsn = pAmdcBlineExternal->Getsn () - pAmdcBlineInternal->Getsn () ;
        double DiffGettw = pAmdcBlineExternal->Gettw () - pAmdcBlineInternal->Gettw () ;
        double DiffGetpg = pAmdcBlineExternal->Getpg () - pAmdcBlineInternal->Getpg () ;
        double DiffGettr = pAmdcBlineExternal->Gettr () - pAmdcBlineInternal->Gettr () ;
        double DiffGeteg = pAmdcBlineExternal->Geteg () - pAmdcBlineInternal->Geteg () ;
        double DiffGetep = pAmdcBlineExternal->Getep () - pAmdcBlineInternal->Getep () ;
        double DiffGeten = pAmdcBlineExternal->Geten () - pAmdcBlineInternal->Geten () ;
        ATH_MSG_DEBUG(setiosflags(std::ios::fixed)
            <<  std::setw(3)                        << pAmdcBlineExternal->GetStationType() 
            <<  std::setw(4)                        << pAmdcBlineExternal->Getjff        () 
            <<  std::setw(4)                        << pAmdcBlineExternal->Getjzz        () 
            <<  std::setw(4)                        << pAmdcBlineExternal->Getjob        ()
            <<  std::setw(16)<<std::setprecision(12)<< DiffGetbz 
            <<  std::setw(16)<<std::setprecision(12)<< DiffGetbp 
            <<  std::setw(16)<<std::setprecision(12)<< DiffGetbn 
            <<  std::setw(16)<<std::setprecision(12)<< DiffGetsp 
            <<  std::setw(16)<<std::setprecision(12)<< DiffGetsn 
            <<  std::setw(16)<<std::setprecision(12)<< DiffGettw 
            <<  std::setw(16)<<std::setprecision(12)<< DiffGetpg 
            <<  std::setw(16)<<std::setprecision(12)<< DiffGettr 
            <<  std::setw(16)<<std::setprecision(12)<< DiffGeteg 
            <<  std::setw(16)<<std::setprecision(12)<< DiffGetep 
            <<  std::setw(16)<<std::setprecision(12)<< DiffGeten);
        getAbsMax(MaxDiffGetbz, DiffGetbz);
        getAbsMax(MaxDiffGetbp, DiffGetbp);
        getAbsMax(MaxDiffGetbn, DiffGetbn);
        getAbsMax(MaxDiffGetsp, DiffGetsp);
        getAbsMax(MaxDiffGetsn, DiffGetsn);
        getAbsMax(MaxDiffGettw, DiffGettw);
        getAbsMax(MaxDiffGetpg, DiffGetpg);
        getAbsMax(MaxDiffGettr, DiffGettr);
        getAbsMax(MaxDiffGeteg, DiffGeteg);
        getAbsMax(MaxDiffGetep, DiffGetep);
        getAbsMax(MaxDiffGeten, DiffGeten);
      }
    }

    ATH_MSG_DEBUG(std::setw(3)                  << "Max" 
        <<  std::setw(4)                        << "Diff" 
        <<  std::setw(4)                        << "    " 
        <<  std::setw(4)                        << "    "
        <<  std::setw(16)<<std::setprecision(12)<< MaxDiffGetbz 
        <<  std::setw(16)<<std::setprecision(12)<< MaxDiffGetbp 
        <<  std::setw(16)<<std::setprecision(12)<< MaxDiffGetbn 
        <<  std::setw(16)<<std::setprecision(12)<< MaxDiffGetsp 
        <<  std::setw(16)<<std::setprecision(12)<< MaxDiffGetsn 
        <<  std::setw(16)<<std::setprecision(12)<< MaxDiffGettw 
        <<  std::setw(16)<<std::setprecision(12)<< MaxDiffGetpg 
        <<  std::setw(16)<<std::setprecision(12)<< MaxDiffGettr 
        <<  std::setw(16)<<std::setprecision(12)<< MaxDiffGeteg 
        <<  std::setw(16)<<std::setprecision(12)<< MaxDiffGetep 
        <<  std::setw(16)<<std::setprecision(12)<< MaxDiffGeten); 

    ATH_MSG_DEBUG(" In Internal and not in External"); 
    for (int Item=0 ; Item<SizeInternal ; Item++){
      const AmdcBline* pAmdcBlineInternal = p_AmdcBlineStoreInternal->GetAmdcBline( Item ) ;
      int JTYP_Internal = pAmdcBlineInternal->Getjtyp() ;
      int JFF_Internal  = pAmdcBlineInternal->Getjff()  ;
      int JZZ_Internal  = pAmdcBlineInternal->Getjzz()  ;
      int JOB_Internal  = pAmdcBlineInternal->Getjob()  ;
      int Item_External = p_AmdcBlineStoreExternal->getData(JTYP_Internal, JFF_Internal ,JZZ_Internal, JOB_Internal);
      if ( Item_External == -1) pAmdcBlineInternal->SuperPrint();
    }

    ATH_MSG_DEBUG(" In External and not in Internal"); 
    for (int Item=0 ; Item<SizeExternal ; Item++){
      const AmdcBline* pAmdcBlineExternal = p_AmdcBlineStoreExternal->GetAmdcBline( Item ) ;
      int JTYP_External = pAmdcBlineExternal->Getjtyp() ;
      int JFF_External  = pAmdcBlineExternal->Getjff()  ;
      int JZZ_External  = pAmdcBlineExternal->Getjzz()  ;
      int JOB_External  = pAmdcBlineExternal->Getjob()  ;
      int Item_Internal = p_AmdcBlineStoreInternal->getData(JTYP_External, JFF_External ,JZZ_External, JOB_External);
      if ( Item_Internal == -1) pAmdcBlineExternal->SuperPrint();
    }
    
  }
  
  ATH_CHECK(LoadBlineStoreExternal());
  ATH_MSG_DEBUG( "Done: LoadBlineStoreExternal" ) ; 

  return StatusCode::SUCCESS;
   
}
StatusCode AmdcsimrecAthenaSvc::SetAmdcIlineStoreExternal()
{

  ATH_MSG_DEBUG( "----> SetAmdcIlineStoreExternal is called " ) ;

  int LoadIer = 0 ;
  ATH_CHECK(SetAmdcIlineStoreFromExternal(p_AmdcIlineStoreExternal,LoadIer));
  if (LoadIer==1){
    ATH_MSG_DEBUG( "SetAmdcIlineStoreFromExternal did not changed the store " ) ; 
    ATH_MSG_DEBUG( "=> will not change the container " ) ; 
    return StatusCode::SUCCESS;
  }
  ATH_MSG_DEBUG( "Done: SetAmdcIlineStoreFromExternal" ) ; 

  m_AmdcABlinesStamp = m_AmdcABlinesStamp + 1 ;
   
  ATH_CHECK(LoadCscInternalAlignmentStoreExternal());
  ATH_MSG_DEBUG( "Done: LoadCscInternalAlignmentStoreExternal" ) ; 

  return StatusCode::SUCCESS;
   
}
StatusCode AmdcsimrecAthenaSvc::SetAmdcAlineStoreFromExternal(AmdcAlineStore* pAmdcAlineStore, int& LoadIer) {
  LoadIer = 1 ;
  
  if ( m_SortAlignContainer == 1 ) pAmdcAlineStore->Sort() ;

  ATH_MSG_DEBUG( "      pAmdcAlineStore->NberOfObjects()" << pAmdcAlineStore->NberOfObjects() ) ;

  return StatusCode::SUCCESS;
   
}
StatusCode AmdcsimrecAthenaSvc::SetAmdcBlineStoreFromExternal(AmdcBlineStore* pAmdcBlineStore,int& LoadIer)
{

  ATH_MSG_DEBUG( "----> SetAmdcBlineStoreFromExternal is called " ) ;
  LoadIer = 1 ;
  ATH_MSG_DEBUG( "      pAmdcBlineStore->NberOfObjects()" << pAmdcBlineStore->NberOfObjects() ) ;

  return StatusCode::SUCCESS;
   
}
StatusCode AmdcsimrecAthenaSvc::SetAmdcIlineStoreFromExternal(AmdcIlineStore* pAmdcIlineStore,int& LoadIer)
{

  ATH_MSG_DEBUG( "----> SetAmdcIlineStoreFromExternal is called " ) ;

  LoadIer = 1 ;  
  ATH_MSG_DEBUG( "Done: GetCscInternalAlignmentMapContainer" ) ; 

  ATH_MSG_DEBUG( "      AmdcIlineStore->NberOfObjects()" << pAmdcIlineStore->NberOfObjects() ) ;

  return StatusCode::SUCCESS;
   
}


void AmdcsimrecAthenaSvc::TestHardSoftStuff(){ p_Amdcsimrec->TestHardSoftStuff(); }
void AmdcsimrecAthenaSvc::TestXtomoStuff(){ p_Amdcsimrec->TestXtomoStuff(); }



int AmdcsimrecAthenaSvc::GetAGDD2GeoSwitchesStamp() { return m_AGDD2GeoSwitchesStamp ;}

std::vector<std::string> AmdcsimrecAthenaSvc::GetAGDD2GeoSwitches()  {return m_AGDD2GeoSwitches;}


StatusCode AmdcsimrecAthenaSvc::SetLocation(const std::string& NameFile, std::string& FileLocation)
{

  ATH_MSG_DEBUG( "----> SetLocation is called " ) ;

  if (""==NameFile) {
    ATH_MSG_FATAL( "NameFile not set !" ) ;
    return StatusCode::FAILURE;
  }

  FileLocation = PathResolver::find_file (NameFile, "DATAPATH");

  if (FileLocation == ""){
    ATH_MSG_FATAL( "Cannot locate " << NameFile << " from ${DATAPATH}" ) ;
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;

}

StatusCode AmdcsimrecAthenaSvc::LoadAlineStoreExternal()
{

  ATH_MSG_DEBUG( "----> LoadAlineStoreExternal is called " ) ;

  std::string StationType ; 
  int         jff         ;
  int         jzz         ;
  int         job         ;
  char STANAME[3];

  int ierr ;

  double DataAline[6];
  double s    ;
  double z    ;
  double t    ;
  double rotS ;
  double rotZ ;
  double rotT ;
  
  jff  = 0 ;
  jzz  = 0 ;
  job  = 0 ;
  ierr = 0 ;

  STANAME[0] = 'Z' ;
  STANAME[1] = 'Z' ;
  STANAME[2] = 'Z' ;
  
  stasetalines_(STANAME, jff, jzz, job, DataAline, ierr);

  int TheSize = p_AmdcAlineStoreExternal->NberOfObjects();
  for (int Index= 0 ; Index <TheSize ; Index++){
    const AmdcAline* pAmdcAline = p_AmdcAlineStoreExternal->GetAmdcAline(Index);
      
      StationType = pAmdcAline->GetStationType();
      jff         = pAmdcAline->Getjff        ();
      jzz         = pAmdcAline->Getjzz        ();
      job         = pAmdcAline->Getjob        ();
      pAmdcAline->GetStationNameChar(STANAME);

      s   = pAmdcAline->Gets    () ;
      z   = pAmdcAline->Getz    () ;
      t   = pAmdcAline->Gett    () ;
      rotS = pAmdcAline->GetrotS () ;
      rotZ = pAmdcAline->GetrotZ () ;
      rotT = pAmdcAline->GetrotT () ;

      DataAline[0] = s    ;
      DataAline[1] = z    ;
      DataAline[2] = t    ;
      DataAline[3] = rotS ;
      DataAline[4] = rotZ ;
      DataAline[5] = rotT ;
      stasetalines_(STANAME, jff, jzz, job, DataAline, ierr);
      if ( ierr != 0 ) {
        ATH_MSG_FATAL( "stasetalines_ failed on " ) ; 
        ATH_MSG_FATAL( "  StationType " << StationType ) ; 
        ATH_MSG_FATAL( "  jff         " << jff         ) ; 
        ATH_MSG_FATAL( "  jzz         " << jzz         ) ; 
        ATH_MSG_FATAL( "  job         " << job         ) ; 
        ATH_MSG_FATAL( "  s    " << s    ) ; 
        ATH_MSG_FATAL( "  z    " << z    ) ; 
        ATH_MSG_FATAL( "  t    " << t    ) ; 
        ATH_MSG_FATAL( "  rotS " << rotS ) ; 
        ATH_MSG_FATAL( "  rotZ " << rotZ ) ; 
        ATH_MSG_FATAL( "  rotT " << rotT ) ; 
        return StatusCode::FAILURE;
      }

  }

  return StatusCode::SUCCESS;
   
}

StatusCode AmdcsimrecAthenaSvc::LoadBlineStoreExternal()
{

  ATH_MSG_DEBUG( "----> LoadBlineStoreExternal is called " ) ;

  std::string StationType ; 
  int         jff         ;
  int         jzz         ;
  int         job         ;
  char STANAME[3];

  int ierr ;

  double DataBline[11];
  double bz ;
  double bp ;
  double bn ;
  double sp ;
  double sn ;
  double tw ;
  double pg ;
  double tr ;
  double eg ;
  double ep ;
  double en ;
  
  jff  = 0 ;
  jzz  = 0 ;
  job  = 0 ;
  ierr = 0 ;

  STANAME[0] = 'Z' ;
  STANAME[1] = 'Z' ;
  STANAME[2] = 'Z' ;
  
  stasetblines_(STANAME, jff, jzz, job, DataBline, ierr);

  int TheSize = p_AmdcBlineStoreExternal->NberOfObjects();
  for (int Index= 0 ; Index <TheSize ; Index++){
    const AmdcBline* pAmdcBline = p_AmdcBlineStoreExternal->GetAmdcBline(Index);
      
      StationType = pAmdcBline->GetStationType();
      jff         = pAmdcBline->Getjff        ();
      jzz         = pAmdcBline->Getjzz        ();
      job         = pAmdcBline->Getjob        ();
      pAmdcBline->GetStationNameChar(STANAME);

      bz = pAmdcBline->Getbz();
      bp = pAmdcBline->Getbp();
      bn = pAmdcBline->Getbn();
      sp = pAmdcBline->Getsp();
      sn = pAmdcBline->Getsn();
      tw = pAmdcBline->Gettw();
      pg = pAmdcBline->Getpg();
      tr = pAmdcBline->Gettr();
      eg = pAmdcBline->Geteg();
      ep = pAmdcBline->Getep();
      en = pAmdcBline->Geten();

      DataBline[ 0] = bz;
      DataBline[ 1] = bp;
      DataBline[ 2] = bn;
      DataBline[ 3] = sp;
      DataBline[ 4] = sn;
      DataBline[ 5] = tw;
      DataBline[ 6] = pg;
      DataBline[ 7] = tr;
      DataBline[ 8] = eg;
      DataBline[ 9] = ep;
      DataBline[10] = en;
      stasetblines_(STANAME, jff, jzz, job, DataBline, ierr);
      if ( ierr != 0 ) {
        ATH_MSG_FATAL( " stasetblines_ failed on " ) ; 
        ATH_MSG_FATAL( "  StationType " << StationType ) ; 
        ATH_MSG_FATAL( "  jff         " << jff         ) ; 
        ATH_MSG_FATAL( "  jzz         " << jzz         ) ; 
        ATH_MSG_FATAL( "  job         " << job         ) ; 
        ATH_MSG_FATAL( "  bz  " << bz ) ; 
        ATH_MSG_FATAL( "  bp  " << bp ) ; 
        ATH_MSG_FATAL( "  bn  " << bn ) ; 
        ATH_MSG_FATAL( "  sp  " << sp ) ; 
        ATH_MSG_FATAL( "  sn  " << sn ) ; 
        ATH_MSG_FATAL( "  tw  " << tw ) ; 
        ATH_MSG_FATAL( "  pg  " << pg ) ; 
        ATH_MSG_FATAL( "  tr  " << tr ) ; 
        ATH_MSG_FATAL( "  eg  " << eg ) ; 
        ATH_MSG_FATAL( "  ep  " << ep ) ;
        ATH_MSG_FATAL( "  en  " << en ) ; 
        return StatusCode::FAILURE;
      }

  }

  return StatusCode::SUCCESS;
   
}
StatusCode AmdcsimrecAthenaSvc::LoadCscInternalAlignmentStoreExternal()
{

  ATH_MSG_DEBUG( "----> LoadCscInternalAlignmentStoreExternal is called " ) ;

  std::string StationType ; 
  int         jff         ;
  int         jzz         ;
  int         job         ;
  int         jlay        ;
  char STANAME[3];

  int ierr ;

  double DataIline[6];
  double s    ;
  double z    ;
  double t    ;
  double rotS ;
  double rotZ ;
  double rotT ;
  
  jff  = 0 ;
  jzz  = 0 ;
  job  = 0 ;
  jlay = 0 ;
  ierr = 0 ;

  STANAME[0] = 'Z' ;
  STANAME[1] = 'Z' ;
  STANAME[2] = 'Z' ;
  
  stasetilines_(STANAME, jff, jzz, job, jlay, DataIline, ierr);

  int TheSize = p_AmdcIlineStoreExternal->NberOfObjects();
  for (int Index= 0 ; Index <TheSize ; Index++){
    const AmdcIline* pAmdcIline = p_AmdcIlineStoreExternal->GetAmdcIline(Index);
      
      StationType = pAmdcIline->GetStationType();
      jff         = pAmdcIline->Getjff        ();
      jzz         = pAmdcIline->Getjzz        ();
      job         = pAmdcIline->Getjob        ();
      jlay        = pAmdcIline->Getjlay       ();
      pAmdcIline->GetStationNameChar(STANAME);

      s   = pAmdcIline->Gets    () ;
      z   = pAmdcIline->Getz    () ;
      t   = pAmdcIline->Gett    () ;
      rotS = pAmdcIline->GetrotS () ;
      rotZ = pAmdcIline->GetrotZ () ;
      rotT = pAmdcIline->GetrotT () ;

      DataIline[0] = s    ;
      DataIline[1] = z    ;
      DataIline[2] = t    ;
      DataIline[3] = rotS ;
      DataIline[4] = rotZ ;
      DataIline[5] = rotT ;
      stasetilines_(STANAME, jff, jzz, job, jlay, DataIline, ierr);
      if ( ierr != 0 ) {
        ATH_MSG_FATAL( "stasetilines_ failed on " ) ; 
        ATH_MSG_FATAL( "  StationType " << StationType ) ; 
        ATH_MSG_FATAL( "  jff         " << jff         ) ; 
        ATH_MSG_FATAL( "  jzz         " << jzz         ) ; 
        ATH_MSG_FATAL( "  job         " << job         ) ; 
        ATH_MSG_FATAL( "  jlay        " << jlay        ) ; 
        ATH_MSG_FATAL( "  s    " << s    ) ; 
        ATH_MSG_FATAL( "  z    " << z    ) ; 
        ATH_MSG_FATAL( "  t    " << t    ) ; 
        ATH_MSG_FATAL( "  rotS " << rotS ) ; 
        ATH_MSG_FATAL( "  rotZ " << rotZ ) ; 
        ATH_MSG_FATAL( "  rotT " << rotT ) ; 
        return StatusCode::FAILURE;
      }

  }
  seticsc_();


  return StatusCode::SUCCESS;
   
}

