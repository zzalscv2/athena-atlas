from G4AtlasApps.SimFlags import simFlags
simFlags.OptionalUserActionList.addAction('G4UA::VerboseSelectorTool',['Event','Tracking','Step'])
simFlags.UserActionConfig.addConfig('G4UA::VerboseSelectorTool','TargetEvent',1)
simFlags.UserActionConfig.addConfig('G4UA::VerboseSelectorTool','VerboseLevel',1)
simFlags.UserActionConfig.addConfig('G4UA::VerboseSelectorTool','TargetPdgIDs',
                                    [
                                        -4110000,4110000 #Monopoles
                                    ])
