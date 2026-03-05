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

// --- MATERIAL & OBJECT STRUCTURE ---
struct Sphere {
    Vec3 center;
    double radius;
    Vec3 albedo; 
    bool is_metal = false;
    bool is_glass = false;
    bool is_light = false; // NEW: The object emits light (The Sun)
    double fuzz = 0.0;     
    double ref_idx = 1.0;  
};

// --- MATH UTILITIES ---

Vec3 reflect(const Vec3& v, const Vec3& n) {
    return v - 2 * dot(v, n) * n;
}

Vec3 refract(const Vec3& uv, const Vec3& n, double etai_over_etat) {
    auto cos_theta = fmin(dot(-1.0 * uv, n), 1.0);
    Vec3 r_out_perp =  etai_over_etat * (uv + cos_theta * n);
    Vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

double reflectance(double cosine, double ref_idx) {
    auto r0 = (1 - ref_idx) / (1 + ref_idx);
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}

Vec3 random_in_unit_sphere() {
    while (true) {
        Vec3 p = Vec3(2.0 * random_double() - 1, 2.0 * random_double() - 1, 2.0 * random_double() - 1);
        if (p.length_squared() >= 1) continue;
        return p;
    }
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

// --- THE SHADER (RAY COLOR) ---
Vec3 ray_color(const Ray& r, const std::vector<Sphere>& world, int depth) {
    if (depth <= 0) return Vec3(0, 0, 0); 

    double closest_so_far = 999999.0;
    bool hit_anything = false;
    Sphere s_hit;

    for (const auto& s : world) {
        double t = hit_sphere_obj(s, r);
        if (t > 0.001 && t < closest_so_far) {
            hit_anything = true;
            closest_so_far = t;
            s_hit = s;
        }
    }

    if (hit_anything) {
        // EMISSIVE LIGHT LOGIC: If we hit the Sun, return its bright light
        if (s_hit.is_light) return s_hit.albedo;

        Vec3 hit_pt = r.at_poin_getter(closest_so_far);
        Vec3 N = unit_vector(hit_pt - s_hit.center);
        Vec3 unit_dir = unit_vector(r.direction_getter());
        Ray scattered;
        Vec3 attenuation = s_hit.albedo;

        if (s_hit.is_glass) {
            double refr_ratio = (dot(unit_dir, N) < 0) ? (1.0 / s_hit.ref_idx) : s_hit.ref_idx;
            double cos_theta = fmin(dot(-1.0 * unit_dir, N), 1.0);
            double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
            bool cannot_refract = refr_ratio * sin_theta > 1.0;
            Vec3 direction;
            if (cannot_refract || reflectance(cos_theta, refr_ratio) > random_double())
                direction = reflect(unit_dir, N);
            else
                direction = refract(unit_dir, N, refr_ratio);
            scattered = Ray(hit_pt, direction);
        }
        else if (s_hit.is_metal) {
            Vec3 reflected = reflect(unit_dir, N);
            scattered = Ray(hit_pt, reflected + s_hit.fuzz * random_in_unit_sphere());
            if (dot(scattered.direction_getter(), N) <= 0) return Vec3(0,0,0);
        }
        else {
            // MATTE STONE
            Vec3 target = hit_pt + N + random_in_unit_sphere();
            scattered = Ray(hit_pt, target - hit_pt);
            attenuation = 0.5 * s_hit.albedo; 
        }

        return attenuation * ray_color(scattered, world, depth - 1);
    }

    // DEEP SPACE: Return pure black if nothing is hit
    return Vec3(0, 0, 0);
}

// --- MAIN RENDER LOOP ---
int main() {
    std::string f_n = FileHelper::file_name();
    Camera cam(Vec3(0, 0, 0));

    std::vector<Sphere> world;
    
    // 1. THE SUN (Bright Light at a distance)
    world.push_back({Vec3(10, 10, 5), 3.0, Vec3(12, 12, 12), false, false, true});

    // 2. MATTE ASTEROID (Bottom)
    world.push_back({Vec3(0, -100.5, -1), 100.0, Vec3(0.4, 0.4, 0.4), false, false, false});
    
    // 3. CRYSTAL BALL (Center)
    world.push_back({Vec3(0, 0, -1), 0.5, Vec3(1.0, 1.0, 1.0), false, true, false, 0.0, 1.5});
    
    // 4. GOLD SPACE HULL (Right - Fuzzy Metal)
    world.push_back({Vec3(1.1, 0, -1.2), 0.5, Vec3(0.8, 0.6, 0.2), true, false, false, 0.1});

    std::ofstream i_f(f_n);
    i_f << "P3\n" << cam.i_w << ' ' << cam.i_h << "\n255\n";

    const int samples = 100; // High quality anti-aliasing
    const int max_depth = 20;

    for (int j = cam.i_h - 1; j >= 0; --j) {
        std::cerr << "\rRemaining Scanlines: " << j << " " << std::flush;
        for (int i = 0; i < cam.i_w; ++i) {
            Vec3 pixel_color(0, 0, 0);
            for (int s = 0; s < samples; ++s) {
                double u = (double(i) + random_double()) / (cam.i_w - 1);
                double v = (double(j) + random_double()) / (cam.i_h - 1);
                pixel_color += ray_color(cam.get_ray(u, v), world, max_depth);
            }

            double scale = 1.0 / samples;
            i_f << static_cast<int>(256 * std::clamp(sqrt(scale * pixel_color.get_h()), 0.0, 0.999)) << ' '
                << static_cast<int>(256 * std::clamp(sqrt(scale * pixel_color.get_v()), 0.0, 0.999)) << ' '
                << static_cast<int>(256 * std::clamp(sqrt(scale * pixel_color.get_d()), 0.0, 0.999)) << '\n';
        }
    }
    std::cerr << "\nRender Finished.\n";
    return 0;
}
