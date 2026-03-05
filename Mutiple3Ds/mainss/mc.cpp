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

struct Sphere {
    Vec3 center;
    double radius;
    Vec3 albedo; 
    bool is_metal;
    double fuzz; 
};

Vec3 reflect(const Vec3& v, const Vec3& n) {
    return v - 2 * dot(v, n) * n;
}

double hit_sphere_obj(const Sphere& s, const Ray& r) {
    Vec3 oc = r.origin_getter() - s.center;
    auto a = r.direction_getter().length_squared();
    auto half_b = dot(oc, r.direction_getter());
    auto c = oc.length_squared() - s.radius * s.radius;
    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) return -1.0;
    return (-half_b - sqrt(discriminant)) / a;
}

Vec3 random_in_unit_sphere() {
    while (true) {
        Vec3 p = Vec3(2.0 * random_double() - 1, 2.0 * random_double() - 1, 2.0 * random_double() - 1);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

Vec3 ray_color(const Ray& r, const std::vector<Sphere>& world, int depth, double lean) {
    if (depth <= 0) return Vec3(0, 0, 0);

    double closest_so_far = 999999.0;
    bool hit_anything = false;
    Sphere closest_s;

    for (const auto& s : world) {
        double t = hit_sphere_obj(s, r);
        if (t > 0.001 && t < closest_so_far) {
            hit_anything = true;
            closest_so_far = t;
            closest_s = s;
        }
    }

    if (hit_anything) {
        Vec3 hit_point = r.at_poin_getter(closest_so_far);
        Vec3 N = unit_vector(hit_point - closest_s.center);

        Ray scattered;
        if (closest_s.is_metal) {
            Vec3 reflected = reflect(unit_vector(r.direction_getter()), N);
            // APPLY FUZZ: Mixes mirror reflection with random scattering
            Vec3 fuzzed_dir = reflected + closest_s.fuzz * random_in_unit_sphere();
            scattered = Ray(hit_point, fuzzed_dir);
            if (dot(scattered.direction_getter(), N) <= 0) return Vec3(0,0,0);
        } else {
            Vec3 target = hit_point + N + random_in_unit_sphere();
            scattered = Ray(hit_point, target - hit_point);
        }

        double attenuation = closest_s.is_metal ? 0.9 : 0.5;
        return lean * attenuation * closest_s.albedo * ray_color(scattered, world, depth - 1, lean);
    }

    Vec3 unit_dir = unit_vector(r.direction_getter());
    double a = 0.5 * (unit_dir.get_v() + 1.0);
    return (1.0 - a) * Vec3(1.0, 1.0, 1.0) + a * Vec3(0.5, 0.7, 1.0);
}

int main() {
    std::string f_n = FileHelper::file_name();
    Camera cam(Vec3(0, 0, 0));
    
    std::vector<Sphere> world;
    // 1. PERFECT MIRROR (Center)
    world.push_back({Vec3(0, 0, -1), 0.5, Vec3(0.8, 0.8, 0.8), true, 0.0}); 
    // 2. BRUSHED METAL (Right)
    world.push_back({Vec3(1.1, 0, -1.2), 0.5, Vec3(0.8, 0.6, 0.2), true, 0.3}); 
    // 3. MATTE STONE GROUND
    world.push_back({Vec3(0, -100.5, -1), 100.0, Vec3(0.5, 0.5, 0.5), false, 0.0}); 

    std::ofstream i_f(f_n);
    i_f << "P3\n" << cam.i_w << ' ' << cam.i_h << "\n255\n";

    for (int j = cam.i_h - 1; j >= 0; --j) {
        std::cerr << "\rRemaining: " << j << "   " << std::flush;
        for (int i = 0; i < cam.i_w; ++i) {
            Vec3 col(0, 0, 0);
            for (int s = 0; s < 50; ++s) {
                double u = (double(i) + random_double()) / (cam.i_w - 1);
                double v = (double(j) + random_double()) / (cam.i_h - 1);
                col += ray_color(cam.get_ray(u, v), world, 10, 1.0);
            }
            double scale = 1.0 / 50.0;
            i_f << static_cast<int>(256 * std::clamp(sqrt(scale * col.get_h()), 0.0, 0.999)) << ' '
                << static_cast<int>(256 * std::clamp(sqrt(scale * col.get_v()), 0.0, 0.999)) << ' '
                << static_cast<int>(256 * std::clamp(sqrt(scale * col.get_d()), 0.0, 0.999)) << '\n';
        }
    }
    return 0;
}
