#ifndef AMR_NB_WRAPPER_H
#define AMR_NB_WRAPPER_H

#include <iostream>
#include "c-code/sp_enc.h"

class AmrNbWrapper {
 public:
  AmrNbWrapper(int mode_int): samples_per_frames_(160), bytes_per_frames_(32) {
    mode_ = MR122;
    SetMode(mode_int);
  };
  void SetMode(int mode_int) {
    if (mode_int > -1 && mode_int < 8) {
      switch (mode_int) {
      case 0:
        mode_ = MR475; break;
      case 1:
        mode_ = MR515; break;
      case 2:
        mode_ = MR59; break;
      case 3:
        mode_ = MR67; break;
      case 4:
        mode_ = MR74; break;
      case 5:
        mode_ = MR795; break;
      case 6:
        mode_ = MR102; break;
      case 7:
        mode_ = MR122; break;
      default:
        mode_ = MR122;
        break;
      }
    }
  };
  void Simulate(const char * pcm_in, int in_samples, char * pcm_out);
  ~AmrNbWrapper() {};
 private:
  void Encode(const char * pcm_in, int in_samples, unsigned char* amr_nb);
  void Decode(const unsigned char *amr_nb, int num_frames, char * pcm_out);
  const int samples_per_frames_; // how many samples in a frame defined by AMR_NB codec
  const int bytes_per_frames_; // how many bytes for an AMR_NB encoded frame

  Mode mode_;
  int num_frames_;
};

#endif