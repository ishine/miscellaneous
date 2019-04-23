#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "feat/wave-reader.h"
#include "dereverb/gwpe.h"
#include "dereverb/spicax-eigen.h"
#include "dereverb/wola.h"
#include <string>

int main(int argc, char** argv) {
  using namespace std;
  using namespace kaldi;
  try {
    const char *usage =
      "apply-gwpe \n"
      "Usage: apply-gwpe [options] <wav-in-file> <wav-out-file>\n"
      " e.g.: apply-gwpe input.wav output.wav\n"
      " e.g.: apply-gwpe input.wav\n";
    ParseOptions po(usage);

    bool normalized = true;
    int startfreq = 0;
    po.Register("out-norm", &normalized, "normalize the output to 32767");
    po.Register("start-freq", &startfreq, "lowest frequency for dereverberation");
    po.Read(argc, argv);
    if ((po.NumArgs() != 2) && (po.NumArgs() != 1)) {
      po.PrintUsage();
      exit(1);
    }

    std::string input_filename = po.GetArg(1);
    kaldi::WaveData input_wave;
    {
      kaldi::Input ki(input_filename);
      input_wave.Read(ki.Stream());
    }
    const kaldi::Matrix<kaldi::BaseFloat> &speech_data = input_wave.Data();
    int num_chan = speech_data.NumRows();
    int wave_samples = speech_data.NumCols();

    kaldi::Matrix<kaldi::BaseFloat> mixed_data;
    int fs = input_wave.SampFreq();
    if (wave_samples > fs) {
      float frame_shift_ms = 0.008;
      float frame_len_ms = frame_shift_ms * 4;
      int frame_len_samples = frame_len_ms * fs;
      int fftlen = 1;
      while (fftlen < frame_len_samples) {fftlen <<= 1;}
      spicax::WolaOptions wola_opts(num_chan, num_chan, fs, frame_shift_ms, frame_len_ms, fftlen);
      spicax::Wola wola(wola_opts);

      spicax::MatrixXs input_short, output_short;
      int num_frames = (wave_samples - wola_opts.frame_len_ + wola_opts.frame_shift_) / wola_opts.frame_shift_;
      int pcm_in_samples = num_frames * wola_opts.frame_shift_ + wola_opts.frame_len_ - wola_opts.frame_shift_;
      input_short.setZero(num_chan, pcm_in_samples);
      output_short.setZero(num_chan, pcm_in_samples);
      for (int ichan = 0; ichan < num_chan; ichan++) {
        for (int isample = 0; isample < pcm_in_samples; isample++) {
          input_short(ichan, isample) = (short)speech_data(ichan, isample);
        }
      }

      spicax::GeneralizedWpeOptions gwpe_opts;
      gwpe_opts.num_chan = num_chan;
      gwpe_opts.num_iter = 3;
      gwpe_opts.lowerfreq = 50;
      gwpe_opts.upperfreq = fs / 2;
      gwpe_opts.fftlen = fftlen;
      gwpe_opts.delta = 3;
      if (num_chan == 1) {
        gwpe_opts.filterlen = 16;
      } else {
        gwpe_opts.filterlen = 16;
      }

      gwpe_opts.startfreq = startfreq;
      spicax::GeneralizedWpe gwpe(gwpe_opts);
      const Eigen::MatrixXcf & in_spec = wola.Decompose(input_short.data(), pcm_in_samples);
      if (in_spec.cols() > 0) {
        Eigen::MatrixXcf & out_spec = wola.GetOutSpec();
        gwpe.Dereverb(in_spec, out_spec);
        int samples_reconstruct = wola.Reconstruct(output_short.data());
        // KALDI_LOG << "samples_reconstruct=" << samples_reconstruct << "\n";
      }
      mixed_data.Resize(num_chan, pcm_in_samples);
      float gain = 1.0f;
      if (normalized) {
        float max_val_org = input_short.cwiseAbs().maxCoeff();
        float max_val = output_short.cwiseAbs().maxCoeff();
        max_val_org = std::max(max_val_org, 15000.0f);
        gain = max_val_org / max_val;
      }
      for (int ichan = 0; ichan < num_chan; ichan++) {
        for (int isample = 0; isample < pcm_in_samples; isample++) {
          mixed_data(ichan, isample) = gain * output_short(ichan, isample);
        }
      }
    } else {
      mixed_data = speech_data;
    }

    std::string output_filename;
    if (po.NumArgs() == 1)output_filename = input_filename.substr(0, input_filename.size() - 4) + "-gwpe.wav";
    if (po.NumArgs() == 2)output_filename = po.GetArg(2);
    kaldi::WaveData wave_data_output(input_wave.SampFreq(), mixed_data);
    bool binary = true;
    kaldi::Output ko(output_filename, binary, false);
    wave_data_output.Write(ko.Stream());

    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << "\n";
    return -1;
  }
}
