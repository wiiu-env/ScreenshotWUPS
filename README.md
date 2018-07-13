# Screenshot tool [![Build Status](https://api.travis-ci.org/Maschell/ScreenshotWUPS.svg?branch=master)](https://travis-ci.org/Maschell/ScreenshotWUPS)

This is just a simple plugin that takes screenshot of the TV and DRC screen. The screenshot will saved on the sd card.

## Wii U Plugin System
This is a plugin for the [Wii U Plugin System (WUPS)](https://github.com/Maschell/WiiUPluginSystem/). To be able to use this plugin you have to place the resulting `.mod` file into the following folder:

```
sd:/wiiu/plugins
```
When the file is placed on the SDCard you can load it with [plugin loader](https://github.com/Maschell/WiiUPluginSystem/).

## Building

For building you need: 
- [wups](https://github.com/Maschell/WiiUPluginSystem)
- [wut](https://github.com/decaf-emu/wut)
- [libutilswut](https://github.com/Maschell/libutils/tree/wut) (WUT version) for common functions.

Install them (in this order) according to their README's. Don't forget the dependencies of the libs itself.

Other external libraries are already located in the `libs` folder.

- libjpeg
- libturbojpeg