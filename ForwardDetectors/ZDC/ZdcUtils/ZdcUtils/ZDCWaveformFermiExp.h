/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef _ZDCWaveformFermiExp_h_
#define _ZDCWaveformFermiExp_h_

#include "ZDCWaveform.h"

//
// The Fermi*negative exponential that we have used since the start of Run 2 to
//   parameterize ZDC pulses
//
class ZDCWaveformFermiExp : virtual public ZDCWaveformBase
{
protected:
  double doEvaluate(double time) const override;

  virtual std::string name() const override {return "FermiExp";}

public:
  ZDCWaveformFermiExp() : 
    ZDCWaveformBase("default", 1, 6, std::vector<std::string>(), std::vector<double>())
  {}

  ZDCWaveformFermiExp(std::string tag, double initialTauRise, double initialTauFall) :
    ZDCWaveformBase(tag, initialTauRise, initialTauFall, std::vector<std::string>(), std::vector<double>())
  {}

  explicit ZDCWaveformFermiExp(const ZDCWaveformFermiExp& instance) :
    ZDCWaveformBase(instance.getTag(), instance.getTauRise(),instance.getTauFall(), std::vector<std::string>(), std::vector<double>()) {};
};

//
// A version of the Fermi*negative exponential that has a shift in baseline before and after T0
//   to describe a small (few %) long tail seen in pulses in the Run 3 data
//
//
class ZDCWaveformFermiExpTail : virtual public ZDCWaveformFermiExp
{
  double doEvaluate(double time) const override
  {
    double tailShift = getAddtlShapeValue(0);
    if (time <= 0) return ZDCWaveformFermiExp::doEvaluate(time);
    else return ZDCWaveformFermiExp::doEvaluate(time) + tailShift;
  }
  
  virtual std::string name() const override {return "FermiExpTail";}

public:
  ZDCWaveformFermiExpTail() : ZDCWaveformFermiExp()
  {
    setAddtlShapeParameters(std::vector<std::string>({"tailFrac"}), std::vector<double>({0}));
  }

  ZDCWaveformFermiExpTail(std::string tag, double initialTauRise, double initialTauFall, double tailShiftFrac) :
    ZDCWaveformFermiExp(tag, initialTauRise, initialTauFall)
  {
    double tailShiftScaled = tailShiftFrac*ZDCWaveformFermiExp::doEvaluate(0);
    setAddtlShapeParameters(std::vector<std::string>({"tailFrac"}), std::vector<double>({tailShiftScaled}));
  }

  ZDCWaveformFermiExpTail(const ZDCWaveformFermiExpTail& instance) : ZDCWaveformBase(static_cast<const ZDCWaveformBase&>(instance)), ZDCWaveformFermiExp(static_cast<const ZDCWaveformFermiExp&>(instance)) {}

  void setTailParameter(double tailShiftFrac) {
    double tailShiftScaled = tailShiftFrac*ZDCWaveformFermiExp::doEvaluate(0);
    setAddtlShapeValue(0, tailShiftScaled);
  }
  
};
#endif
