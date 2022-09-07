# Screenshot Plugin

This is just a simple plugin that takes screenshot of the TV and DRC screen. The screenshot will saved on the sd card in the folder "sd:/wiiu/screenshots/"

## Usage
(`[ENVIRONMENT]` is a placeholder for the actual environment name.)

1. Copy the file `screenshot.wps` into `sd:/wiiu/environments/[ENVIRONMENT]/plugins`.  
2. Requires the [WiiUPluginLoaderBackend](https://github.com/wiiu-env/WiiUPluginLoaderBackend) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.
3. Requires the [MemoryMappingModule](https://github.com/wiiu-env/MemoryMappingModule) in `sd:/wiiu/environments/[ENVIRONMENT]/modules`.

## Building

For building you need: 
- [wups](https://github.com/wiiu-env/WiiUPluginSystem)
- [wut](https://github.com/decaf-emu/wut)
- [libmappedmemory](https://github.com/wiiu-env/libmappedmemory)
- libturbojpeg (install via `pacman -Syu dkp-libs/ppc-libjpeg-turbo`)

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
