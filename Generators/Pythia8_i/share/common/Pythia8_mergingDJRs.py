
if "UserHooks" in genSeq.Pythia8.__slots__.keys():  
    genSeq.Pythia8.UserHooks += ['mergingDJRs']
else:
    genSeq.Pythia8.UserHook = 'mergingDJRs'


