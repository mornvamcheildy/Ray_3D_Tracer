#include <iostream>
#include <fstream>
#include "Vec3.h"
#include "Ray.h"
#include "FileHelper.h"
#include "Random.h"
#include "Camera.h"

double hit_sphere(const Vec3 &center, double radius, const Ray &r)
{
    Vec3 oc = r.origin_getter() - center;
    auto a = dot(r.direction_getter(), r.direction_getter());
    auto b = 2.0 * dot(oc, r.direction_getter());
    auto c = dot(oc, oc) - radius * radius;
    auto discriminant = b * b - 4 * a * c;
    return (discriminant < 0) ? -1.0 : (-b - sqrt(discriminant)) / (2.0 * a);
}

int main()
{
    // 1. Get the dynamic filename from your FileHelper
    std::string f_n = FileHelper::file_name();

    // 2. Initialize Camera and extract its properties
    Camera cam(Vec3(0, 0, 0));

    // Use the values directly from the cam object to avoid magic numbers
    int i_w = cam.i_w;
    int i_h = cam.i_h;

    std::cout << "Enter contrast/lean value: ";
    double lean;
    std::cin >> lean;

    std::ofstream i_f(f_n);
    i_f << "P3\n"
        << i_w << ' ' << i_h << "\n255\n";

    const int i_e_s = 100; // Samples per pixel for anti-aliasing

    for (int j = i_h - 1; j >= 0; --j)
    {
        // Add a progress indicator so you know the render is working
        std::cerr << "\rScanlines remaining: " << j << ' ' << std::flush;

        for (int i = 0; i < i_w; ++i)
        {
            Vec3 accumulated_color(0, 0, 0);

            for (int s = 0; s < i_e_s; ++s)
            {
                // Correctly map pixel coordinates to 0.0 - 1.0 range
                double u = (double(i) + random_double()) / (i_w - 1);
                double v = (double(j) + random_double()) / (i_h - 1);

                // Use the camera class to generate the ray
                Ray r = cam.get_ray(u, v);

                double t = hit_sphere(Vec3(0, 0, -1), 0.5, r);
                Vec3 current_sample_color;

                if (t > 0.001)
                {
                    Vec3 N = unit_vector(r.at_poin_getter(t) - Vec3(0, 0, -1));
                    // Apply the 'lean' contrast value to the normal coloring
                    current_sample_color = lean * Vec3(N.get_h() + 1, N.get_v() + 1, N.get_d() + 1);
                }
                else
                {
                    Vec3 unit_dir = unit_vector(r.direction_getter());
                    double a = 0.5 * (unit_dir.get_v() + 1.0);
                    current_sample_color = (1.0 - a) * Vec3(1.0, 1.0, 1.0) + a * Vec3(0.5, 0.7, 1.0);
                }

                accumulated_color += current_sample_color;
            }

            // Average the color (Anti-aliasing)
            Vec3 pixel_color = (1.0 / i_e_s) * accumulated_color;

            i_f << static_cast<int>(255.999 * pixel_color.get_h()) << ' '
                << static_cast<int>(255.999 * pixel_color.get_v()) << ' '
                << static_cast<int>(255.999 * pixel_color.get_d()) << '\n';
        }
    }
    std::cerr << "\nDone. Image saved as: " << f_n << "\n";
}
