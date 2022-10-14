/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "xAODForward/ZdcModuleToString.h"
#include <sstream>

std::string ZdcModuleToString(const xAOD::ZdcModule& zm) 
{
  std::stringstream o;
  o << " ID=" << zm.zdcId();
  o << " S/M/T/C=" << zm.zdcSide();
  o << "/" << zm.zdcModule(); 
  o << "/" << zm.zdcType();
  o << "/" << zm.zdcChannel();
  o << "\n";
  for (auto s : {"g0data","g1data","g0d0data","g0d1data","g1d0data","g1d1data"} )
    {
      if (zm.isAvailable<std::vector<uint16_t>>(s))
	{
      	  o << s << ": ";
	  const std::vector<uint16_t>& v = zm.getWaveform(s);
	  for (uint16_t d : v)
	    {
	      o << " " << std::to_string(d);
	    }
	  o << "\n";
	}

    }
  if (zm.isAvailable<uint16_t>("LucrodTriggerAmp"))
    {
      o << "Trigger amp:" << zm.auxdata<uint16_t>("LucrodTriggerAmp") << "\n";
    }
  if (zm.isAvailable<uint16_t>("LucrodTriggerSideAmp"))
    {
      o << "Trigger side amp:" << zm.auxdata<uint16_t>("LucrodTriggerSideAmp") << "\n";
    }
  return o.str();
}

std::string ZdcModuleToString(const xAOD::ZdcModuleContainer& zc) 
{
  std::stringstream o;
  for (auto z: zc) { o << ZdcModuleToString(*z); }
  return o.str();
}
