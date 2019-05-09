#include "amr-nb-wrapper.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include "c-code/typedef.h"
#include "c-code/interf_enc.h"
#include "c-code/interf_dec.h"

void AmrNbWrapper::Encode(const char *pcm_in, int in_samples, unsigned char * amr_nb) {
  /* input speech vector */
  short speech[160];
  /* counters */
  int byte_counter, frames = 0, bytes = 0;
  /* pointer to encoder state structure */
  unsigned char serial_data[32];
  void * enstate = Encoder_Interface_init(0);
  /* read file */
  const char *p_input = pcm_in;
  unsigned char *p_amr_nb = amr_nb;
  // std::cout << "mode_=" << mode_ << "\n";
  for (int iframe = 0; iframe < num_frames_; iframe++) {
    std::memcpy(speech, p_input, sizeof(short) * samples_per_frames_);
    p_input += sizeof(short) * samples_per_frames_;
    frames ++;
    /* call encoder */
    byte_counter = Encoder_Interface_Encode(enstate, mode_, speech, serial_data, 0);
    bytes += byte_counter;
    std::memcpy(p_amr_nb, serial_data, sizeof(UWord8) * byte_counter);
    p_amr_nb += byte_counter;
  }
  Encoder_Interface_exit(enstate);
}

void AmrNbWrapper::Decode(const unsigned char * amr_nb, int num_frames, char * pcm_out) {
  short synth[160];
  int frames = 0;
  int read_size;
  unsigned char analysis[32];
  int dec_mode;
  short block_size[16] = { 12, 13, 15, 17, 19, 20, 26, 31, 5, 0, 0, 0, 0, 0, 0, 0 };

  void *destate = Decoder_Interface_init();
  /* find mode, read file */
  const unsigned char * p_encoded = amr_nb;
  short * p_pcmout = (short *)pcm_out;
  for (int iframe = 0; iframe < num_frames; iframe++) {
    analysis[0] = p_encoded[0];
    dec_mode = ((analysis[0] >> 3) & 0x000F);
    read_size = block_size[dec_mode];
    std::memcpy(&analysis[1], &p_encoded[1], sizeof(unsigned char ) * read_size);
    p_encoded += read_size + 1;
    frames ++;
    /* call decoder */
    Decoder_Interface_Decode(destate, analysis, synth, 0);
    std::memcpy(p_pcmout, synth, sizeof(short) * samples_per_frames_);
    p_pcmout += samples_per_frames_;
  }
  Decoder_Interface_exit(destate);
}

void AmrNbWrapper::Simulate(const char * pcm_in, int in_samples, char * pcm_out) {
  num_frames_ = in_samples / samples_per_frames_;
  unsigned char * encoded_amr_nb = new unsigned char [num_frames_ * bytes_per_frames_];
  Encode(pcm_in, in_samples, encoded_amr_nb);
  Decode(encoded_amr_nb, num_frames_, pcm_out);
  delete []encoded_amr_nb;
};