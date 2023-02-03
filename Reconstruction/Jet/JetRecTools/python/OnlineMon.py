# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def getMonTool_Algorithm(flags, path):
    from AthenaMonitoringKernel.GenericMonitoringTool import GenericMonitoringTool
    monTool = GenericMonitoringTool(flags, 'MonTool',
                                    HistPath = path)

    monTool.defineHistogram( 'TIME_constitmod', path='EXPERT', type='TH1F', title='Counts',
                             xbins=100, xmin=0, xmax=250 )

    return monTool
