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


// --- SHAPES ---
struct Sphere {
    Vec3 center;
    double radius;
    Vec3 albedo; 
    bool is_metal;
    double fuzz;
};

struct Triangle {
    Vec3 v0, v1, v2;
    Vec3 albedo;
    bool is_metal;
    double fuzz;
};

// --- INTERSECTION MATH ---

double hit_sphere_obj(const Sphere& s, const Ray& r) {
    Vec3 oc = r.origin_getter() - s.center;
    auto a = r.direction_getter().length_squared();
    auto half_b = dot(oc, r.direction_getter());
    auto c = oc.length_squared() - s.radius * s.radius;
    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) return -1.0;
    return (-half_b - sqrt(discriminant)) / a;
}

// Möller–Trumbore Triangle Intersection Algorithm
double hit_triangle_obj(const Triangle& tri, const Ray& r, Vec3& out_normal) {
    const double EPSILON = 0.0000001;
    Vec3 edge1 = tri.v1 - tri.v0;
    Vec3 edge2 = tri.v2 - tri.v0;
    Vec3 h = cross(r.direction_getter(), edge2);
    double a = dot(edge1, h);

    if (a > -EPSILON && a < EPSILON) return -1.0; // Ray is parallel

    double f = 1.0 / a;
    Vec3 s = r.origin_getter() - tri.v0;
    double u = f * dot(s, h);
    if (u < 0.0 || u > 1.0) return -1.0;

    Vec3 q = cross(s, edge1);
    double v = f * dot(r.direction_getter(), q);
    if (v < 0.0 || u + v > 1.0) return -1.0;

    double t = f * dot(edge2, q);
    if (t > EPSILON) {
        out_normal = unit_vector(cross(edge1, edge2));
        return t;
    }
    return -1.0;
}

// --- SHADING & MATERIALS ---

Vec3 reflect(const Vec3& v, const Vec3& n) {
    return v - 2 * dot(v, n) * n;
}

Vec3 random_in_unit_sphere() {
    while (true) {
        Vec3 p = Vec3(2.0 * random_double() - 1, 2.0 * random_double() - 1, 2.0 * random_double() - 1);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

Vec3 ray_color(const Ray& r, const std::vector<Sphere>& spheres, const std::vector<Triangle>& tris, int depth) {
    if (depth <= 0) return Vec3(0, 0, 0);

    double closest_t = 999999.0;
    bool hit_anything = false;
    Vec3 N, albedo;
    bool is_metal = false;
    double fuzz = 0.0;

    // Check Spheres
    for (const auto& s : spheres) {
        double t = hit_sphere_obj(s, r);
        if (t > 0.001 && t < closest_t) {
            hit_anything = true;
            closest_t = t;
            N = unit_vector(r.at_poin_getter(t) - s.center);
            albedo = s.albedo;
            is_metal = s.is_metal;
            fuzz = s.fuzz;
        }
    }

    // Check Triangles
    for (const auto& tri : tris) {
        Vec3 tri_normal;
        double t = hit_triangle_obj(tri, r, tri_normal);
        if (t > 0.001 && t < closest_t) {
            hit_anything = true;
            closest_t = t;
            N = tri_normal;
            albedo = tri.albedo;
            is_metal = tri.is_metal;
            fuzz = tri.fuzz;
        }
    }

    if (hit_anything) {
        Ray scattered;
        Vec3 hit_pt = r.at_poin_getter(closest_t);
        if (is_metal) {
            Vec3 reflected = reflect(unit_vector(r.direction_getter()), N);
            scattered = Ray(hit_pt, reflected + fuzz * random_in_unit_sphere());
        } else {
            Vec3 target = hit_pt + N + random_in_unit_sphere();
            scattered = Ray(hit_pt, target - hit_pt);
        }
        double attenuation = is_metal ? 0.9 : 0.5;
        return attenuation * albedo * ray_color(scattered, spheres, tris, depth - 1);
    }

    // Sky Background
    Vec3 unit_dir = unit_vector(r.direction_getter());
    double a = 0.5 * (unit_dir.get_v() + 1.0);
    return (1.0 - a) * Vec3(1.0, 1.0, 1.0) + a * Vec3(0.5, 0.7, 1.0);
}

int main() {
    std::string f_n = FileHelper::file_name();
    Camera cam(Vec3(0, 0, 0));

    // WORLD CONTAINERS
    std::vector<Sphere> spheres;
    std::vector<Triangle> triangles;

    // Add a Stone Sphere
    spheres.push_back({Vec3(0, 0, -1), 0.5, Vec3(0.1, 0.2, 0.5), false, 0.0});
    // Add a Metal Ground Sphere
    spheres.push_back({Vec3(0, -100.5, -1), 100.0, Vec3(0.8, 0.8, 0.8), true, 0.1});

    // --- ADDING TRIANGLES (The Pyramid) ---
    // A Red Metal Triangle pointing up
    triangles.push_back({Vec3(-1, 0, -1.5), Vec3(1, 0, -1.5), Vec3(0, 1.5, -1.5), Vec3(0.8, 0.1, 0.1), true, 0.0});

    std::ofstream i_f(f_n);
    i_f << "P3\n" << cam.i_w << ' ' << cam.i_h << "\n255\n";

    for (int j = cam.i_h - 1; j >= 0; --j) {
        std::cerr << "\rScanlines remaining: " << j << "   " << std::flush;
        for (int i = 0; i < cam.i_w; ++i) {
            Vec3 col(0, 0, 0);
            for (int s = 0; s < 50; ++s) {
                double u = (double(i) + random_double()) / (cam.i_w - 1);
                double v = (double(j) + random_double()) / (cam.i_h - 1);
                col += ray_color(cam.get_ray(u, v), spheres, triangles, 10);
            }
            double scale = 1.0 / 50.0;
            i_f << static_cast<int>(256 * std::clamp(sqrt(scale * col.get_h()), 0.0, 0.999)) << ' '
                << static_cast<int>(256 * std::clamp(sqrt(scale * col.get_v()), 0.0, 0.999)) << ' '
                << static_cast<int>(256 * std::clamp(sqrt(scale * col.get_d()), 0.0, 0.999)) << '\n';
        }
    }
    return 0;
}
