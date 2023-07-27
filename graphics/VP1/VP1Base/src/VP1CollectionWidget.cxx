/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class VP1CollectionWidget               //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: June 2008                                //
//                                                            //
////////////////////////////////////////////////////////////////

#include "VP1Base/VP1CollectionWidget.h"
#include "VP1Base/VP1StdCollection.h"
#include "VP1Base/VP1MaterialButton.h"
#include "VP1Base/VP1Msg.h"

#include <QSpacerItem>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSet>
#include <QList>
#include <QRegExp>

//____________________________________________________________________
class VP1CollectionWidget::Imp {
public:
  Imp (VP1CollectionWidget*tc) : theclass(tc), collWidget(0), appropriateFixedWidth(0) { }
  VP1CollectionWidget * theclass;
  QWidget * collWidget;
  int appropriateFixedWidth;
  QList<QWidget *> widgetsFromCollections;
  QList<VP1Collection*> collections;
  VP1CollStates states;
  QList<qint32> last_visibleStdCollectionTypes;
  QList<VP1StdCollection*> last_visibleStdCollections;
  QList<qint32> visibleStdCollectionTypesFromVisStdCols(const QList<VP1StdCollection*>& l) const;
  void repopulateGUIFromCollections();
  void clearGui(bool deleteGuiElements);

  void updateStatesWithCurrentInfo();

};

//____________________________________________________________________
VP1CollectionWidget::VP1CollectionWidget(QWidget * parent)
  : QWidget(parent), VP1HelperClassBase(0,"VP1CollectionWidget"), m_d(new Imp(this))
{
}

//____________________________________________________________________
VP1CollectionWidget::~VP1CollectionWidget()
{
  delete m_d;
}

//____________________________________________________________________
QList<VP1Collection*> VP1CollectionWidget::collections() const
{
  return m_d->collections;
}

//____________________________________________________________________
void VP1CollectionWidget::Imp::clearGui(bool deleteGuiElements)
{
  if (!deleteGuiElements) {
    for(QWidget *w : widgetsFromCollections) {
      w->hide();
      w->setParent(0);
    }
  }
  delete collWidget;
  collWidget = 0;

}

//____________________________________________________________________
void VP1CollectionWidget::clear(bool deleteCollections,bool deleteGuiElements)
{
  m_d->updateStatesWithCurrentInfo();
  m_d->clearGui(deleteGuiElements);
  m_d->widgetsFromCollections.clear();

  if (deleteCollections) {
    for(VP1Collection*col : m_d->collections)
      delete col;
  }
  m_d->collections.clear();

  //Fixme: update report-on-width
  m_d->appropriateFixedWidth = 0;

  possibleChange_visibleStdCollections();
  visibleContentsChanged();
}

//____________________________________________________________________
void VP1CollectionWidget::Imp::repopulateGUIFromCollections()
{
  theclass->setUpdatesEnabled(false);
  if (collWidget)
    clearGui(false);

  collWidget = new QWidget();//We only add the widget in the end (to avoid needless updates).

  QVBoxLayout * vlayout = new QVBoxLayout;
  vlayout->setSpacing(0);
  vlayout->setMargin(0);
  if (collections.isEmpty()) {
    QHBoxLayout * labellayout = new QHBoxLayout;
    labellayout->setSpacing(0);
    labellayout->addWidget(new QLabel("No collections in event."),0,Qt::AlignLeft);
    labellayout->addStretch(1);
    vlayout->addLayout(labellayout);
  }

  QSet<QString> sections;
  for (VP1Collection* col :collections)
    sections.insert(col->section());
  QList<QString> sectionsSorted = sections.values();
  std::sort(sectionsSorted.begin(), sectionsSorted.end());
  theclass->sortSections(sectionsSorted);

  const bool nosectionlabels = sectionsSorted.count() == 1 && sectionsSorted.at(0).isEmpty();

  int maxFirstColumnCheckBoxWidth(0);
  QList<QCheckBox*> firstColumnCheckBoxes;

  int gridmargins(0);
  for  (QString section : sectionsSorted) {
    //type section header:
    QLabel * sectionlabel(0);
    if (!nosectionlabels) {
      QHBoxLayout * labellayout = new QHBoxLayout;
      labellayout->setSpacing(0);
      sectionlabel =new QLabel;
      sectionlabel->setTextFormat(Qt::RichText);
      sectionlabel->setText("<b>"+section+"</b>");
      labellayout->addWidget(sectionlabel,0,Qt::AlignLeft);
      labellayout->addStretch(1);
      vlayout->addLayout(labellayout);
    }
    //Add individual widgets for collections:
    QGridLayout * gridLayout = new QGridLayout;
    gridLayout->setHorizontalSpacing(6);
    gridLayout->setVerticalSpacing(0);
    gridLayout->setMargin(0);
    if (!nosectionlabels)
      gridLayout->addItem(new QSpacerItem(10, 1, QSizePolicy::Fixed, QSizePolicy::Fixed),0,0);
    vlayout->addLayout(gridLayout);
    for (VP1Collection* col : collections) {
      if (col->section()!=section)
	continue;
      if (sectionlabel) {
	if (!col->sectionToolTip().isEmpty())
	  sectionlabel->setToolTip(col->sectionToolTip());
	sectionlabel = 0;
      }
      int newrow = gridLayout->rowCount();
      int i(1);
      for (QWidget*w : col->widgetsForGuiRow()) {
	if (!w) {
	  theclass->message("WARNING: Ignoring null widget provided by widgetsForGuiRow() (perhaps due to a VP1StdCollection you didn't init()?)");
	  continue;
	}
	//Smaller font:
	QFont f(w->font());
	//	f.setPointSizeF(f.pointSizeF()*0.9);
	w->setFont(f);
	int maxheight = static_cast<int>(0.5+QFontMetricsF(f).height()*1.05+2);
	w->setMaximumHeight(maxheight);
	VP1MaterialButton * mb = dynamic_cast<VP1MaterialButton *>(w);
	if (mb)
	  mb->setDimension(maxheight);
	if (i==1) {
	  QCheckBox * cb = dynamic_cast<QCheckBox *>(w);
	  if (cb) {
	    firstColumnCheckBoxes << cb;
	    if (maxFirstColumnCheckBoxWidth<cb->sizeHint().width())
	      maxFirstColumnCheckBoxWidth = cb->sizeHint().width();
	  }
	}
	widgetsFromCollections << w;
	w->setParent(collWidget);
	w->setVisible(true);
	gridLayout->addWidget( w, newrow, i++);
      }
      gridLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding),newrow,i);
    }
    if (gridmargins==0) {
      int left, top, right, bottom;
      gridLayout->getContentsMargins ( &left, &top, &right, &bottom );
      gridmargins = left+right;
    }
  }

  if (maxFirstColumnCheckBoxWidth>0) {
    for (QCheckBox *cb : firstColumnCheckBoxes)
      cb->setMinimumWidth(maxFirstColumnCheckBoxWidth);
  }

  vlayout->addStretch(1);

  QHBoxLayout * hlayout = new QHBoxLayout;
  hlayout->setMargin(0);
  hlayout->setSpacing(0);
  hlayout->addLayout(vlayout);
  hlayout->addStretch(1);
  collWidget->setLayout(hlayout);

  //Put collWidget into the collection widget:
  delete theclass->layout();
  QHBoxLayout * hlayoutPageCol = new QHBoxLayout;
  collWidget->setParent(theclass);
  hlayoutPageCol->addWidget(collWidget);
  theclass->setLayout(hlayoutPageCol);
  theclass->setContentsMargins(0,0,0,0);
  collWidget->setContentsMargins(0,0,0,0);
  hlayoutPageCol->setContentsMargins(0,0,0,0);
  //Fixme: reshuffle the above or turn off updates to avoid flicker.

  theclass->setContentsMargins(0,0,0,0);
  collWidget->setContentsMargins(0,0,0,0);
  appropriateFixedWidth = collWidget->sizeHint().width()//Fixme: before we add the collwidget.
    +gridmargins;

  theclass->setUpdatesEnabled(true);
  theclass->visibleContentsChanged();

}


//____________________________________________________________________
void VP1CollectionWidget::addCollections(QList<VP1Collection*> cols, bool applySavedStates )
{
  messageVerbose("addCollections called with "+str(cols.count())+" new collections");

  m_d->collections << cols;//FIXME: Test not already there.

  m_d->repopulateGUIFromCollections();

  if (applySavedStates)
    VP1Collection::applyStates(m_d->collections, m_d->states);

  possibleChange_visibleStdCollections();
  for (VP1Collection* col : cols) {
    VP1StdCollection* stdcol = dynamic_cast<VP1StdCollection*>(col);
    if (stdcol)
      connect(stdcol,SIGNAL(visibilityChanged(bool)),this,SLOT(possibleChange_visibleStdCollections()));
  }

}

//____________________________________________________________________
void VP1CollectionWidget::setCollections(QList<VP1Collection*> cols,bool applySavedStates)
{
  m_d->collections = cols;

  m_d->repopulateGUIFromCollections();

  if (applySavedStates)
    VP1Collection::applyStates(m_d->collections, m_d->states);

  possibleChange_visibleStdCollections();
  for (VP1Collection* col : cols) {
    VP1StdCollection* stdcol = dynamic_cast<VP1StdCollection*>(col);
    if (stdcol)
      connect(stdcol,SIGNAL(visibilityChanged(bool)),this,SLOT(possibleChange_visibleStdCollections()));
  }
}

//____________________________________________________________________
int VP1CollectionWidget::appropriateFixedWidth() const
{
  return m_d->appropriateFixedWidth;
}

//____________________________________________________________________
void VP1CollectionWidget::Imp::updateStatesWithCurrentInfo()
{
	VP1Msg::messageDebug("VP1CollectionWidget::Imp::updateStatesWithCurrentInfo()");
  VP1Collection::updateStates(states,VP1Collection::getStates(collections));
}

//____________________________________________________________________
void VP1CollectionWidget::addStateInfo(const VP1CollStates& newinfo, bool overwritesExisting )
{
  m_d->updateStatesWithCurrentInfo();
  QMapIterator<QByteArray,QByteArray> it(newinfo);
  while (it.hasNext()) {
    it.next();
    if (!overwritesExisting&&m_d->states.contains(it.key()))
      continue;
    m_d->states.insert(it.key(),it.value());
  }
  if (overwritesExisting)
    VP1Collection::applyStates(m_d->collections, newinfo);
}

//____________________________________________________________________
VP1CollStates VP1CollectionWidget::states() const
{
  m_d->updateStatesWithCurrentInfo();
  return m_d->states;
}


//____________________________________________________________________
QList<qint32> VP1CollectionWidget::Imp::visibleStdCollectionTypesFromVisStdCols(const QList<VP1StdCollection*>& l) const
{
  QSet<qint32> vt;
  for (VP1StdCollection* stdcol : l)
    vt.insert(stdcol->collTypeID());
  QList<qint32> vistypes = vt.values();
  std::sort(vistypes.begin(), vistypes.end());
  return vistypes;

}

//____________________________________________________________________
QList<qint32> VP1CollectionWidget::visibleStdCollectionTypes() const
{
  return m_d->visibleStdCollectionTypesFromVisStdCols(visibleStdCollections());
}

//____________________________________________________________________
QList<VP1StdCollection*> VP1CollectionWidget::visibleStdCollections() const
{
  QList<VP1StdCollection*> l;
  for(VP1StdCollection* stdcol : collections<VP1StdCollection>()) {
    if (stdcol->visible())
      l << stdcol;
  }
  return l;
}

//____________________________________________________________________
void VP1CollectionWidget::possibleChange_visibleStdCollections()
{
  QList<VP1StdCollection*> visstdcols = visibleStdCollections();
  QList<qint32> vistypes = m_d->visibleStdCollectionTypesFromVisStdCols(visstdcols);

  if ( m_d->last_visibleStdCollections != visstdcols ) {
    m_d->last_visibleStdCollections = visstdcols;
    visibleStdCollectionsChanged(visstdcols);
  }

  if ( m_d->last_visibleStdCollectionTypes != vistypes ) {
    m_d->last_visibleStdCollectionTypes = vistypes;
    visibleStdCollectionTypesChanged(vistypes);
  }

}



//____________________________________________________________________
void VP1CollectionWidget::ensureFirst(const QString& wildcard,QList<QString>& strs)
{
  QRegExp rx(wildcard,Qt::CaseInsensitive,QRegExp::Wildcard);
  QList<QString> l;
  for (QString str : strs)
    if (rx.exactMatch(str))
      l << str;
  for (QString str : l)
    strs.removeAll(str);
  for (QString str : strs)
    l << str;
  strs = l;

}

//____________________________________________________________________
void VP1CollectionWidget::ensureLast(const QString& wildcard,QList<QString>& strs)
{
  QRegExp rx(wildcard,Qt::CaseInsensitive,QRegExp::Wildcard);
  QList<QString> l;
  for(QString str : strs)
    if (rx.exactMatch(str))
      l << str;
  for (QString str : l)
    strs.removeAll(str);
  strs << l;
}
