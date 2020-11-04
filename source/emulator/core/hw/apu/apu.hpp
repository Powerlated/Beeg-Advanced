/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <common/dsp/resampler.hpp>
#include <common/dsp/ring_buffer.hpp>
#include <emulator/config/config.hpp>
#include <emulator/core/hw/dma.hpp>
#include <emulator/core/scheduler.hpp>
#include <mutex>

#include "channel/channel_quad.hpp"
#include "channel/channel_wave.hpp"
#include "channel/channel_noise.hpp"
#include "channel/fifo.hpp"
#include "registers.hpp"

namespace nba::core {

class APU {
public:
  APU(Scheduler* scheduler, DMA* dma, std::shared_ptr<Config>);

  void Reset();
  void OnTimerOverflow(int timer_id, int times, int samplerate);

  struct MMIO {
    FIFO fifo[2];

    SoundControl soundcnt { fifo };
    BIAS bias;
  } mmio;

  QuadChannel psg1;
  QuadChannel psg2;
  WaveChannel psg3;
  NoiseChannel psg4;

  std::int8_t latch[2];
  std::shared_ptr<common::dsp::RingBuffer<float>> fifo_buffer[2];
  std::unique_ptr<common::dsp::Resampler<float>> fifo_resampler[2];
  int fifo_samplerate[2];

  std::mutex buffer_mutex;
  std::shared_ptr<common::dsp::StereoRingBuffer<float>> buffer;
  std::unique_ptr<common::dsp::StereoResampler<float>> resampler;

private:
  void Generate(int cycles_late);

  Scheduler* scheduler;
  DMA* dma;
  std::shared_ptr<Config> config;
  std::function<void(int)> event_cb = [this](int cycles_late) {
    this->Generate(cycles_late);
  };

  int resolution_old = 0;
};

} // namespace nba::core
