from G4AtlasApps.SimFlags import simFlags
simFlags.OptionalUserActionList.addAction('G4UA::VerboseSelectorTool')
simFlags.UserActionConfig.addConfig('G4UA::VerboseSelectorTool','TargetEvent',1)
simFlags.UserActionConfig.addConfig('G4UA::VerboseSelectorTool','VerboseLevel',1)
simFlags.UserActionConfig.addConfig('G4UA::VerboseSelectorTool','TargetPdgIDs',
                                    [
                                        -1000011,1000011, #~e(L)
                                        -2000011,2000011, #~e(R)
                                        -1000013,1000013, #~mu(L)
                                        -2000013,2000013, #~mu(R)
                                        -1000015,1000015, #~tau(L)
                                        -2000015,2000015, #~tau(R)
                                        1000039 # ~G
                                    ])

