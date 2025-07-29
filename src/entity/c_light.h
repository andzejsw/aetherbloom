#ifndef C_LIGHT_H
#define C_LIGHT_H

#include "../util/util.h"
#include "../world/light.h"

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