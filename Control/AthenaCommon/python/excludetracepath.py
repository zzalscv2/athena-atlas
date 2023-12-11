# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

# Used for the --tracelevel option of ThinCAWrapper.sh
# to exclude system and ROOT libraries from tracing 

import sys

xp=[x for x in sys.path if x.find("ROOT")!=-1]
xp.append(sys.prefix)

print (":".join(xp))
