/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FASTCALOSIMEVENT_TFCS1DFunctionHistogram_h
#define ISF_FASTCALOSIMEVENT_TFCS1DFunctionHistogram_h

#include "ISF_FastCaloSimEvent/TFCS1DFunction.h"
#include "TH1.h"
#include <vector>

class TFCS1DFunctionHistogram:public TFCS1DFunction
{

  public:
    TFCS1DFunctionHistogram() {};
    TFCS1DFunctionHistogram(TH1* hist, double);

    void Initialize(TH1* hist, double);

    using TFCS1DFunction::rnd_to_fct;
    virtual double rnd_to_fct(double rnd) const;
    TH1* vector_to_histo();
    double get_inverse(double rnd) const;
    double linear(double y1,double y2,double x1,double x2,double x) const;
    double non_linear(double y1,double y2,double x1,double x2,double x) const;
    
    double  get_maxdev(TH1*, TH1D*);
    void    smart_rebin_loop(TH1* hist, double);
    double  get_change(TH1*);
    TH1D*   smart_rebin(TH1D*);
    double* histo_to_array(TH1*);
    double  sample_from_histo(TH1* hist, double);
    double  sample_from_histovalues(double);

    std::vector<float> get_HistoBorders() {return m_HistoBorders;};
    std::vector<float> get_HistoContents()  {return m_HistoContents;};

  protected:

    std::vector<float> m_HistoBorders;
    std::vector<float> m_HistoContents;

  ClassDef(TFCS1DFunctionHistogram,1)  //TFCS1DFunctionHistogram

};

#endif
