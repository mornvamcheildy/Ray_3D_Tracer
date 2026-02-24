#ifndef VEC3_H
#define VEC3_H
#include <cmath>
#include <iostream>

class Vec3
{
public:
    double coord_3d[3];
    
    Vec3() : coord_3d{0, 0, 0}{}
    Vec3(double coord_h, double coord_v, double coord_d) : coord_3d{coord_h, coord_v, coord_d}{}

    double get_h() const
    {
        return coord_3d[0];
    }

    double get_v() const
    {
        return coord_3d[1];
    }

    double get_d() const
    {
        return coord_3d[2];
    }

    Vec3 &operator+=(const Vec3 &v)
    {
        coord_3d[0] += v.coord_3d[0];
        coord_3d[1] += v.coord_3d[1];
        coord_3d[2] += v.coord_3d[2];
        return *this;
    }

    double length_squared() const { return coord_3d[0] * coord_3d[0] + coord_3d[1] * coord_3d[1] + coord_3d[2] * coord_3d[2]; }
    double length() const { return std::sqrt(length_squared()); }
};

inline Vec3 operator+(const Vec3 &u, const Vec3 &v) { return Vec3(u.coord_3d[0] + v.coord_3d[0], u.coord_3d[1] + v.coord_3d[1], u.coord_3d[2] + v.coord_3d[2]); }
inline Vec3 operator-(const Vec3 &u, const Vec3 &v) { return Vec3(u.coord_3d[0] - v.coord_3d[0], u.coord_3d[1] - v.coord_3d[1], u.coord_3d[2] - v.coord_3d[2]); }
inline Vec3 operator*(double t, const Vec3 &v) { return Vec3(t * v.coord_3d[0], t * v.coord_3d[1], t * v.coord_3d[2]); }
inline double dot(const Vec3 &u, const Vec3 &v) { return u.coord_3d[0] * v.coord_3d[0] + u.coord_3d[1] * v.coord_3d[1] + u.coord_3d[2] * v.coord_3d[2]; }
inline Vec3 unit_vector(Vec3 v) { return (1.0 / v.length()) * v; } // KEY FOR 3D DEPTH
#endif
