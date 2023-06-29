/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INAVIGABLE_H
#define INAVIGABLE_H
///////////////////////////////////////////////////////////////////////////////
//
// Common base type for classes representing navigable objects
//
///////////////////////////////////////////////////////////////////////////////

#include <any>

class INavigationToken;

class INavigable
{
 public:

  virtual ~INavigable() = default;

  // enforce fillToken(,) method in derived classes!
  virtual void fillToken( INavigationToken & thisToken ) const = 0;
  virtual void fillToken( INavigationToken & thisToken, 
			  const std::any& weight ) const = 0;

};
#endif
