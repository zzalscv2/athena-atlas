/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class HistValProfile                      //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: July 2009                                //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef HISTVALPROFILE_H
#define HISTVALPROFILE_H

#include "HistValBase.h"
#include "TProfile.h"
#include "LWHists/TProfile_LW.h"

class HistValProfile : public HistValBase {
public:

  HistValProfile( const std::string& name, const std::string& title,
                  int nbins, const double& xmin, const double& xmax,
                  const double& profparmin=0, const double& profparmax=0 );
  template <class TFloat>
  HistValProfile( const std::string& name, const std::string& title,
                  int nbins, const TFloat* xbins );

  virtual ~HistValProfile();

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
  void fill(const double& x, const double& y);
  void fill(const double& x, const double& y, const double& w);
  void setBinEntries(unsigned, const double& entries);
  void setBinContent(unsigned, const double& content);
  void setBinError(unsigned, const double& error);
  void setBinInfo(unsigned, const double& entries, const double& content,const double& error);

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
  TProfile * m_h1;
  TProfile_LW * m_h2;
};

#include "HistValProfile.icc"

#endif
