/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file StoreGate/test/WriteDecorHandle_test.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Apr, 2017
 * @brief Tests for WriteDecorHandle.
 */


#undef NDEBUG
#include "AthenaKernel/ExtendedEventContext.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthContainers/DataVector.h"
#include "AthContainers/AuxElement.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/AuxStoreInternal.h"
#include "SGTools/TestStore.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/errorcheck.h"
#include "TestTools/initGaudi.h"
#include "TestTools/expect_exception.h"
#include <cassert>
#include <iostream>


class MyObj
  : public SG::AuxElement
{
public:
  MyObj(int x=0) : x(x) {}
  int x;
};
CLASS_DEF (MyObj, 293847295, 1)
static const CLID MyCLID = 293847295;

typedef DataVector<MyObj> MyObjCont;
CLASS_DEF (MyObjCont, 293847296, 1)


// Ctors
void test1()
{
  std::cout << "test1\n";

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t ityp = r.getAuxID<int> ("aaa");

  SG::WriteDecorHandleKey<MyObj> k3 ("asd.aaa");
  assert (k3.initialize().isSuccess());

  SG::WriteDecorHandle<MyObj, int> h1 (k3);
  assert (h1.clid() == MyCLID);
  assert (h1.key() == "asd");
  assert (h1.decorKey() == "asd.aaa");
  assert (h1.storeHandle().name() == "StoreGateSvc");
  assert (h1.mode() == Gaudi::DataHandle::Writer);
  assert (h1.auxid() == ityp);
  assert (!h1.isPresent());

  SGTest::TestStore dumstore;
  EventContext ctx5;
  ctx5.setExtension( Atlas::ExtendedEventContext(&dumstore) );

  SG::WriteDecorHandle<MyObj, int> h5 (k3, ctx5);
  assert (h5.clid() == MyCLID);
  assert (h5.key() == "asd");
  assert (h5.decorKey() == "asd.aaa");
  assert (h5.storeHandle().name() == "StoreGateSvc");
  assert (h5.mode() == Gaudi::DataHandle::Writer);
  assert (h5.store() == "TestStore");
  assert (h5.auxid() == ityp);

  SG::WriteDecorHandleKey<MyObj> k6 ("asd.aaa", "OtherStore");
  assert (k6.initialize().isSuccess());
  SG::WriteDecorHandle<MyObj, int> h6 (k6, ctx5);
  assert (h6.clid() == MyCLID);
  assert (h6.key() == "asd");
  assert (h6.decorKey() == "asd.aaa");
  assert (h6.storeHandle().name() == "OtherStore");
  assert (h6.mode() == Gaudi::DataHandle::Writer);
  assert (h6.store() == "OtherStore" || h6.store() == "OtherStore_Impl");
  assert (h6.auxid() == ityp);

  {
    typedef SG::WriteDecorHandle<MyObj, int> handle_t;
    SG::WriteDecorHandleKey<MyObj> k7 ("asd.aaa", "BazSvc");
    k7.initialize().ignore();
    EXPECT_EXCEPTION (SG::ExcUninitKey, handle_t h7 (k7, ctx5));
  }
}


// Copy.
void test2()
{
  std::cout << "test2\n";
  SGTest::TestStore testStore;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t ityp  = r.getAuxID<int> ("aaa");
  SG::auxid_t ityp2 = r.getAuxID<int> ("bbb");

  MyObj* fooptr = new MyObj(13);
  SG::AuxStoreInternal foostore;
  fooptr->setStore (&foostore);
  testStore.record (fooptr, "foo");
  SG::DataProxy* foo_proxy = testStore.proxy (MyCLID, "foo");
  assert (foo_proxy->refCount() == 1);

  MyObj* barptr = new MyObj(14);
  SG::AuxStoreInternal barstore;
  barptr->setStore (&barstore);
  testStore.record (barptr, "bar");
  SG::DataProxy* bar_proxy = testStore.proxy (MyCLID, "bar");
  assert (bar_proxy->refCount() == 1);

  SG::WriteDecorHandleKey<MyObj> k1 ("foo.aaa");
  assert (k1.initialize());
  SG::WriteDecorHandle<MyObj, int> h1 (k1);
  assert (h1.store() == "StoreGateSvc");
  assert (h1.setProxyDict (&testStore).isSuccess());
  assert (h1.store() == "TestStore");
  assert (h1.auxid() == ityp);
  assert (h1.cptr() == fooptr);
  // + 1 from the alias.
  assert (foo_proxy->refCount() == 2);

  SG::WriteDecorHandle<MyObj, int> h2 (h1);
  assert (h2.key() == "foo");
  assert (h2.decorKey() == "foo.aaa");
  assert (h2.store() == "TestStore");
  assert (h2.isInitialized());
  assert (h2.auxid() == ityp);
  assert (h2.cptr() == fooptr);
  assert (foo_proxy->refCount() == 2);

  SG::WriteDecorHandle <MyObj, int> h3 (std::move(h2));
  assert (h3.key() == "foo");
  assert (h3.decorKey() == "foo.aaa");
  assert (h3.store() == "TestStore");
  assert (h3.isInitialized());
  assert (h3.cptr() == fooptr);
  assert (foo_proxy->refCount() == 2);
  assert (h2.store() == "TestStore");
  assert (!h2.isInitialized());
  assert (h2.cachedPtr() == nullptr);
  assert (h2.auxid() == ityp);
  assert (h3.auxid() == ityp);

  SG::WriteDecorHandleKey<MyObj> k4 ("bar.bbb");
  assert (k4.initialize().isSuccess());
  SG::WriteDecorHandle<MyObj, int> h4 (k4);
  assert (h4.setProxyDict (&testStore).isSuccess());
  assert (h4.cptr() == barptr);
  assert (bar_proxy->refCount() == 2);
  assert (h4.auxid() == ityp2);

  h3 = h4;
  assert (h3.key() == "bar");
  assert (h3.decorKey() == "bar.bbb");
  assert (h3.store() == "TestStore");
  assert (h3.isInitialized());
  assert (h3.cptr() == barptr);
  assert (h4.key() == "bar");
  assert (h4.decorKey() == "bar.bbb");
  assert (h4.store() == "TestStore");
  assert (h4.isInitialized());
  assert (h4.cptr() == barptr);
  assert (h3.auxid() == ityp2);
  assert (h4.auxid() == ityp2);

  assert (foo_proxy->refCount() == 2);
  assert (bar_proxy->refCount() == 2);

  // h1: foo, h2: unint, h3: bar, h4: bar

  h2 = std::move(h3);
  assert (h2.key() == "bar");
  assert (h2.decorKey() == "bar.bbb");
  assert (h2.store() == "TestStore");
  assert (h2.isInitialized());
  assert (h2.cptr() == barptr);
  assert (h3.store() == "TestStore");
  assert (!h3.isInitialized());
  assert (h3.cachedPtr() == nullptr);
  assert (h2.auxid() == ityp2);
  assert (h3.auxid() == ityp2);

  assert (foo_proxy->refCount() == 2);
  assert (bar_proxy->refCount() == 2);
}


// Decorate, for container.
void test3()
{
  std::cout << "test3\n";
  SGTest::TestStore testStore;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t ityp  = r.getAuxID<int> ("aaa");

  SG::AuxStoreInternal auxstore;
  auto cont = std::make_unique<MyObjCont>();
  cont->setStore (&auxstore);
  cont->push_back (new MyObj(1));
  cont->push_back (new MyObj(2));
  cont->push_back (new MyObj(3));
  const MyObjCont* pcont = cont.get();
  testStore.record (std::move (cont), "foo");

  SG::WriteDecorHandleKey<MyObjCont> k1 ("foo.aaa");
  assert (k1.initialize().isSuccess());
  SG::WriteDecorHandle<MyObjCont, int> h1 (k1);
  assert (h1.setProxyDict (&testStore).isSuccess());
  assert (h1.auxid() == ityp);
  assert (!h1.isAvailable());
  assert (h1.isPresent());

  h1.getDecorationArray()[0] = 10;
  h1 (*(*pcont)[1]) = 11;
  h1 (2) = 12;
  assert (h1.isAvailable());

  MyObj::Accessor<int> adec ("aaa");
  assert (adec (*(*pcont)[0]) == 10);
  assert (adec (*(*pcont)[1]) == 11);
  assert (adec (*(*pcont)[2]) == 12);

  MyObj::Decorator<int> bdec ("bbb");
  bdec (*(*pcont)[0]) = 110;
  bdec (*(*pcont)[1]) = 111;
  bdec (*(*pcont)[2]) = 112;

  SG::WriteDecorHandleKey<MyObjCont> k2 ("foo.bbb");
  assert (k2.initialize().isSuccess());
  SG::WriteDecorHandle<MyObjCont, int> h2 (k2);
  assert (h2.setProxyDict (&testStore).isSuccess());
  assert (h2.auxid() == r.getAuxID<int> ("bbb"));
  assert (h2.isAvailable());
  assert (h2.isPresent());

  SG::AuxStoreInternal auxstore2;
  auto cont2 = std::make_unique<MyObjCont>();
  cont2->setStore (&auxstore2);
  cont2->push_back (new MyObj(1));
  cont2->push_back (new MyObj(2));
  cont2->push_back (new MyObj(3));
  const MyObjCont* pcont2 = cont2.get();
  EXPECT_EXCEPTION( SG::ExcBadDecorElement, h1 (*(*pcont2)[1]) = 11 );
}


// Decorate, standalone.
void test4()
{
  std::cout << "test4\n";
  SGTest::TestStore testStore;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t ityp  = r.getAuxID<int> ("bbb");

  SG::AuxStoreInternal auxstore;
  auto obj = std::make_unique<MyObj>(5);
  obj->setStore (&auxstore);
  const MyObj* pobj = obj.get();
  testStore.record (std::move (obj), "bar");

  SG::WriteDecorHandleKey<MyObj> k1 ("bar.bbb");
  assert (k1.initialize().isSuccess());
  SG::WriteDecorHandle<MyObj, int> h1 (k1);
  assert (h1.setProxyDict (&testStore).isSuccess());
  assert (h1.auxid() == ityp);
  assert (!h1.isAvailable());

  h1 (*pobj) = 22;

  MyObj::Accessor<int> bdec ("bbb");
  assert (bdec (*pobj) == 22);
  assert (h1.isAvailable());
}


// makeHandle
void test5()
{
  std::cout << "test5\n";
  SGTest::TestStore testStore;

  SG::AuxTypeRegistry& r = SG::AuxTypeRegistry::instance();
  SG::auxid_t ityp = r.getAuxID<int> ("aaa");

  SG::WriteDecorHandleKey<MyObj> k1 ("asd.aaa");
  assert (k1.initialize().isSuccess());
  auto h1 = SG::makeHandle<int> (k1);
  assert (h1.clid() == MyCLID);
  assert (h1.key() == "asd");
  assert (h1.decorKey() == "asd.aaa");
  assert (h1.storeHandle().name() == "StoreGateSvc");
  assert (h1.mode() == Gaudi::DataHandle::Writer);
  assert (h1.auxid() == ityp);

  SG::WriteDecorHandleKey<MyObj> k2 ("asd.aaa", "BazSvc");
  k2.initialize().ignore();
  EXPECT_EXCEPTION (SG::ExcUninitKey, SG::makeHandle<int> (k2));

  SGTest::TestStore dumstore;
  EventContext ctx;
  ctx.setExtension( Atlas::ExtendedEventContext(&dumstore) );
  auto h2 = SG::makeHandle<int> (k1, ctx);
  assert (h2.clid() == MyCLID);
  assert (h2.key() == "asd");
  assert (h2.decorKey() == "asd.aaa");
  assert (h2.storeHandle().name() == "StoreGateSvc");
  assert (h2.mode() == Gaudi::DataHandle::Writer);
  assert (h2.store() == "TestStore");
  assert (h2.auxid() == ityp);

  SG::WriteDecorHandleKey<MyObj> k3 ("asd.aaa", "OtherStore");
  assert (k3.initialize().isSuccess());
  auto h3 = SG::makeHandle<int> (k3, ctx);
  assert (h3.clid() == MyCLID);
  assert (h3.key() == "asd");
  assert (h3.decorKey() == "asd.aaa");
  assert (h3.storeHandle().name() == "OtherStore");
  assert (h3.mode() == Gaudi::DataHandle::Writer);
  assert (h3.store() == "OtherStore" || h3.store() == "OtherStore_Impl");
  assert (h3.auxid() == ityp);

  SG::WriteDecorHandleKey<MyObj> k4 ("asd.aaa", "BazSvc");
  k4.initialize().ignore();
  EXPECT_EXCEPTION (SG::ExcUninitKey, SG::makeHandle<int> (k4, ctx));
}


// renounce
void test6()
{
  std::cout << "test6\n";

  SGTest::TestStore testStore;

  SG::AuxStoreInternal auxstore1;
  auto cont1 = std::make_unique<MyObjCont>();
  cont1->setStore (&auxstore1);
  cont1->push_back (new MyObj(1));
  const MyObjCont* pcont1 = cont1.get();
  testStore.record (std::move (cont1), "foo1");
  SG::DataProxy* foo1_proxy = testStore.proxy (ClassID_traits<MyObjCont>::ID(), "foo1");

  SG::AuxStoreInternal auxstore2;
  auto cont2 = std::make_unique<MyObjCont>();
  cont2->setStore (&auxstore2);
  cont2->push_back (new MyObj(2));
  const MyObjCont* pcont2 = cont2.get();
  testStore.record (std::move (cont2), "foo2");
  SG::DataProxy* foo2_proxy = testStore.proxy (ClassID_traits<MyObjCont>::ID(), "foo2");

  SG::WriteDecorHandleKey<MyObjCont> k1 ("foo1.aaa");
  assert (k1.initialize().isSuccess());
  SG::WriteDecorHandle<MyObjCont, int> h1 (k1);
  assert (h1.setProxyDict (&testStore).isSuccess());

  assert (foo1_proxy->refCount() == 1);
  h1 (*(*pcont1)[0]) = 11;
  assert (foo1_proxy->refCount() == 2);
  EXPECT_EXCEPTION( SG::ExcBadDecorElement, h1 (*(*pcont2)[0]) = 12 );

  SG::WriteDecorHandleKey<MyObjCont> k2 ("foo2.aaa");
  assert (k2.initialize().isSuccess());
  k2.renounce();
  SG::WriteDecorHandle<MyObjCont, int> h2 (k2);
  assert (h2.setProxyDict (&testStore).isSuccess());

  assert (foo2_proxy->refCount() == 1);
  h2 (*(*pcont2)[0]) = 13;
  assert (foo2_proxy->refCount() == 1);
  h2 (*(*pcont1)[0]) = 14; // ok, h2 has been renounced.
}


// locking
void test7()
{
  std::cout << "test7\n";

  SGTest::TestStore testStore;

  SG::AuxStoreInternal auxstore;
  auto cont = std::make_unique<MyObjCont>();
  cont->setStore (&auxstore);
  cont->push_back (new MyObj(1));
  const MyObjCont* pcont = cont.get();
  cont->lock();
  testStore.record (std::move (cont), "foo");
  

  MyObj::Decorator<int> bdec ("aaa");

  SG::WriteDecorHandleKey<MyObjCont> k1 ("foo.aaa");
  assert (k1.initialize().isSuccess());
  {
    SG::WriteDecorHandle<MyObjCont, int> h1 (k1);
    assert (h1.setProxyDict (&testStore).isSuccess());
    h1 (*(*pcont)[0]) = 11;
    assert (bdec (*(*pcont)[0]) == 11);
  }

  EXPECT_EXCEPTION (SG::ExcStoreLocked, bdec (*(*pcont)[0]) = 12);
}


int main()
{
  errorcheck::ReportMessage::hideErrorLocus();
  ISvcLocator* svcloc;
  //need MessageSvc
  if (!Athena_test::initGaudi("StoreGate/VarHandleBase_test.txt", svcloc)) {
    return 1;
  }

  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  test7();
  return 0;
}
