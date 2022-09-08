from G4AtlasApps.SimFlags import simFlags
simFlags.OptionalUserActionList.addAction('G4UA::VerboseSelectorTool')
simFlags.UserActionConfig.addConfig('G4UA::VerboseSelectorTool','TargetEvent',1)
simFlags.UserActionConfig.addConfig('G4UA::VerboseSelectorTool','VerboseLevel',1)
simFlags.UserActionConfig.addConfig('G4UA::VerboseSelectorTool','TargetPdgIDs',
                                    [
                                        1000022, # ~chi(0,1)
                                        -1000024,1000024 # ~chi(+,1)
                                    ])

