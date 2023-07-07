#!/bin/sh
# art-type: build
# art-include: main/Athena
# art-ci: master

athena --threads=10 --concurrent-events=2 ByteStreamCnvSvcBase/testROBDataProviderSvcMT.py


