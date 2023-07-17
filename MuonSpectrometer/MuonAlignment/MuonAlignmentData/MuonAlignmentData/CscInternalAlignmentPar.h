/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONALIGNMENTDATA_CSCLINEPAR_H
#define MUONALIGNMENTDATA_CSCLINEPAR_H

#include <string>
/// Class will be deleted soon....
class CscInternalAlignmentPar  {
public:
    // Default constructor
    CscInternalAlignmentPar() = default;
    // destructor
    ~CscInternalAlignmentPar() = default;

    void setAmdbId(const std::string& type, int jff, int jzz, int job, int wireLayer);
    void getAmdbId(std::string& type, int& jff, int& jzz, int& job, int& wireLayer) const;

    void setParameters(float s, float z, float t, float rotS, float rotZ, float rotT);

    void getParameters(float& s, float& z, float& t, float& rotS, float& rotZ, float& rotT) const;
    void isNew(bool){}
    bool isNew() const {return true;}
private:
    std::string m_Type{};
    int m_Jff{0};
    int m_Jzz{0};
    int m_Job{0};
    // wire layer identifier
    int m_wireLayer;

    // translation parameters
    float m_S{0.f};
    float m_Z{0.f};
    float m_T{0.f};
    // rotation parameters
    float m_rotS{0.f};
    float m_rotZ{0.f};
    float m_rotT{0.f};
};

#endif  // MUONALIGNMENTDATA_CSCLINEPAR_H
