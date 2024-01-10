// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimHoughTransformTool.cxx
 * @author Riley Xu - riley.xu@cern.ch
 * @date October 31st, 2020
 * @brief See header file.
 */

#include "FPGATrackSimObjects/FPGATrackSimTypes.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimConfTools/IFPGATrackSimEventSelectionSvc.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimConstants.h"
#include "FPGATrackSimMaps/IFPGATrackSimMappingSvc.h"
#include "FPGATrackSimMaps/FPGATrackSimPlaneMap.h"
#include "FPGATrackSimMaps/FPGATrackSimRegionMap.h"
#include "FPGATrackSimBanks/IFPGATrackSimBankSvc.h"
#include "FPGATrackSimBanks/FPGATrackSimSectorBank.h"
#include "FPGATrackSimHoughTransformTool.h"

#include <sstream>
#include <cmath>
#include <algorithm>

static inline int quant(double min, double max, unsigned nSteps, double val);
static inline double unquant(double min, double max, unsigned nSteps, int step);
template <typename T>
static inline std::string to_string(const std::vector<T> &v);

///////////////////////////////////////////////////////////////////////////////
// AthAlgTool

FPGATrackSimHoughTransformTool::FPGATrackSimHoughTransformTool(const std::string& algname, const std::string &name, const IInterface *ifc) :
  base_class(algname, name, ifc)
{
  declareInterface<IFPGATrackSimRoadFinderTool>(this);
}


StatusCode FPGATrackSimHoughTransformTool::initialize()
{

  // Move temp variables over from properties to struct
  m_parMin.phi = m_tempMin_phi;
  m_parMin.qOverPt = m_tempMin_qOverPt;
  m_parMin.d0 = m_tempMin_d0;
  m_parMax.phi = m_tempMax_phi;
  m_parMax.qOverPt = m_tempMax_qOverPt;
  m_parMax.d0 = m_tempMax_d0;


  // Debug
  ATH_MSG_INFO("Image size: " << m_imageSize_x << " x " << m_imageSize_y);
  ATH_MSG_INFO("Convolution size: " << m_convSize_x << " x " << m_convSize_y);
  ATH_MSG_INFO("Convolution: " << to_string(const_cast<std::vector<int>&>(m_conv.value())));
  ATH_MSG_INFO("Hit Extend: " << to_string(const_cast<std::vector<unsigned>&>(m_hitExtend_x.value())));

  // Retrieve info
  ATH_CHECK(m_FPGATrackSimBankSvc.retrieve());
  ATH_CHECK(m_FPGATrackSimMapping.retrieve());
  m_nLayers = m_FPGATrackSimMapping->PlaneMap_1st()->getNLogiLayers();

  // Error checking
  // TODO check bounds are set correctly
  bool ok = false;
  if (!m_imageSize_x || !m_imageSize_y)
    ATH_MSG_FATAL("initialize() Image size must be greater than 0");
  else if (m_conv.size() != m_convSize_x * m_convSize_y)
    ATH_MSG_FATAL("initialize() Convolution sizes don't match");
  else if (!m_conv.empty() && (m_convSize_x % 2 == 0 || m_convSize_y % 2 == 0))
    ATH_MSG_FATAL("initialize() Convolution sizes must be odd");
  else if (m_hitExtend_x.size() % m_nLayers)
    ATH_MSG_FATAL("initialize() Hit extentsion list must have size % nLayers");
  else if (!m_combineLayers.empty() && m_combineLayers.size() != m_nLayers)
    ATH_MSG_FATAL("initialize() Combine layers list must have size = nLayers");
  else if (m_threshold.size() % 2 != 1)
    ATH_MSG_FATAL("initialize() Threshold size must be odd");
  else if (!m_binScale.empty() && m_binScale.size() != m_nLayers)
    ATH_MSG_FATAL("initialize() Bin scale list must have size = nLayers");
  else if (std::any_of(m_binScale.begin(), m_binScale.end(), [&](unsigned i){ return m_imageSize_y % i != 0; }))
    ATH_MSG_FATAL("initialize() The imagesize is not divisible by scale");
  else
    ok = true;
  if (!ok) return StatusCode::FAILURE;

  // Warnings / corrections
  if (m_localMaxWindowSize && !m_traceHits)
    {
      ATH_MSG_WARNING("initialize() localMaxWindowSize requires tracing hits, turning on automatically");
      m_traceHits = true;
    }
  if (m_idealGeoRoads)
    {
      if (m_useSectors)
        {
	  ATH_MSG_WARNING("initialize() idealGeoRoads conflicts with useSectors, switching off FPGATrackSim sector matching");
	  m_useSectors = false;
        }
      if (!m_traceHits)
        {
	  ATH_MSG_WARNING("initialize() idealGeoRoads requires tracing hits, turning on automatically");
	  m_traceHits = true;
        }
    }
  if (m_binScale.empty()) m_binScale.value().resize(m_nLayers, 1);
		 
  // Fill convenience variables
  m_step_x = (m_parMax[m_par_x] - m_parMin[m_par_x]) / m_imageSize_x;
  m_step_y = (m_parMax[m_par_y] - m_parMin[m_par_y]) / m_imageSize_y;
  for (unsigned i = 0; i <= m_imageSize_x; i++)
    m_bins_x.push_back(unquant(m_parMin[m_par_x], m_parMax[m_par_x], m_imageSize_x, i));
  for (unsigned i = 0; i <= m_imageSize_y; i++)
    m_bins_y.push_back(unquant(m_parMin[m_par_y], m_parMax[m_par_y], m_imageSize_y, i));

  // Initialize combine layers
  if (!m_combineLayers.empty())
    {
      m_nCombineLayers = *std::max_element(m_combineLayers.begin(), m_combineLayers.end()) + 1;
      m_combineLayer2D.resize(m_nCombineLayers);
      for (unsigned i = 0; i < m_combineLayers.size(); i++)
	      m_combineLayer2D[m_combineLayers[i]].push_back(i);
    }
  else
    {
      m_nCombineLayers = m_nLayers;
      for (unsigned i = 0; i < m_nLayers; i++)
	      m_combineLayer2D.push_back({ i });
    }
    
  return StatusCode::SUCCESS;
}



///////////////////////////////////////////////////////////////////////////////
// Main Algorithm

StatusCode FPGATrackSimHoughTransformTool::getRoads(const std::vector<const FPGATrackSimHit*> & hits, std::vector<FPGATrackSimRoad*> & roads) 
{
  roads.clear();
  m_roads.clear();

  m_image = createImage(hits);
  if (!m_conv.empty()) m_image = convolute(m_image);

  for (unsigned y = 0; y < m_imageSize_y; y++)
    for (unsigned x = 0; x < m_imageSize_x; x++)
      if (passThreshold(m_image, x, y)) {
	      if (m_traceHits)
	        addRoad(m_image(y, x).second, x, y);
	      else
	        addRoad(hits, x, y);
	    }
    
  roads.reserve(m_roads.size());
  for (FPGATrackSimRoad_Hough & r : m_roads) roads.push_back(&r);
    
  return StatusCode::SUCCESS;
}

FPGATrackSimHoughTransformTool::Image FPGATrackSimHoughTransformTool::createLayerImage(std::vector<unsigned> const & layers, std::vector<FPGATrackSimHit const *> const & hits, unsigned const scale) const
{
  Image image(m_imageSize_y, m_imageSize_x);

  for (FPGATrackSimHit const * hit : hits)
    {
      if (std::find(layers.begin(), layers.end(), hit->getLayer()) == layers.end()) continue;
      if (m_subRegion >= 0 && !m_FPGATrackSimMapping->SubRegionMap()->isInRegion(m_subRegion, *hit)) continue;

      // This scans over y (pT) because that is more efficient in memory, in C.
      // Unknown if firmware will want to scan over x instead.
      unsigned new_size_y  = m_imageSize_y / scale;
      for (unsigned y_ = 0; y_ < new_size_y; y_++)
        {
	  unsigned y_bin_min = scale * y_;
	  unsigned y_bin_max = scale * (y_ + 1);

	  // Find the min/max x bins
	  auto xBins = yToXBins(y_bin_min, y_bin_max, hit);

	  // Update the image
	  for (unsigned y = y_bin_min; y < y_bin_max; y++)
	    for (unsigned x = xBins.first; x < xBins.second; x++)
	      {
		image(y, x).first++;
		if (m_traceHits) image(y, x).second.insert(hit);
	      }
        }
    }

  return image;
}

FPGATrackSimHoughTransformTool::Image FPGATrackSimHoughTransformTool::createImage(std::vector<FPGATrackSimHit const *> const & hits) const
{
  Image image(m_imageSize_y, m_imageSize_x);

  for (unsigned i = 0; i < m_nCombineLayers; i++)
    {
      Image layerImage = createLayerImage(m_combineLayer2D[i], hits, m_binScale[i]);
      for (unsigned x = 0; x < m_imageSize_x; ++x)
	for (unsigned y = 0; y < m_imageSize_y; ++y)
	  if (layerImage(y, x).first > 0)
	    {
	      image(y, x).first++;
	      image(y, x).second.insert(layerImage(y, x).second.begin(), layerImage(y, x).second.end());
	    }
    }

  return image;
}

FPGATrackSimHoughTransformTool::Image FPGATrackSimHoughTransformTool::convolute(Image const & image) const
{
  Image out(m_imageSize_y, m_imageSize_x);

  for (unsigned y0 = 0; y0 < m_imageSize_y; y0++)     // Loop over out
    for (unsigned x0 = 0; x0 < m_imageSize_x; x0++) 
      for (unsigned r = 0; r < m_convSize_y; r++)     // Loop over conv
      	for (unsigned c = 0; c < m_convSize_x; c++) {
	        int y = -static_cast<int>(m_convSize_y) / 2 + r + y0; // Indices of input
	        int x = -static_cast<int>(m_convSize_x) / 2 + c + x0; //
	  
      	  if (y >= 0 && y < static_cast<int>(m_imageSize_y) && x >= 0 && x < static_cast<int>(m_imageSize_x)) {
	          int val = m_conv[r * m_convSize_x + c] * image(y, x).first;
	          if (val > 0) {
	            out(y0, x0).first += val;
	            out(y0, x0).second.insert(image(y, x).second.begin(), image(y, x).second.end());
	          }
	        }
	      }  
  return out;
}

bool FPGATrackSimHoughTransformTool::passThreshold(Image const & image, unsigned x, unsigned y) const
{
  // Pass window threshold
  unsigned width = m_threshold.size() / 2;
  if (x < width || (image.size(1) - x) < width) return false;
  for (unsigned i = 0; i < m_threshold.size(); i++) {
    if (image(y, x - width + i).first < m_threshold[i]) return false;
  }
  
  // Pass local-maximum check
  if (m_localMaxWindowSize) {
    for (int j = -m_localMaxWindowSize; j <= m_localMaxWindowSize; j++) {
      for (int i = -m_localMaxWindowSize; i <= m_localMaxWindowSize; i++) {
      	if (i == 0 && j == 0) continue;
	      if (y + j < image.size(0) && x + i < image.size(1)) {
	        if (image(y+j, x+i).first > image(y, x).first) return false;
	        if (image(y+j, x+i).first == image(y, x).first) {
	          if (image(y+j, x+i).second.size() > image(y, x).second.size()) return false;
	          if (image(y+j, x+i).second.size() == image(y, x).second.size() && j <= 0 && i <= 0) return false; // favor bottom-left (low phi, low neg q/pt)
	        }
	      }
      }
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////////
// Helpers


// Quantizes val, given a range [min, max) split into nSteps. Returns the bin below.
static inline int quant(double min, double max, unsigned nSteps, double val)
{
  return static_cast<int>((val - min) / (max - min) * nSteps);
}

// Returns the lower bound of the bin specified by step
static inline double unquant(double min, double max, unsigned nSteps, int step)
{
  return min + (max - min) * step / nSteps;
}

template <typename T>
static inline std::string to_string(const std::vector<T> &v)
{
  std::ostringstream oss;
  oss << "[";
  if (!v.empty())
    {
      std::copy(v.begin(), v.end()-1, std::ostream_iterator<T>(oss, ", "));
      oss << v.back();
    }
  oss << "]";
  return oss.str();
}


double FPGATrackSimHoughTransformTool::fieldCorrection(unsigned region, double qpt, double r)
{
  r = r / 1000; // convert to meters
  if (region == 3)
    {
      double cor = 0.1216 * r * r - 0.0533 * r + 0.0069;
      return -cor * qpt;
    }
  else if (region == 4)
    {
      double cor = 0.4265 * r * r - 0.0662 * r + 0.0036;
      return -cor * qpt;
    }
  else return 0;
}

double FPGATrackSimHoughTransformTool::yToX(double y, FPGATrackSimHit const * hit) const
{
  double x = 0;

  if (m_par_x == FPGATrackSimTrackPars::IPHI && m_par_y == FPGATrackSimTrackPars::IHIP)
    {
      double r = hit->getR(); // mm
      double phi_hit = hit->getGPhi(); // radians
      double d0 = std::isnan(m_parMin.d0) ? 0 : m_parMin.d0; // mm, assume min = max
      x = asin(r * htt::A * y - d0 / r) + phi_hit;

      if (m_fieldCorrection) x += fieldCorrection(m_EvtSel->getRegionID(), y, r);
    }
  else
    {
      ATH_MSG_ERROR("yToX() not defined for the current m_par selection");
    }

  return x;
}

// Find the min/max x bins of the hit's line, in each y bin. Max is exclusive.
// Note this assumes yToX is monotonic. Returns {0, 0} if hit lies out of bounds.
std::pair<unsigned, unsigned> FPGATrackSimHoughTransformTool::yToXBins(size_t yBin_min, size_t yBin_max, FPGATrackSimHit const * hit) const
{
  // Get float values
  double x_min = yToX(m_bins_y[yBin_min], hit);
  double x_max = yToX(m_bins_y[yBin_max], hit);
  if (x_min > x_max) std::swap(x_min, x_max);
  if (x_max < m_parMin[m_par_x] || x_min > m_parMax[m_par_x])
    return { 0, 0 }; // out of bounds

  // Get bins
  int x_bin_min = quant(m_parMin[m_par_x], m_parMax[m_par_x], m_imageSize_x, x_min);
  int x_bin_max = quant(m_parMin[m_par_x], m_parMax[m_par_x], m_imageSize_x, x_max) + 1; // exclusive

  // Extend bins
  unsigned extend = getExtension(yBin_min, hit->getLayer());
  x_bin_min -= extend;
  x_bin_max += extend;

  // Clamp bins
  if (x_bin_min < 0) x_bin_min = 0;
  if (x_bin_max > static_cast<int>(m_imageSize_x)) x_bin_max = m_imageSize_x;

  return { x_bin_min, x_bin_max };
}

// We allow variable extension based on the size of m_hitExtend_x. See comments below.
unsigned FPGATrackSimHoughTransformTool::getExtension(unsigned y, unsigned layer) const
{
  if (m_hitExtend_x.size() == m_nLayers) return m_hitExtend_x[layer];
  if (m_hitExtend_x.size() == m_nLayers * 2)
    {
      // different extension for low pt vs high pt, split in half but irrespective of sign
      // first nLayers entries of m_hitExtend_x is for low pt half, rest are for high pt half
      if (y < m_imageSize_y / 4 || y > 3 * m_imageSize_y / 4) return m_hitExtend_x[layer];
      return m_hitExtend_x[m_nLayers + layer];
    }
  return 0;
}

void FPGATrackSimHoughTransformTool::matchIdealGeoSector(FPGATrackSimRoad_Hough & r) const
{
  float pt = r.getY()*0.001; // convert to MeV
  auto bounds = std::equal_range(htt::QOVERPT_BINS.begin(),htt::QOVERPT_BINS.end(),pt);
  int sectorbin = bounds.first-htt::QOVERPT_BINS.begin()-1;

  // those bins are for tracks between the values, can't be below first value or more than the last value
  if (sectorbin < 0) sectorbin = 0;
  if (sectorbin > static_cast<int>(htt::QOVERPT_BINS.size()-2)) sectorbin =  htt::QOVERPT_BINS.size()-2;
  std::vector<module_t> modules;

  for (unsigned int il = 0; il < r.getNLayers(); il++) {
    if (r.getNHits_layer()[il] == 0) {
      modules.push_back(-1);
      layer_bitmask_t wc_layers = r.getWCLayers();
      wc_layers |= (0x1 << il);
      r.setWCLayers(wc_layers);

      FPGATrackSimHit *wcHit = new FPGATrackSimHit();
      wcHit->setHitType(HitType::wildcard);
      wcHit->setLayer(il);
      wcHit->setDetType(m_FPGATrackSimMapping->PlaneMap_1st()->getDetType(il));
      std::vector<const FPGATrackSimHit*> wcHits;
      wcHits.push_back(wcHit);
      r.setHits(il,wcHits);
    }
    else
      modules.push_back(sectorbin);
  }
  const FPGATrackSimSectorBank* sectorbank = m_FPGATrackSimBankSvc->SectorBank_1st();
  r.setSector(sectorbank->findSector(modules));
}

// Creates a road from hits that pass through the given bin (x, y), and pushes it onto m_roads
void FPGATrackSimHoughTransformTool::addRoad(std::vector<std::vector<const FPGATrackSimHit*>> const & hits, layer_bitmask_t hitLayers, unsigned x, unsigned y)
{
  m_roads.emplace_back();
  FPGATrackSimRoad_Hough & r = m_roads.back();

  r.setRoadID(m_roads.size() - 1);
  r.setPID(y * m_imageSize_y + x);
  r.setHits(hits);
  if (m_useSectors) r.setSector(m_FPGATrackSimBankSvc->SectorBank_1st()->findSector(hits));
  else if (m_idealGeoRoads) matchIdealGeoSector(r);
  r.setHitLayers(hitLayers);
  r.setSubRegion(m_subRegion);
  r.setX(m_bins_x[x] + m_step_x/2);
  r.setY(m_bins_y[y] + m_step_y/2);
  r.setXBin(x);
  r.setYBin(y);
}


// Creates a road from hits that pass through the given bin (x, y), and pushes it onto m_roads
void FPGATrackSimHoughTransformTool::addRoad(std::unordered_set<const FPGATrackSimHit*> const & hits, unsigned x, unsigned y)
{
  layer_bitmask_t hitLayers = 0;
  for (FPGATrackSimHit const * hit : hits)
    hitLayers |= 1 << hit->getLayer();

  auto sorted_hits = ::sortByLayer(hits);
  sorted_hits.resize(m_nLayers); // If no hits in last layer, return from sortByLayer will be too short

  addRoad(sorted_hits, hitLayers, x, y);
}

// Use this version of addRoad when hit tracing is turned off
void FPGATrackSimHoughTransformTool::addRoad(std::vector<const FPGATrackSimHit*> const & hits, unsigned x, unsigned y)
{
  // Get the road hits
  std::vector<FPGATrackSimHit const *> road_hits;
  layer_bitmask_t hitLayers = 0;
  for (const FPGATrackSimHit * hit : hits)
    {
      if (m_subRegion >= 0 && !m_FPGATrackSimMapping->SubRegionMap()->isInRegion(m_subRegion, *hit)) continue;

      // Find the min/max y bins (after scaling)
      unsigned int y_bin_min = (y / m_binScale[hit->getLayer()]) * m_binScale[hit->getLayer()];
      unsigned int y_bin_max = y_bin_min + m_binScale[hit->getLayer()];

      // Find the min/max x bins
      auto xBins = yToXBins(y_bin_min, y_bin_max, hit);
      if (x >= xBins.first && x < xBins.second)
        {
	  road_hits.push_back(hit);
	  hitLayers |= 1 << hit->getLayer();
        }
    }

  auto sorted_hits = ::sortByLayer(road_hits);
  sorted_hits.resize(m_nLayers); // If no hits in last layer, return from sortByLayer will be too short

  addRoad(sorted_hits, hitLayers, x, y);
}
