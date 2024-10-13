//
// Created by wogob on 9/24/2024.
//

#ifndef STEG_MATHUTILITIES_H
#define STEG_MATHUTILITIES_H

float clamp(float value, float min, float max) {
    if(value >= min && value <= max) {
        return value;
    }
    if(value < min) {
        return min;
    }
    if(value > max) {
        return max;
    }
    return value;
}

#endif //STEG_MATHUTILITIES_H
