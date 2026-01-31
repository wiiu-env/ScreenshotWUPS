FROM ghcr.io/wiiu-env/devkitppc:20260126

COPY --from=ghcr.io/wiiu-env/wiiupluginsystem:20260126 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libmappedmemory:20260131 /artifacts $DEVKITPRO
COPY --from=ghcr.io/wiiu-env/libnotifications:20260131 /artifacts $DEVKITPRO

WORKDIR project
