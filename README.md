<div align="center">

![Header Image](/doc/logo/infinitime-logo-small.jpg)
<p float="left">
  <img src="/doc/gettingStarted/ui/default_watchface.png" width="200" style="margin-right: 10px; border-radius: 10px; box-shadow: 0 2px 6px rgba(0,0,0,0.15);" />
  <img src="/doc/gettingStarted/ui/first_app_page.png" width="200" style="margin-right: 10px; border-radius: 10px; box-shadow: 0 2px 6px rgba(0,0,0,0.15);" />
  <img src="/doc/gettingStarted/AppsScreenshots/calculator.png" width="200" style="border-radius: 10px; box-shadow: 0 2px 6px rgba(0,0,0,0.15);" />
  <img src="/doc/gettingStarted/AppsScreenshots/weather.png" width="200" style="border-radius: 10px; box-shadow: 0 2px 6px rgba(0,0,0,0.15);" />
  <img src="/doc/gettingStarted/AppsScreenshots/calendar.png" width="200" style="border-radius: 10px; box-shadow: 0 2px 6px rgba(0,0,0,0.15);" />
  <img src="/doc/gettingStarted/AppsScreenshots/music.png" width="200" style="border-radius: 10px; box-shadow: 0 2px 6px rgba(0,0,0,0.15);" />
</p>

<br>

# InfiniTime

*A forked alternative to the [InfiniTime](https://github.com/InfiniTimeOrg/InfiniTime) firmware for the [PineTime smartwatch](https://pine64.org/devices/pinetime/) with more useful default apps, quicker updates, new features, and expanded customization*

<br>

</div>

## New to InfiniTime?

- [Getting started with InfiniTime](doc/gettingStarted/gettingStarted-1.0.md)
- [Updating the software](doc/gettingStarted/updating-software.md)
- [About the firmware and bootloader](doc/gettingStarted/about-software.md)
- [Available apps](doc/gettingStarted/Applications.md)
- [Available watch faces](/doc/gettingStarted/Watchfaces.md)
- [PineTimeStyle Watch face](https://pine64.org/documentation/PineTime/Watchfaces/PineTimeStyle)
  - [Weather integration](https://pine64.org/documentation/PineTime/Software/InfiniTime_weather/)

### Companion apps

- [Gadgetbridge](https://gadgetbridge.org/) (Android)
- [InfiniLink](https://github.com/InfiniTimeOrg/InfiniLink) (iOS)
- [InfiniTimeExplorer](https://infinitimeexplorer.netlify.app) (Web)

<br>

> *InfiniTimeExplorer is only compatible with web browsers that support Web BLE. Current fully supported browsers include Chrome and Microsoft Edge.* 

## Development

- [Rough structure of the code](doc/code/Intro.md)
- [How to implement an application](doc/code/Apps.md)
- [Generate the fonts and symbols](src/displayapp/fonts/README.md)
- [Bootloader, OTA and DFU](bootloader/README.md)
- [External resources](doc/ExternalResources.md)

### Contributing

- [Coding conventions](doc/coding-convention.md)

### Build, flash and debug

- [InfiniTime simulator (for this fork)](https://github.com/JustScott/InfiniSim)
- [Build the project](doc/buildAndProgram.md)
- [Build the project with Docker](doc/buildWithDocker.md)
- [Flash the firmware using OpenOCD and STLinkV2](doc/openOCD.md)
- [Flash the firmware using SWD interface](doc/SWD.md)
- [Flash the firmware using JLink](doc/jlink.md)
- [Flash the firmware using GDB](doc/gdb.md)
- [Stub using NRF52-DK](doc/PinetimeStubWithNrf52DK.md)

### API

- [BLE implementation and API](doc/ble.md)

### Architecture and technical topics

- [Memory analysis](doc/MemoryAnalysis.md)

### Project management

- [Versioning](doc/versioning.md)
- [Project branches](doc/branches.md)
- [Files included in the release notes](doc/filesInReleaseNotes.md)
- [Files needed by the factory](doc/files-needed-by-factory.md)

## Licenses

This project is released under the GNU General Public License version 3 or, at your option, any later version.

It integrates the following projects:

- RTOS: **[FreeRTOS](https://freertos.org)** under the MIT license
- UI: **[LittleVGL/LVGL](https://lvgl.io/)** under the MIT license
- BLE stack: **[NimBLE](https://github.com/apache/mynewt-nimble)** under the Apache 2.0 license
- Font: **[Jetbrains Mono](https://www.jetbrains.com/fr-fr/lp/mono/)** under the Apache 2.0 license
