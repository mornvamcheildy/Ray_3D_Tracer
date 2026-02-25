#ifndef CAMERA_H
#define CAMERA_H

#include "Vec3.h"
#include "Ray.h"

class Camera
{
public:
    Vec3 origin;

    // Camera Vision zooming control variable
    double z_c = 1.0;

    // Image sizing declaration, defination and assignetion
    const int i_w = 500; // Horizontal size
    const int i_h = 300; // Vertical size

    // Aspect Ratio adjosting calculation
    double a_r = double(i_w) / i_h;

    // Camera Window ViewPort dimension configuration calculating from image H and W
    double v_h = double(i_h) / 100; // ViewPort H
    double v_w = a_r * v_h;         // ViewPort W

    Camera(Vec3 cam_origin)
    {
        origin = cam_origin;
    }

    // Camera Vision Zoom Value Fetcher
    double get_zm() const { return z_c;}

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
