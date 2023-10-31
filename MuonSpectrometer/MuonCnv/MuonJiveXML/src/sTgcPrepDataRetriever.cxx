/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "sTgcPrepDataRetriever.h"

#include "MuonFullIDHelper.h"
#include "MuonReadoutGeometry/sTgcReadoutElement.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonReadoutGeometry/MuonChannelDesign.h"
#include "MuonReadoutGeometry/MuonPadDesign.h"
#include "CxxUtils/ArrayHelper.h"
#include "GeoPrimitives/GeoPrimitives.h"


namespace JiveXML {

  //--------------------------------------------------------------------------

  sTgcPrepDataRetriever::sTgcPrepDataRetriever(const std::string& type,const std::string& name, const IInterface* parent):
    AthAlgTool(type, name, parent)
  {

    declareInterface<IDataRetriever>(this);

  }
 
  //--------------------------------------------------------------------------

  StatusCode sTgcPrepDataRetriever::initialize(){
    
    ATH_MSG_DEBUG("Initializing retriever for " << dataTypeName());
    ATH_CHECK(m_sgKey.initialize());
    ATH_CHECK( m_idHelperSvc.retrieve() );

    return StatusCode::SUCCESS;
  }

  //--------------------------------------------------------------------------

  StatusCode sTgcPrepDataRetriever::retrieve(ToolHandle<IFormatTool> &FormatTool) {

    //be verbose
    ATH_MSG_DEBUG("Retrieving " << dataTypeName());

    SG::ReadHandle<Muon::sTgcPrepDataContainer> stgcContainer(m_sgKey);

    int ndata = 0;
    for (const auto stgcCollection : *stgcContainer){
       ndata += stgcCollection->size();
    }
    //Make the vectors to contain the information and reserve space accordingly
    DataVect x; x.reserve(ndata);
    DataVect y; y.reserve(ndata);
    DataVect z; z.reserve(ndata);
    DataVect lengthVec; lengthVec.reserve(ndata);
    DataVect swidthVec; swidthVec.reserve(ndata);
    DataVect lwidthVec; lwidthVec.reserve(ndata);
    DataVect idVec; idVec.reserve(ndata);
    DataVect identifierVec; identifierVec.reserve(ndata);
    DataVect barcode; barcode.reserve(ndata);

    for (const auto stgcCollection : *stgcContainer){
        for (const auto data : *stgcCollection){

        const MuonGM::sTgcReadoutElement *element = data->detectorElement();
        Identifier id = data->identify();

        if (!element) {
          ATH_MSG_WARNING("No MuonGM::sTgcReadoutElement for hit " << id);
          continue;
        }

        Amg::Vector3D globalPos{Amg::Vector3D::Zero()}; 
        Amg::Vector2D pos{Amg::Vector2D::Zero()}; 
        double shortWidth=0, longWidth=0, length=0;
        int channel = m_idHelperSvc->stgcIdHelper().channel(id);
        const Trk::PlaneSurface surface = element->surface(id);
        element->stripGlobalPosition(id, globalPos);
        element->stripPosition(id, pos);

        if (m_idHelperSvc->stgcIdHelper().channelType(id)==0) { // pad
           length = element->channelPitch(id); // Height of a pad
           std::array<Amg::Vector2D, 4> corners{make_array<Amg::Vector2D, 4>(Amg::Vector2D::Zero())};
           element->padCorners(id, corners); // BotLeft, BotRight, TopLeft, TopRight
           shortWidth = (corners.at(1)-corners.at(0)).norm();
           longWidth = (corners.at(3)-corners.at(2)).norm();

        } else if (m_idHelperSvc->stgcIdHelper().channelType(id)==1) { // strip
           const MuonGM::MuonChannelDesign* design = element->getDesign(id);
           length = design->channelLength(channel);
           shortWidth = element->channelPitch(id); // Full pitch of strips
           longWidth = shortWidth;

        } else { // wire
           const MuonGM::MuonChannelDesign* design = element->getDesign(id);
           if(!design) {
              ATH_MSG_WARNING("No wire design for hit " << id);
              continue;
           }

           // recalculate length and globalPos for wires, because design->channelLength(channel) doesn't look sensible
           double fulllength = design->xSize();
           double locY = design->firstPos() + (channel-1) * design->inputPitch * design->groupWidth;
           if (std::abs(locY) > 0.5 * design->minYSize()) { // triangle region
              double dY = 0.5 * (design->maxYSize() - design->minYSize());
              length = (0.5 * design->maxYSize()  - std::abs(locY)) / dY * fulllength;
              if (std::abs(locY) > 873) { // trapezoid region in the outter most part of the large sector
                 length += 0.5*fulllength;
              }
              element->surface(id).localToGlobal(Amg::Vector2D(pos.x(), pos.y()+0.5*(fulllength-length)), Amg::Vector3D::Zero(), globalPos);
           }
           else { // rectangular region
              length = fulllength;
           }

           shortWidth = element->channelPitch(id); // Width of a full wire group
           longWidth = shortWidth;
        }

        x.push_back(DataType(globalPos.x()/CLHEP::cm));
        y.push_back(DataType(globalPos.y()/CLHEP::cm));
        z.push_back(DataType(globalPos.z()/CLHEP::cm));
        swidthVec.push_back(DataType(shortWidth/CLHEP::cm));
        lwidthVec.push_back(DataType(longWidth/CLHEP::cm));
        lengthVec.push_back(DataType(length/CLHEP::cm));
        identifierVec.push_back(DataType(MuonFullIDHelper::getFullID(id, m_idHelperSvc->stgcIdHelper())));
        idVec.push_back(DataType( id.get_compact() ));
        barcode.push_back(DataType(0));

    }
  }      

    DataMap myDataMap;
    myDataMap["x"] = x;
    myDataMap["y"] = y;
    myDataMap["z"] = z;
    myDataMap["length"] = lengthVec;
    myDataMap["swidth"] = swidthVec;
    myDataMap["lwidth"] = lwidthVec;
    myDataMap["identifier"] = identifierVec;
    myDataMap["id"] = idVec;
    myDataMap["barcode"] = barcode;

    //Be verbose
    ATH_MSG_DEBUG(dataTypeName() << ": "<< x.size());

    //forward data to formating tool
    // Atlantis can't deal with SGkey in xml output in CSCD (freezes)
    // So not output SGKey for now. jpt 20Aug09
    std::string emptyStr="";
    return FormatTool->AddToEvent(dataTypeName(), m_sgKey.key(), &myDataMap);  
  }
}
