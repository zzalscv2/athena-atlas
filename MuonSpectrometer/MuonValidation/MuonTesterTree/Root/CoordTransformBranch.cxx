/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonTesterTree/CoordTransformBranch.h>
namespace MuonVal {

CoordTransformBranch::CoordTransformBranch(MuonTesterTree& tree, const std::string& vec_name):
        MuonTesterBranch(tree, vec_name) {}

bool CoordTransformBranch::fill(const EventContext&) { return true; }
bool CoordTransformBranch::init() { return true; }
void CoordTransformBranch::operator=(const Amg::Transform3D& trans) { set(trans); }

void CoordTransformBranch::set(const Amg::Transform3D& trans){
    m_transform.set(Amg::Vector3D{trans.translation()}, 0);
    m_transform.set(Amg::Vector3D{trans.linear()*Amg::Vector3D::UnitX()}, 1);
    m_transform.set(Amg::Vector3D{trans.linear()*Amg::Vector3D::UnitY()}, 2);
    m_transform.set(Amg::Vector3D{trans.linear()*Amg::Vector3D::UnitZ()}, 3);   
}

CoordSystemsBranch::CoordSystemsBranch(MuonTesterTree& tree, const std::string& vec_name):
        MuonTesterBranch(tree, vec_name) {}

bool CoordSystemsBranch::fill(const EventContext&) { return true; }
bool CoordSystemsBranch::init() { return true; }
size_t CoordSystemsBranch::size() const { return m_translation.size(); }
void CoordSystemsBranch::operator+=(const Amg::Transform3D& trans) { push_back(trans); }
void CoordSystemsBranch::push_back(const Amg::Transform3D& trans){ set(trans, size());}

void CoordSystemsBranch::set(const Amg::Transform3D& trans, size_t pos) {
    m_translation.set(Amg::Vector3D{trans.translation()}, pos);
    m_linearCol1.set(Amg::Vector3D{trans.linear()*Amg::Vector3D::UnitX()}, pos);
    m_linearCol2.set(Amg::Vector3D{trans.linear()*Amg::Vector3D::UnitY()}, pos);
    m_linearCol3.set(Amg::Vector3D{trans.linear()*Amg::Vector3D::UnitZ()}, pos);    
}

}