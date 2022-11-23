/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Header file for class TH2I_LW                             //
//                                                            //
//  Description: LightWeight version of TH2I.                 //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: March 2009                               //
//                                                            //
////////////////////////////////////////////////////////////////

#ifndef TH2I_LW_H
#define TH2I_LW_H

#include "LWHists/LWHist2D.h"
class TH2I;

class TH2I_LW : public LWHist2D {
public:
  typedef int bin_type_t;

  //To allocate from pool - remember to delete with LWHist::safeDelete(..):
  static TH2I_LW * create( const char* name, const char* title,
			   unsigned nbinsx, const double& xmin, const double& xmax,
			   unsigned nbinsy, const double& ymin, const double& ymax );
  static TH2I_LW * create( const char* name, const char* title,
			   unsigned nbinsx, const double* xbins,
			   unsigned nbinsy, const double& ymin, const double& ymax );
  static TH2I_LW * create( const char* name, const char* title,
			   unsigned nbinsx, const double& xmin, const double& xmax,
			   unsigned nbinsy, const double* ybins );
  static TH2I_LW * create( const char* name, const char* title,
			   unsigned nbinsx, const double* xbins,
			   unsigned nbinsy, const double* ybins );
  static TH2I_LW * create( const char* name, const char* title,
			   unsigned nbinsx, const float* xbins,
			   unsigned nbinsy, const float* ybins );

  virtual void Fill(const double& x, const double& y) override;
  virtual void Fill(const double& x, const double& y, const double& w) override;
  virtual unsigned GetNbinsX() const override;
  virtual unsigned GetNbinsY() const override;
  virtual double GetBinContent(unsigned binx, unsigned biny) const override;
  virtual double GetBinError(unsigned binx, unsigned biny) const override;
  virtual void SetBinContent(unsigned binx, unsigned biny, const double& ) override;
  virtual void SetBinError(unsigned binx, unsigned biny, const double& ) override;
  virtual unsigned GetEntries() const override;
  virtual void SetEntries(unsigned) override;
  virtual void SetBins(unsigned nbinsx,double xmin,double xmax,
                       unsigned nbinsy,double ymin,double ymax) override;

  virtual void GetBinContentAndError(unsigned binx, unsigned biny,double& content, double&error) const override;
  virtual void SetBinContentAndError(unsigned binx, unsigned biny,const double& content, const double& error) override;
  virtual double getXMin() const override;
  virtual double getXMax() const override;
  virtual double getYMin() const override;
  virtual double getYMax() const override;

  virtual void Reset() override;

  virtual void getSums( double& sumW, double& sumW2,
                        double& sumWX,double& sumWX2,
                        double& sumWY, double& sumWY2,
                        double& sumWXY) const override;
  virtual void setSums( const double& sumW, const double&sumW2,
                        const double& sumWX, const double& sumWX2,
                        const double& sumWY, const double& sumWY2,
                        const double& sumWXY ) override;

         TH2I* getROOTHist();
  virtual TH1* getROOTHistBase() override;

  virtual double Integral() const override;

  //For fast looping, skipping bins where (content,error)==(0,0):
  virtual void resetActiveBinLoop() override;
  virtual bool getNextActiveBin(unsigned& binx, unsigned& biny, double& content, double& error) override;

  virtual void scaleContentsAndErrors( const double& fact ) override;//C.f. comment in LWHist1D.h

private:
  friend class LWHistInt;
  friend class LWHistVal;
  virtual void clear() override;
  double getBinCenterX(int bin) const;
  double getBinCenterY(int bin) const;
  virtual TH1* getROOTHistBaseNoAlloc() const override;
  virtual void clearKeptROOTHist() override;//Does nothing if root-backend.
  const float * getVarBinsX() const;//null if fixed bin-widths
  const float * getVarBinsY() const;//null if fixed bin-widths
  virtual double actualGetBinCenterX(int bin) const override;
  virtual double actualGetBinCenterY(int bin) const override;
  virtual unsigned actualFindBinX(const double&) const override;
  virtual unsigned actualFindBinY(const double&) const override;
  TH2I_LW( const char* name, const char* title,
	   unsigned nbinsx, const double& xmin, const double& xmax,
	   unsigned nbinsy, const double& ymin, const double& ymax, bool rootbackend );
  TH2I_LW( const char* name, const char* title,
	   unsigned nbinsx, const double* xbins,
	   unsigned nbinsy, const double& ymin, const double& ymax, bool rootbackend );
  TH2I_LW( const char* name, const char* title,
	   unsigned nbinsx, const double& xmin, const double& xmax,
	   unsigned nbinsy, const double* ybins, bool rootbackend );
  TH2I_LW( const char* name, const char* title,
	   unsigned nbinsx, const double* xbins,
	   unsigned nbinsy, const double* ybins, bool rootbackend );
  TH2I_LW( const char* name, const char* title,
	   unsigned nbinsx, const float* xbins,
	   unsigned nbinsy, const float* ybins, bool rootbackend );
  virtual ~TH2I_LW();
  TH2I_LW( const TH2I_LW & );
  TH2I_LW & operator= ( const TH2I_LW & );
  void * m_flexHisto;
  TH2I * m_rootHisto;
  unsigned m_rootbackend_fastloopbin;
  bool m_ownsRootSumw2;

};

#endif
