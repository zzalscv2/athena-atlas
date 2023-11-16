// Dear emacs, this is -*-c++-*-

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
   MuonAlignmentErrorData is condition data which is derived and recorded by MuonAlignmentErrorDbAlg
 */

#ifndef MUONALIGNMENTERRORDATA_H
#define MUONALIGNMENTERRORDATA_H

#include <boost/regex.hpp>
#include <vector>


// Struct for per-Station Deviations Information

class MuonAlignmentErrorData {
    friend class MuonAlignmentErrorDbAlg;

public:
    struct Deviation {
        boost::regex stationName {""};
        boost::regex multilayer {""};
        double translation {0.0};
        double rotation {0.0};
    };

    MuonAlignmentErrorData() = default;
    virtual ~MuonAlignmentErrorData() = default;

    void setDeviations(std::vector<Deviation> vec);
    [[nodiscard]] const std::vector<Deviation>& getDeviations() const;

    void setClobVersion(std::string clobVersion);
    [[nodiscard]] const std::string& getClobVersion() const;

    void setHasNswHits(bool val);
    [[nodiscard]] bool hasNswHits() const;

private:
    std::vector<Deviation> m_deviations {};
    std::string m_clobVersion {"0.1"};
    bool m_hasNswHits {false};
};

#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF(MuonAlignmentErrorData, 115867308, 1)
#include "AthenaKernel/CondCont.h"
CLASS_DEF(CondCont<MuonAlignmentErrorData>, 265772564, 0)

#endif
