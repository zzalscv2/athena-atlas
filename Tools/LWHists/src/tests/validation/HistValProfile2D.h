/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class HistValProfile2D                    //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: July 2009                                //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef HISTVALPROFILE2D_H
#define HISTVALPROFILE2D_H

#include "HistValBase.h"
#include "TProfile2D.h"
#include "LWHists/TProfile2D_LW.h"

class HistValProfile2D : public HistValBase {
public:

  HistValProfile2D( const std::string& name, const std::string& title,
                    int nbinsx, const double& xmin, const double& xmax,
                    int nbinsy, const double& ymin, const double& ymax,
                    const double& profparmin=0, const double& profparmax=0 );
  HistValProfile2D( const std::string& name, const std::string& title,
                    int nbinsx, const double* xbins,
                    int nbinsy, const double* ybins );
  virtual ~HistValProfile2D();

  virtual unsigned getXAxis_NBins() const override;
  virtual unsigned getYAxis_NBins() const override;

  virtual void setXAxis_BinLabel(unsigned bin, const char* label) override;
  virtual void setYAxis_BinLabel(unsigned bin, const char* label) override;

  virtual void setXAxis_LabelSize(float) override;
  virtual void setYAxis_LabelSize(float) override;
  virtual void setXAxis_Title(const std::string&) override;
  virtual void setYAxis_Title(const std::string&) override;
          void setZAxis_Title(const std::string&);

  virtual void SetMarkerColor( short c) override;
  virtual void SetMarkerStyle( short s ) override;
  virtual void SetMarkerSize( float sz) override;

  virtual void setName(const std::string& name) override;
  virtual void setTitle(const std::string& title) override;
  virtual void setNameTitle(const std::string& name,
                            const std::string& title) override;

  virtual void setMinimum(const double& m = -1111) override;
  virtual void setMaximum(const double& m = -1111) override;

  //Test various way to fill the histogram:
  void fill(const double& x, const double& y, const double& z);
  void fill(const double& x, const double& y, const double& z, const double& w);
  void setBinEntries(unsigned, unsigned, const double& /*entries*/){}//Do nothing, since root's 2D histograms don't support this method!;
  void setBinContent(unsigned, unsigned, const double& content);
  void setBinError(unsigned, unsigned, const double& error);
  void setBinInfo(unsigned, unsigned, const double& entries, const double& content,const double& error);

  virtual void compareBinContents() const override;
  virtual void compareMetaData() override;
  virtual void compareTH1s() override;
  virtual void compareFastLoop() override;

  virtual void triggerConversionToROOTHist() override
  {
    if (!m_triggeredConversionToROOTHist) {
      m_triggeredConversionToROOTHist=true;
      m_h1->SetName(convertedRootName(m_h1->GetName()).c_str());
      m_h2->getROOTHist();
    }
  }

private:
  TProfile2D * m_h1;
  TProfile2D_LW * m_h2;
};

#include "HistValProfile2D.icc"

#endif
