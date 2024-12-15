#include "mathutilities.h"

float clamp(float value, float min, float max) {
    if (value >= min && value <= max) {
        return value;
    }
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}
