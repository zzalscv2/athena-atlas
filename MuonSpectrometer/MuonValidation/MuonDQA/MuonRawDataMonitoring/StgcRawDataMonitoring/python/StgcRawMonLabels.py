#
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration #
#

LumiblockYlabel = []

for i in range(102) : 
    LumiblockYlabel.append('')
for side in ['A', 'C']:
    for sector in range(1, 51, 3):
        index = 51 + sector if side=='A' else 51 - sector
        if sector//3 != 0 :
            LumiblockYlabel[index-1] = '%s%02d' % (side, sector//3)
