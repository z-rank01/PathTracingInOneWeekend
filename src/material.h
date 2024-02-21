#ifndef MATERIAL_H
#define MATERIAL_H

#include "utility.h"
#include "texture.h"
#include "onb.h"

class intersect_record;

class material
{
    public:
        virtual ~material() = default;

        // virtual bool scatter(const intersect_record& rec, const ray& ray_in, ray& ray_out, color& attenuation) const = 0;
        virtual bool scatter(const intersect_record& rec, const ray& ray_in, ray& ray_out, color& albedo, double& pdf) const { return 0; };
        virtual color emitted(const intersect_record&rec, const ray& ray_in, double u, double v, const point3& p) const { return color(0,0,0); };
        virtual double scattering_pdf(const intersect_record&rec, const ray& ray_in, const ray& ray_out) const { return 0; }
};


// lambertian diffuse

class lambertian : public material
{
    public:
        // lambertian(const color& a) : albedo(a) {}
        lambertian(const color& a) : tex(std::make_shared<solid_color>(a)) {}
        lambertian(std::shared_ptr<texture> t) : tex(t) {}
        

        // bool scatter(const intersect_record& rec, const ray& ray_in, ray& ray_out, color& attenuation) const override 
        // {
        //     // auto scatter_direction = rec.normal + randomSample_unit_vector_normalize(); // lambertian diffuse
        //     auto scatter_direction = randomSample_unit_hemisphere(rec.normal);             // uniform diffuse

        //     // if randomly generated vector is opposite to the normal, 
        //     // the sum will be zero leading to bad scenario of NaNs.

        //     if (scatter_direction.near_zero())
        //         scatter_direction = rec.normal;   // if the direction is near zero, then let the normal be the direction

        //     ray_out = ray(rec.p, scatter_direction, ray_in.time());
        //     attenuation = tex->get_value(rec.u, rec.v, rec.p);

        //     return true;
        // }

        bool scatter(const intersect_record& rec, const ray& ray_in, ray& ray_out, color& albedo, double& pdf) const override 
        {
            // ---uniform hemisphere diffuse---
            // auto bounce_direction = randomSample_unit_hemisphere(rec.normal);             // uniform diffuse
            // pdf = 1 / (2 * pi);
            // // update parameter
            // ray_out = ray(rec.p, bounce_direction, ray_in.time());

            // ---lambertian diffuse---
            // auto bounce_direction = rec.normal + randomSample_unit_vector_normalize(); // lambertian diffuse
            
            // // if randomly generated vector is opposite to the normal, 
            // // the sum will be zero leading to bad scenario of NaNs.
            // if (bounce_direction.near_zero())
            //     bounce_direction = rec.normal;   // if the direction is near zero, then let the normal be the direction
            // // update parameter
            // ray_out = ray(rec.p, bounce_direction, ray_in.time());
            // auto cos_theta = dot(rec.normal, unit_vector(ray_out.direction()));
            // pdf = cos_theta < 0 ? 0 : cos_theta / pi;

            // ---cosine diffuse---
            // create a coordinate system based on normal
            onb uvw;
            uvw.build_from_w(rec.normal);

            auto bounce_direction = uvw.local(randomSample_cosine_direction());
            ray_out = ray(rec.p, bounce_direction, ray_in.time());
            pdf = dot(uvw.w(), ray_out.direction()) / pi;   // PDF for cosine diffuse = cos_theta / pi


            // update parameters

            albedo = tex->get_value(rec.u, rec.v, rec.p);

            return true;
        }

        double scattering_pdf(const intersect_record& rec, const ray& ray_in, const ray& ray_out) const override
        {
            // ---lambertian PDF---
            auto cos_theta = dot(rec.normal, unit_vector(ray_out.direction()));
            return cos_theta < 0 ? 0 : cos_theta / pi;
            
            // ---uniform hemisphere PDF---
            // return 1 / (2 * pi);

            // ---cosine PDF---
            // onb uvw;
            // uvw.build_from_w(rec.normal);
            // return dot(uvw.w(), ray_out.direction()) / pi;
        }

  private:
    std::shared_ptr<texture> tex;
    // color albedo;
};


class metal : public material
{
    public:
        metal(const color& a, double f) : albedo(a), fuzz(f) {}

        bool scatter(const intersect_record& rec, const ray& ray_in, ray& ray_out, color& alb, double& pdf) const override
        {
            auto reflected_direction = reflect(unit_vector(ray_in.direction()), rec.normal);

            ray_out = ray(rec.p, reflected_direction + fuzz * randomSample_unit_vector_normalize(), ray_in.time());  // add a fuzzy factor and current time
            alb = albedo;
            pdf = 0.0;

            return (dot(ray_out.direction(), rec.normal) > 0);
        }

    private:
        color albedo;
        double fuzz;
};


class dielectric : public material
{
    public:
        dielectric(double index_of_refraction) : ir(index_of_refraction) {}

        bool scatter(const intersect_record& rec, const ray& ray_in, ray& ray_out, color& albedo, double& pdf) const override
        {
            albedo = color(1.0, 1.0, 1.0);
            pdf = 0.0;
            double refract_ratio = rec.front_face ? (1.0 / ir) : ir;

            vec3 unit_direction = unit_vector(ray_in.direction());

            double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
            double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

            bool cannot_refract = refract_ratio * sin_theta > 1.0;   // judge if the part is refracted.
            vec3 direction;

            if (cannot_refract || reflectance(cos_theta, refract_ratio) > random_double())
            {
                direction = reflect(unit_direction, rec.normal);     // reflect if no solution to Snell's law.
            }
            else
            {
                direction = refract(unit_direction, rec.normal, refract_ratio);   // refrac if it has solution to Snell's law.
            }
            

            ray_out = ray(rec.p, direction, ray_in.time());
            return true;
        }

    private:
        double ir;

        static double reflectance(double cosine, double ref_idx) 
        {
            // Use Schlick's approximation for reflectance.
            auto r0 = (1 - ref_idx) / (1 + ref_idx);
            r0 = r0 * r0;
            return r0 + (1 - r0) * pow((1 - cosine), 5);
        }
};


class diffuse_light : public material 
{
  public:
    diffuse_light(shared_ptr<texture> a) : emit(a) {}
    diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

    bool scatter(const intersect_record& rec, const ray& r_in, ray& ray_out, color& albedo, double& pdf) const override 
    {
        return false;
    }

    color emitted(const intersect_record& rec, const ray& ray_in, double u, double v, const point3& p) const override {
        if (!rec.front_face)
            return color(0,0,0);
        return emit->get_value(u, v, p);
    }

  private:
    shared_ptr<texture> emit;
};

class isotropic : public material {
  public:
    isotropic(color c) : albedo(make_shared<solid_color>(c)) {}
    isotropic(shared_ptr<texture> a) : albedo(a) {}

    bool scatter(const intersect_record& rec, const ray& ray_in, ray& ray_out, color& alb, double& pdf) const override 
    {
        ray_out = ray(rec.p, randomSample_unit_vector_normalize(), ray_in.time());
        alb = albedo->get_value(rec.u, rec.v, rec.p);
        pdf = 1 / (4 * pi);
        return true;
    }

    double scattering_pdf(const intersect_record& rec, const ray& ray_in, const ray& ray_out) const override
    {
        return 1 / (4 * pi);
    }

  private:
    shared_ptr<texture> albedo;
};

#endif //MATERIAL_H