#ifndef GSM_EFR_WRAPPER_H
#define GSM_EFR_WRAPPER_H

#include <iostream>

class GsmEfrWrapper {
 public:
  GsmEfrWrapper(): samples_per_frame_(160){};
  void Simulate(const char * pcm_in, int in_samples, char * pcm_out);
  ~GsmEfrWrapper() {};
 private:
  void Encode(const char * pcm_in, int in_samples, short * efr_enc);
  void Decode(const short *efr_dec, int num_frames, char * pcm_out);
  void EncodeToDecode(const short *efr_enc, short *efr_dec);
  const int samples_per_frame_; // how many samples in a frame defined by AMR_NB codec
  // const int bytes_per_frame_; // how many bytes for an AMR_NB encoded frame
  // const int chars_per_frame_;
  int num_frames_;
};

#endif