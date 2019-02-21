#ifndef GWPE_H
#define GWPE_H
#include <cmath>
#include <algorithm>
#include <vector>
#include "spicax-eigen.h"
#include "base/kaldi-common.h"

namespace spicax {
struct GeneralizedWpeOptions {
  int num_chan;//N
  int delta;//delay
  int filterlen; // K
  int num_iter;
  int fs;
  int fftlen;
  int lowerfreq;
  int upperfreq;
  int psd_context;
  int startfreq;
  GeneralizedWpeOptions()
    : num_chan(1), delta(3), filterlen(10), num_iter(3), fs(16000),
      fftlen(1024), lowerfreq(100), upperfreq(7900), psd_context(2), startfreq(lowerfreq) {};
};

class GeneralizedWpe {
 public:
  GeneralizedWpe() = default;
  GeneralizedWpe(const GeneralizedWpeOptions & opts) {Init(opts);};
  bool Init(const GeneralizedWpeOptions & opts);
  void Dereverb(const Eigen::MatrixXcf &array_spec, Eigen::MatrixXcf &out_spec);
 private:
  void BuildYTilde(const Eigen::MatrixXcf &array_spec);
  float EstimateInvLambda(const Eigen::MatrixXcf &out_spec, int ibin, int iframe);
  void CalculateRr(const Eigen::MatrixXcf &array_spec, Eigen::MatrixXcf &out_spec);
  void EstimateG(const Eigen::MatrixXcf &array_spec, Eigen::MatrixXcf &out_spec);
  GeneralizedWpeOptions opts_;
  int num_bins_;
  int lowerbin_;
  int startbin_;
  int upperbin_;
  int num_chan_;
  int num_iter_;
  int filterlen_;
  std::vector<Eigen::MatrixXcf> y_tilde_;//num_bins * (NK * num_frames)
  std::vector<Eigen::MatrixXcf> g_; // num_bins * (NN * K)
  Eigen::MatrixXcf gbin_;
  Eigen::MatrixXcf R_;// NK * NK
  Eigen::MatrixXcf r_;//NK * N
  Eigen::MatrixXf inv_x_power_;
  Eigen::VectorXf inv_Lamba_;
};
}
#endif