# Set of utilities to facilitate data transfer operations that are EFTracking specific

# Benchmarks
The package contains also a set of benchmark programs, they exercise various operations related to the data transfer.
Current benchmark measures time needed to write and read sets of SpacePoint to or from SpacePointContainer.
There are currently 5 scenarios in 5 different sizes.

Sycl benchmark test measures time need to copy simple vector to another using oneAPI SYCL.
To enable oneAPI SYCL on lxplus machine you need to call following after asetup command:
- source /cvmfs/projects.cern.ch/intelsw/oneAPI/linux/24-all-setup.sh
- export SYCLCXX="`which icpx` -fsycl"

Otherwise tests will not build.

