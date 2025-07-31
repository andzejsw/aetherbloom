#include "block.h"

void air_init() {
    struct Block air = BLOCK_DEFAULT;
    air.id = AIR;
    air.transparent = true;
    air.opacity = 0;
    air.solid = false;
    BLOCKS[AIR] = air;
}