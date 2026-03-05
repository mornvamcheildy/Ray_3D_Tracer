#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include "Vec3.h"
#include "Ray.h"
#include "FileHelper.h"
#include "Random.h"
#include "Camera.h"

// --- MATERIAL TOOLS ---

// Generates a random point inside a unit sphere
// This is the "engine" that creates the soft, matte look of stone.
Vec3 random_in_unit_sphere()
{
    while (true)
    {
        // Generates a point in a cube from -1 to 1 on all axes
        Vec3 p = Vec3(2.0 * random_double() - 1, 2.0 * random_double() - 1, 2.0 * random_double() - 1);
        if (p.length_squared() >= 1)
            continue; // Reject points outside the sphere to keep distribution uniform
        return p;
    }
}

// Function to check if a ray hits a sphere
double hit_sphere(const Vec3 &center, double radius, const Ray &r)
{
    Vec3 oc = r.origin_getter() - center;
    auto a = r.direction_getter().length_squared();
    auto half_b = dot(oc, r.direction_getter());
    auto c = oc.length_squared() - radius * radius;
    auto discriminant = half_b * half_b - a * c;

    if (discriminant < 0)
        return -1.0;
    return (-half_b - sqrt(discriminant)) / a;
}

// The "Shader": Recursively decides what color a ray sees
Vec3 ray_color(const Ray &r, int depth, double lean)
{
    if (depth <= 0)
        return Vec3(0, 0, 0);

    // 1. THE STONE (Small sphere)
    Vec3 stone_center(0, 0, -1);
    double t_stone = hit_sphere(stone_center, 0.5, r);

    // 2. THE GROUND (Massive sphere below)
    // This is what creates the shadow that makes it look like stone!
    Vec3 ground_center(0, -100.5, -1);
    double t_ground = hit_sphere(ground_center, 100.0, r);

    // Identify the closest hit
    double t_hit = -1.0;
    Vec3 current_center;
    Vec3 albedo; // The color of the material

    if (t_stone > 0.001 && (t_ground < 0.001 || t_stone < t_ground))
    {
        t_hit = t_stone;
        current_center = stone_center;
        albedo = Vec3(0.5, 0.5, 0.5); // Grey Stone color
    }
    else if (t_ground > 0.001)
    {
        t_hit = t_ground;
        current_center = ground_center;
        albedo = Vec3(0.8, 0.8, 0.0); // Yellowish dirt/sand
    }

    if (t_hit > 0.001)
    {
        Vec3 hit_point = r.at_poin_getter(t_hit);
        Vec3 N = unit_vector(hit_point - current_center);

        // DIFFUSE BOUNCE (The "Matte" math)
        // Light hits the stone and scatters in a random direction
        Vec3 target = hit_point + N + random_in_unit_sphere();

        // Final color = Surface Tint (Albedo) * Intensity (Lean) * Bounced light
        return lean * albedo * ray_color(Ray(hit_point, target - hit_point), depth - 1, lean);
    }

    // BACKGROUND (Sky)
    Vec3 unit_dir = unit_vector(r.direction_getter());
    double a = 0.5 * (unit_dir.get_v() + 1.0);
    return (1.0 - a) * Vec3(1.0, 1.0, 1.0) + a * Vec3(0.5, 0.7, 1.0);
}

int main()
{
    // Initialize Camera and File
    std::string f_n = FileHelper::file_name();
    Camera cam(Vec3(0, 0, 0));

    int i_w = cam.i_w;
    int i_h = cam.i_h;

    std::cout << "--- RAYTRACER INITIALIZED ---\n";
    std::cout << "Target File: " << f_n << "\n";
    std::cout << "Resolution : " << i_w << "x" << i_h << "\n";

    // User input for surface reflectivity
    std::cout << "Enter material reflectivity/lean (0.5 is standard matte): ";
    double lean;
    std::cin >> lean;

    std::ofstream i_f(f_n);
    if (!i_f.is_open())
    {
        std::cerr << "Error: Could not create the file " << f_n << "\n";
        return 1;
    }

    // PPM Header
    i_f << "P3\n"
        << i_w << ' ' << i_h << "\n255\n";

    const int i_e_s = 100;    // Samples per pixel (Anti-Aliasing)
    const int max_depth = 10; // Ray bounce depth limit

    for (int j = i_h - 1; j >= 0; --j)
    {
        // Progress tracker
        std::cerr << "\rRemaining Rows: " << std::setw(4) << j << " " << std::flush;

        for (int i = 0; i < i_w; ++i)
        {
            Vec3 accumulated_color(0, 0, 0);

            for (int s = 0; s < i_e_s; ++s)
            {
                double u = (double(i) + random_double()) / (i_w - 1);
                double v = (double(j) + random_double()) / (i_h - 1);

                Ray r = cam.get_ray(u, v);
                accumulated_color += ray_color(r, max_depth, lean);
            }

            // GAMMA CORRECTION & AVERAGING
            double scale = 1.0 / i_e_s;

            // Standard Gamma 2.0 (sqrt) transformation
            double r_col = sqrt(scale * accumulated_color.get_h());
            double g_col = sqrt(scale * accumulated_color.get_v());
            double b_col = sqrt(scale * accumulated_color.get_d());

            // Final Write with clamping safety
            i_f << static_cast<int>(256 * std::clamp(r_col, 0.0, 0.999)) << ' '
                << static_cast<int>(256 * std::clamp(g_col, 0.0, 0.999)) << ' '
                << static_cast<int>(256 * std::clamp(b_col, 0.0, 0.999)) << '\n';
        }
    }

    std::cout << "\n\n--- RENDER COMPLETE ---\n";
    std::cout << "File saved to: " << f_n << "\n";

    return 0;
}
