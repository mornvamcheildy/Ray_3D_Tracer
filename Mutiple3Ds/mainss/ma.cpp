#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector> // NEW: To hold our list of spheres
#include <cmath>
#include <algorithm>
#include "Vec3.h"
#include "Ray.h"
#include "FileHelper.h"
#include "Random.h"
#include "Camera.h"

// Struct to represent a Sphere in our "Space"
struct Sphere {
    Vec3 center;
    double radius;
    Vec3 albedo; // The actual color of the stone (e.g., Grey)
};

// Function to check if a ray hits a specific sphere
double hit_sphere_obj(const Sphere& s, const Ray& r) {
    Vec3 oc = r.origin_getter() - s.center;
    auto a = r.direction_getter().length_squared();
    auto half_b = dot(oc, r.direction_getter());
    auto c = oc.length_squared() - s.radius * s.radius;
    auto discriminant = half_b * half_b - a * c;
    if (discriminant < 0) return -1.0;
    return (-half_b - sqrt(discriminant)) / a;
}

// Random point for Matte/Stone scattering
Vec3 random_in_unit_sphere() {
    while (true) {
        Vec3 p = Vec3(2.0 * random_double() - 1, 2.0 * random_double() - 1, 2.0 * random_double() - 1);
        if (p.length_squared() >= 1) continue;
        return p;
    }
}

// THE SHADER: Loops through the "World" to find the closest hit
Vec3 ray_color(const Ray& r, const std::vector<Sphere>& world, int depth, double lean) {
    if (depth <= 0) return Vec3(0, 0, 0);

    double closest_so_far = 999999.0;
    bool hit_anything = false;
    Sphere closest_sphere;

    // Check every object in the world
    for (const auto& s : world) {
        double t = hit_sphere_obj(s, r);
        if (t > 0.001 && t < closest_so_far) {
            hit_anything = true;
            closest_so_far = t;
            closest_sphere = s;
        }
    }

    if (hit_anything) {
        Vec3 hit_point = r.at_poin_getter(closest_so_far);
        Vec3 N = unit_vector(hit_point - closest_sphere.center);

        // STONE BOUNCE: Light scatters off the rough surface
        Vec3 target = hit_point + N + random_in_unit_sphere();
        
        // Color = (Surface Color) * (Bounce Reduction) * (Next Ray Color)
        // Multiplying by lean allows you to brighten the overall stone texture
        return lean * closest_sphere.albedo * ray_color(Ray(hit_point, target - hit_point), world, depth - 1, lean);
    }

    // BACKGROUND: Sky Gradient
    Vec3 unit_dir = unit_vector(r.direction_getter());
    double a = 0.5 * (unit_dir.get_v() + 1.0);
    return (1.0 - a) * Vec3(1.0, 1.0, 1.0) + a * Vec3(0.5, 0.7, 1.0);
}

int main() {
    std::string f_n = FileHelper::file_name();
    Camera cam(Vec3(0, 0, 0));
    int i_w = cam.i_w; int i_h = cam.i_h;

    std::cout << "Enter brightness/lean (1.0 is standard): ";
    double lean; std::cin >> lean;

    // --- CREATE THE WORLD ---
    std::vector<Sphere> world;
    // The "Stone" Sphere
    world.push_back({Vec3(0, 0, -1), 0.5, Vec3(0.5, 0.5, 0.5)}); 
    // The "Floor" Sphere (Massive sphere creates the shadow under the stone)
    world.push_back({Vec3(0, -100.5, -1), 100.0, Vec3(0.3, 0.6, 0.3)}); 

    std::ofstream i_f(f_n);
    i_f << "P3\n" << i_w << ' ' << i_h << "\n255\n";

    const int i_e_s = 50; // Samples (Anti-Aliasing)
    const int max_depth = 10;

    for (int j = i_h - 1; j >= 0; --j) {
        std::cerr << "\rRendering Row: " << j << "   " << std::flush;
        for (int i = 0; i < i_w; ++i) {
            Vec3 accumulated_color(0, 0, 0);
            for (int s = 0; s < i_e_s; ++s) {
                double u = (double(i) + random_double()) / (i_w - 1);
                double v = (double(j) + random_double()) / (i_h - 1);
                accumulated_color += ray_color(cam.get_ray(u, v), world, max_depth, lean);
            }

            double scale = 1.0 / i_e_s;
            // Gamma Correction (sqrt) makes the stone highlights pop
            i_f << static_cast<int>(256 * std::clamp(sqrt(scale * accumulated_color.get_h()), 0.0, 0.999)) << ' '
                << static_cast<int>(256 * std::clamp(sqrt(scale * accumulated_color.get_v()), 0.0, 0.999)) << ' '
                << static_cast<int>(256 * std::clamp(sqrt(scale * accumulated_color.get_d()), 0.0, 0.999)) << '\n';
        }
    }
    return 0;
}
