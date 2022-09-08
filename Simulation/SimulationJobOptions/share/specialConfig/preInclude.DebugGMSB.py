from G4AtlasApps.SimFlags import simFlags
simFlags.OptionalUserActionList.addAction('G4UA::VerboseSelectorTool')
simFlags.UserActionConfig.addConfig('G4UA::VerboseSelectorTool','TargetEvent',1)
simFlags.UserActionConfig.addConfig('G4UA::VerboseSelectorTool','VerboseLevel',1)
simFlags.UserActionConfig.addConfig('G4UA::VerboseSelectorTool','TargetPdgIDs',
                                    [
                                        -1000001,1000001, # ~d(L)
                                        -1000002,1000002, # ~u(L)
                                        -1000003,1000003, # ~s(L)
                                        -1000004,1000004, # ~c(L)
                                        -1000005,1000005, # ~b(1)
                                        -1000006,1000006, # ~t(1)
                                        -1000011,1000011, # ~e(L)
                                        -1000013,1000013, # ~mu(L)'
                                        -1000015,1000015, # ~tau(L)
                                        -2000001,2000001, # ~d(R)
                                        -2000002,2000002, # ~u(R)
                                        -2000003,2000003, # ~s(R)
                                        -2000004,2000004, # ~c(R)
                                        -2000005,2000005, # ~b(2)
                                        -2000006,2000006, # ~t(2)
                                        -2000011,2000011, # ~e(R)
                                        -2000013,2000013, # ~mu(R)'
                                        -2000015,2000015, # ~tau(R)
                                        1000021, # ~g
                                        1000022, # ~chi(0,1)
                                        1000023, # ~chi(0,2)
                                        -1000024,1000024, # ~chi(+,1)
                                        1000025, # ~chi(0,3)
                                        1000035, # ~chi(0,4)
                                        -1000037,1000037, # ~chi(+,2)
                                        1000039 # ~G
                                    ])

