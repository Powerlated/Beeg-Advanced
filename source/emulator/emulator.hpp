/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <emulator/core/cpu.hpp>
#include <memory>
#include <string>

namespace nba {

class Emulator {
public:
  enum class StatusCode {
    BiosNotFound,
    GameNotFound,
    BiosWrongSize,
    GameWrongSize,
    Ok
  };
  
  Emulator(std::shared_ptr<Config> config);

  void Reset();
  auto LoadGame(std::string const& path) -> StatusCode;
  void Run(int cycles);
  void Frame();
  
private:
  static auto DetectBackupType(std::uint8_t* rom, size_t size) -> Config::BackupType;
  static auto CreateBackupInstance(Config::BackupType backup_type, std::string save_path) -> Backup*;
  static auto CalculateMirrorMask(size_t size) -> std::uint32_t;
  
  auto LoadBIOS() -> StatusCode; 
  
  core::CPU cpu;
  bool bios_loaded = false;
  std::shared_ptr<Config> config;
};

} // namespace nba
