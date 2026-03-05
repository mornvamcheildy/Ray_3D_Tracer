#ifndef RAY_H
#define RAY_H
#include "Vec3.h"

class Ray
{
public:
    Vec3 orig, dir;
    Ray() {}
    Ray(const Vec3 &orig_mem_ref, const Vec3 &dir_mem_ref) : orig(orig_mem_ref), dir(dir_mem_ref) {}
    
    Vec3 origin_getter() const { return orig; }
    Vec3 direction_getter() const { return dir; }
    Vec3 at_poin_getter(double t) const { return orig + t * dir; }
};
#endif
