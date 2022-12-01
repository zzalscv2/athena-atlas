/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class TH1I_LW                             //
//                                                            //
//  Description: LightWeight version of TH1I.                 //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: March 2009                               //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef TH1I_LW_H
#define TH1I_LW_H

#include "LWHists/LWHist1D.h"
class TH1I;

class TH1I_LW : public LWHist1D {
public:
  typedef int bin_type_t;

  //To allocate from pool - remember to delete with LWHist::safeDelete(..):
  static TH1I_LW * create( const char* name, const char* title,
			   unsigned nbinsx, const double& xlow, const double& xup );
  static TH1I_LW * create( const char* name, const char* title, unsigned nbinsx, const double* xbins );
  static TH1I_LW * create( const char* name, const char* title, unsigned nbinsx, const float* xbins );

  virtual void Fill(const double& x) override;
  virtual void Fill(const double& x, const double& w) override;
  virtual unsigned GetNbinsX() const override;
  virtual double GetBinContent(unsigned bin) const override;
  virtual double GetBinError(unsigned bin) const override;
  virtual void SetBinContent(unsigned bin, const double& ) override;
  virtual void SetBinError(unsigned bin, const double& ) override;
  virtual unsigned GetEntries() const override;
  virtual void SetEntries(unsigned) override;
  virtual void SetBins(unsigned nbins,double xmin,double xmax) override;
  virtual void Reset() override;

  virtual double getXMin() const override;
  virtual double getXMax() const override;
  virtual void GetBinContentAndError(unsigned bin,double& content, double&error) const override;
  virtual void SetBinContentAndError(unsigned bin,const double& content, const double& error) override;

  virtual void getSums( double& sumW, double& sumW2,
                        double& sumWX,double& sumWX2 ) const override;
  virtual void setSums( const double& sumW,const double&sumW2,
                        const double& sumWX,const double& sumWX2 ) override;

         TH1I* getROOTHist();
  virtual TH1* getROOTHistBase() override;

  virtual double Integral() const override;

  //For fast looping, skipping bins where (content,error)==(0,0):
  virtual void resetActiveBinLoop() override;
  virtual bool getNextActiveBin(unsigned& bin, double& content, double& error) override;

  virtual void scaleContentsAndErrors( const double& fact ) override;//C.f. comment in LWHist1D.h

private:
  friend class LWHistInt;
  friend class LWHistVal;
  virtual void clear() override;
  virtual unsigned actualFindBinX(const double&) const override;
  virtual double actualGetBinCenterX(int bin) const override;
  virtual TH1* getROOTHistBaseNoAlloc() const override;
  virtual void clearKeptROOTHist() override;//Does nothing if root-backend.
  const float * getVarBins() const;//null if fixed bin-widths
  float * getVarBins();//null if fixed bin-widths

  TH1I_LW( const char* name, const char* title,
	   unsigned nbinsx, const double& xlow, const double& xup, bool rootbackend );
  TH1I_LW( const char* name, const char* title, unsigned nbinsx, const double* xbins, bool rootbackend );
  TH1I_LW( const char* name, const char* title, unsigned nbinsx, const float* xbins, bool rootbackend );
  virtual ~TH1I_LW();
  TH1I_LW( const TH1I_LW & );
  TH1I_LW & operator= ( const TH1I_LW & );
  void * m_flexHisto;
  TH1I * m_rootHisto;
  unsigned m_rootbackend_fastloopbin;
  bool m_ownsRootSumw2;
};

#endif
