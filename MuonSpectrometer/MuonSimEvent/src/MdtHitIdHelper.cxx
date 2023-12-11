/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSimEvent/MdtHitIdHelper.h"

#include <iostream>
#include <iomanip>
#include <array>

namespace {
    constexpr std::array<char, 4> v1 = {'B','E','T','C'};
    constexpr std::array<char, 9> v2 = {'I','M','O','E','1','2','3','4','S'};
    constexpr std::array<char, 7> v3 = {'S','L','E','R','F','G','M'};
}

//private constructor
MdtHitIdHelper::MdtHitIdHelper(const unsigned int nTubes) : HitIdHelper()
{
  InitializeStationName();
  Initialize(nTubes);
}

const MdtHitIdHelper* MdtHitIdHelper::GetHelper(const unsigned int nTubes)
{
  static const MdtHitIdHelper helper(nTubes);
  return &helper;
}

void MdtHitIdHelper::Initialize(const unsigned int nTubes)
{
  InitializeField("PhiSector",1,8);
  InitializeField("ZSector",-8,8);
  InitializeField("MultiLayer",1,2);
  InitializeField("Layer",1,4);
  InitializeField("Tube",1,nTubes);
}

void MdtHitIdHelper::InitializeStationName()
{
  InitializeField("Station[1]",0,sizeof(v1));
  InitializeField("Station[2]",0,sizeof(v2));
  InitializeField("Station[3]",0,sizeof(v3));
}

void MdtHitIdHelper::SetStationName(const std::string& name, int& hid) const
{
  for (unsigned int i=0;i<sizeof(v1);i++)
    if (v1[i]==name[0]) SetFieldValue("Station[1]",i,hid);
  for (unsigned int i=0;i<sizeof(v2);i++)
    if (v2[i]==name[1]) SetFieldValue("Station[2]",i,hid);
  for (unsigned int i=0;i<sizeof(v3);i++)
    if (v3[i]==name[2]) SetFieldValue("Station[3]",i,hid);


}
std::string MdtHitIdHelper::GetStationName(const int& hid) const
{
  char v[4];
  v[0]=v1[this->GetFieldValue("Station[1]",hid)];
  v[1]=v2[this->GetFieldValue("Station[2]",hid)];
  v[2]=v3[this->GetFieldValue("Station[3]",hid)];
  v[3]='\0';
  std::string temp=v;
  return temp;
}

int MdtHitIdHelper::GetPhiSector(const int& hid) const
{
  return this->GetFieldValue("PhiSector",hid);
}

int MdtHitIdHelper::GetZSector(const int& hid) const
{
  return this->GetFieldValue("ZSector",hid);
}

//----MDT
int MdtHitIdHelper::GetMultiLayer(const int& hid) const
{
  return this->GetFieldValue("MultiLayer",hid);
}

int MdtHitIdHelper::GetLayer(const int& hid) const
{
  return this->GetFieldValue("Layer",hid);
}

int MdtHitIdHelper::GetTube(const int& hid) const
{
  return this->GetFieldValue("Tube",hid);
}

//packing method
int MdtHitIdHelper::BuildMdtHitId(const std::string& statName, const int phiSect,
                                  const int zSect, const int multiLayer, const int layer, const int tube) const
{
  int theID(0);
  this->SetStationName(statName, theID);
  this->SetFieldValue("PhiSector", phiSect, theID);
  this->SetFieldValue("ZSector", zSect, theID);
  this->SetFieldValue("MultiLayer", multiLayer, theID);
  this->SetFieldValue("Layer", layer, theID);
  this->SetFieldValue("Tube", tube, theID);
  return theID;
}
