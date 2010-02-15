#
# Config file for tracter to force 16K basic front-end
#
export ASRFactory_Frontend=Basic
export FileSource_FrameRate=16000
export FileSource_SampleFreq=16000
export MelFilter_HiHertz=8000
export MelFilter_MaxHertz=8000

# The latter of these two is for older compiles.  Frame_ is the new way.
export Frame_Period=160
export Periodogram_FramePeriod=160

# Again, these all do the same thing.  The last one is for older compiles.
export Basic_DeltaOrder=2
export BasicVAD_DeltaOrder=2
export ASRFactory_DeltaOrder=2
