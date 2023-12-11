#                                                           
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

# x-y labels for pad occupancy plots

labelColumns = []

for i in range(1, 9):
    labelColumns.append(f'S{i:02d}')
    for _ in range(9):
        labelColumns.append('')

labelRows = []

for i in range(1, 4):
    labelRows.append(f'Q{i}')
    for _ in range(20):
        labelRows.append('')
