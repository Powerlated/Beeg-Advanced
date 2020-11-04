/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include "../resampler.hpp"

namespace common::dsp {

template <typename T>
class NearestResampler : public Resampler<T> {
public:
  NearestResampler(std::shared_ptr<WriteStream<T>> output) 
    : Resampler<T>(output)
  { }
  
  void Write(T const& input) final {
    while (resample_phase < 1.0) {
      this->output->Write(input);
      resample_phase += this->resample_phase_shift;
    }
    
    resample_phase = resample_phase - 1.0;
  }
  
private:
  float resample_phase = 0;
};

template <typename T>
using NearestStereoResampler = NearestResampler<StereoSample<T>>;

} // namespace common::dsp
