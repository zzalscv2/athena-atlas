// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 */
/**
 * @file GeoModelUtilities/GeoRef.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Aug, 2022
 * @brief Simple smart-pointer class for GeoModel objects.
 *
 * GeoModel itself has a Link class, but it only handles the volume base class.
 */


#ifndef GEOMODELUTILITIES_GEOREF_H
#define GEOMODELUTILITIES_GEOREF_H


template <class T>
class GeoRef
{
public:
  GeoRef (T* p = nullptr) noexcept
    : m_p (p)
  {
    if (m_p) m_p->ref();
  }


  ~GeoRef() noexcept
  {
    if (m_p) m_p->unref();
  }


  GeoRef (const GeoRef& o) noexcept
    : m_p (o.m_p)
  {
    if (m_p) m_p->ref();
  }


  template <class U>
  GeoRef (GeoRef<U>& o) noexcept
    : m_p (o.get())
  {
    if (m_p) m_p->ref();
  }


  GeoRef (GeoRef&& o) noexcept
    : m_p (o.m_p)
  {
    o.m_p = nullptr;
  }


  GeoRef& operator= (const GeoRef& o) noexcept
  {
    if (this != &o) {
      if (m_p) m_p->unref();
      m_p = o.m_p;
      if (m_p) m_p->ref();
    }
    return *this;
  }


  template <class U>
  GeoRef& operator= (GeoRef<U>& o) noexcept
  {
    if (m_p) m_p->unref();
    m_p = o.get();
    if (m_p) m_p->ref();
    return *this;
  }

 
  GeoRef& operator= (GeoRef&& o) noexcept
  {
    if (this != &o) {
      if (m_p) m_p->unref();
      m_p = o.m_p;
      o.m_p = nullptr;
    }
    return *this;
  }


  operator T*() noexcept
  {
    return m_p;
  }

  
  T* get() noexcept
  {
    return m_p;
  }


  T* operator->() noexcept
  {
    return m_p;
  }


  T& operator*() noexcept
  {
    return *m_p;
  }


private:
  T* m_p;
};



#endif // not GEOMODELUTILITIES_GEOREF_H
