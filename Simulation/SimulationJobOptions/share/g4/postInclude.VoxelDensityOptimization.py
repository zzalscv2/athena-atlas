#########################################################
#
# Voxelization Optimization
# Mustafa Schmidt
#
# Optimized voxelization parameters for all subdetectors
# using the built-in VoxelDensityTool
#
#########################################################

ToolSvc.G4AtlasDetectorConstructionTool.GeometryConfigurationTools["VoxelDensityTool"].VolumeVoxellDensityLevel = {"Pixel": 0.1, "SCT": 0.25, "Tile": 0.1, "TRT": 0.1}