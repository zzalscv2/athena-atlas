/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
*  MM_ElectronicsResponseSimulation.cxx
*  MC for micromegas athena integration
*
**/
#include "MM_Digitization/MM_ElectronicsResponseSimulation.h"

#include <numeric>

#include "GaudiKernel/MsgStream.h"
#include "AthenaKernel/getMessageSvc.h"
#include "TF1.h"


/*******************************************************************************/
void MM_ElectronicsResponseSimulation::initialize()
{
  // let the VMM peak search run in a wider window in order to simulate the dead time and to avoid a bug at the upper limit of the time window
  m_vmmShaper = std::make_unique<VMM_Shaper>(m_peakTime, m_timeWindowLowerOffset - m_vmmDeadtime, m_timeWindowUpperOffset+m_vmmUpperGrazeWindow); 
  m_vmmShaper->initialize();
}
/*******************************************************************************/
void MM_ElectronicsResponseSimulation::clearValues()
{
  m_tStripElectronicsAbThr.clear();
  m_qStripElectronics.clear();
  m_nStripElectronics.clear();
}
/*******************************************************************************/
MM_DigitToolOutput MM_ElectronicsResponseSimulation::getPeakResponseFrom(const MM_ElectronicsToolInput & digiInput)
{
  clearValues();
  
  vmmPeakResponseFunction(digiInput.NumberOfStripsPos(), digiInput.chipCharge(), digiInput.chipTime(), digiInput.stripThreshold() );
  
  /// ToDo: include loop for calculating Trigger study vars
  // MM_DigitToolOutput(bool hitWasEff, std::vector <int> strpos, std::vector<float> time, std::vector<int> charge, std::vector<float> threshold, int strTrig, float strTimeTrig ):
  MM_DigitToolOutput tmp(true, m_nStripElectronics, m_tStripElectronicsAbThr, m_qStripElectronics, 5, 0.3);
  
  return tmp;
}
/*******************************************************************************/
MM_DigitToolOutput MM_ElectronicsResponseSimulation::getThresholdResponseFrom(const MM_ElectronicsToolInput & digiInput)
{
  clearValues();
  vmmThresholdResponseFunction(digiInput.NumberOfStripsPos(), digiInput.chipCharge(), digiInput.chipTime(), digiInput.stripThreshold());
  MM_DigitToolOutput tmp(true, m_nStripElectronics, m_tStripElectronicsAbThr, m_qStripElectronics, 5, 0.3);
  return tmp;
}
/*******************************************************************************/
void MM_ElectronicsResponseSimulation::vmmPeakResponseFunction(const std::vector <int> & numberofStrip, const std::vector<std::vector <float>> & qStrip, const std::vector<std::vector <float>> & tStrip, const std::vector<float>& stripsElectronicsThreshold){
  for (unsigned int ii = 0; ii < numberofStrip.size(); ii++) {

    // find min and max times for each strip:
    bool thisStripFired = false;
    double leftStripFired = false;
    double rightStripFired = false;

    // find the maximum charge:
    if (m_useNeighborLogic) {  // only check neighbor strips if VMM neighbor logic is enabled
      if ( ii > 0 ) {
       leftStripFired =  m_vmmShaper->hasChargeAboveThreshold(qStrip.at(ii-1), tStrip.at(ii-1), stripsElectronicsThreshold.at(ii-1));
      }

      if ( ii+1 < numberofStrip.size() ) {
       rightStripFired =  m_vmmShaper->hasChargeAboveThreshold(qStrip.at(ii+1), tStrip.at(ii+1), stripsElectronicsThreshold.at(ii+1) );
      }
    }

    thisStripFired = m_vmmShaper->hasChargeAboveThreshold(qStrip.at(ii), tStrip.at(ii), stripsElectronicsThreshold.at(ii));

    // check if neighbor strip was above threshold
    bool neighborFired = leftStripFired || rightStripFired;

    // Look at strip if it or its neighbor was above threshold  and if neighbor logic of the VMM is enabled:
    if (thisStripFired || (m_useNeighborLogic && neighborFired)) {
      double charge = FLT_MAX;
	  double time = FLT_MAX;

      // if strip is below threshold but read through NL reduce threshold to low value of 1e to still find the peak
      float tmpScaledThreshold = (thisStripFired ? stripsElectronicsThreshold.at(ii) : 1);

      bool foundPeak = m_vmmShaper->vmmPeakResponse(qStrip.at(ii), tStrip.at(ii), tmpScaledThreshold, charge, time);
      if(!foundPeak) continue; // if no peak was found within the enlarged time window the strip is skipped and no digit is created. 
	  if( time < m_timeWindowLowerOffset || time > m_timeWindowUpperOffset ) continue; // only accept strips in the correct time window

      m_nStripElectronics.push_back(numberofStrip.at(ii));
      m_tStripElectronicsAbThr.push_back(time);
      m_qStripElectronics.push_back(charge);
    }
  }
}


void MM_ElectronicsResponseSimulation::vmmThresholdResponseFunction(const std::vector <int> & numberofStrip, const std::vector<std::vector <float>> & qStrip, const std::vector<std::vector <float>> & tStrip, const std::vector<float> &electronicsThreshold){
  
  for (unsigned int ii = 0; ii < numberofStrip.size(); ii++) {
    double localThresholdt = FLT_MAX;
    double localThresholdq = FLT_MAX;

    bool crossedThreshold = m_vmmShaper->vmmThresholdResponse(qStrip.at(ii), tStrip.at(ii), electronicsThreshold.at(ii), localThresholdq, localThresholdt);
    if(!crossedThreshold) continue; // if no threshold crossing was found within the time window the strip is skipped and no digits are created.
	if( localThresholdt < m_timeWindowLowerOffset ||  localThresholdt > m_timeWindowUpperOffset ) continue; // only accept strips in the correct time window

    
    m_nStripElectronics.push_back(numberofStrip.at(ii));
    m_tStripElectronicsAbThr.push_back(localThresholdt);
    m_qStripElectronics.push_back(localThresholdq);
    
  }
}

MM_ElectronicsToolTriggerOutput MM_ElectronicsResponseSimulation::getTheFastestSignalInVMM(
	const MM_DigitToolOutput & ElectronicThresholdOutput,
	const int chMax,
	const int stationEta){

	const std::vector<int>  & electronicsThresholdStripPos    = ElectronicThresholdOutput.stripPos();
	const std::vector<float> & electronicsThresholdStripTime   = ElectronicThresholdOutput.stripTime();
	const std::vector<float> & electronicsThresholdStripCharge = ElectronicThresholdOutput.stripCharge();
	std::vector<int> trigger_MMFE_VMM_id;
	std::vector<int> trigger_VMM_id;
	getVMMId(electronicsThresholdStripPos, chMax, stationEta, trigger_VMM_id, trigger_MMFE_VMM_id);

	std::vector<int>   electronicsTriggerStripPos;
	std::vector<float> electronicsTriggerStripTime;
	std::vector<float> electronicsTriggerStripCharge;
	std::vector<int>   electronicsTriggerVMMid;
	std::vector<int>   electronicsTriggerMMFEid;
	

	const int BXtime = 250; // (ns)
	int nstep = (int)(m_timeWindowUpperOffset-m_timeWindowLowerOffset)/BXtime;
	if((int)(m_timeWindowUpperOffset-m_timeWindowLowerOffset)%BXtime>0 ) nstep += 1;

	for(int istep = 0; istep<nstep ; istep++){
		float timeWindowLower = (float) m_timeWindowLowerOffset+istep*BXtime;
		float timeWindowUpper = (float) m_timeWindowLowerOffset+(istep+1)*BXtime;

		std::vector<int> tmp_VMM_id; tmp_VMM_id.clear();
		for(size_t i = 0; i<electronicsThresholdStripPos.size(); i++){
			if(electronicsThresholdStripTime.at(i) >= timeWindowLower &&
				electronicsThresholdStripTime.at(i) < timeWindowUpper){

				int VMM_id = trigger_VMM_id.at(i);
				std::vector< int >::iterator VMM_flag = std::find(tmp_VMM_id.begin(), tmp_VMM_id.end(), VMM_id);
				if(VMM_flag != tmp_VMM_id.end()) continue; // Already filled
				tmp_VMM_id.push_back(VMM_id);

				// Get id for the fastest signal in a VMM
				int theFastestSignal = getIdTheFastestSignalInVMM(
					electronicsThresholdStripTime.at(i),
					VMM_id,
					trigger_VMM_id,
					electronicsThresholdStripTime,
					m_timeWindowLowerOffset+istep*BXtime,
					m_timeWindowLowerOffset+(istep+1)*BXtime);

				electronicsTriggerStripPos.push_back(    electronicsThresholdStripPos.at(theFastestSignal)    );
				electronicsTriggerStripTime.push_back(   electronicsThresholdStripTime.at(theFastestSignal)   );
				electronicsTriggerStripCharge.push_back( electronicsThresholdStripCharge.at(theFastestSignal) );
				electronicsTriggerVMMid.push_back(       trigger_VMM_id.at(theFastestSignal)                    );
				electronicsTriggerMMFEid.push_back(      trigger_MMFE_VMM_id.at(theFastestSignal)               );

			}
		}
	}

	MM_ElectronicsToolTriggerOutput ElectronicsTriggerOutput (
		electronicsTriggerStripPos,
		electronicsTriggerStripCharge,
		electronicsTriggerStripTime,
		electronicsTriggerVMMid,
		electronicsTriggerMMFEid );

	return ElectronicsTriggerOutput;
}

int MM_ElectronicsResponseSimulation::getIdTheFastestSignalInVMM(
	float time,
	int VMM_id,
	std::vector<int> trigger_VMM_id,
	const std::vector<float>& electronicsThresholdStripTime,
	float timeWindowLower,
	float timeWindowUpper){
	int theFastestSignal = -1;
	float min_time = 9999.0;
	for(size_t ii = 0; ii<trigger_VMM_id.size(); ii++){
		if(electronicsThresholdStripTime.at(ii)>= timeWindowLower &&
			electronicsThresholdStripTime.at(ii) < timeWindowUpper){

			if(trigger_VMM_id.at(ii)==VMM_id){
				if(time<min_time){
					theFastestSignal = ii;
					min_time=time;
				}
			}
		}
	}
    if(theFastestSignal==-1) {
        MsgStream log(Athena::getMessageSvc(),"MM_ElectronicsResponseSimulation");
        log << MSG::WARNING << "There is something wrong in getIdTheFastestSignalInVMM" << endmsg;
    }

	return theFastestSignal;
}

void MM_ElectronicsResponseSimulation::getVMMId(const std::vector< int > & electronicsThresholdStripPos,
	const int chMax,
	const int stationEta,
	std::vector< int > & trigger_VMM_id,
	std::vector< int > & trigger_MMFE_VMM_id){

	trigger_MMFE_VMM_id.clear();
	trigger_VMM_id.clear();
	const int VMMch = 64;
	electronics VmmMapping;
	for (size_t i=0;i<electronicsThresholdStripPos.size(); i++){
		int vmm_id = (int) electronicsThresholdStripPos.at(i)/VMMch;
		int mmfe_vmm_id = VmmMapping.elec(
			electronicsThresholdStripPos.at(i),
			"VMM",
			stationEta,
			chMax);
		trigger_MMFE_VMM_id.push_back(mmfe_vmm_id);
		trigger_VMM_id.push_back(vmm_id);
	}
}

MM_DigitToolOutput MM_ElectronicsResponseSimulation::applyDeadTimeStrip(const MM_DigitToolOutput & electronicsTriggerOutput){

	const std::vector<int>   & electronicsStripPos    = electronicsTriggerOutput.stripPos();
	const std::vector<float> & electronicsStripTime   = electronicsTriggerOutput.stripTime();
	const std::vector<float> & electronicsStripCharge = electronicsTriggerOutput.stripCharge();
	std::vector<int>   electronicsAppliedDeadtimeStripPos;
	std::vector<float> electronicsAppliedDeadtimeStripTime;
	std::vector<float> electronicsAppliedDeadtimeStripCharge;
	

	float deadTime = m_stripDeadTime;
	std::vector<int> v_id = electronicsStripPos;

	for(size_t i = 0; i<electronicsStripPos.size(); i++){
		int id = v_id.at(i);
		float time = electronicsStripTime.at(i);
		bool DEAD = deadChannel(id, time, electronicsAppliedDeadtimeStripPos, electronicsAppliedDeadtimeStripTime, deadTime);
		if(!DEAD){
			electronicsAppliedDeadtimeStripPos.push_back(electronicsStripPos.at(i));
			electronicsAppliedDeadtimeStripTime.push_back(electronicsStripTime.at(i));
			electronicsAppliedDeadtimeStripCharge.push_back(electronicsStripCharge.at(i));
		}
	}

	MM_DigitToolOutput ElectronicsTriggerOutputAppliedDeadTime(
		true,  // meaningless
		electronicsAppliedDeadtimeStripPos,
		electronicsAppliedDeadtimeStripTime,
		electronicsAppliedDeadtimeStripCharge,
		5,  // meaningless
		0.0  ); // meaningless

	return ElectronicsTriggerOutputAppliedDeadTime;
}


MM_ElectronicsToolTriggerOutput MM_ElectronicsResponseSimulation::applyARTTiming(const MM_ElectronicsToolTriggerOutput & electronicsTriggerOutput, float jitter, float offset){



	const std::vector<int>  & electronicsTriggerStripPos     = electronicsTriggerOutput.NumberOfStripsPos();
	const std::vector<float> & electronicsTriggerStripTime   = electronicsTriggerOutput.chipTime();
	const std::vector<float> & electronicsTriggerStripCharge = electronicsTriggerOutput.chipCharge();
	const std::vector<int> & electronicsTriggerVMMid         = electronicsTriggerOutput.VMMid();
	const std::vector<int> & electronicsTriggerMMFEid        = electronicsTriggerOutput.MMFEVMMid();
	std::vector<int>   electronicsTriggerAppliedTimingStripPos;
	std::vector<float> electronicsTriggerAppliedTimingStripTime;
	std::vector<float> electronicsTriggerAppliedTimingStripCharge;
	std::vector<int>   electronicsTriggerAppliedTimingVMMId;
	std::vector<int>   electronicsTriggerAppliedTimingMMFEId;
	
	//bunchTime = bunchTime + artOffset + jitter;

	TF1 gaussianSmearing("timingSmearing","gaus",offset-jitter*5,offset+jitter*5);
	gaussianSmearing.SetParameters(1,offset,jitter);

	for(size_t i = 0; i<electronicsTriggerStripPos.size(); i++){
		// std::default_random_engine generator;
		// std::normal_distribution<double> distribution(offset,jitter);
		// float timingTransformation = distribution(generator);

		electronicsTriggerAppliedTimingStripPos.push_back(electronicsTriggerStripPos.at(i));

		if(jitter || offset)    electronicsTriggerAppliedTimingStripTime.push_back(electronicsTriggerStripTime.at(i) + gaussianSmearing.GetRandom() );
		else electronicsTriggerAppliedTimingStripTime.push_back(electronicsTriggerStripTime.at(i) );

		electronicsTriggerAppliedTimingStripCharge.push_back(electronicsTriggerStripCharge.at(i));
		electronicsTriggerAppliedTimingVMMId.push_back(electronicsTriggerVMMid.at(i));
		electronicsTriggerAppliedTimingMMFEId.push_back(electronicsTriggerMMFEid.at(i));

	}

	MM_ElectronicsToolTriggerOutput electronicsTriggerOutputAppliedTiming(
		electronicsTriggerAppliedTimingStripPos,
		electronicsTriggerAppliedTimingStripCharge,
		electronicsTriggerAppliedTimingStripTime,
		electronicsTriggerAppliedTimingVMMId,
		electronicsTriggerAppliedTimingMMFEId );

	return electronicsTriggerOutputAppliedTiming;
}


MM_ElectronicsToolTriggerOutput MM_ElectronicsResponseSimulation::applyDeadTimeART(const MM_ElectronicsToolTriggerOutput & electronicsTriggerOutput){

	const std::vector<int>  & electronicsTriggerStripPos     = electronicsTriggerOutput.NumberOfStripsPos();
	const std::vector<float> & electronicsTriggerStripTime   = electronicsTriggerOutput.chipTime();
	const std::vector<float> & electronicsTriggerStripCharge = electronicsTriggerOutput.chipCharge();
	const std::vector<int> & electronicsTriggerVMMid         = electronicsTriggerOutput.VMMid();
	const std::vector<int> & electronicsTriggerMMFEid        = electronicsTriggerOutput.MMFEVMMid();
	std::vector<int>   electronicsTriggerAppliedDeadtimeStripPos;
	std::vector<float> electronicsTriggerAppliedDeadtimeStripTime;
	std::vector<float> electronicsTriggerAppliedDeadtimeStripCharge;
	std::vector<int>   electronicsTriggerAppliedDeadtimeVMMid;
	std::vector<int>   electronicsTriggerAppliedDeadtimeMMFEid;
	
	float deadtime = m_artDeadTime;
	std::vector<int> v_id = electronicsTriggerVMMid;

	for(size_t i = 0; i<electronicsTriggerStripPos.size(); i++){
		int id = v_id.at(i);
		float time = electronicsTriggerStripTime.at(i);
		bool DEAD = deadChannel(id,
			time,
			electronicsTriggerAppliedDeadtimeVMMid,
			electronicsTriggerAppliedDeadtimeStripTime,
			deadtime);

		if(!DEAD){
			electronicsTriggerAppliedDeadtimeStripPos.push_back(electronicsTriggerStripPos.at(i));
			electronicsTriggerAppliedDeadtimeStripTime.push_back(electronicsTriggerStripTime.at(i));
			electronicsTriggerAppliedDeadtimeStripCharge.push_back(electronicsTriggerStripCharge.at(i));
			electronicsTriggerAppliedDeadtimeVMMid.push_back(electronicsTriggerVMMid.at(i));
			electronicsTriggerAppliedDeadtimeMMFEid.push_back(electronicsTriggerMMFEid.at(i));
		}
	}

	MM_ElectronicsToolTriggerOutput electronicsTriggerOutputAppliedDeadTime(
		electronicsTriggerAppliedDeadtimeStripPos,
		electronicsTriggerAppliedDeadtimeStripCharge,
		electronicsTriggerAppliedDeadtimeStripTime,
		electronicsTriggerAppliedDeadtimeVMMid,
		electronicsTriggerAppliedDeadtimeMMFEid );

	return electronicsTriggerOutputAppliedDeadTime;
}
bool MM_ElectronicsResponseSimulation::deadChannel(int id, float time, std::vector<int> & v_id, const std::vector<float> & v_time, float deadtime){
	for(size_t ii = 0; ii<v_id.size(); ii++){
		if(id == v_id.at(ii)){
			if(v_time.at(ii)<time && time-v_time.at(ii)<deadtime){
				return true;				
			}
		}
	}
	return false;
}


