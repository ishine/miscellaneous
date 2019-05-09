# miscellaneous

A C++ implementation of some telecommunication channel simulators for data augmentation. 
Most source files of codecs are borrowed from the 3GPP, and I just add a c++ wrapper.
Currently supported codecs are:
- GSM-EFR
- G729
- AMR-NB

Note these simulators use kaldi IO interface to support Linux cmd pipeline.

A C++ implementation of multi-channel Generalized Weighted Prediction Error(GWPE) for speech dereverberation is also provided.
We use Eigen3 library as the basic math lib.
See also [nara_wpe](https://github.com/fgnt/nara_wpe).
