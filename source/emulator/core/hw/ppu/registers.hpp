/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

namespace nba::core {

struct DisplayControl {
  int mode;
  int cgb_mode;
  int frame;
  int hblank_oam_access;
  int oam_mapping_1d;
  int forced_blank;
  int enable[8];

  bool _mode_is_dirty;

  void Reset();
  auto Read(int address) -> std::uint8_t;
  void Write(int address, std::uint8_t value);
};

struct DisplayStatus {
  class PPU* ppu = nullptr;

  int vblank_flag;
  int hblank_flag;
  int vcount_flag;
  int vblank_irq_enable;
  int hblank_irq_enable;
  int vcount_irq_enable;
  int vcount_setting;

  void Reset();
  auto Read(int address) -> std::uint8_t;
  void Write(int address, std::uint8_t value);
};

struct BackgroundControl {
  int priority;
  int tile_block;
  int unused;
  int mosaic_enable;
  int full_palette;
  int map_block;
  int wraparound = false;
  int size;

  BackgroundControl(int id) : id(id) {}
  
  void Reset();
  auto Read(int address) -> std::uint8_t;
  void Write(int address, std::uint8_t value);

private:
  int id;
};

struct ReferencePoint {
  std::int32_t initial;
  std::int32_t _current;
  
  void Reset();
  void Write(int address, std::uint8_t value);
};
  
struct BlendControl {
  enum Effect {
    SFX_NONE,
    SFX_BLEND,
    SFX_BRIGHTEN,
    SFX_DARKEN
  } sfx;
  
  int targets[2][6];

  void Reset();
  auto Read(int address) -> std::uint8_t;
  void Write(int address, std::uint8_t value);
};

struct WindowRange {
  int min;
  int max;
  bool _changed;

  void Reset();
  void Write(int address, std::uint8_t value);
};

struct WindowLayerSelect {
  int enable[2][6];

  void Reset();
  auto Read(int offset) -> std::uint8_t;
  void Write(int offset, std::uint8_t value);
};

struct Mosaic {
  struct {
    int size_x;
    int size_y;
    int _counter_y;
  } bg, obj;
  
  void Reset();
  void Write(int address, std::uint8_t value);
};

} // namespace nba::core
