#include "gwpe.h"
#include <complex>

namespace spicax {
bool GeneralizedWpe::Init(const GeneralizedWpeOptions & opts) {
  opts_ = opts;
  lowerbin_ = opts_.lowerfreq * opts_.fftlen / opts_.fs;
  startbin_ = opts_.startfreq * opts_.fftlen / opts_.fs;
  upperbin_ = opts_.upperfreq * opts_.fftlen / opts_.fs;
  num_bins_ = opts_.fftlen / 2 + 1;
  num_chan_ = opts_.num_chan;
  filterlen_ = opts_.filterlen;
  num_iter_ = opts_.num_iter;
  y_tilde_.resize(num_bins_);
  g_.resize(num_bins_);
  for (int ibin = startbin_; ibin < upperbin_; ibin++) {
    g_[ibin].setZero(num_chan_ * num_chan_, filterlen_);
  }
  gbin_.setZero(num_chan_ * num_chan_, filterlen_);
  R_.setZero(num_chan_ * filterlen_, num_chan_ * filterlen_);
  r_.setZero(num_chan_ * filterlen_, num_chan_);
  return true;
}

//array_spec: FN * num_frames
void GeneralizedWpe::BuildYTilde(const Eigen::MatrixXcf &array_spec) {
  int num_frames = array_spec.cols();
  Eigen::VectorXcf y_t;
  for (int ibin = startbin_; ibin < upperbin_; ibin++) {
    y_tilde_[ibin].setZero(num_chan_ * filterlen_, num_frames);
  }

  for (int iframe = 0; iframe < (num_frames - opts_.delta); iframe++) {
    Eigen::Map<const Eigen::MatrixXcf> array_frame(&(array_spec(0, iframe)), num_bins_, num_chan_);
    for (int ibin = startbin_; ibin < upperbin_; ibin++) {
      y_tilde_[ibin].col(iframe + opts_.delta).head(num_chan_) = array_frame.row(ibin).head(num_chan_);
    }
  }

  for (int ibin = startbin_; ibin < upperbin_; ibin++) {
    for (int k = 1; k < filterlen_; k++) {
      y_tilde_[ibin].middleRows(k * num_chan_, num_chan_).rightCols(num_frames - 1) =
        y_tilde_[ibin].middleRows((k - 1) * num_chan_, num_chan_).leftCols(num_frames - 1);
    }
  }
}

float GeneralizedWpe::EstimateInvLambda(const Eigen::MatrixXcf &out_spec, int ibin, int iframe) {
  int psd_context = opts_.psd_context;
  Eigen::MatrixXcf psd_mat;
  // int psd_context_all = psd_context * 2 + 1;

  // psd_mat.setZero(psd_context_all, num_chan_ * psd_context_all);
  int num_frames = out_spec.cols();

  int frame_start = std::max(iframe - psd_context, 0);
  int frame_end   = std::min(iframe + psd_context, num_frames - 1);
  int psd_mat_cols = frame_end - frame_start + 1;
  int bin_start   = std::max(ibin - psd_context, 0);
  int bin_end     = std::min(ibin + psd_context, num_bins_ - 1);
  int psd_mat_rows = bin_end - bin_start + 1;

  psd_mat.setZero(num_chan_ * psd_mat_rows, psd_mat_cols);
  // KALDI_LOG << "psd_mat.rows()=" << psd_mat.rows() << ",psd_mat.cols()=" << psd_mat.cols() << "\n";

  for (int ichan = 0; ichan < num_chan_; ichan++) {
    psd_mat.middleRows(ichan * psd_mat_rows, psd_mat_rows)
      = out_spec.block(ichan * num_bins_ + bin_start, frame_start, psd_mat_rows, psd_mat_cols);
  }

  // for (int frame_context = -psd_context; frame_context <= psd_context; frame_context++) {
  //   int current_frame = iframe + frame_context;
  //   current_frame = std::max(std::min(current_frame, num_frames - 1), 0);
  //   Eigen::Map<const Eigen::MatrixXcf> output_frame(&(out_spec(0, current_frame)), num_bins_, num_chan_);
  //   for (int bin_context = -psd_context; bin_context <= psd_context; bin_context++) {
  //     int current_bin = ibin + bin_context;
  //     current_bin = std::max(std::min(current_bin, num_bins_ - 1), 0);
  //     psd_mat.block(bin_context + psd_context, (frame_context + psd_context) * num_chan_, 1, num_chan_) = output_frame.row(current_bin);
  //   }
  // }

  // psd_mat.middleCols();
  float lambda = psd_mat.cwiseAbs2().mean();
  // lambda = std::max(lambda, 1000.0f);
  lambda = 1.0f / (lambda + 1.0f);
  // lambda = std::min(lambda, 5e-8f);
  return lambda;
}

void GeneralizedWpe::CalculateRr(const Eigen::MatrixXcf &array_spec, Eigen::MatrixXcf &out_spec) {
  Eigen::VectorXcf y_tilde_1;
  Eigen::VectorXcf y_t, est_reverb;
  Eigen::VectorXcf est_g;
  int num_frames = array_spec.cols();
  Eigen::LLT<Eigen::MatrixXcf> qr(num_chan_ * filterlen_);
  for (int ibin = startbin_; ibin < upperbin_; ibin++) {
    for (int iter = 0; iter < num_iter_; iter++) {
      R_.setZero(num_chan_ * filterlen_, num_chan_ * filterlen_);
      r_.setZero(num_chan_ * filterlen_, num_chan_);
      for (int iframe = 0 ; iframe < num_frames; iframe++) {
        // Eigen::Map<const Eigen::MatrixXcf> array_frame(&(array_spec(0, iframe)), num_bins_, num_chan_);
        // y_t = array_frame.row(ibin);
        y_tilde_1 = y_tilde_[ibin].col(iframe);
        float Lambda = EstimateInvLambda(out_spec, ibin, iframe);

        R_ +=  y_tilde_1 * y_tilde_1.adjoint() * Lambda;
        for (int ichan = 0; ichan < num_chan_; ichan++) {
          // r_.col(ichan) += y_tilde_1 * std::conj(y_t(ichan)) * Lambda;
          r_.col(ichan) += y_tilde_1 * std::conj(array_spec(ichan * num_bins_ + ibin, iframe)) * Lambda;
        }
      }

      float R_trace = R_.trace().real() * 1e-4f;
      for (int j = 0; j < num_chan_ * filterlen_; j++) {R_(j, j) += R_trace;}
      R_ *= 1e-3f;
      r_ *= 1e-3f;
      // KALDI_LOG << "iter=" << iter << ",ibin=" << ibin << ",R.trace()=" << R_.trace().real() << ",R.det()=" << R_.determinant().real() << "\n";
      qr.compute(R_);
      for (int ichan = 0; ichan < num_chan_; ichan++) {
        // est_g = R_.colPivHouseholderQr().solve(r_.col(ichan));
        est_g = qr.solve(r_.col(ichan));
        Eigen::Map<const Eigen::MatrixXcf> est_g_reshape(&(est_g(0, 0)), num_chan_, filterlen_);
        gbin_.middleRows(ichan * num_chan_, num_chan_) = est_g_reshape;
      }

      //filter
      for (int iframe = 0; iframe < num_frames; iframe++) {
        Eigen::Map<const Eigen::MatrixXcf> array_frame(&(array_spec(0, iframe)), num_bins_, num_chan_);
        y_t = array_frame.row(ibin);
        est_reverb.setZero(num_chan_);
        for (int k = 0; k < filterlen_; k++) {
          Eigen::Map<const Eigen::MatrixXcf> Gk(&(gbin_(0, k)), num_chan_, num_chan_);
          Eigen::Map<const Eigen::VectorXcf> y_tilde_2(&(y_tilde_[ibin](k * num_chan_, iframe)), num_chan_);
          est_reverb += Gk.adjoint() * y_tilde_2;
        }
        Eigen::Map<Eigen::MatrixXcf> output_frame(&(out_spec(0, iframe)), num_bins_, num_chan_);
        output_frame.row(ibin) = y_t - est_reverb;
      }
    }
  }
}
void GeneralizedWpe::EstimateG(const Eigen::MatrixXcf &array_spec, Eigen::MatrixXcf &out_spec) {
  CalculateRr(array_spec, out_spec);
}
void GeneralizedWpe::Dereverb(const Eigen::MatrixXcf &array_spec, Eigen::MatrixXcf &out_spec) {
  BuildYTilde(array_spec);
  out_spec = array_spec.topRows(num_bins_ * num_chan_);

  EstimateG(array_spec, out_spec);
  for(int ichan = 0; ichan < num_chan_; ichan++){
    out_spec.middleRows(ichan * num_bins_, lowerbin_).setZero();
  }
}
}
