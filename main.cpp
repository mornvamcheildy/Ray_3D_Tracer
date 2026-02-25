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

    // Calling FileHelper class to get name for the next file
    std::string f_n = FileHelper::file_name();


    Camera cam(Vec3(0, 0, 0));

    double z_c_v = cam.get_zm(); // Zooming Control Value
    int i_h = cam.i_h; // Image Height Size value
    int i_w = cam.i_w; // Image Width Size value
    int v_h = cam.v_h; // Viewport Height Size value
    int v_w = cam.v_w; // Viewport Height Size value

    std::ofstream i_f(f_n);// image file

    i_f << "P3\n"
        << i_w << ' ' << i_h << "\n255\n";

    Vec3 origin(0, 0, 0);

    const int i_e_s = 100; // making the image edge smoother instead of stairs from 0 - 100
    double lean; // image contrast len value cin input holder
    std::cin >> lean;

    for (int j = i_h - 1; j >= 0; --j)
    {
        for (int i = 0; i < i_w; ++i)
        {

            Vec3 accumulated_color(0, 0, 0);

            for (int s = 0; s < i_e_s; ++s)
            {
                double u = (double(i) + random_double()) / (i_w - 1);
                double v = (double(j) + random_double()) / (i_h - 1);

                Vec3 direction(v_w * u - (v_w / 2.0), v_h * v - (v_h / 2.0), -z_c_v);

                Ray r = cam.get_ray(u, v);

                double t = hit_sphere(Vec3(0, 0, -1), 0.5, r);
                Vec3 current_sample_color;

                if (t > 0.001)
                {
                    Vec3 N = unit_vector(r.at_poin_getter(t) - Vec3(0, 0, -1));
                    current_sample_color = double(lean) * Vec3(N.get_h() + 1, N.get_v() + 1, N.get_d() + 1);
                }
                else
                {
                    Vec3 unit_dir = unit_vector(r.direction_getter());
                    double a = 0.5 * (unit_dir.get_v() + 1.0);
                    current_sample_color = (1.0 - a) * Vec3(1.0, 1.0, 1.0) + a * Vec3(0.5, 0.7, 1.0);
                }

                accumulated_color += current_sample_color;
            }

            Vec3 pixel_color = (1.0 / i_e_s) * accumulated_color;

            i_f << static_cast<int>(255.999 * pixel_color.get_h()) << ' '
                << static_cast<int>(255.999 * pixel_color.get_v()) << ' '
                << static_cast<int>(255.999 * pixel_color.get_d()) << '\n';
        }
    }
}
