# Screenshot Plugin

This is just a simple plugin that takes screenshots of the TV and GamePad screens. 
The screenshots will be saved on the SD card in the folder `sd:/wiiu/screenshots`

## Installation
(`[ENVIRONMENT]` is a placeholder for the actual environment name.)

1. Copy the file `screenshot.wps` into `sd:/wiiu/environments/[ENVIRONMENT]/plugins`.  
2. Requires the [WiiUPluginLoaderBackend](https://github.com/wiiu-env/WiiUPluginLoaderBackend) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.
3. Requires the [MemoryMappingModule](https://github.com/wiiu-env/MemoryMappingModule) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.
4. Requires the [NotificationModule](https://github.com/wiiu-env/NotificationModule) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.

## Usage
Press a button combo on the GamePad, Pro Controller or Classic Controller to take a screenshot.

Via the plugin config menu (press L, DPAD Down and Minus on the GamePad, Pro Controller or Classic Controller) you can configure the plugin. The available options are the following:
- **Settings**: 
  - Enabled: (Default is true)
    - Enables or disables the screenshot plugin.
  - Button combo: (Default is TV-Button)
    - Changes the button combo for taking screenshots.
  - Screen: (Default is TV and GamePad)
    - Determines from which screen a screenshot should be taken. Possible options: TV & GamePad, TV only, GamePad only.
  - Output format: (Default is JPEG)
    - Determines which file is used. Currently saving screens as .jpg, .png and .bmp is supported.
  - JPEG quality: (Default is 90)
      - Determines the quality when saving as JPEG. Lowest possible quality is 10, highest 100.
  - Check ReservedBit for taking screenshots: (Default is true)
      - Enables taking screenshots when the "ReservedBit" on the Pro Controller is set, regardless of the configured button combo. For example this allows to take screenshots with the screenshot button of Switch Pro Controller (when using a compatible [Bloopair](https://github.com/GaryOderNichts/Bloopair) version):

## Building

For building you need: 
- [wups](https://github.com/wiiu-env/WiiUPluginSystem)
- [wut](https://github.com/decaf-emu/wut)
- [libmappedmemory](https://github.com/wiiu-env/libmappedmemory)
- [libnotifications](https://github.com/wiiu-env/libnotifications)
- PPC versions of zlib, libgd, libpng, libjpeg (install via `pacman -Syu ppc-zlib ppc-libgd ppc-libpng ppc-libjpeg-turbo`)

## Building using the Dockerfile

It's possible to use a docker image for building. This way you don't need anything installed on your host system.

```
# Build docker image (only needed once)
docker build . -t screenshot-plugin-builder

# make 
docker run -it --rm -v ${PWD}:/project screenshot-plugin-builder make

# make clean
docker run -it --rm -v ${PWD}:/project screenshot-plugin-builder make clean
```

## Format the code via docker

`docker run --rm -v ${PWD}:/src wiiuenv/clang-format:13.0.0-2 -r ./src -i`
