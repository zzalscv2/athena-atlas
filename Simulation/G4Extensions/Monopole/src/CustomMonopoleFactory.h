/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MONOPOLE_CustomMonopoleFactory_h
#define MONOPOLE_CustomMonopoleFactory_h 1

// package headers
#include "CustomMonopole.h"
// STL headers
#include <set>


// ######################################################################
// ###                          Monopole                              ###
// ######################################################################

class CustomMonopoleFactory
{
public:
  static const CustomMonopoleFactory& instance();
  void loadCustomMonopoles();
  bool isCustomMonopole(CustomMonopole *particle) const;

private:
  CustomMonopoleFactory();
  std::set<CustomMonopole *> m_particles;


};


#endif // MONOPOLE_CustomMonopoleFactory_h
