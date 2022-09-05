/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class TH3F;

class Interp3D {
public:
    struct VetoInterp {
        std::vector<std::pair<double, double>> xRange;
        std::vector<std::pair<double, double>> yRange;
    };

    Interp3D() = default;
    Interp3D(const std::map<std::string, VetoInterp>& noInterp) : m_NoInterp(noInterp) {}

    ~Interp3D() = default;
    double Interpol3d(double x, double y, double z, std::shared_ptr<TH3F> h) const;
    void debug(bool debug = true) { m_debug = debug; }

private:
    std::map<std::string, VetoInterp> m_NoInterp;
    bool m_debug{false};
};
