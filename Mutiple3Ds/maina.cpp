#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <algorithm>
#include "Vec3.h"
#include "Ray.h"
#include "FileHelper.h"
#include "Random.h"
#include "Camera.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- ROTATION ---
Vec3 rotate_y(const Vec3 &p, double angle_degrees)
{
    double radians = angle_degrees * M_PI / 180.0;
    double s = sin(radians);
    double c = cos(radians);
    return Vec3(c * p.get_h() + s * p.get_d(), p.get_v(), -s * p.get_h() + c * p.get_d());
}

// --- SHAPES ---
struct Sphere
{
    Vec3 center;
    double radius;
    Vec3 albedo;
    bool is_metal;
    double fuzz;
};

struct Triangle
{
    Vec3 v0, v1, v2;
    Vec3 albedo;
    bool is_metal;
    double fuzz;
};

// --- MATH ---
double hit_sphere_obj(const Sphere &s, const Ray &r)
{
    Vec3 oc = r.origin_getter() - s.center;
    auto a = r.direction_getter().length_squared();
    auto half_b = dot(oc, r.direction_getter());
    auto c = oc.length_squared() - s.radius * s.radius;
    auto discriminant = half_b * half_b - a * c;
    return (discriminant < 0) ? -1.0 : (-half_b - sqrt(discriminant)) / a;
}

double hit_triangle_obj(const Triangle &tri, const Ray &r, Vec3 &normal)
{
    const double EPSILON = 0.0000001;
    Vec3 e1 = tri.v1 - tri.v0;
    Vec3 e2 = tri.v2 - tri.v0;
    Vec3 h = cross(r.direction_getter(), e2);
    double a = dot(e1, h);
    if (a > -EPSILON && a < EPSILON)
        return -1.0;
    double f = 1.0 / a;
    Vec3 s = r.origin_getter() - tri.v0;
    double u = f * dot(s, h);
    if (u < 0.0 || u > 1.0)
        return -1.0;
    Vec3 q = cross(s, e1);
    double v = f * dot(r.direction_getter(), q);
    if (v < 0.0 || u + v > 1.0)
        return -1.0;
    double t = f * dot(e2, q);
    if (t > EPSILON)
    {
        normal = unit_vector(cross(e1, e2));
        return t;
    }
    return -1.0;
}

Vec3 reflect(const Vec3 &v, const Vec3 &n) { return v - 2 * dot(v, n) * n; }

// --- SHADER ---
Vec3 ray_color(const Ray &r, const std::vector<Sphere> &spheres, int depth)
{
    if (depth <= 0)
        return Vec3(0, 0, 0);

    double closest_t = 999999.0;
    bool hit = false;
    Vec3 N, albedo;
    bool metal = false;
    double fuzz = 0.0;

    for (const auto &s : spheres)
    {
        double t = hit_sphere_obj(s, r);
        if (t > 0.001 && t < closest_t)
        {
            hit = true;
            closest_t = t;
            N = unit_vector(r.at_poin_getter(t) - s.center);
            albedo = s.albedo;
            metal = s.is_metal;
            fuzz = s.fuzz;
        }
    }

    if (hit)
    {
        Vec3 hit_pt = r.at_poin_getter(closest_t);
        if (dot(r.direction_getter(), N) > 0)
            N = -1.0 * N;
        Ray scattered;
        if (metal)
        {
            Vec3 refl = reflect(unit_vector(r.direction_getter()), N);
            scattered = Ray(hit_pt, refl + fuzz * Vec3(random_double() - 0.5, random_double() - 0.5, random_double() - 0.5));
        }
        else
        {
            Vec3 target = hit_pt + N + Vec3(random_double() - 0.5, random_double() - 0.5, random_double() - 0.5);
            scattered = Ray(hit_pt, target - hit_pt);
        }
        return (metal ? 0.9 : 0.5) * albedo * ray_color(scattered, spheres, depth - 1);
    }

    // Sky Background
    Vec3 unit_dir = unit_vector(r.direction_getter());
    double a = 0.5 * (unit_dir.get_v() + 1.0);

    return (1.0 - a) * Vec3(1.0, 1.0, 1.0) + a * Vec3(0.5, 0.7, 1.0);
}

int main()
{
    std::string f_n = FileHelper::file_name();
    Camera cam(Vec3(0, 0, 1));

    std::vector<Sphere> spheres;

    // 1. Metal Floor
    spheres.push_back({Vec3(0, -100.5, -1), 100, Vec3(0.8, 0.8, 0.8), true, 0.05});

    // --- THE CLOSE-UP BLOB MAN ---
    Vec3 skin(0.8, 0.6, 0.5);  // Flesh tone
    Vec3 shirt(0.2, 0.2, 0.5); // Blue shirt

    // 1. HEAD (Closer and higher)
    spheres.push_back({Vec3(0, 0.55, -1.0), 0.18, skin, false, 0.0});

    // 2. TORSO (The main body)
    spheres.push_back({Vec3(0, 0.15, -1.0), 0.28, shirt, false, 0.0});

    // 3. RIGHT SHOULDER & ARM (3D offset: Z is slightly different for depth)
    spheres.push_back({Vec3(0.35, 0.25, -1.0), 0.12, skin, false, 0.0});
    spheres.push_back({Vec3(0.45, 0.05, -1.0), 0.10, skin, false, 0.0}); // Forearm

    // 4. LEFT SHOULDER & ARM
    spheres.push_back({Vec3(-0.35, 0.25, -1.0), 0.12, skin, false, 0.0});
    spheres.push_back({Vec3(-0.45, 0.05, -1.0), 0.10, skin, false, 0.0}); // Forearm

    // 5. LEGS (Connecting to the floor)
    spheres.push_back({Vec3(-0.15, -0.25, -1.0), 0.15, Vec3(0.1, 0.1, 0.1), false, 0.0}); // Left Leg
    spheres.push_back({Vec3(0.15, -0.25, -1.0), 0.15, Vec3(0.1, 0.1, 0.1), false, 0.0});  // Right Leg

    std::ofstream i_f(f_n);
    i_f << "P3\n"
        << cam.i_w << ' ' << cam.i_h << "\n255\n";

    for (int j = cam.i_h - 1; j >= 0; --j)
    {
        for (int i = 0; i < cam.i_w; ++i)
        {
            Vec3 color(0, 0, 0);
            for (int s = 0; s < 100; ++s)
            {
                double u = (double(i) + random_double()) / (cam.i_w - 1);
                double v = (double(j) + random_double()) / (cam.i_h - 1);
                color += ray_color(cam.get_ray(u, v), spheres, 10);
            }

            double sc = 1.0 / 100.0;
            i_f << (int)(256 * std::clamp(sqrt(sc * color.get_h()), 0.0, 0.999)) << ' '
                << (int)(256 * std::clamp(sqrt(sc * color.get_v()), 0.0, 0.999)) << ' '
                << (int)(256 * std::clamp(sqrt(sc * color.get_d()), 0.0, 0.999)) << '\n';
        }

        std::cerr << "\rRows left: " << j << "   " << std::flush;
    }
    return 0;
}
