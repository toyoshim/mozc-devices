# Firmware Development Guide

For the entire keyboard assembly, please refer to the [Build Guide](../buildguide.md).

## Directory List

- prebuilt/ : Pre-built firmware for each board
- common/ : Source files for libraries used in common by each board
- main/ : Source files and project for the 9-dial edition main chip
- sub/ : Source files and project for the 9-dial edition sub chip
- one_dial/ : Source files and project for the 1-dial edition Raspberry Pi Pico

## Pre-built Firmware

We have included pre-built firmware under `prebuilt/`, so you can use them as is if no changes are needed.

Use `main.uf2` and `sub.uf2` for the 9-dial edition, and `one_dial.uf2` for the 1-dial edition.

## Guide to Developing Your Own Firmware

Install [Visual Studio Code](https://code.visualstudio.com/) and add the [Raspberry Pi Pico extension](https://marketplace.visualstudio.com/items?itemName=raspberry-pi.raspberry-pi-pico).

From the `Raspberry Pi Pico Projects` extension in the Activity Bar, use `Import Project` to open each project's directory.
If you are using the extension for the first time, the development environment installation will begin. Once the project is imported, you can develop using `CMake` and `Run and Debug` in the Activity Bar.

As with the official Pico series boards, the 9-dial edition has `SWCLK` and `SWDIO` land pairs labeled `SWC/D` on the board for each chip. By preparing development equipment that supports SWD, such as the official Debug Probe, you can perform step execution in the editor.

### Standard Output

On the 9-dial edition, there are lands labeled `TX1` for the main chip and `TX2` for the sub-chip on the board. With the standard settings, the standard output is output from these pins as UART at a speed of 115200.

On the other hand, by changing `pico_enable_stdio_uart` to `0` and `pico_enable_stdio_semihosting` to `1` in `CMakeLists.txt`, you can switch the output to go through the debugger. This is convenient as you don't need to wire TX, but be aware that the operation will stop at the output point if debugger support is not enabled. It will also stop when the debugger is not attached, so it cannot be used for release builds. Debugger support can be enabled with `monitor arm semihosting enable`, but it is tedious to set it every time you start the debugger, so it is convenient to register it in each command setting in `.vscode/launch.json` as follows.

```
"postLaunchCommands": [
    "monitor arm semihosting enable"
]
```

### Using PICO W or PICO 2 (W)

The 1-dial edition is set to use the official Raspberry Pi Pico board, but you can support other boards by changing the board setting from `pico` to `pico_w`, `pico2`, `pico2_w`, etc. in `set(PICO_BOARD pico CACHE STRING "Board type")` in `CMakeFiles.txt`. You should also be able to support 3rd party boards with `none`, etc.

### About EEPROM

Since the board setting for the 9-dial edition is `none`, a general-purpose driver for EEPROM will be included with a prescaler setting of 4. This should work with most EEPROMs. If you are using the reference parts, we have confirmed that the driver for W25Q080 works with a prescaler setting of 2, similar to the official board. By creating a `board_name.h` in `~/.pico-sdk/sdk/2.2.0/src/boards/include/boards/` and defining

```
#define PICO_BOOT_STAGE_2_CHOOSE_W25Q080 1
#define PICO_FLASH_SPI_CLKDIV 2
```

and specifying it in the board settings of `CMakeFiles.txt`, you can optimize EEPROM access via QSPI in case of a cache miss and draw out further performance.

### About Sensor Adjustment

By default, the sensor is pulled up by an internal resistor. If you want to adjust it by pulling up with an external resistor, change `gpio_pull_up(gpio);` to `gpio_disable_pulls(gpio);` in `PhotoSensor::PhotoSensor()` in `photo_sensor.cc`.

Alternatively, it is also possible to pull up with a combined resistance of the internal and external resistors while the internal resistor is enabled. In this case, the internal resistor has a nominal value of 50-80KΩ and is connected in parallel with the external resistor.

For details, please check the [Board Assembly Guide](../board/README.md).
