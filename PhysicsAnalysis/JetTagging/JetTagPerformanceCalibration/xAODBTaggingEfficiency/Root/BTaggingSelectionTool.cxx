/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#include "CxxUtils/checker_macros.h"
#include "xAODBTaggingEfficiency/BTaggingSelectionTool.h"
#include "xAODBTagging/BTagging.h"
#include "xAODBTagging/BTaggingUtilities.h"
#include "CalibrationDataInterface/CalibrationDataInterfaceROOT.h"
#include "CalibrationDataInterface/CalibrationDataVariables.h"
#include "CalibrationDataInterface/CalibrationDataContainer.h"

#include "PATInterfaces/SystematicRegistry.h"
#include "PathResolver/PathResolver.h"

#include "TFile.h"
#include "TObjArray.h"
#include "TObjString.h"
#include "TMatrixD.h"

#include <algorithm>
#include <string>

using std::string;

using CP::CorrectionCode;
using CP::SystematicSet;
using CP::SystematicVariation;
using CP::SystematicRegistry;

using Analysis::Uncertainty;
using Analysis::CalibrationDataVariables;
using Analysis::CalibrationDataContainer;
using Analysis::CalibResult;
using Analysis::CalibrationStatus;
using Analysis::Total;
using Analysis::SFEigen;
using Analysis::SFNamed;
using Analysis::None;

using xAOD::IParticle;

BTaggingSelectionTool::BTaggingSelectionTool( const std::string & name)
  : asg::AsgTool( name ), m_acceptinfo( "JetSelection" )
{
  m_initialised = false;
  declareProperty( "MaxEta", m_maxEta = 2.5 );
  declareProperty( "MinPt", m_minPt = -1 /*MeV*/);
  declareProperty( "MaxRangePt", m_maxRangePt = 3000000 /*MeV*/);
  declareProperty( "FlvTagCutDefinitionsFileName", m_CutFileName = "", "name of the files containing official cut definitions (uses PathResolver)");
  declareProperty( "TaggerName",                    m_taggerName="",    "tagging algorithm name");
  declareProperty( "OperatingPoint",                m_OP="",            "operating point");
  declareProperty( "JetAuthor",                     m_jetAuthor="",     "jet collection");
  declareProperty( "ErrorOnTagWeightFailure",       m_ErrorOnTagWeightFailure=true, "optionally ignore cases where the tagweight cannot be retrived. default behaviour is to give an error, switching to false will turn it into a warning");
  declareProperty( "CutBenchmarksContinuousWP",     m_ContinuousBenchmarks="", "comma separated list of tag bins that will be accepted as tagged: 1,2,3 etc.. ");
  declareProperty( "useCTagging",                   m_useCTag=false, "Enabled only for FixedCut or Continuous WPs: define wether the cuts refer to b-tagging or c-tagging");
}

StatusCode BTaggingSelectionTool::initialize() {
  m_initialised = true;

  if (""==m_OP){
    ATH_MSG_ERROR( "BTaggingSelectionTool wasn't given a working point name" );
    return StatusCode::FAILURE;
  }

  TString pathtofile =  PathResolverFindCalibFile(m_CutFileName);
  m_inf = TFile::Open(pathtofile, "read");
  if (0==m_inf) {
    ATH_MSG_ERROR( "BTaggingSelectionTool couldn't access tagging cut definitions" );
    return StatusCode::FAILURE;
  }

  // check the CDI file for the selected tagger and jet collection
  TString check_CDI = m_taggerName;
  if(!m_inf->Get(check_CDI)){
     ATH_MSG_ERROR( "Tagger: "+m_taggerName+" not found in this CDI file: "+m_CutFileName);
    return StatusCode::FAILURE;
  }
  check_CDI = m_taggerName+"/"+m_jetAuthor;
  if(!m_inf->Get(check_CDI)){
     ATH_MSG_ERROR( "Tagger: "+m_taggerName+" and Jet Collection : "+m_jetAuthor+"  not found in this CDI file: "+m_CutFileName);
    return StatusCode::FAILURE;
  }
 
  //set taggerEnum to avid string comparison:
  m_taggerEnum = SetTaggerEnum(m_taggerName);

  //special requirement for VR TrackJets:
  if((m_jetAuthor.find("AntiKt2PV0TrackJets") != std::string::npos) ||
     (m_jetAuthor.find("AntiKt4PV0TrackJets") != std::string::npos) ||
     (m_jetAuthor.find("AntiKtVR30Rmax4Rmin02TrackJets") != std::string::npos)) {
    m_StoreNConstituents = true;
 }
 
 // Change the minPt cut if the user didn't touch it
 if (m_minPt < 0) {
   ATH_MSG_ERROR( "Tagger: "+m_taggerName+" and Jet Collection : "+m_jetAuthor+" do not have a minimum jet pT cut set.");
   return StatusCode::FAILURE;
 }
 
 // Operating point reading
 TString cutname = m_OP;
 m_continuous   = false;
 m_continuous2D = false;

 if(cutname.Contains("Continuous2D")){
   ATH_MSG_INFO("Working with Continuous2D WP.");
   m_continuous   = true;
   m_continuous2D = true; 
   m_useCTag      = false; //important for backward compatibility in getTaggerWeight methods.
   cutname = m_taggerName+"/"+m_jetAuthor+"/Continuous2D/cutvalue";
   m_tagger.name = m_taggerName;
   TMatrixD* matrix = (TMatrixD*) m_inf->Get(cutname);
   m_tagger.cuts2D = matrix;

   for (int bin = 0; bin < m_tagger.cuts2D->GetNrows(); bin++)
     ATH_MSG_DEBUG("INITIALIZATION c-cuts : " <<m_tagger.get2DCutValue(bin,0) <<" "
		   <<m_tagger.get2DCutValue(bin,1) <<" b-cuts : "
		   <<m_tagger.get2DCutValue(bin,2) <<" "
		   <<m_tagger.get2DCutValue(bin,3));
   
   if (m_tagger.cuts2D == nullptr) ATH_MSG_ERROR( "Invalid operating point" );
   
   m_tagger.spline = nullptr;
   
   TString fraction_data_name = m_taggerName+"/"+m_jetAuthor+"/Continuous2D/fraction_b";
   TVector *fraction_data = (TVector*) m_inf->Get(fraction_data_name);
   if(fraction_data!=nullptr)
     m_tagger.fraction_b = fraction_data[0](0);
   else
     ATH_MSG_ERROR("Tagger fraction_b in Continuous2D WP not available");
   
   //now the c-fraction:
   fraction_data_name = m_taggerName+"/"+m_jetAuthor+"/Continuous2D/fraction_c";
   fraction_data = (TVector*) m_inf->Get(fraction_data_name);
   if(fraction_data!=nullptr){
     m_tagger.fraction_c = fraction_data[0](0);}
   else{
     ATH_MSG_ERROR("Tagger fraction_c in Continuous2D WP not available");
   }
 } //Continuous2D
 else if ("Continuous"==cutname(0,10))  // For continuous tagging load all flat-cut WPs
   {
     if(m_useCTag)
      ATH_MSG_WARNING( "Running in Continuous WP and using 1D c-tagging");

     m_continuous   = true;
     m_continuouscuts[0] = -1.e4;
     cutname = m_taggerName+"/"+m_jetAuthor+"/FixedCutBEff_85/cutvalue";
     m_tagger.constcut = (TVector*) m_inf->Get(cutname);
     
     if (m_tagger.constcut!=nullptr) m_continuouscuts[1] = m_tagger.constcut[0](0);
     else ATH_MSG_ERROR( "Continuous tagging is trying to use an unvalid operating point: FixedCutBEff_85" );
     
     cutname = m_taggerName+"/"+m_jetAuthor+"/FixedCutBEff_77/cutvalue";
     m_tagger.constcut = (TVector*) m_inf->Get(cutname);
     if (m_tagger.constcut!=nullptr) m_continuouscuts[2] = m_tagger.constcut[0](0);
     else ATH_MSG_ERROR( "Continuous tagging is trying to use an unvalid operating point: FixedCutBEff_77" );
     
     cutname = m_taggerName+"/"+m_jetAuthor+"/FixedCutBEff_70/cutvalue";
     m_tagger.constcut = (TVector*) m_inf->Get(cutname);
     if (m_tagger.constcut!=nullptr) m_continuouscuts[3] = m_tagger.constcut[0](0);
     else ATH_MSG_ERROR( "Continuous tagging is trying to use an unvalid operating point: FixedCutBEff_70" );
     
     cutname = m_taggerName+"/"+m_jetAuthor+"/FixedCutBEff_60/cutvalue";
     m_tagger.constcut = (TVector*) m_inf->Get(cutname);
     if (m_tagger.constcut!=nullptr) m_continuouscuts[4] = m_tagger.constcut[0](0);
     else ATH_MSG_ERROR( "Continuous tagging is trying to use an unvalid operating point: FixedCutBEff_60" );
     
     //0% efficiency => MVXWP=+infinity
     m_continuouscuts[5]= +1.e4;
     
     //The WP is not important. This is just to retrieve the c-fraction. 
     ExtractTaggerProperties(m_tagger,m_taggerName, "FixedCutBEff_60");
   }
 else {  // FixedCut Working Point: load only one WP
   if(m_useCTag)
    ATH_MSG_WARNING( "Running in FixedCut WP and using c-tagging");

   ExtractTaggerProperties(m_tagger,m_taggerName, m_OP);
 }

 //set the working points
 if(m_continuous){
   std::vector<std::string> tag_benchmarks_names = split(m_ContinuousBenchmarks, ',');
   std::vector<int> tag_benchmarks;
   for (const std::string& tagbin : tag_benchmarks_names){
     tag_benchmarks.push_back(std::atoi(tagbin.c_str()));
     ATH_MSG_INFO("adding " <<tag_benchmarks.back() <<" as tagged bin ");
   }
   m_tagger.benchmarks = tag_benchmarks;
 }
 
 m_inf->Close();
 
 m_acceptinfo.addCut( "Eta", "Selection of jets according to their pseudorapidity" );
 m_acceptinfo.addCut( "Pt",  "Selection of jets according to their transverse momentum" );
 m_acceptinfo.addCut( "WorkingPoint",  "Working point for flavour-tagging of jets according to their b-tagging weight" );
 
 return StatusCode::SUCCESS;
}

void BTaggingSelectionTool::ExtractTaggerProperties(taggerproperties &tagger, std::string taggerName, std::string OP){

  TString cutname = OP;

  //set the name
  tagger.name = taggerName;

  if ("FlatBEff"==cutname(0,8) || "HybBEff"==cutname(0,7) ){
    cutname = taggerName+"/"+m_jetAuthor+"/"+OP+"/cutprofile";
    tagger.spline = (TSpline3*) m_inf->Get(cutname);
    if (tagger.spline == nullptr) ATH_MSG_ERROR( "Invalid operating point" );
    tagger.constcut = nullptr;
  }
  else {
    cutname = taggerName+"/"+m_jetAuthor+"/"+OP+"/cutvalue";
    tagger.constcut = (TVector*) m_inf->Get(cutname);
    if (tagger.constcut == nullptr) ATH_MSG_ERROR( "Invalid operating point" );
    tagger.spline = nullptr;
  }

  //retrive the "fraction" used in the DL1 log likelihood from the CDI, if its not there, use the hard coded values
  // (backwards compatibility)
  if( (m_taggerEnum == Tagger::DL1) || (m_taggerEnum == Tagger::GN1) || (m_taggerEnum == Tagger::GN2)){

    TString fraction_data_name = taggerName+"/"+m_jetAuthor+"/"+OP+"/fraction";
    TVector *fraction_data = (TVector*) m_inf->Get(fraction_data_name);
    
    double fraction = -1;
    if(fraction_data!=nullptr){
      fraction = fraction_data[0](0);
    }else{
      if("DL1"    ==taggerName){ fraction = 0.08; }
      if("DL1mu"  ==taggerName){ fraction = 0.08; }
      if("DL1rnn" ==taggerName){ fraction = 0.03; }
    }
    tagger.fraction_c = fraction;
    tagger.fraction_b = fraction;

    delete fraction_data;
  }
}

CorrectionCode BTaggingSelectionTool::getTaggerWeight( const xAOD::Jet& jet, double & tagweight) const{
  return getTaggerWeight(jet, tagweight, m_useCTag);
}

CorrectionCode BTaggingSelectionTool::getTaggerWeight( const xAOD::Jet& jet, double & tagweight, bool getCTagW) const{

  std::string taggerName = m_tagger.name;
  tagweight = -100.;

   if(!m_continuous2D && (getCTagW != m_useCTag) ){
    ATH_MSG_ERROR("Difference between initialisation and getTaggerWeight request! useCTagging property set to " <<m_useCTag <<" while getTaggerWeight use c-tag is set to " <<getCTagW <<".");
    return CorrectionCode::Error;
  }

  if ( m_taggerEnum == Tagger::MV2c10 ){

    const xAOD::BTagging* btag = xAOD::BTaggingUtilities::getBTagging( jet );

    if ((!btag) || (!btag->MVx_discriminant(taggerName, tagweight))){
      if(m_ErrorOnTagWeightFailure){
        ATH_MSG_ERROR("Failed to retrieve "+taggerName+" weight!");
        return CorrectionCode::Error;
      }else{
        ATH_MSG_WARNING("Failed to retrieve "+taggerName+" weight!");
        return CorrectionCode::Ok;
      }
    }
    ATH_MSG_VERBOSE( taggerName << " " <<  tagweight );
    return  CorrectionCode::Ok;
  } //MV2
  else{ 
    //DL1r or DL1
  double dl1_pb(-10.);
  double dl1_pc(-10.);
  double dl1_pu(-10.);

  const xAOD::BTagging* btag = xAOD::BTaggingUtilities::getBTagging( jet );

  if ((!btag)){
   ATH_MSG_ERROR("Failed to retrieve the BTagging information");
   return CorrectionCode::Error;
  }

  if ( (!btag->pb(taggerName, dl1_pb ))
   || (!btag->pc(taggerName, dl1_pc ))
   || (!btag->pu(taggerName, dl1_pu )) ){

     if(m_ErrorOnTagWeightFailure){
       ATH_MSG_ERROR("Failed to retrieve "+taggerName+" weight!");
       return CorrectionCode::Error;
     }else{
       ATH_MSG_WARNING("Failed to retrieve "+taggerName+" weight!");
       return CorrectionCode::Ok;
     }
  }

   return getTaggerWeight(dl1_pb, dl1_pc, dl1_pu, tagweight, getCTagW);

  }

  //if we got here the tagger name is not configured properly
  ATH_MSG_ERROR("BTaggingSelectionTool doesn't support tagger: "+m_taggerName);
  return CorrectionCode::Error;

}

CorrectionCode BTaggingSelectionTool::getTaggerWeight( double pb, double pc, double pu , double & tagweight) const{
  return getTaggerWeight(pb, pc, pu, tagweight, m_useCTag);
}

CorrectionCode BTaggingSelectionTool::getTaggerWeight( double pb, double pc, double pu , double & tagweight, bool getCTagW) const {

  std::string taggerName = m_tagger.name;

  if(!m_continuous2D && (getCTagW != m_useCTag) ){
    ATH_MSG_ERROR("Difference between initialisation and getTaggerWeight request! useCTagging property set to " <<m_useCTag <<" while getTaggerWeight use c-tag is set to " <<getCTagW <<".");
    return CorrectionCode::Error;
  }

  tagweight = -100.;
  if( (m_taggerEnum == Tagger::DL1) || (m_taggerEnum == Tagger::GN1) || (m_taggerEnum == Tagger::GN2)){

    bool valid_input = (!std::isnan(pu) && pb>=0 && pc>=0 && pu>=0);

    if (!valid_input){
      if(m_ErrorOnTagWeightFailure){
        ATH_MSG_ERROR("Invalid inputs for "+taggerName+" pb " << pb << " pc " << pc << " pu " << pu << " ");
        return CorrectionCode::Error;
      }else{
        ATH_MSG_WARNING("Invalid inputs for "+taggerName+" pb " << pb << " pc " << pc << " pu " << pu << " ");
        return CorrectionCode::Ok;
      }
    }

    if(getCTagW){
     tagweight = log(pc / (m_tagger.fraction_b * pb + (1. - m_tagger.fraction_b) * pu));
    }
    else{
     tagweight = log(pb / (m_tagger.fraction_c * pc + (1. - m_tagger.fraction_c) * pu) );
    }

    ATH_MSG_VERBOSE( "pb " <<  pb );
    ATH_MSG_VERBOSE( "pc " <<  pc );
    ATH_MSG_VERBOSE( "pu " <<  pu );
    ATH_MSG_VERBOSE( "tagweight " <<  tagweight );

    return CorrectionCode::Ok;
  }

  //if we got here the tagger name is not configured properly
  ATH_MSG_ERROR("this call to getTaggerWeight only works for DL1/GNx taggers");
  return CorrectionCode::Error;

}



asg::AcceptData BTaggingSelectionTool::accept( const xAOD::IParticle* p ) const {

  // Check if this is a jet:
  if( p->type() != xAOD::Type::Jet ) {
    ATH_MSG_ERROR( "accept(...) Function received a non-jet" );
    return asg::AcceptData (&m_acceptinfo);
  }

  // Cast it to a jet:
  const xAOD::Jet* jet = dynamic_cast< const xAOD::Jet* >( p );
  if( ! jet ) {
    ATH_MSG_FATAL( "accept(...) Failed to cast particle to jet" );
    return asg::AcceptData (&m_acceptinfo);
  }

  // Let the specific function do the work:
  return accept( *jet );
}

asg::AcceptData BTaggingSelectionTool::accept( const xAOD::Jet& jet ) const {
  asg::AcceptData acceptData (&m_acceptinfo);

  if (! m_initialised) {
    ATH_MSG_ERROR("BTaggingSelectionTool has not been initialised");
    return acceptData;
  }

  if  (m_StoreNConstituents){
    // We want at least 2 tracks in a track jet
    acceptData.setCutResult( "NConstituents", jet.numConstituents() >= 2 );
  }

  double pT = jet.pt();
  double eta = jet.eta();

  if(m_continuous2D){
    double taggerweight_b(-100);
    double taggerweight_c(-100);
    if( (getTaggerWeight( jet, taggerweight_b, false)!=CorrectionCode::Ok) ||
	   (getTaggerWeight( jet, taggerweight_c, true )!=CorrectionCode::Ok) )
      return acceptData;
    
    return accept(pT, eta, taggerweight_b,taggerweight_c);
  }
  else{ //if here, we are in 1D mode
    double taggerweight(-100);
    if( getTaggerWeight( jet ,taggerweight, m_useCTag)!=CorrectionCode::Ok)
      return acceptData;
    
    return accept(pT, eta, taggerweight);
  }
}

asg::AcceptData BTaggingSelectionTool::accept(double pT, double eta, double tag_weight) const
{
  
  asg::AcceptData acceptData (&m_acceptinfo);

  if (! m_initialised) {
    ATH_MSG_ERROR("BTaggingSelectionTool has not been initialised");
    return acceptData;
  }

  eta = std::abs(eta);
  if (! checkRange(pT, eta, acceptData))
    return acceptData;

  // After initialization, either m_tagger.spline or m_tagger.constcut should be non-zero
  // Else, the initialization was incorrect and should be revisited
  if(m_continuous){
    for(auto bin : m_tagger.benchmarks){
      if(bin == 0){
	      ATH_MSG_ERROR("bin == 0 in the list of tagged bins. you should not be here. Wrong convention");
	      return acceptData;
      }
      double cutvalue_low = m_continuouscuts[bin-1]; 
      double cutvalue_hig = m_continuouscuts[bin];
      ATH_MSG_DEBUG("bin " <<bin    <<" taggerWeight "
		    <<tag_weight   <<" cutvalue low " 
		    <<cutvalue_low <<" cutvalue hig "
		    <<cutvalue_hig );
      
      if (tag_weight > cutvalue_low && tag_weight <= cutvalue_hig){	    
	      acceptData.setCutResult( "WorkingPoint", true );
	      break;
      }
    }
  }
  else{ //FixedCut
    double cutvalue(DBL_MAX);
    if( getCutValue(pT, cutvalue )!=CorrectionCode::Ok ){
      return acceptData;
    }
    
    if ( tag_weight < cutvalue ){
      return acceptData;
    }
    
    acceptData.setCutResult( "WorkingPoint", true );
  }
  
  // Return the result:
  return acceptData;
}

asg::AcceptData BTaggingSelectionTool::accept(double pT, double eta, double taggerWeight_b, double taggerWeight_c) const
{
  asg::AcceptData acceptData (&m_acceptinfo);

  if (! m_initialised) {
    ATH_MSG_ERROR("BTaggingSelectionTool has not been initialised");
    return acceptData;
  }

  eta = std::abs(eta);

  if (! checkRange(pT, eta, acceptData))
    return acceptData;

  for(auto bin : m_tagger.benchmarks){

    ATH_MSG_DEBUG("bin" <<bin <<" taggerWeight_c " 
		  <<taggerWeight_c <<" taggerWeight_b " 
		  <<taggerWeight_b);
    
    ATH_MSG_DEBUG(" c-cuts : " <<m_tagger.get2DCutValue(bin,0) <<" " 
		  <<m_tagger.get2DCutValue(bin,1) <<" b-cuts : " 
		  <<m_tagger.get2DCutValue(bin,2) <<" " 
		  <<m_tagger.get2DCutValue(bin,3));
    
    if (taggerWeight_c >  m_tagger.get2DCutValue(bin,0) &&  //ctag low
	taggerWeight_c <= m_tagger.get2DCutValue(bin,1) &&  //ctag max
	taggerWeight_b >  m_tagger.get2DCutValue(bin,2) &&  //btag low
	taggerWeight_b <= m_tagger.get2DCutValue(bin,3)  )  //btag max
      {
	acceptData.setCutResult( "WorkingPoint", true ); // IF we arrived here, the jet is tagged
	break;	
      }
  } //for loop

  // Return the result:
  return acceptData;
}


asg::AcceptData BTaggingSelectionTool::accept(double pT, double eta, double pb, double pc, double pu) const
 {
   asg::AcceptData acceptData (&m_acceptinfo);

   if (! m_initialised) {
     ATH_MSG_ERROR("BTaggingSelectionTool has not been initialised");
     return acceptData;
   }

   eta = std::abs(eta);
   if (! checkRange(pT, eta, acceptData))
     return acceptData;

   // After initialization, either m_tagger.spline or m_tagger.constcut should be non-zero
   // Else, the initialization was incorrect and should be revisited
   double cutvalue(DBL_MAX);

   if( getCutValue(pT, cutvalue)!=CorrectionCode::Ok){
    return acceptData;
   };
  
   if(m_continuous2D){
     double tagger_weight_b(-100);
     double tagger_weight_c(-100);
     if( ( getTaggerWeight(pb, pc, pu, tagger_weight_b, false)!=CorrectionCode::Ok) ||
	     ( getTaggerWeight(pb, pc, pu, tagger_weight_c, true )!=CorrectionCode::Ok) )
       return acceptData;
     return accept(pT, eta, tagger_weight_b, tagger_weight_c);
   }
   else{
     double tagger_weight(-100);
     if( getTaggerWeight(pb, pc, pu, tagger_weight, m_useCTag)!=CorrectionCode::Ok)
       return acceptData;
     if ( tagger_weight < cutvalue )
       return acceptData;
   }
   //if you made it here, the jet is tagged
   acceptData.setCutResult( "WorkingPoint", true );
   return acceptData;
 }

int BTaggingSelectionTool::getQuantile( const xAOD::IParticle* p ) const {
  // Check if this is a jet:


  if( p->type() != xAOD::Type::Jet ) {
    ATH_MSG_ERROR( "accept(...) Function received a non-jet" );
    return -1;
  }

  // Cast it to a jet:
  const xAOD::Jet* jet = dynamic_cast< const xAOD::Jet* >( p );
  if( ! jet ) {
    ATH_MSG_FATAL( "accept(...) Failed to cast particle to jet" );
    return -1;
  }

  // Let the specific function do the work:
  return getQuantile( *jet );
}

int BTaggingSelectionTool::getQuantile( const xAOD::Jet& jet ) const{
  double pT = jet.pt();
  double eta = std::abs( jet.eta() );
  int quantile = -1;
  
  if (m_continuous2D){
    double tag_weight_b(-100.);
    double tag_weight_c(-100.);
    if ( (getTaggerWeight(jet, tag_weight_b, false) == CP::CorrectionCode::Error) ||
	     (getTaggerWeight(jet, tag_weight_c, true ) == CP::CorrectionCode::Error) ){
      ATH_MSG_WARNING("getQuantile: Failed to retrieve tag weight for Continuous2D!");
      return -1;
    }
    quantile = getQuantile(pT, eta, tag_weight_b, tag_weight_c );
  }
  else{
    // Retrieve the tagger weight which was assigned to the jet
    double tag_weight(-100.);
    if (getTaggerWeight(jet, tag_weight, m_useCTag)==CorrectionCode::Error){
      ATH_MSG_WARNING("getQuantile: Failed to retrieve "+m_taggerName+" weight!");
      return -1;
    }
    ATH_MSG_VERBOSE( m_taggerName << " " <<  tag_weight);
    quantile = getQuantile(pT, eta, tag_weight);
  }
  return quantile;
}


int BTaggingSelectionTool::getQuantile(double pT, double eta, double tag_weight ) const
{
  if (! m_initialised) {
    ATH_MSG_ERROR("BTaggingSelectionTool has not been initialised");
  }
  //////////////////////
  // Cheatsheet:
  // returns 5 if between 60% and 0%
  // returns 4 if between 70% and 60%
  // returns 3 if between 77% and 70%
  // returns 2 if between 85% and 77%
  // returns 1 if between 100% and 85%
  // return -1 not in b-tagging acceptance
  //////////////////////

  int bin_index(-1);
  asg::AcceptData acceptData (&m_acceptinfo);
  if (! checkRange(pT, eta,acceptData)) return bin_index;

  // If in b-tagging acceptance, cont.tagging
  for (int i=1; i<=5; ++i) {
    if (tag_weight < m_continuouscuts[i]) {
      bin_index = i;
      break;
    }
    else if (tag_weight >= m_continuouscuts[5]){
      bin_index = 5;
      break;
    }
  }
  return bin_index;
}

int BTaggingSelectionTool::getQuantile(double pT, double eta, double tag_weight_b, double tag_weight_c ) const
{
  //////////////////////
  /// Cheatsheet:
  /// returns 4 if pass B_tight (?)
  /// returns 3 if pass B_loose
  /// returns 2 if pass C_tight + fail B_loose 
  /// returns 1 if pass C_loose + fail C_tight + fail B_loose
  /// returns 0 if fail C_loose + fail B_loose
  /// return -1 not in b-tagging acceptance
  //////////////////////

  //More details here: https://indico.cern.ch/event/1116952/#4-mr49953-implements-the-conti

  ATH_MSG_DEBUG("inside getQuantile 2D " <<pT <<" " <<eta <<" " <<tag_weight_b <<" " <<tag_weight_c);
  int bin_index(-1);

  asg::AcceptData acceptData (&m_acceptinfo);
  if (! checkRange(pT, eta, acceptData)) return bin_index;

  int ncuts = m_tagger.cuts2D->GetNrows();
  ATH_MSG_VERBOSE("ncuts: " <<ncuts);

  //loop over all the cuts
  for(int i = 0; i < ncuts ; i++){
    double c_cut_low = m_tagger.get2DCutValue(i,0);
    double c_cut_hig = m_tagger.get2DCutValue(i,1);
    double b_cut_low = m_tagger.get2DCutValue(i,2);
    double b_cut_hig = m_tagger.get2DCutValue(i,3);

    ATH_MSG_DEBUG("bin " <<i <<" c_cut low " <<c_cut_low <<" c_cut hig " <<c_cut_hig <<" c_cut low " <<b_cut_low <<" b_ct hig" <<b_cut_hig);
    if (tag_weight_c >  c_cut_low && 
	tag_weight_c <= c_cut_hig &&
	tag_weight_b >  b_cut_low && 
	tag_weight_b <= b_cut_hig){
      bin_index = i;
      break;
    }
  }
    
  ATH_MSG_VERBOSE("bin_index " <<bin_index);
  return bin_index;
}

bool BTaggingSelectionTool::checkRange(double pT, double eta,asg::AcceptData& acceptData) const
{
  // Do the |eta| cut:
  if( std::abs(eta) > m_maxEta ) {
    return false;
  }
  acceptData.setCutResult( "Eta", true );

  // Do the pT cut:
  ATH_MSG_VERBOSE( "Jet pT: " << pT );
  if( pT < m_minPt ) {
    return false;
  }
  acceptData.setCutResult( "Pt", true );

  return true;
}

CorrectionCode BTaggingSelectionTool::getCutValue(double pT, double & cutval) const
{
   cutval = DBL_MAX;

   // flat cut for out of range pTs
   if (pT>m_maxRangePt)
     pT = m_maxRangePt;

   taggerproperties tagger ATLAS_THREAD_SAFE = m_tagger;

   if (tagger.spline != nullptr && tagger.constcut == nullptr) {
     pT = pT/1000.0;
     double maxsplinept = tagger.spline->GetXmax();
     if (pT>maxsplinept){ pT = maxsplinept; }
     cutval = tagger.spline->Eval(pT);
   }

   else if (tagger.constcut != nullptr && tagger.spline == nullptr) {
     cutval = tagger.constcut[0](0);
   }
   else{
    ATH_MSG_ERROR( "Bad cut configuration!" );
    return CorrectionCode::Error;
   }


   ATH_MSG_VERBOSE( "Cut value " << cutval );

   return CorrectionCode::Ok;
}

std::vector<std::string> BTaggingSelectionTool::split (const std::string &input, const char &delimiter){
  std::vector<std::string> v;
  std::istringstream buf(input);
  for(std::string token; std::getline(buf, token, delimiter); )
    v.push_back(token);
  return v;
}
