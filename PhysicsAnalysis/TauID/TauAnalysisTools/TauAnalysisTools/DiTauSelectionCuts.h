/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TAUANALYSISTOOLS_DITAUSELECTIONCUTS_H
#define TAUANALYSISTOOLS_DITAUSELECTIONCUTS_H

/*
  original author: Dirk Duschinger
  mail: dirk.duschinger@cern.ch
  contact email: antonio.de.maria@cern.ch
  documentation in: https://gitlab.cern.ch/atlas/athena/-/blob/master/PhysicsAnalysis/TauID/TauAnalysisTools/doc/README-TauSelectionTool.rst
*/

// Framework include(s):
#include "xAODTau/DiTauJet.h"
#include "PATCore/AcceptData.h"
#include "PATCore/AcceptInfo.h"

// ROOT include(s):
#include "TH1F.h"

namespace TauAnalysisTools
{

class DiTauSelectionTool;

class DiTauSelectionCut
{

public:
  DiTauSelectionCut(const std::string& sName, TauAnalysisTools::DiTauSelectionTool* tDTST);
  virtual ~DiTauSelectionCut();

  void writeControlHistograms();
  void fillHistogramCutPre(const xAOD::DiTauJet& xTau);
  void fillHistogramCut(const xAOD::DiTauJet& xTau);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const = 0;
  virtual bool accept(const xAOD::DiTauJet& xTau,
                      asg::AcceptData& accept) = 0;
  TH1F* CreateControlPlot(const char* sName, const char* sTitle, int iBins, double dXLow, double dXUp);

  std::string getName()
  {
    return m_sName;
  };

  void setProperty(const std::string& name, const std::string& value);

protected:
  std::string m_sName;

  TH1F* m_hHistCutPre;
  TH1F* m_hHistCut;

  DiTauSelectionTool* m_tDTST;

  void declareProperty(const std::string& name, std::string& loc);
  std::map<std::string, std::string&> m_mProperties;
  std::string getProperty(const std::string& name);

private:
  virtual void fillHistogram(const xAOD::DiTauJet& xTau, TH1F& hHist) const = 0;
};

class DiTauSelectionCutPt
  : public DiTauSelectionCut
{
public:
  DiTauSelectionCutPt(DiTauSelectionTool* tDTST);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const override;
  virtual bool accept(const xAOD::DiTauJet& xTau,
                      asg::AcceptData& accept) override;
private:
  virtual void fillHistogram(const xAOD::DiTauJet& xTau, TH1F& hHist) const override;
};

class DiTauSelectionCutAbsEta
  : public DiTauSelectionCut
{
public:
  DiTauSelectionCutAbsEta(DiTauSelectionTool* tDTST);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const override;
  virtual bool accept(const xAOD::DiTauJet& xTau,
                      asg::AcceptData& accept) override;
private:
  virtual void fillHistogram(const xAOD::DiTauJet& xTau, TH1F& hHist) const override;
};

class DiTauSelectionCutNSubjets
  : public DiTauSelectionCut
{
public:
  DiTauSelectionCutNSubjets(DiTauSelectionTool* tDTST);
  virtual void setAcceptInfo (asg::AcceptInfo& info) const override;
  virtual bool accept(const xAOD::DiTauJet& xTau,
                      asg::AcceptData& accept) override;
private:
  virtual void fillHistogram(const xAOD::DiTauJet& xTau, TH1F& hHist) const override;
};


}

#endif // TAUANALYSISTOOLS_DITAUSELECTIONCUTS_H
