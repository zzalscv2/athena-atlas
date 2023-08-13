/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class VP1EtaPhiCutWidget                //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: July 2008                                //
//
//  Updates: 
//  - 2022 Nov, Riccardo Maria BIANCHI - added numerical phi cuts
//  - 2023 Mar, Riccardo Maria BIANCHI - set dynamic widget size
//
////////////////////////////////////////////////////////////////

#include "VP1Base/VP1EtaPhiCutWidget.h"
#include "VP1Base/VP1Serialise.h"
#include "VP1Base/VP1Deserialise.h"

#include "ui_vp1etaphicutform.h"

#include <cmath>

//____________________________________________________________________
class VP1EtaPhiCutWidget::Imp {

public:

  Ui::VP1EtaPhiCutForm ui;
  VP1Interval last_allowedEta;
  QList<VP1Interval> last_allowedPhi;

  void adaptSpinBoxRangesForSymmetry(bool symmetric);
};


//____________________________________________________________________
VP1EtaPhiCutWidget::VP1EtaPhiCutWidget(QWidget * parent,IVP1System * sys)
  : QWidget(parent), VP1HelperClassBase(sys,"VP1EtaPhiCutWidget"), m_d(new Imp)
{
  m_d->ui.setupUi(this);

  m_d->last_allowedEta = allowedEta();
  m_d->last_allowedPhi = allowedPhi();

  // -> allowedEta (a bit special due to the "force symmetric" ability)
  connect(m_d->ui.checkBox_cut_etarange,SIGNAL(toggled(bool)),this,SLOT(possibleChange_allowedEta()));
  connect(m_d->ui.checkBox_cut_etarange_forcesymmetric,SIGNAL(toggled(bool)),this,SLOT(handleEtaCutSymmetry()));
  connect(m_d->ui.doubleSpinBox_cut_etalower,SIGNAL(valueChanged(double)),this,SLOT(handleEtaCutSymmetry()));
  connect(m_d->ui.doubleSpinBox_cut_etaupper,SIGNAL(valueChanged(double)),this,SLOT(handleEtaCutSymmetry()));

  // -> allowedPhi
  connect(m_d->ui.checkBox_cut_phi,SIGNAL(toggled(bool)),this,SLOT(possibleChange_allowedPhi()));
  connect(m_d->ui.phisectionwidget,SIGNAL(enabledPhiRangesChanged(const QList<VP1Interval>&)),
	  this,SLOT(possibleChange_allowedPhi())); 

  connect(m_d->ui.checkBox_cut_phi,SIGNAL(toggled(bool)),this,SLOT(togglePhiCheckboxes()));
  connect(m_d->ui.checkBox_phiCuts,SIGNAL(toggled(bool)),this,SLOT(togglePhiCheckboxes()));
  
  connect(m_d->ui.checkBox_phiCuts,SIGNAL(toggled(bool)),this,SLOT(possibleChange_allowedPhi()));
  connect(m_d->ui.dsb_phiCuts_min,SIGNAL(valueChanged(double)),this,SLOT(possibleChange_allowedPhi()));
  connect(m_d->ui.dsb_phiCuts_max,SIGNAL(valueChanged(double)),this,SLOT(possibleChange_allowedPhi()));

}

//____________________________________________________________________
VP1EtaPhiCutWidget::~VP1EtaPhiCutWidget()
{
  delete m_d;
}

//____________________________________________________________________
VP1Interval VP1EtaPhiCutWidget::allowedEta() const
{
  if(VP1Msg::verbose()){
	 messageVerbose("VP1EtaPhiCutWidget::allowedEta()");
  }

  // if "eta cut" is not set: we return an interval ]-inf,inf[, so all objects will pass the internal eta cut
  if (!m_d->ui.checkBox_cut_etarange->isChecked()) {
    return VP1Interval(-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity());
  }

  const double min = m_d->ui.doubleSpinBox_cut_etalower->value();
  const double max = m_d->ui.doubleSpinBox_cut_etaupper->value();

  // if max < min: we return what??
  if (max<min)
    return VP1Interval();

  // FIXME: checkBox_etacut_excludeRange is not actually used now, check and fix!
  // if "Exclude Eta range" is selected, we set the "excludeInterval" flag in the instance of the VP1Interval class
  if (m_d->ui.checkBox_etacut_excludeRange) {
    if(VP1Msg::verbose()){
	    messageVerbose("excludeRange=true");
    }
	  return VP1Interval(min, max, true, true, true);
  }
  return VP1Interval( min, max );//fixme: closed interval?? Ckeck!
}

//____________________________________________________________________
QList<VP1Interval> VP1EtaPhiCutWidget::allowedPhi() const
{
  QList<VP1Interval> l;
  
  if (!m_d->ui.checkBox_cut_phi && !m_d->ui.checkBox_phiCuts)
    return l;


  if ( !m_d->ui.checkBox_cut_phi->isChecked() && !m_d->ui.checkBox_phiCuts->isChecked() ) {
    l << VP1Interval(-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity());
    return l;
  }

  if (  m_d->ui.checkBox_cut_phi->isChecked() && ( !m_d->ui.phisectionwidget || m_d->ui.phisectionwidget->allSectorsOff() ) )
    return l;

  if ( m_d->ui.checkBox_cut_phi->isChecked() && m_d->ui.phisectionwidget->allSectorsOn() ) {
    l << VP1Interval(-std::numeric_limits<double>::infinity(),std::numeric_limits<double>::infinity());
    return l;
  } else if ( m_d->ui.checkBox_cut_phi->isChecked() ) { 
    return m_d->ui.phisectionwidget->enabledPhiRanges();
  }
  if ( m_d->ui.checkBox_phiCuts->isChecked() ) {
      double phi_min = m_d->ui.dsb_phiCuts_min->value();
      double phi_max = m_d->ui.dsb_phiCuts_max->value();
    return m_d->ui.phisectionwidget->enabledPhiRanges(phi_min, phi_max);
  }

  return l; // we should not get here

}

//____________________________________________________________________
void VP1EtaPhiCutWidget::Imp::adaptSpinBoxRangesForSymmetry(bool symmetric)
{
  if (symmetric) {
    ui.doubleSpinBox_cut_etalower->setMaximum(0.0);
    ui.doubleSpinBox_cut_etaupper->setMinimum(0.0);
  } else {
    const double rangemax = ui.doubleSpinBox_cut_etaupper->maximum();
    ui.doubleSpinBox_cut_etalower->setMaximum(rangemax);
    ui.doubleSpinBox_cut_etaupper->setMinimum(-rangemax);
  }
}

//____________________________________________________________________
void VP1EtaPhiCutWidget::handleEtaCutSymmetry()
{
  if (!m_d->ui.checkBox_cut_etarange_forcesymmetric)
    return;
  if (sender()==m_d->ui.checkBox_cut_etarange_forcesymmetric) {
    //update allowed ranges, change values if necessary.
    m_d->adaptSpinBoxRangesForSymmetry(m_d->ui.checkBox_cut_etarange_forcesymmetric->isChecked());
    if (m_d->ui.checkBox_cut_etarange_forcesymmetric->isChecked()) {
      //Enforce symmetry:
      const double eta = std::max(fabs(m_d->ui.doubleSpinBox_cut_etalower->value()),fabs(m_d->ui.doubleSpinBox_cut_etaupper->value()));
      m_d->ui.doubleSpinBox_cut_etalower->setValue(-eta);
      m_d->ui.doubleSpinBox_cut_etaupper->setValue(eta);
    }
  } else if (m_d->ui.checkBox_cut_etarange_forcesymmetric->isChecked()) {
    //Update other value:
    if (sender()==m_d->ui.doubleSpinBox_cut_etalower) {
      m_d->ui.doubleSpinBox_cut_etaupper->setValue(-m_d->ui.doubleSpinBox_cut_etalower->value());
    } else if (sender()==m_d->ui.doubleSpinBox_cut_etaupper) {
      m_d->ui.doubleSpinBox_cut_etalower->setValue(-m_d->ui.doubleSpinBox_cut_etaupper->value());
    }
  }
  possibleChange_allowedEta();
}

//____________________________________________________________________
void VP1EtaPhiCutWidget::possibleChange_allowedEta()
{
  VP1Interval newAllowedEta = allowedEta();
  if ( m_d->last_allowedEta == newAllowedEta )
    return;
  m_d->last_allowedEta = newAllowedEta;
  if(VP1Msg::verbose()){
    messageVerbose("Emitting allowedEtaChanged("+newAllowedEta.toString()+")");
  }
  emit allowedEtaChanged(newAllowedEta);
}


void VP1EtaPhiCutWidget::togglePhiCheckboxes() {

  // If 'range' is checked, then 'cuts' must be unchecked; and viceversa
  // We only use one option at a time: or we use the dots/ranges, or the numerical values
    if (sender()==m_d->ui.checkBox_phiCuts) {
        if ( m_d->ui.checkBox_phiCuts->isChecked() ) m_d->ui.checkBox_cut_phi->setChecked(false);
    } else if (sender()==m_d->ui.checkBox_cut_phi) {
        if ( m_d->ui.checkBox_cut_phi->isChecked() ) m_d->ui.checkBox_phiCuts->setChecked(false);
    }
}

//____________________________________________________________________
void VP1EtaPhiCutWidget::possibleChange_allowedPhi()
{


  QList<VP1Interval> newAllowedPhi = allowedPhi();
  if ( m_d->last_allowedPhi == newAllowedPhi )
    return;
  m_d->last_allowedPhi = newAllowedPhi;
  if(VP1Msg::verbose()){
    QString s;
    for(int i=0;i<newAllowedPhi.count();++i)
      s+= newAllowedPhi.at(i).toString()+(i==newAllowedPhi.count()-1?"":", ");
    messageVerbose("Emitting allowedPhiChanged("+s+")");
  }
  emit allowedPhiChanged(newAllowedPhi);
}

//____________________________________________________________________
void VP1EtaPhiCutWidget::setEtaCutEnabled(bool b)
{
  if (b==m_d->ui.checkBox_cut_etarange->isChecked())
    return;
  m_d->ui.checkBox_cut_etarange->setChecked(b);
  possibleChange_allowedEta();
}

//____________________________________________________________________
void VP1EtaPhiCutWidget::setEtaCut(const double& e)
{
  setEtaCut(-fabs(e),fabs(e));
}


//____________________________________________________________________
void VP1EtaPhiCutWidget::showEtaCut(bool b)
{
	m_d->ui.checkBox_cut_etarange->setChecked(b);
	m_d->ui.widget_etacut->setVisible(b);
}
//____________________________________________________________________
void VP1EtaPhiCutWidget::showPhiCut(bool b)
{
	m_d->ui.checkBox_cut_phi->setChecked(b);
	m_d->ui.widget_phicut->setVisible(b);
}

//____________________________________________________________________
void VP1EtaPhiCutWidget::setEtaCut(const double& a,const double&b)
{
  double e1(a),e2(b);
  const double rangemax = m_d->ui.doubleSpinBox_cut_etaupper->maximum();
  e1 = std::max(-rangemax,e1);
  e1 = std::min(rangemax,e1);
  e2 = std::max(-rangemax,e2);
  e2 = std::min(rangemax,e2);
  if (e1>=e2||e1!=e1||e2!=e2) {
    e1 = 0;
    e2 = 0;
  }

  bool save = blockSignals(true);
  bool save1 = m_d->ui.doubleSpinBox_cut_etalower->blockSignals(true);
  bool save2 = m_d->ui.doubleSpinBox_cut_etaupper->blockSignals(true);
  bool save3 = m_d->ui.checkBox_cut_etarange->blockSignals(true);
  bool save4 = m_d->ui.checkBox_cut_etarange_forcesymmetric->blockSignals(true);

  m_d->ui.checkBox_cut_etarange->setChecked(true);
  m_d->ui.checkBox_cut_etarange_forcesymmetric->setChecked(e1==-e2);
  m_d->adaptSpinBoxRangesForSymmetry(e1==-e2);
  m_d->ui.doubleSpinBox_cut_etalower->setValue(e1);
  m_d->ui.doubleSpinBox_cut_etaupper->setValue(e2);

  blockSignals(save);
  m_d->ui.doubleSpinBox_cut_etalower->blockSignals(save1);
  m_d->ui.doubleSpinBox_cut_etaupper->blockSignals(save2);
  m_d->ui.checkBox_cut_etarange->blockSignals(save3);
  m_d->ui.checkBox_cut_etarange_forcesymmetric->blockSignals(save4);
  possibleChange_allowedEta();
}

//____________________________________________________________________
QByteArray VP1EtaPhiCutWidget::saveState() const
{
  //NB: We can't use the VP1Serialise::save(VP1EtaPhiCutWidget*) here
  //(that would give infinite recursion).

  VP1Serialise serialise(1/*version*/,systemBase());
  serialise.save(m_d->ui.checkBox_cut_etarange);
  serialise.save(m_d->ui.checkBox_cut_etarange_forcesymmetric);
  serialise.save(m_d->ui.doubleSpinBox_cut_etalower);
  serialise.save(m_d->ui.doubleSpinBox_cut_etaupper);
  serialise.save(m_d->ui.checkBox_cut_phi);
  serialise.save(m_d->ui.phisectionwidget);//Version 0 saved the old-style phisection widget states.
  serialise.widgetHandled(this);
  serialise.warnUnsaved(this);
  return serialise.result();
}

//____________________________________________________________________
void VP1EtaPhiCutWidget::restoreFromState( const QByteArray& ba)
{
  //NB: We can't use the VP1Deserialise::restore(VP1EtaPhiCutWidget*) here
  //(that would give infinite recursion).

  VP1Deserialise state(ba,systemBase());
  if (state.version()<0||state.version()>1)
    return;//Ignore silently

  state.restore(m_d->ui.checkBox_cut_etarange);
  state.restore(m_d->ui.checkBox_cut_etarange_forcesymmetric);
  state.restore(m_d->ui.doubleSpinBox_cut_etalower);
  state.restore(m_d->ui.doubleSpinBox_cut_etaupper);
  state.restore(m_d->ui.checkBox_cut_phi);
  if (state.version()==0) {
    state.ignoreObsoletePhiSectionWidgetState();
    state.ignoreWidget(m_d->ui.phisectionwidget);
  } else {
    state.restore(m_d->ui.phisectionwidget);
  }
  state.widgetHandled(this);
  state.warnUnrestored(this);

  possibleChange_allowedEta();
  possibleChange_allowedPhi();
}
