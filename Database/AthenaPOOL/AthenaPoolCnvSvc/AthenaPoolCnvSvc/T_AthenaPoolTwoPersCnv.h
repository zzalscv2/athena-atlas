/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPOOLCNVSVC_T_ATHENAPOOLTWOPERSCNV_H
#define ATHENAPOOLCNVSVC_T_ATHENAPOOLTWOPERSCNV_H

/** @file T_AthenaPoolTwoPersCnv.h
 *  @brief this file contains the class definition for the templated T_AthenaPoolTwoPersCnv class
 *         that is able to write two different persistent representations depending on runtime settings
 *  @author Marcin.Nowak@cern.ch
 **/

#include "T_AthenaPoolCustomCnv.h"

/**
 * @class T_AthenaPoolTwoPersCnv
 * @brief This templated class extends T_AthenaPoolCustomCnv
 * with the ability to write odditional persistent representation 
 * (usually an old one) if the user-provided writeOldPers() method returns true
 */
template<class TRANS, class PERS, class PERSOLD>
class T_AthenaPoolTwoPersCnv : public T_AthenaPoolCustomCnv< TRANS, PERS >
{
   typedef T_AthenaPoolCustomCnv<TRANS, PERS>    BaseCnv;
   typedef T_AthenaPoolCustomCnv<TRANS, PERSOLD> BaseOldCnv;

   // define an AthenaPool converter for the old representation PERSOLD
   class OldCnv : public BaseOldCnv
   {
      T_AthenaPoolTwoPersCnv      *m_ownerCnv;
   public:
      using BaseOldCnv::initialize;
      using BaseOldCnv::DataObjectToPool;
      OldCnv( ISvcLocator* svcloc, T_AthenaPoolTwoPersCnv *owner )
         : BaseOldCnv( svcloc ), m_ownerCnv( owner )
      {}

      // forward createPersistent() request to the main converter, where the user
      // provides createOldPersistent() method to do that
      virtual PERSOLD*  createPersistent( TRANS* transCont ) override final {
         return m_ownerCnv->createOldPersistent( transCont );
      }
      // unused virtual method that still needs to be defined 
      virtual TRANS*    createTransient() override final { return nullptr; };
   };

   // this is the converter that knows how to produce and manage PERSOLD representations
   std::unique_ptr<OldCnv>   m_oldAPCnv;

public:
   T_AthenaPoolTwoPersCnv( ISvcLocator* svcloc ) : BaseCnv(svcloc)  { }
   
protected:
   /// DataObjectToPool() that dispatches to the right converter based on writeOldPers()
   virtual StatusCode DataObjectToPool(IOpaqueAddress* pAddr, DataObject* pObj) override final {
      if( writeOldPers() ) {
         if( !m_oldAPCnv ) {
            m_oldAPCnv = std::make_unique<OldCnv>( this->serviceLocator() , this );
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

   /// additional user-supplied method that creates PERSOLD representation 
   virtual PERSOLD*     createOldPersistent( TRANS* ) = 0;
   /// user-supplied method that makes this converter write PERS if False or PERSOLD if True
   /// in the current Athena configuration for Event data layout it should returned
   /// a fixed value throughout the whole job
   virtual bool         writeOldPers() = 0; 
};


#endif
