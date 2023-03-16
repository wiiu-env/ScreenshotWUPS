FROM ghcr.io/wiiu-env/devkitppc:20221228

COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20230126 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libmappedmemory:20220904 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20230126 /artifacts $DEVKITPRO

WORKDIR project