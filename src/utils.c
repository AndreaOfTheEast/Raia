#include "../includes/utils.h"
#include "../includes/raymath.h"

float clamp_angle(float angle) {
    if (angle > PI * 2) angle -= PI * 2;
    if (angle < 0) angle += PI * 2;
    return angle;
}