/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "AFP_Calibration/AFP_PixelHistoFiller.h"


AFP_PixelHistoFiller::AFP_PixelHistoFiller(const std::string& name, ISvcLocator* pSvcLocator) :
  AthAlgorithm(name, pSvcLocator)
{
}


StatusCode AFP_PixelHistoFiller::initialize()
{
	ATH_CHECK( m_eventInfoKey.initialize() );
	ATH_CHECK( m_afpHitContainerKey.initialize() );
	
	m_nPixelsX=AFP_CONSTANTS::SiT_Pixel_amount_x;
	m_nPixelsY=AFP_CONSTANTS::SiT_Pixel_amount_y;
	
	for(int st=0;st<m_nStations;++st)
	{
		for(int la=0;la<m_nLayers;++la)
		{
			m_pixelHits[st][la].clear();
		}
	}
	
	return StatusCode::SUCCESS;
}


StatusCode AFP_PixelHistoFiller::execute()
{
	const EventContext& ctx = Gaudi::Hive::currentContext();
	
	// get event info
	SG::ReadHandle<xAOD::EventInfo> eventInfo (m_eventInfoKey, ctx);
	if (!eventInfo.isValid())
	{
		ATH_MSG_WARNING("cannot get eventInfo");
		return StatusCode::SUCCESS;
	}
	else ATH_MSG_DEBUG("successfully got eventInfo");
	
	// make sure all the histograms are defined
	int current_lb=eventInfo->lumiBlock();
		
	if(static_cast<int>(m_pixelHits[0][0].size())-1<current_lb/m_LBRangeLength)
	{
		for(int st=0;st<m_nStations;++st)
		{
			for(int la=0;la<m_nLayers;++la)
			{
				for(int add=static_cast<int>(m_pixelHits[st][la].size());add<=current_lb/m_LBRangeLength;++add)
				{
					TH2F hist(Form("pixel_hits_lb_%d_%d_station_%d_layer_%d", add*m_LBRangeLength,(add+1)*m_LBRangeLength-1,st,la), 
					          Form("pixel hits, lb %d-%d, station %d, layer %d", add*m_LBRangeLength,(add+1)*m_LBRangeLength-1,st,la),
					          m_nPixelsX,0.5,m_nPixelsX+0.5, m_nPixelsY,0.5,m_nPixelsY+1);
					
					m_pixelHits[st][la].push_back(hist);
				}
			}
		}
	}
	
	int lb_index=current_lb/m_LBRangeLength;
	
	SG::ReadHandle<xAOD::AFPSiHitContainer> afpHitContainer (m_afpHitContainerKey, ctx);
	if (!afpHitContainer.isValid())
	{
		ATH_MSG_WARNING("cannot get AFPSiHitContainer");
		return StatusCode::SUCCESS;
	}
	else ATH_MSG_DEBUG("successfully got AFPSiHitContainer");
	
  	for (const xAOD::AFPSiHit* hit : *afpHitContainer)
  	{
		int st=hit->stationID();
		int la=hit->pixelLayerID();
		
		if(st<0 || m_nStations<=st)
		{
			ATH_MSG_INFO("stationID = " <<st<<", but expected values are from 0 to "<<m_nStations-1 );
			continue;
		}
		if(la<0 || m_nLayers<=la)
		{
			ATH_MSG_INFO("pixelLayerID = " <<la<<", but expected values are from 0 to "<<m_nLayers-1 );
			continue;
		}
		
		m_pixelHits[st][la].at(lb_index).Fill(hit->pixelRowIDChip(),hit->pixelColIDChip());
	}
	
	return StatusCode::SUCCESS;
}


StatusCode AFP_PixelHistoFiller::finalize()
{	
	std::unique_ptr<TFile> output_file(new TFile("AFP_PixelHistoFiller.root","recreate"));
	
	TH1I lb("LBRangeLength","LBRangeLength",2,0,2);
	lb.Fill(0.5);
	lb.Fill(1.5,m_LBRangeLength);
	lb.Write();
	
	for(int st=0;st<m_nStations;++st)
	{
		for(int la=0;la<m_nLayers;++la)
		{
			for(TH2F& hist: m_pixelHits[st][la])
			{
				hist.Write();
			}
		}
	}
	
	output_file->Close();
	
	return StatusCode::SUCCESS;
}

