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
    std::string path = "./fileb";

    int current_files = FileHelper::count_files("./fileb");

    const int image_width = 500;
    const int image_height = 300;

    double asp_tio = double(image_width) / image_height;

    double viewport_h = double(image_height) / 100;
    double viewport_w = asp_tio * viewport_h;

    Camera cam(Vec3(0, 0, 0), viewport_h, asp_tio);
    double zooming_ctrl_val = cam.get_zoom();


    std::string filename = "./fileb/image" + std::to_string(current_files + 1) + ".ppm";

    std::ofstream image_file(filename);

    image_file << "P3\n"
               << image_width << ' ' << image_height << "\n255\n";

    Vec3 origin(0, 0, 0);

    const int edges_pixel_smoothness = 100;
    double lean;
    std::cin >> lean;

    for (int j = image_height - 1; j >= 0; --j)
    {
        for (int i = 0; i < image_width; ++i)
        {

            Vec3 accumulated_color(0, 0, 0);

            for (int s = 0; s < edges_pixel_smoothness; ++s)
            {
                double u = (double(i) + random_double()) / (image_width - 1);
                double v = (double(j) + random_double()) / (image_height - 1);

                Vec3 direction(viewport_w * u - (viewport_w / 2.0), viewport_h * v - (viewport_h / 2.0), -zooming_ctrl_val);

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

            Vec3 pixel_color = (1.0 / edges_pixel_smoothness) * accumulated_color;

            image_file << static_cast<int>(255.999 * pixel_color.get_h()) << ' '
                       << static_cast<int>(255.999 * pixel_color.get_v()) << ' '
                       << static_cast<int>(255.999 * pixel_color.get_d()) << '\n';
        }
    }
}
