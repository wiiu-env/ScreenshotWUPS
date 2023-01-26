FROM wiiuenv/devkitppc:20221228

COPY --from=wiiuenv/wiiupluginsystem:20230126 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libmappedmemory:20220904 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libnotifications:20230126 /artifacts $DEVKITPRO

WORKDIR project