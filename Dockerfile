FROM ghcr.io/wiiu-env/devkitppc:20240423

COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20240425 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libmappedmemory:20230621 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20240426 /artifacts $DEVKITPRO

WORKDIR project
