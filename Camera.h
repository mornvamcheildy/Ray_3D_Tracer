#ifndef CAMERA_H
#define CAMERA_H

#include "Vec3.h"
#include "Ray.h"

class Camera {
public:
    Vec3 origin;
    double v_w, v_h;
    double zooming_ctrl_val = 1.0;

    Camera(Vec3 cam_origin, double viewport_h, double aspect_ratio) {
        origin = cam_origin;
        
        v_h = viewport_h; 
        v_w = aspect_ratio * v_h;
    }

    double get_zoom(){
        return zooming_ctrl_val;
    }

    Ray get_ray(double u, double v) const {
        Vec3 direction(
            v_w * u - (v_w / 2.0), 
            v_h * v - (v_h / 2.0), 
            -zooming_ctrl_val
        );
        
        return Ray(origin, direction);
    }
};

#endif
