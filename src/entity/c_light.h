#ifndef C_LIGHT_H
#define C_LIGHT_H

#include "../util/util.h"
#include "../world/light.h"

#define BLOCKLIGHT_MASK 0x000000000FFFF0000
#define BLOCKLIGHT_OFFSET 16

#define SUNLIGHT_MASK 0x0000000F00000000
#define SUNLIGHT_OFFSET 32

#define LIGHT_MASK 0x0000000FFFFF0000
#define LIGHT_OFFSET 16

struct LightComponent {
    Blocklight light;

    struct {
        ivec3s pos;
        Blocklight light;
        bool enabled;
    } last;
    
    struct {
        bool enabled: 1;
    } flags;
};

#endif