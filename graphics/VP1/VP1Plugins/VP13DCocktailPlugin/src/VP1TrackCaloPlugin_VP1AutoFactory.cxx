
/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
//                                                             //
//  Implementation of class VP1TrackCaloPlugin_VP1AutoFactory  //
//                                                             //
//  Author: Riccardo Maria BIANCHI <rbianchi@cern.ch>          //
//                                                             //
//  Update version: Dec 2017                                   //
//                                                             //
/////////////////////////////////////////////////////////////////


// Originally, this file was autogenerated by CMT with VP1 Factory Code Header File
// Now we add this file to plugins by hand, to simplify the CMake compilation

#include <QtPlugin>
#include "VP13DCocktailPlugin/VP1TrackCaloPlugin_VP1AutoFactory.h"
#include "VP13DCocktailPlugin/VP1TrackCaloChannel.h"

QStringList VP1TrackCaloPlugin_VP1AutoFactory::channelWidgetsProvided() const
{
  return QStringList()
        << "TrackCalo"
         ;
}

IVP1ChannelWidget * VP1TrackCaloPlugin_VP1AutoFactory::getChannelWidget(const QString & channelwidget)
{
  if (channelwidget == "TrackCalo")
    return new VP1TrackCaloChannel();

  return 0;
}

