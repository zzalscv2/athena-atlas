/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifdef XAOD_STANDALONE
#include "xAODRootAccess/TEvent.h"
#else
#include "POOLRootAccess/TEvent.h"
#endif
#include <AsgTools/StandaloneToolHandle.h>
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"

#include "PATInterfaces/CorrectionCode.h"
#include "xAODJet/JetAuxContainer.h"
#include "xAODJet/JetContainer.h"
#include "xAODBTagging/BTaggingAuxContainer.h"
#include "xAODBTagging/BTagging.h"
#include "xAODBTagging/BTaggingUtilities.h"

#include <string>
#include <iomanip>
#include "TFile.h"

using CP::CorrectionCode;

int main() {


  std::string taggerName = "GN2v00NewAliasWP";
  std::string workingPointName = "FixedCutBEff_77";


  asg::StandaloneToolHandle<IBTaggingSelectionTool> tool("BTaggingSelectionTool/BTagSelecTest");
  StatusCode code1 = tool.setProperty( "FlvTagCutDefinitionsFileName","xAODBTaggingEfficiency/13p6TeV/2023-22-13p6TeV-MC21-CDI_Test_2023-08-1_v1.root" );
  StatusCode code2 = tool.setProperty("TaggerName",    taggerName  );
  StatusCode code3 = tool.setProperty("OperatingPoint", workingPointName);
  StatusCode code4 = tool.setProperty("JetAuthor",      "AntiKt4EMPFlowJets" );
  StatusCode code5 = tool.setProperty("MinPt", 20000);
  // A successful initialisation ought to be checked for
  StatusCode code6 = tool.initialize();

  if (code1 != StatusCode::SUCCESS || code2 != StatusCode::SUCCESS || code3 != StatusCode::SUCCESS || code4 != StatusCode::SUCCESS || code5 != StatusCode::SUCCESS || code6 != StatusCode::SUCCESS) {
    std::cout << "Initialization of tool " << tool->name() << " failed! " << std::endl;
    return -1;
  }
  else {
    std::cout << "Initialization of tool " << tool->name() << " finished." << std::endl;
  }


  //load some jets to show how to use the tool

  //  xAOD::TEvent event;
  #ifdef XAOD_STANDALONE
  xAOD::TEvent event(xAOD::TEvent::kClassAccess);
  #else
  POOL::TEvent event(POOL::TEvent::kClassAccess);
  #endif


  TFile* m_file = TFile::Open("/afs/cern.ch/work/b/bdong/public/DAOD_PHYS.root","read");

  if(!event.readFrom(m_file).isSuccess()){ std::cout << "failed to load file" << std::endl; return -1; }

  event.getEntry(0);

  const xAOD::JetContainer* jets = 0;

  if (!event.retrieve( jets, "AntiKt4EMPFlowJets" ).isSuccess() ){ std::cout << " error retrieving jets " << std::endl; return -1;}

  int jet_index = 0;
  for (const xAOD::Jet* jet : *jets) {

    //getting a tagging decision, is the jet tagged or not
    bool tagged = static_cast<bool>(tool->accept(*jet));

    //you can get the tagger weight,
    double tagweight;
    if( tool->getTaggerWeight( *jet ,tagweight)!=CorrectionCode::Ok ){ std::cout << " error retrieving tagger weight " << std::endl; return -1; }

    std::cout << "jet " << jet_index << " " <<  taggerName  << "  " << workingPointName << " is tagged " << tagged << " tag weight " << tagweight << std::endl;




    //if you have GN2v00 weights, you can get the tagger weight this way
    const xAOD::BTagging *btag = xAOD::BTaggingUtilities::getBTagging( *jet );

    float gn2_pb = btag->auxdata<float>("GN2v00_pb");
    float gn2_pc = btag->auxdata<float>("GN2v00_pc");
    float gn2_pu = btag->auxdata<float>("GN2v00_pu");


    if(  tool->getTaggerWeight(gn2_pb,gn2_pc,gn2_pu, tagweight) !=CorrectionCode::Ok ){ std::cout << " error retrieving tagger weight " << std::endl; return -1; }

    std::cout << " tagweight " << tagweight << std::endl;

    double pT = jet->pt();
    double eta = jet->eta();

    // you can see if the jet is tagged using its pt/eta and tagweight

    tagged = static_cast<bool>(tool->accept(pT,eta,tagweight));

    //you can also extract the cut value (which may or may not depend on the jet pt)

    double cutval;
     //provide the pt in MeV
    if( tool->getCutValue( jet->pt() , cutval)!=CorrectionCode::Ok ){ std::cout << " error retrieving cut value " << std::endl; return -1; }

    std::cout << " tagged " << tagged << " cut value " << cutval << std::endl;

    jet_index++;

  }

  //Continuous working points
  //*************************
  //with a selection tool using the Continuous working point,
  //you can get the jets tag weight bin (between the different fixedcutBEff working points, 60,70,77,85)
  taggerName = "GN2v00NewAliasWP";
  workingPointName = "Continuous";
  asg::StandaloneToolHandle<IBTaggingSelectionTool> tool_Continuous("BTaggingSelectionTool/BTagSelContinuousTest");
  code1 = tool_Continuous.setProperty( "FlvTagCutDefinitionsFileName","xAODBTaggingEfficiency/13p6TeV/2023-22-13p6TeV-MC21-CDI_Test_2023-08-1_v1.root" );
  code2 = tool_Continuous.setProperty("TaggerName",    taggerName  );
  code3 = tool_Continuous.setProperty("OperatingPoint", workingPointName );
  code4 = tool_Continuous.setProperty("JetAuthor",      "AntiKt4EMPFlowJets" );
  code5 = tool_Continuous.setProperty("MinPt", 20000);
  code6 = tool_Continuous.initialize();

  if (code1 != StatusCode::SUCCESS || code2 != StatusCode::SUCCESS || code3 != StatusCode::SUCCESS || code4 != StatusCode::SUCCESS || code5 != StatusCode::SUCCESS || code6 != StatusCode::SUCCESS) {
    std::cout << "Initialization of tool " << tool_Continuous->name() << " failed! " << std::endl;
    return -1;
  }
  else {
    std::cout << "Initialization of tool " << tool_Continuous->name() << " finished." << std::endl;
  }

  jet_index = 0;
  for (const xAOD::Jet* jet : *jets) {

    double tagweight;
    if( tool->getTaggerWeight( *jet ,tagweight)!=CorrectionCode::Ok ){ 
      std::cout << " error retrieving tagger weight " << std::endl; return -1; 
    }
    int quantile = tool_Continuous->getQuantile(*jet);
    

    std::cout << "jet " << jet_index << " " <<  taggerName  << "  " << workingPointName << " tag weight " << tagweight <<  " quantile " << quantile << std::endl;

    jet_index++;

  }


  //CTagging
  //**************************
  //for example, FixedCutBEff_70 but in ctagging mode
  //the selection tool will require the jet to be
  //tagged by the standard working point
  //and to not be tagged by the secondary tagger and working point

  taggerName = "GN2v00NewAliasWP";
  workingPointName = "Continuous";

  asg::StandaloneToolHandle<IBTaggingSelectionTool> tool_ctag("BTaggingSelectionTool/BTagSelecTest");
  code1 = tool_ctag.setProperty( "FlvTagCutDefinitionsFileName","xAODBTaggingEfficiency/13p6TeV/2023-22-13p6TeV-MC21-CDI_Test_2023-08-1_v1.root");
  code2 = tool_ctag.setProperty("TaggerName",    taggerName  );
  code3 = tool_ctag.setProperty("OperatingPoint", workingPointName );
  code4 = tool_ctag.setProperty("JetAuthor",      "AntiKt4EMPFlowJets" );
  code5 = tool_ctag.setProperty("MinPt", 20000);
  code6 = tool_ctag.setProperty("useCTagging",    true );
  auto code7 = tool_ctag.initialize();

  if (code1 != StatusCode::SUCCESS || code2 != StatusCode::SUCCESS || code3 != StatusCode::SUCCESS 
   || code4 != StatusCode::SUCCESS || code5 != StatusCode::SUCCESS || code6 != StatusCode::SUCCESS || code7 != StatusCode::SUCCESS) {
    std::cout << "Initialization of tool " << tool->name() << " failed! " << std::endl;
    return -1;
  }
  else {
    std::cout << "Initialization of tool " << tool->name() << " finished." << std::endl;
  }


  jet_index = 0;
  for (const xAOD::Jet* jet : *jets) {

    //getting a tagging decision, is the jet tagged or not
    bool tagged = static_cast<bool>(tool->accept(*jet));

    // if you are using a format without xAOD::Jets or where the jet does not have a properly filled b-tagging object,
    // you need the two tagger weights, for the nominal tagger and th tagger.

    //you can get the tagger weight,
    double tagweight_nominal;
    double tagweight_ctag;
    if( tool->getTaggerWeight( *jet ,tagweight_nominal)!=CorrectionCode::Ok ){ std::cout << " error retrieving tagger weight " << std::endl; return -1; }
    if( tool_ctag->getTaggerWeight( *jet ,tagweight_ctag, true)!=CorrectionCode::Ok ){ std::cout << " error retrieving tagger weight in ctag mode " << std::endl; return -1; }
    //if you have GN2v00 weights, you can get the tagger weight this way
    const xAOD::BTagging *btag = xAOD::BTaggingUtilities::getBTagging( *jet );

    float gn2_pb = btag->auxdata<float>("GN2v00_pb");
    float gn2_pc = btag->auxdata<float>("GN2v00_pc");
    float gn2_pu = btag->auxdata<float>("GN2v00_pu");

    //the 5th argument tells it to retrive the tagger weight in ctagging mode instead of btagging
    if(  tool_ctag->getTaggerWeight(gn2_pb,gn2_pc,gn2_pu, tagweight_ctag,true) !=CorrectionCode::Ok ){ std::cout << " error retrieving tagger weight " << std::endl; return -1; }

    double pT = jet->pt();
    double eta = jet->eta();

    bool tagged_withtagweights = static_cast<bool>(tool_ctag->accept(pT,eta,tagweight_nominal,tagweight_ctag));

    std::cout << "jet " << jet_index << " " <<  taggerName  << "  " << workingPointName << " is tagged " << tagged << " tagged (with tagweights) "<< tagged_withtagweights << std::endl;

    jet_index++;

  }

  return 0;
}
