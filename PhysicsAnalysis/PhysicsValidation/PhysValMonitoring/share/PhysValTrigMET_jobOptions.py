from TrigMETMonitoring.TrigMETMonitoringConfig import HLTMETMonitoringTool
for tool1 in HLTMETMonitoringTool():
    tool1.EnableLumi = False
    tool1.OutputLevel = WARNING
    #tool1.DetailLevel = 10
    
    tool1.EnableLumi = False
    monMan.AthenaMonTools += [tool1]
