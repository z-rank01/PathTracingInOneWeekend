#ifndef CAMERA_H
#define CAMERA_H

#include <chrono>
#include "utility.h"
#include "color.h"
#include "object.h"
#include "material.h"
#include "pdf.h"
#include "quad.h"

#include <iostream>

class camera
{
    public:

        // define by user

        double vfov               = 90;   // Vertical field of view
        point3 lookfrom           = point3(0, 0, -1);
        point3 lookat             = point3(0, 0, 0);
        vec3 vup                  = vec3(0, 1, 0);

        double aspect_ratio       = 1.0;  // Ratio of image width over height
        int    img_width          = 100;  // Rendered image width in pixel count
        int    samples_per_pixel  = 10;   // Count of random samples for each pixel
        int    sample_max_depth   = 10;   // Maximum number of ray bounces into scene

        double defocus_angle = 0;
        double focus_distance = 10;

        color  background;                // Scene background color


        // render

        void render(const object& world, const object& lights)
        {
            // timer

            auto start = std::chrono::steady_clock::now();
            
            // initialize output window

            initialize();

            // output imga

            std::cout << "P3\n" << img_width << ' ' << img_height << "\n255\n";

            for(int j = 0; j < img_height; ++j)
            {
                std::clog << "\rScanlines remaining: " << (img_height - j) << ' ' << std::flush;
                for (int i = 0; i < img_width; ++i)
                {
                    color pixel_color(0, 0, 0);

                    // multi-sampling

                    for(int sample = 0; sample < samples_per_pixel; ++sample)
                    {
                        // cast ray
                        ray r = cast_cay(i, j);  // each casted ray randomly offset from center location

                        // trace ray
                        pixel_color += trace(r, sample_max_depth, world, lights);
                    }

                    // write pixel color to image

                    pixel_to_image(std::cout, pixel_color, samples_per_pixel);
                }
            }
            auto end = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
            std::clog << "\rDone with " << duration.count() << "s                  " << std::endl;
        }



    private:
        int img_height;
        point3 center;
        point3 pixel00_loc;
        vec3 viewport_delta_u;
        vec3 viewport_delta_v;
        vec3 u, v, w;
        vec3 defocus_disk_u;
        vec3 defocus_disk_v;

        // a camera maintains image and viewport.

        void initialize() 
        {
            // image definition
            
            img_height = static_cast<int>(img_width / aspect_ratio); // this will lead to double-int conversion.
            img_height = (img_height < 1) ? 1 : img_height;


            // camera center
            
            center = lookfrom;


            // viewport definition

            // auto focal_length = (lookfrom - lookat).length();
            auto theta = degrees_to_radians(vfov);
            auto h = tan(theta / 2);
            auto viewport_height = 2.0 * h * focus_distance;   // define viewport height through fov and focus distance
            auto viewport_width = viewport_height * (static_cast<double>(img_width) / img_height); // use true aspect_ratio(considering the conversion)


            // camera space basis vectors
            w = unit_vector(lookfrom - lookat);
            u = unit_vector(cross(vup, w));
            v = cross(w, u);


            // get viewport coordinates based on camera space
            vec3 viewport_u = viewport_width * u;
            vec3 viewport_v = viewport_height * - v;

            // differential distance between pixels.
            viewport_delta_u = viewport_u / img_width;
            viewport_delta_v = viewport_v / img_height; // we define upper-left as the viewport origin.

            // pixel location in 3D world space.
            auto viewport_upper_left = center - (focus_distance * w) - viewport_u / 2 - viewport_v / 2;
            pixel00_loc = viewport_upper_left + 0.5 * (viewport_delta_u + viewport_delta_v);

            // defocus disk basis vectors
            auto defocus_radius = focus_distance * tan(degrees_to_radians(defocus_angle / 2));
            defocus_disk_u = u * defocus_radius;
            defocus_disk_v = v * defocus_radius;
        }

        color trace(const ray& r, int depth, const object& world, const object& lights) const 
        {
            intersect_record rec;
            
            if (depth <= 0)
            {
                return color(0, 0, 0);
            }
            

            if(!world.intersect(r, interval(0.001, infinity), rec))
            {
                return background;
            }

            // gather color contribution
            
            ray r_bounce;
            color albedo;
            double pdf;

            // direct

            color c_dir = rec.mat->emitted(rec, r, rec.u, rec.v, rec.p);


            // indirect

            
            // if there are no indirect contributions return only direct contributions
            if (!rec.mat->scatter(rec, r, r_bounce, albedo, pdf))
            {
                return c_dir;
            }

            // cosine diffuse and pdf
            // cosine_pdf surface_pdf(rec.normal);
            // r_bounce = ray(rec.p, surface_pdf.generate_randomDir(), r.time());
            // pdf = surface_pdf.get_value(r_bounce.direction());

            // light source only and pdf
            // object_pdf light_pdf(lights, rec.p);
            // r_bounce = ray(rec.p, light_pdf.generate_randomDir(), r.time());
            // pdf = light_pdf.get_value(r_bounce.direction());

            // mixture pdf: light and surface(cosine)
            auto p0 = make_shared<object_pdf>(lights, rec.p);  // light source pdf
            auto p1 = make_shared<cosine_pdf>(rec.normal);     // cosine surface pdf
            mixture_pdf mixed_pdf(p0, p1);

            r_bounce = ray(rec.p, mixed_pdf.generate_randomDir(), r.time());
            pdf = mixed_pdf.get_value(r_bounce.direction());

            auto scattering_pdf = rec.mat->scattering_pdf(rec, r, r_bounce);

            // else we keep tracing on
            color c_indir = (albedo * scattering_pdf * trace(r_bounce, depth - 1, world, lights)) / pdf;

            return c_dir + c_indir;
        }

        ray cast_cay(int i, int j)
        {
            // get random point on a pixel

            auto pixel_center = pixel00_loc + (i * viewport_delta_u) + (j * viewport_delta_v);
            auto pixel_sample = pixel_center + pixel_random_sample();  // center + offset (random).

            // construct ray

            // auto ray_origin = center;
            auto ray_origin = (defocus_angle < 0) ? center : defocus_disk_sample();  // randomly shooting ray on a disk (act as a camera len) instead of a point
            auto ray_direction = pixel_sample - ray_origin;
            auto ray_time = random_double();   /// randomly generate a shutter time for rendering 

            return ray(ray_origin, ray_direction, ray_time);
        }

        point3 defocus_disk_sample() const 
        {
            // Returns a random point in the camera defocus disk.
            auto p = randomSample_unit_disk();
            return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
        }

        vec3 pixel_random_sample()
        {
            auto px = -0.5 + random_double();
            auto py = -0.5 + random_double();
            return (px * viewport_delta_u + py * viewport_delta_v);
        }
};


#endif //CAMERA_H