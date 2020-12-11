# Screenshot tool [![Build Status](https://api.travis-ci.org/Maschell/ScreenshotWUPS.svg?branch=master)](https://travis-ci.org/Maschell/ScreenshotWUPS)

This is just a simple plugin that takes screenshot of the TV and DRC screen. The screenshot will saved on the sd card in the folder "sd:/wiiu/screenshots/"

## Wii U Plugin System
This is a plugin for the [Wii U Plugin System (WUPS)](https://github.com/wiiu-env/WiiUPluginSystem/). To be able to use this plugin you have to place the resulting `.wps` file into the following folder:

```
sd:/wiiu/plugins
```
When the file is placed on the SDCard you can load it with [WUPS backend module](https://github.com/Maschell/WiiUPluginLoaderBackend/).
You also need the [Memory Mapping Module](https://github.com/wiiu-env/MemoryMappingModule/).

## Building

For building you need: 
- [wups](https://github.com/wiiu-env/WiiUPluginSystem)
- [wut](https://github.com/decaf-emu/wut)
- [libmappedmemory](https://github.com/wiiu-env/libmappedmemory)

Install them (in this order) according to their README's. Don't forget the dependencies of the libs itself.

Other external libraries are already located in the `libs` folder.

- libjpeg
- libturbojpeg

## Building via docker

```
# Build docker image (only needed once)
docker build . -t screenshot-plugin-builder

# make 
docker run -it --rm -v ${PWD}:/project screenshot-plugin-builder make

# make clean
docker run -it --rm -v ${PWD}:/project screenshot-plugin-builder make clean
```
