/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonTesterTree/TwoVectorBranch.h>
namespace MuonVal {

bool TwoVectorBranch::fill(const EventContext&) { return true; }
bool TwoVectorBranch::init() { return true; }

TwoVectorBranch::TwoVectorBranch(MuonTesterTree& tree, const std::string& vec_name) : MuonTesterBranch(tree, vec_name) {}
size_t TwoVectorBranch::size() const { return m_x.size(); }

void TwoVectorBranch::push_back(const float x, const float y) {
    m_x += x;
    m_y += y;
}
void TwoVectorBranch::set(const float x, const float y, size_t pos) {
    m_x[pos] = x;
    m_y[pos] = y;
}
void TwoVectorBranch::push_back(const Amg::Vector2D& vec) { push_back(vec[0], vec[1]); }
void TwoVectorBranch::set(const Amg::Vector2D& vec, size_t pos) { set(vec[0], vec[1], pos); }
void TwoVectorBranch::operator+=(const Amg::Vector2D& vec) { push_back(vec); }
void TwoVectorBranch::push_back(const TVector2& vec) { push_back(vec.X(), vec.Y()); }
void TwoVectorBranch::operator+=(const TVector2& vec) { push_back(vec); }
void TwoVectorBranch::set(const TVector2& vec, size_t pos) { set(vec.X(), vec.Y(), pos); }
}