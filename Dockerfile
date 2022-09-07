FROM wiiuenv/devkitppc:20220806

COPY --from=wiiuenv/wiiupluginsystem:20220904 /artifacts $DEVKITPRO
COPY --from=wiiuenv/libmappedmemory:20220904 /artifacts $DEVKITPRO

WORKDIR project