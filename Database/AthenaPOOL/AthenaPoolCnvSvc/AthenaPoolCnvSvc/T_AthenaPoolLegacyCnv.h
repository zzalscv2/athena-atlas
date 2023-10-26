/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPOOLCNVSVC_T_ATHENAPOOLLEGACYCNV_H
#define ATHENAPOOLCNVSVC_T_ATHENAPOOLLEGACYCNV_H

/** @file T_AthenaPoolLegacyCnv.h
 *  @brief this file contains the class definition for the templated T_AthenaPoolLegacyCnv class
 *         that is able to write two different persistent representations depending on runtime settings
 *  @author Marcin.Nowak@cern.ch
 **/

#include "T_AthenaPoolCustomCnv.h"

/**
 * @class T_AthenaPoolLegacyCnv
 * @brief This templated class extends T_AthenaPoolCustomCnv
 * with the ability to write odditional persistent representation 
 * (usually an old one) if the user-provided writingLegacy() method returns true
 */
template<class TRANS, class PERS, class LEGACY>
class T_AthenaPoolLegacyCnv : public T_AthenaPoolCustomCnv< TRANS, PERS >
{
   typedef T_AthenaPoolCustomCnv<TRANS, PERS>    BaseCnv;
   typedef T_AthenaPoolCustomCnv<TRANS, LEGACY>  BaseLegacyCnv;

   // define an internal AthenaPool converter that will create the old representation LEGACY
   class LegacyCnv : public BaseLegacyCnv
   {
      T_AthenaPoolLegacyCnv      *m_ownerCnv;
   public:
      using BaseLegacyCnv::initialize;
      using BaseLegacyCnv::DataObjectToPool;
      LegacyCnv( ISvcLocator* svcloc, T_AthenaPoolLegacyCnv *owner )
         : BaseLegacyCnv( svcloc ), m_ownerCnv( owner )
      {}

      // forward createPersistent() request to the main converter, where the user
      // provides createLegacy() method to do that
      virtual LEGACY*   createPersistent( TRANS* transCont ) override final {
         return m_ownerCnv->createLegacy( transCont );
      }
      // unused virtual method that still needs to be defined 
      virtual TRANS*    createTransient() override final { return nullptr; };
   };

   // this is the converter that knows how to produce and manage LEGACY representations
   std::unique_ptr<LegacyCnv>   m_oldAPCnv;

public:
   T_AthenaPoolLegacyCnv( ISvcLocator* svcloc ) : BaseCnv(svcloc)  { }
   
protected:
   /// DataObjectToPool() that dispatches to the right converter based on writingLegacy()
   virtual StatusCode DataObjectToPool(IOpaqueAddress* pAddr, DataObject* pObj) override final {
      if( writingLegacy() ) {
         if( !m_oldAPCnv ) {
            m_oldAPCnv = std::make_unique<LegacyCnv>( this->serviceLocator() , this );
            StatusCode sc = m_oldAPCnv->initialize();
            if( !sc.isSuccess() ) {
               ATH_MSG_ERROR("Failed to initialize old converter");
               return sc;
            }
         }
         return m_oldAPCnv->DataObjectToPool( pAddr, pObj );
      }
      return BaseCnv::DataObjectToPool( pAddr, pObj );
   }
   
   /// standard user-supplied methods to create TRANS and PERS representations
   virtual TRANS*       createTransient () override = 0;
   virtual PERS*        createPersistent( TRANS* ) override = 0;

   /// additional user-supplied method that creates LEGACY representation
   virtual LEGACY*      createLegacy( TRANS* ) = 0;
   /// user-supplied method that makes this converter write PERS if False or LEGACY if True
   /// in the current Athena configuration for Event data layout it should returned
   /// a fixed value throughout the whole job
   virtual bool         writingLegacy() = 0;
};


#endif
