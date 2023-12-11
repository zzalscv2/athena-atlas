/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonSimEvent/TgcHitIdHelper.h"

#include <iomanip>
#include <array>

namespace {
    constexpr std::array<char, 4> v1 = {'B','E','T','C'};
    constexpr std::array<char, 9> v2 = {'I','M','O','E','1','2','3','4','S'};
    constexpr std::array<char, 6> v3 = {'S','L','E','R','F','G'};
}

//private constructor
TgcHitIdHelper::TgcHitIdHelper() : HitIdHelper()
{
  InitializeStationName();
  Initialize();
}

const TgcHitIdHelper* TgcHitIdHelper::GetHelper()
{
  static const TgcHitIdHelper helper;
  return &helper;
}

void TgcHitIdHelper::Initialize()
{
  InitializeField("StationPhi",1,48);
  InitializeField("StationEta",-5,5);
  InitializeField("GasGap",1,3);
}

void TgcHitIdHelper::InitializeStationName()
{
  InitializeField("Station[1]",0,sizeof(v1));
  InitializeField("Station[2]",0,sizeof(v2));
  InitializeField("Station[3]",0,sizeof(v3));
}

void TgcHitIdHelper::SetStationName(const std::string& name, int& hid) const
{
  for (unsigned int i=0;i<sizeof(v1);i++)
    if (v1[i]==name[0]) SetFieldValue("Station[1]",i,hid);
  for (unsigned int i=0;i<sizeof(v2);i++)
    if (v2[i]==name[1]) SetFieldValue("Station[2]",i,hid);
  for (unsigned int i=0;i<sizeof(v3);i++)
    if (v3[i]==name[2]) SetFieldValue("Station[3]",i,hid);
}
std::string TgcHitIdHelper::GetStationName(const int& hid) const
{
  std::string temp;
  temp+=v1[this->GetFieldValue("Station[1]",hid)];
  temp+=v2[this->GetFieldValue("Station[2]",hid)];
  temp+=v3[this->GetFieldValue("Station[3]",hid)];
  return temp;
}

int TgcHitIdHelper::GetStationPhi(const int& hid) const
{
  return this->GetFieldValue("StationPhi",hid);
}

int TgcHitIdHelper::GetStationEta(const int& hid) const
{
  return this->GetFieldValue("StationEta",hid);
}

int TgcHitIdHelper::GetGasGap(const int& hid) const
{
  return this->GetFieldValue("GasGap",hid);
}

//packing method
int TgcHitIdHelper::BuildTgcHitId(const std::string& statName, const int statPhi,
                                  const int statEta, const int gasG) const
{
  int theID(0);
  this->SetStationName(statName, theID);
  this->SetFieldValue("StationPhi", statPhi, theID);
  this->SetFieldValue("StationEta", statEta, theID);
  this->SetFieldValue("GasGap", gasG, theID);
  return theID;
}
