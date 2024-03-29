/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class VP1MissingEtSystem                //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: May 2007, rewritten July 2008            //
//  Updated By: Giorgi Gvaberidze(ggvaberi@cern.ch) July 2010 //
////////////////////////////////////////////////////////////////

#include "VP1MissingEtSystems/VP1MissingEtSystem.h"
#include "VP1MissingEtSystems/VP1MissingEtHandle.h"
#include "VP1MissingEtSystems/VP1MissingEtCaloHandle.h"
#include "VP1MissingEtSystems/VP1MissingEtTruthHandle.h"
#include "VP1MissingEtSystems/VP1MissingEtCollWidget.h"
//***
#include "VP1MissingEtSystems/missingEtController.h"

#include "VP1Base/VP1Serialise.h"
#include "VP1Base/VP1Deserialise.h"
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoSwitch.h>
#include <Inventor/SoPath.h>

//____________________________________________________________________
class VP1MissingEtSystem::Imp {
public:
  Imp(VP1MissingEtSystem* tc) : theclass(tc), collWidget(0), controller(0) {}
  VP1MissingEtSystem* theclass;
  VP1MissingEtCollWidget* collWidget;
  //***
  missingEtController* controller;
};

//_____________________________________________________________________________________
VP1MissingEtSystem::VP1MissingEtSystem()
: IVP13DSystemSimple ("EtMiss",
					  "Displaying missing ET.",
			          "Thomas.Kittelmann@cern.ch, ggvaberi@cern.ch"), m_d(new Imp(this))
{
}

//____________________________________________________________________
VP1MissingEtSystem::~VP1MissingEtSystem()
{
  delete m_d;
}

//_____________________________________________________________________________________
void VP1MissingEtSystem::systemerase ( )
{
  //Save present states and then clear all event data and related gui elements.
  m_d->collWidget->clear();
}

//_____________________________________________________________________________________
void VP1MissingEtSystem::buildEventSceneGraph ( StoreGateSvc*, SoSeparator *root )
{
  //Create collection list based on contents of event store, populate gui and apply states:
  QList<VP1StdCollection*> cols;
  for (VP1StdCollection*col : VP1MissingEtHandle::createCollections(this))
    cols << col;
  for (VP1StdCollection*col : VP1MissingEtCaloHandle::createCollections(this))
    cols << col;
  for (VP1StdCollection*col : VP1MissingEtTruthHandle::createCollections(this))
    cols << col;

  m_d->collWidget->setCollections(cols);

  //Add collections to event scenegraph:
  for (VP1StdCollection* col : m_d->collWidget->collections<VP1StdCollection>())
    root->addChild(col->collSwitch());
}

//_____________________________________________________________________________________
QWidget * VP1MissingEtSystem::buildController()
{
  //Updated: replaced to predefined controller
  m_d->controller = new missingEtController(this);
  m_d->collWidget = (VP1MissingEtCollWidget*)m_d->controller->collWidget();
  return m_d->controller;
  //m_d->collWidget = new VP1MissingEtCollWidget;
  //return m_d->collWidget;
}

//_____________________________________________________________________________________
void VP1MissingEtSystem::userPickedNode(SoNode*, SoPath * pickedPath)
{
  //Find in which collection an object was picked and let that deal with the pick:
  for (VP1MissingEtHandle* handle : m_d->collWidget->collections<VP1MissingEtHandle>()) {
    if (handle->visible()&&pickedPath->containsNode(handle->collSep())) {
      message(handle->clicked(pickedPath));
      return;
    }
  }
  message("Error: Unknown picked node.");
}

//_____________________________________________________________________________________
QByteArray VP1MissingEtSystem::saveState()
{
  ensureBuildController();
  VP1Serialise serialise(1/*version*/, this);
  serialise.save(IVP13DSystemSimple::saveState());
  serialise.save((VP1CollectionWidget*)m_d->collWidget);
  serialise.save(m_d->controller->saveSettings());
  serialise.warnUnsaved(controllerWidget());
  return serialise.result();
}

//_____________________________________________________________________________________
void VP1MissingEtSystem::restoreFromState(QByteArray ba)
{
  VP1Deserialise state(ba,this);
  if (state.version() < 0 || state.version() > 1) {
    message("Warning: State data in .vp1 file is in wrong format - ignoring!");
    return;
  }
  ensureBuildController();
  IVP13DSystemSimple::restoreFromState(state.restoreByteArray());
  state.restore((VP1CollectionWidget*)m_d->collWidget);

  if (state.version() >= 1)
   m_d->controller->restoreSettings(state.restoreByteArray());
  state.warnUnrestored(controllerWidget());
}
