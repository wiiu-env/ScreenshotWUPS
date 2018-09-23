FROM wups/core-with-wut:0.1

# Get dependencies
COPY --from=wiiuwut/libutils:0.1 /artifacts $WUT_ROOT

WORKDIR project
