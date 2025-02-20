/*  Copyright (C) 2025 JustScott

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// TODO: Will need to add SleepController to quicksettings in the future.
//       Have to add to definitions in displayapp as well.

#include "components/sleep/SleepController.h"
#include <chrono>
#include <libraries/log/nrf_log.h>

using namespace Pinetime::Controllers;
using namespace std::chrono_literals;

SleepController::SleepController(Controllers::DateTime& dateTimeController, Controllers::FS& fs)
  : dateTimeController {dateTimeController}, fs {fs} {
}

// TODO: When adding an init function, don't forget to init in SystemTask.cpp
//       below alarmController

void SleepController::ScheduleAlarm() {
}

// uint32_t SleepController::SecondsToAlarm() const {
//   return std::chrono::duration_cast<std::chrono::seconds>(alarmTime - dateTimeController.CurrentDateTime()).count();
// }

void SleepController::LoadSettingsFromFile() {
  lfs_file_t sleepFile;
  SleepSettings sleepBuffer;

  if (fs.FileOpen(&sleepFile, "/.system/sleep.dat", LFS_O_RDONLY) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[SleepController] Failed to open sleep data file");
    return;
  }

  fs.FileRead(&sleepFile, reinterpret_cast<uint8_t*>(&sleepBuffer), sizeof(sleepBuffer));
  fs.FileClose(&sleepFile);
  // TODO: consider creating a migration function that can convert
  //       sleepSettings between versions
  if (sleepBuffer.version != sleepFormatVersion) {
    NRF_LOG_WARNING("[SleepController] Loaded sleep settings has version %u instead of %u, discarding",
                    sleepBuffer.version,
                    sleepFormatVersion);
    return;
  }

  sleep = sleepBuffer;
  NRF_LOG_INFO("[SleepController] Loaded sleep settings from file");
}

void SleepController::SaveSettingsToFile() const {
  lfs_dir systemDir;
  if (fs.DirOpen("/.system", &systemDir) != LFS_ERR_OK) {
    fs.DirCreate("/.system");
  }
  fs.DirClose(&systemDir);
  lfs_file_t sleepFile;
  if (fs.FileOpen(&sleepFile, "/.system/sleep.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[SleepController] Failed to open sleep data file for saving");
    return;
  }

  fs.FileWrite(&sleepFile, reinterpret_cast<const uint8_t*>(&sleep), sizeof(sleep));
  fs.FileClose(&sleepFile);
  NRF_LOG_INFO("[SleepController] Saved sleep settings with format version %u to file", sleep.version);
}
