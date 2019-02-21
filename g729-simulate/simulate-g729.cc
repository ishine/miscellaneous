/* ITU-T G.729 Software Package Release 2 (November 2006) */
/*
   ITU-T G.729A Speech Coder    ANSI-C Source Code
   Version 1.1    Last modified: September 1996

   Copyright (c) 1996,
   AT&T, France Telecom, NTT, Universite de Sherbrooke
   All rights reserved.
*/

/*-----------------------------------------------------------------*
 * Main program of the G.729a 8.0 kbit/s decoder.                  *
 *                                                                 *
 *    Usage : decoder  bitstream_file  synth_file                  *
 *                                                                 *
 *-----------------------------------------------------------------*/

// #include <stdlib.h>
// #include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include "typedef.h"
#include "codecParameters.h"
#include "utils.h"
#include "testUtils.h"
#include "encoder.h"
#include "decoder.h"
#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "feat/wave-reader.h"

// Word16 bad_lsf;        /* bad LSF indicator   */

/*
   This variable should be always set to zero unless transmission errors
   in LSP indices are detected.
   This variable is useful if the channel coding designer decides to
   perform error checking on these important parameters. If an error is
   detected on the  LSP indices, the corresponding flag is
   set to 1 signalling to the decoder to perform parameter substitution.
   (The flags should be set back to 0 for correct transmission).
*/


/*-----------------------------------------------------------------*
 *            Main decoder routine                                 *
 *-----------------------------------------------------------------*/

int main(int argc, char *argv[] ) {
  try {
    using namespace kaldi;
    const char *usage =
      "simulate G.729 narrow-band(8kHz) codec\n"
      "Usage: simulate-g729 [options] <wav-in-file> <wav-out-file>\n"
      " e.g.: simulate-g729 input.wav output.wav\n";
    ParseOptions po(usage);
    po.Read(argc, argv);
    if (po.NumArgs() != 2) {
      po.PrintUsage();
      exit(1);
    }

    unsigned char bit_data[11] = {0};
    short in_pcm[160] = {0};
    short out_pcm[160] = {0};

    // Word16  i, frame;

    // std::ifstream is;
    // std::ofstream os;

    std::string wav_rxfilename = po.GetArg(1), wav_wxfilename = po.GetArg(2);
    // is.open(wav_rxfilename, std::ios::in | std::ios::binary);
    // if (!is.is_open()) {
    //   return -1;
    // }
    // std::string file_out = wav_wxfilename + ".pcm";
    // os.open(file_out, std::ios::out | std::ios::binary);

    kaldi::Input wave_input(wav_rxfilename);
    kaldi::WaveData wave_data;
    wave_data.Read(wave_input.Stream());
    KALDI_ASSERT(wave_data.Data().NumRows() == 1);
    kaldi::SubVector<kaldi::BaseFloat> data(wave_data.Data(), 0);

    kaldi::Matrix<kaldi::BaseFloat> output_data(1, data.Dim());


    // g729a_encode_frame_state enc;
    // g729a_enc_init(&enc);
    bcg729EncoderChannelContextStruct *encoderChannelContext = initBcg729EncoderChannel();
    bcg729DecoderChannelContextStruct *decoderChannelContext = initBcg729DecoderChannel();
    // g729a_decode_frame_state dec;
    // g729a_dec_init(&dec);

    int num_frames = data.Dim() / L_FRAME;
    // KALDI_LOG << "num_frames=" << num_frames << ",fs=" << wave_data.SampFreq() << ",L_FRAME=" << L_FRAME << "\n";
    // while ( fread(in_pcm, sizeof(Word16), L_FRAME, f_in) == L_FRAME) {
    for (int iframe = 0; iframe < num_frames ; iframe++) {
      for (int isample = 0; isample < L_FRAME; isample++) {
        in_pcm[isample] = (short)data(iframe * L_FRAME + isample);
      }
      bcg729Encoder(encoderChannelContext, in_pcm, bit_data);
      bcg729Decoder(decoderChannelContext, bit_data, 0, out_pcm);
      // g729a_enc_process(&enc, in_pcm, bit_data);
      // g729a_dec_process(&dec, bit_data, out_pcm, 0);
      for (int isample = 0; isample < L_FRAME; isample++) {
        output_data(0, iframe * L_FRAME + isample) = out_pcm[isample];
      }
    }

    kaldi::WaveData wave_data_output(wave_data.SampFreq(), output_data);
    bool binary = true;
    kaldi::Output ko(wav_wxfilename, binary, false);
    wave_data_output.Write(ko.Stream());

    // g729a_dec_deinit(&dec);
    // g729a_enc_deinit(&enc);
    closeBcg729EncoderChannel(encoderChannelContext);
    closeBcg729DecoderChannel(decoderChannelContext);
    // is.close();
    // os.close();

    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    return -1;
  }
}

