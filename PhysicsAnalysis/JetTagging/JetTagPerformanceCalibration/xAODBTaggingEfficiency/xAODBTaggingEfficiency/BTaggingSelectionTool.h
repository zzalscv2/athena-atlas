// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// BTaggingSelectionTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
/**
  @class BTaggingSelectionTool
  Tool to apply flavour-tagging requirements on jets
  @author C. Lüdtke, M. Ughetto
  @contact cluedtke@cern.ch, mughetto@cern.ch

  Note for getTaggerWeight and getCutValue: the LAST defaulted argument
  is now 'getCTagW', but before AB 21.2.221 it was 'useVetoWP', which has the 
  **opposite** meaning when set to true. 

*/

#ifndef CPBTAGGINGSELECTIONTOOL_H
#define CPBTAGGINGSELECTIONTOOL_H

#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"
#include "xAODBTagging/BTagging.h"

#include "AsgTools/AsgTool.h"
#include "PATCore/IAsgSelectionTool.h"
#include "CxxUtils/checker_macros.h"
#include "TFile.h"
#include "TSpline.h"
#include "TVector.h"
#include "TMatrixD.h"
#include <string>
#include <set>
#include <vector>
#include <map>

class BTaggingSelectionTool: public asg::AsgTool,
			     public virtual IBTaggingSelectionTool,
			     public virtual IAsgSelectionTool  {
  typedef double (xAOD::BTagging::* tagWeight_member_t)() const;

  /// Create a proper constructor for Athena
  ASG_TOOL_CLASS2( BTaggingSelectionTool , IAsgSelectionTool, IBTaggingSelectionTool )

  public:
  /// Create a constructor for standalone usage
  BTaggingSelectionTool( const std::string& name );
  StatusCode initialize() override;

  /// Get the decision using a generic IParticle pointer
  virtual asg::AcceptData accept( const xAOD::IParticle* p ) const override;
  virtual asg::AcceptData accept( const xAOD::Jet& jet ) const override;

  /// Get the decision using thet jet's pt and tag weight values
  virtual asg::AcceptData accept(double /* jet pt */, double /* jet eta */, double /* tag_weight */ ) const override;
  virtual asg::AcceptData accept(double /* jet pt */, double /* jet eta*/ , double /* taggerWeight_b */, double /* taggerWeight_c */) const override;
  virtual asg::AcceptData accept(double /* jet pt */, double /* jet eta */, double /* dl1pb */, double /* dl1pc  */ , double /* dl1pu  */) const override;

  /// Decide in which quantile of the tag weight distribution the jet belongs (continuous tagging)
  /// The return value represents the bin index of the quantile distribution
  virtual int getQuantile( const xAOD::IParticle* ) const override;
  virtual int getQuantile( const xAOD::Jet& ) const override;
  virtual int getQuantile( double /* jet pt */, double /* jet eta */, double /* tag weight */  ) const override;
  virtual int getQuantile( double /*pT*/, double /*eta*/, double /*tag_weight_b*/, double /*tag_weight_c*/ ) const override;

  virtual CP::CorrectionCode getCutValue(double /* jet pt */, double & cutval) const override;
   //1D tagging wrapper
  virtual CP::CorrectionCode getTaggerWeight( const xAOD::Jet& jet, double & tagweight) const override;
  virtual CP::CorrectionCode getTaggerWeight( double pb, double pc, double pu , double & tagweight) const override;

  //flexibility for Continuous2D
  virtual CP::CorrectionCode getTaggerWeight( const xAOD::Jet& jet, double & weight ,bool getCTagW) const override;
  virtual CP::CorrectionCode getTaggerWeight( double /* dl1pb */, double /* dl1pc  */ , double /* dl1pu  */ , double & weight, bool getCTagW) const override;
  const asg::AcceptInfo& getAcceptInfo( ) const  override {return m_acceptinfo;} 
private:
  /// Helper function that decides whether a jet belongs to the correct jet selection for b-tagging
  virtual bool checkRange( double /* jet pt */, double /* jet eta */ , asg::AcceptData& ) const;
  //fill the spline or vector that store the cut values for a particular working point
  void InitializeTaggerVariables(std::string taggerName,std::string OP, TSpline3 *spline, TVector *constcut, double &fraction);

  bool m_initialised;
  bool m_ErrorOnTagWeightFailure;
  bool m_StoreNConstituents = false;
  bool m_continuous   = false; //Continuous1D
  bool m_continuous2D = false; //Continuous2D
  bool m_useCTag = false; //use c-tagging or b-tagging in 1D
  /// Object used to store the last decision
  asg::AcceptInfo m_acceptinfo;  

  double m_maxEta;
  double m_minPt;
  double m_maxRangePt;

  std::string m_CutFileName;
  std::string m_taggerName;
  std::string m_OP;
  std::string m_jetAuthor;
  std::string m_ContinuousBenchmarks;

  TFile *m_inf;
  double m_continuouscuts[6];

  struct taggerproperties{
    std::string  name;
    double fraction_b;
    double fraction_c;
    TSpline3*  spline;
    TVector* constcut; 
    TMatrixD*  cuts2D; //useful only in Continuous2D
    std::vector<int>  benchmarks; //useful only in Continuous WP. list of bins that are considered as tagged. 

    double get2DCutValue(int row, int column) const{
      TMatrixD& cuts2D_safe ATLAS_THREAD_SAFE = *(this->cuts2D); 
      double cut = cuts2D_safe(row,column);
      return cut;
    }

  };

  taggerproperties m_tagger;

  enum Tagger{UNKNOWN, DL1, GN1, GN2, MV2c10};
  Tagger m_taggerEnum;

  Tagger SetTaggerEnum(const std::string& taggerName){
    if(taggerName.find("DL1") != std::string::npos) return Tagger::DL1;
    else if(taggerName.find("GN1") != std::string::npos) return Tagger::GN1;
    else if(taggerName.find("GN2") != std::string::npos) return Tagger::GN2;
    else if(taggerName == "MV2c10") return Tagger::MV2c10;
    else 
      ATH_MSG_ERROR("Tagger Name NOT supported.");
    return Tagger::UNKNOWN;
  };
  //get from the CDI file the taggers cut object(that holds the definition of cut values)
  //and flaovur fraction (for DL1 tagger) and store them in the right taggerproperties struct
  void ExtractTaggerProperties(taggerproperties& tagger, std::string taggerName, std::string OP);

  std::vector<std::string> split (const std::string &input, const char &delimiter);

};

#endif // CPBTAGGINGSELECTIONTOOL_H
