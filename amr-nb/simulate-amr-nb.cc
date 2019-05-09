#include <iostream>
#include <fstream>
#include <string>
#include "amr-nb-wrapper.h"
#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "feat/wave-reader.h"
#include "base/timer.h"
// #include "c-code/sp_enc.h"

int main(int argc, char *argv[] ) {
  try {
    using namespace kaldi;
    const char *usage =
      "simulate GSM-EFR(8kHz) codec\n"
      "Usage: simulate-gsm-efr [options] <wav-in-file> <wav-out-file>\n"
      " e.g.: simulate-gsm-efr input.wav output.wav\n";
    ParseOptions po(usage);
    int mode_int = 0;
    po.Register("mode", &mode_int, "rate mode[0, 7], [worst]0:4.75kbps, [best]7:12.2kbps");
    po.Read(argc, argv);
    if (po.NumArgs() != 2) {
      po.PrintUsage();
      exit(1);
    }

    std::string wav_rxfilename = po.GetArg(1), wav_wxfilename = po.GetArg(2);

    kaldi::Input wave_input(wav_rxfilename);
    kaldi::WaveData wave_data;
    wave_data.Read(wave_input.Stream());
    KALDI_ASSERT(wave_data.Data().NumRows() == 1);
    KALDI_ASSERT((int)(wave_data.SampFreq()) == 8000);
    kaldi::SubVector<kaldi::BaseFloat> data(wave_data.Data(), 0);

    kaldi::Matrix<kaldi::BaseFloat> output_data(1, data.Dim());
    AmrNbWrapper amrnb_simulator(mode_int);
    short *pcm_in = new short [data.Dim()];
    short *pcm_out = new short [data.Dim()];
    for (int isample = 0; isample < data.Dim(); isample++) {
      pcm_in[isample] = (short) data(isample);
    }
    std::memset(pcm_out, 0x0, sizeof(short) * data.Dim());
    int in_samples = data.Dim();
    kaldi::Timer ATimer;
    ATimer.Reset();
    amrnb_simulator.Simulate((const char*)pcm_in, in_samples, (char*)pcm_out);
    float time_elapsed = ATimer.Elapsed();
    std::cout << "RTF:" <<  time_elapsed / (data.Dim() / 8000.0f) << "\n";
    for (int isample = 0; isample < data.Dim(); isample++) {
      output_data(0, isample) = pcm_out[isample];
    }

    kaldi::WaveData wave_data_output(wave_data.SampFreq(), output_data);
    bool binary = true;
    kaldi::Output ko(wav_wxfilename, binary, false);
    wave_data_output.Write(ko.Stream());

    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    return -1;
  }
}

