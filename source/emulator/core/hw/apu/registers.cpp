/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "registers.hpp"

namespace nba::core {

void SoundControl::Reset() {
  master_enable = false;
  psg.volume = 0;
  psg.master[0] = 0;
  psg.master[1] = 0;
  psg.enable[0][0] = false;
  psg.enable[0][1] = false;
  psg.enable[0][2] = false;
  psg.enable[0][3] = false;
  psg.enable[1][0] = false;
  psg.enable[1][1] = false;
  psg.enable[1][2] = false;
  psg.enable[1][3] = false;
  dma[0].volume = 0;
  dma[0].enable[0] = false;
  dma[0].enable[1] = false;
  dma[0].timer_id = 0;
  dma[1].volume = 0;
  dma[1].enable[0] = false;
  dma[1].enable[1] = false;
  dma[1].timer_id = 0;
}

auto SoundControl::Read(int address) -> std::uint8_t {
  switch (address) {
    case 0:
      return (psg.master[SIDE_RIGHT] << 0) |
           (psg.master[SIDE_LEFT]  << 4);
    case 1:
      return (psg.enable[SIDE_RIGHT][0] ? 1   : 0) |
             (psg.enable[SIDE_RIGHT][1] ? 2   : 0) |
             (psg.enable[SIDE_RIGHT][2] ? 4   : 0) |
             (psg.enable[SIDE_RIGHT][3] ? 8   : 0) |
             (psg.enable[SIDE_LEFT ][0] ? 16  : 0) |
             (psg.enable[SIDE_LEFT ][1] ? 32  : 0) |
             (psg.enable[SIDE_LEFT ][2] ? 64  : 0) |
             (psg.enable[SIDE_LEFT ][3] ? 128 : 0);
    case 2:
      return  psg.volume |
             (dma[DMA_A].volume << 2) |
             (dma[DMA_B].volume << 3);
    case 3:
      return (dma[DMA_A].enable[SIDE_RIGHT] ? 1  : 0) |
             (dma[DMA_A].enable[SIDE_LEFT ] ? 2  : 0) |
             (dma[DMA_A].timer_id       ? 4  : 0) |
             (dma[DMA_B].enable[SIDE_RIGHT] ? 16 : 0) |
             (dma[DMA_B].enable[SIDE_LEFT ] ? 32 : 0) |
             (dma[DMA_B].timer_id       ? 64 : 0);
    case 4:
      /* FIXME: emulate bits 0-3 PSG ON/OFF state. */
      return 0b1111 | (master_enable ? 128 : 0);
      
    default: return 0;
  }
}

void SoundControl::Write(int address, std::uint8_t value) {
  switch (address) {
    case 0:
      psg.master[SIDE_RIGHT] = (value >> 0) & 7;
      psg.master[SIDE_LEFT ] = (value >> 4) & 7;
      break;
    case 1:
      psg.enable[SIDE_RIGHT][0] = value & 1;
      psg.enable[SIDE_RIGHT][1] = value & 2;
      psg.enable[SIDE_RIGHT][2] = value & 4;
      psg.enable[SIDE_RIGHT][3] = value & 8;
      psg.enable[SIDE_LEFT ][0] = value & 16;
      psg.enable[SIDE_LEFT ][1] = value & 32;
      psg.enable[SIDE_LEFT ][2] = value & 64;
      psg.enable[SIDE_LEFT ][3] = value & 128;
      break;
    case 2:
      psg.volume = (value >> 0) & 3;
      dma[DMA_A].volume = (value >> 2) & 1;
      dma[DMA_B].volume = (value >> 3) & 1;
      break;
    case 3:
      dma[DMA_A].enable[SIDE_RIGHT] = value & 1;
      dma[DMA_A].enable[SIDE_LEFT ] = value & 2;
      dma[DMA_A].timer_id = (value >> 2) & 1;
      dma[DMA_B].enable[SIDE_RIGHT] = value & 16;
      dma[DMA_B].enable[SIDE_LEFT ] = value & 32;
      dma[DMA_B].timer_id = (value >> 6) & 1;

      if (value & 0x08) fifos[0].Reset();
      if (value & 0x80) fifos[1].Reset();

      break;
    case 4:
      master_enable = value & 128;
      break;
  }
}

void BIAS::Reset() {
  level = 0x200;
  resolution = 0;
}

auto BIAS::Read(int address) -> std::uint8_t {
  switch (address) {
    case 0:  return   level & 0xFF;
    case 1:  return ((level >> 8) & 3) | (resolution << 6);
    default: return 0;
  }
}

void BIAS::Write(int address, std::uint8_t value) {
  switch (address) {
    case 0: {
      level = (level & ~0xFF) | (value & ~1);
      break;
    }
    case 1: {
      level = (level & 0xFF) | ((value & 3) << 8);
      resolution = value >> 6;
      break;
    }
  }
}

} // namespace nba::core
