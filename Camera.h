#ifndef CAMERA_H
#define CAMERA_H

#include "Vec3.h"
#include "Ray.h"

class Camera
{
public:
    // Constants calculated once at compile-time
    static constexpr int    i_w = 500;
    static constexpr int    i_h = 300;
    static constexpr double a_r = static_cast<double>(i_w) / i_h;

    Vec3 origin;
    double z_c = 1.0; // Zoom control

    // Viewport dimensions
    double v_h = static_cast<double>(i_h) / 100.0;
    double v_w = a_r * v_h;

    // Use a Member Initializer List here
    Camera(Vec3 cam_origin) : origin(cam_origin) {}

    double get_zm() const { return z_c; }

    Ray get_ray(double u, double v) const
    {
        Vec3 direction(
            v_w * u - (v_w / 2.0),
            v_h * v - (v_h / 2.0),
            -z_c);

        return Ray(origin, direction);
    }
};

#endif
