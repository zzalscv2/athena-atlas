/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTESTER_MUONTWOVECTORBRANCH_H
#define MUONTESTER_MUONTWOVECTORBRANCH_H

#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include "MuonTesterTree/MuonTesterTree.h"
#include "MuonTesterTree/VectorBranch.h"

#include <TVector2.h>
/// Helper class to dump spatial vectors in their x,y representation
/// to the n-tuple
namespace MuonVal {
class TwoVectorBranch : public MuonTesterBranch {
public:
    TwoVectorBranch(MuonTesterTree& tree, const std::string& vec_name);

    /// interface using the Amg::Vector3D
    void push_back(const Amg::Vector2D& vec);
    void operator+=(const Amg::Vector2D& vec);
    void set(const Amg::Vector2D& vec, size_t pos);

    void push_back(const TVector2& vec);
    void operator+=(const TVector2& vec);
    void set(const TVector2& vec, size_t pos);

    void push_back(const float x, const float y);
    void set(const float x, const float y, size_t pos);

    size_t size() const;

    bool fill(const EventContext&) override final;
    bool init() override final;

private:
    VectorBranch<float>& m_x{parent().newVector<float>(name() + "X")};
    VectorBranch<float>& m_y{parent().newVector<float>(name() + "Y")};
};
}
#endif
