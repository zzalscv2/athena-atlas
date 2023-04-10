/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkDriftCircleMath/DCSLFitter.h"

#include <iostream>

namespace TrkDriftCircleMath {

bool DCSLFitter::fit(Segment& result, const Line& line, const DCOnTrackVec& dcs,
                     const HitSelection& selection, double) const{

  unsigned int N = dcs.size();

  if (N < 2) {
    return false;
  }

  if (selection.size() != N) {
    return false;
  } else {
    int used(0);
    for (unsigned int i = 0; i < N; ++i) {
      if (selection[i] == 0)
        ++used;
    }
    if (used < 2) {
      return false;
    }
  }

  double S{0}, Sz{0}, Sy{0};
  double Zc{0}, Yc{0};
  // reserve enough space for hits
  std::vector<FitData> data;
  data.reserve(N);
  unsigned int ii = 0;
  for (const DCOnTrack& it : dcs) {
    FitData datum{};
    datum.y = it.y();
    datum.z = it.x();
    datum.r = std::abs(it.r());
    datum.w = std::pow(it.dr() > 0. ? 1. / it.dr() : 0., 2);

    if (selection[ii] == 0) {
      S += datum.w;
      Sz += datum.w * datum.z;
      Sy += datum.w * datum.y;
    }
    data.push_back(std::move(datum));
    ++ii;
  }
  Zc = Sz / S;
  Yc = Sy / S;

  //
  //    shift hits
  //
  Sy = 0;
  Sz = 0;
  double Syy{0}, Szy{0}, Syyzz{0};

  for (unsigned int i = 0; i < N; ++i) {
    FitData& datum = data[i];

    datum.y -= Yc;
    datum.z -= Zc;

    if (selection[i])
      continue;

    datum.rw = datum.r * datum.w;
    datum.ryw = datum.rw * datum.y;
    datum.rzw = datum.rw * datum.z;

    Syy += datum.y * datum.y * datum.w;
    Szy += datum.y * datum.z * datum.w;
    Syyzz += (datum.y - datum.z) * (datum.y + datum.z) * datum.w;
  }

  unsigned int count{0};
  double R{0}, Ry{0}, Rz{0}, Att{0}, Add{S}, Bt{0}, Bd{0}, Stt{0}, Sd{0};

  double theta = line.phi();
  double cosin = std::cos(theta);
  double sinus = std::sin(theta);

  // make sure 0 <= theta < PI
  if (sinus < 0.0) {
    sinus = -sinus;
    cosin = -cosin;
  } else if (sinus == 0.0 && cosin < 0.0) {
    cosin = -cosin;
  }
  //
  // calculate shift
  double d = line.y0() + Zc * sinus - Yc * cosin;

  while (count < 100) {
    R = Ry = Rz = 0;

    for (unsigned int i = 0; i < N; ++i) {
      if (selection[i])
        continue;

      FitData& datum = data[i];

      double dist = datum.y * cosin - datum.z * sinus;
      if (dist > d) {
        R -= datum.rw;
        Ry -= datum.ryw;
        Rz -= datum.rzw;
      } else {
        R += datum.rw;
        Ry += datum.ryw;
        Rz += datum.rzw;
      }
    }
    Att = Syy + cosin * (2 * sinus * Szy - cosin * Syyzz);
    Bt = -Szy + cosin * (sinus * Syyzz + 2 * cosin * Szy + Rz) + sinus * Ry;
    Bd = -S * d + R;
    if (Att == 0) {
      if (data.capacity() > 100) {
        data.reserve(100);
        result.dcs().reserve(100);
      }
      return false;
    }
    theta += Bt / Att;
    if (theta < 0.0)
      theta += M_PI;
    if (theta >= M_PI)
      theta -= M_PI;
    cosin = std::cos(theta);
    sinus = std::sqrt(1 - cosin * cosin);
    d = R / S;
    if (std::abs(Bt / Att) < 0.001 && std::abs(Bd / Add) < 0.001) {
      Stt = std::sqrt(1 / Att);
      Sd = std::sqrt(1 / Add);
      break;
    }
    ++count;
  }
  // Fit did not converge
  if (count >= 100) {
    return false;
  }

  double yl{0}, chi2{0};

  double dtheta = Stt;
  double dy0 = Sd;

  result.dcs().clear();
  result.clusters().clear();
  result.emptyTubes().clear();

  unsigned int nhits{0};

  // calculate predicted hit positions from track parameters
  result.dcs().reserve(N);
  for (unsigned int i = 0; i < N; ++i) {
    FitData& datum = data[i];
    yl = cosin * datum.y - sinus * datum.z - d;
    double dth = -(sinus * datum.y + cosin * datum.z) * Stt;
    double errorResiduals = std::hypot(dth, Sd);
    double residuals = std::abs(yl) - datum.r;
    if (selection[i] == 0) {
      ++nhits;
      chi2 += residuals * residuals * datum.w;
    }
    result.dcs().emplace_back(dcs[i]);
    result.dcs().back().residual(residuals);
    result.dcs().back().errorTrack(errorResiduals);
  }

  result.set(chi2, nhits - 2, dtheta, dy0);
  result.line().set(LocVec2D(Zc - sinus * d, Yc + cosin * d), theta);

  return true;
}
}  // namespace TrkDriftCircleMath
