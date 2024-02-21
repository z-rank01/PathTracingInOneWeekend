#ifndef CONSTANT_MEDIUM_H
#define CONSTANT_MEDIUM_H

#include "utility.h"

#include "scene.h"
#include "material.h"
#include "texture.h"

class constant_medium : public object {
  public:
    constant_medium(shared_ptr<scene> b, double d, shared_ptr<texture> a)
      : boundary(b), neg_inv_density(-1/d), phase_function(make_shared<isotropic>(a))
    {}

    constant_medium(shared_ptr<object> b, double d, color c)
      : boundary(b), neg_inv_density(-1/d), phase_function(make_shared<isotropic>(c))
    {}

    bool intersect(const ray& r, interval ray_t, intersect_record& rec) const override {
        // Print occasional samples when debugging. To enable, set enableDebug true.
        const bool enableDebug = false;
        const bool debugging = enableDebug && random_double() < 0.00001;

        intersect_record rec1, rec2;

        if (!boundary->intersect(r, interval::universe, rec1))
            return false;

        if (!boundary->intersect(r, interval(rec1.t+0.0001, infinity), rec2))
            return false;

        if (debugging) std::clog << "\nray_tmin=" << rec1.t << ", ray_tmax=" << rec2.t << '\n';

        if (rec1.t < ray_t.min) rec1.t = ray_t.min;
        if (rec2.t > ray_t.max) rec2.t = ray_t.max;

        if (rec1.t >= rec2.t)
            return false;

        if (rec1.t < 0)
            rec1.t = 0;

        auto ray_length = r.direction().length();
        auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;
        auto hit_distance = neg_inv_density * log(random_double());

        if (hit_distance > distance_inside_boundary)
            return false;

        rec.t = rec1.t + hit_distance / ray_length;
        rec.p = r.at(rec.t);

        if (debugging) {
            std::clog << "hit_distance = " <<  hit_distance << '\n'
                      << "rec.t = " <<  rec.t << '\n'
                      << "rec.p = " <<  rec.p << '\n';
        }

        rec.normal = vec3(1,0,0);  // arbitrary
        rec.front_face = true;     // also arbitrary
        rec.mat = phase_function;

        return true;
    }

    bbox get_bbox() const override { return boundary->get_bbox(); }

    // double get_pdf(const point3& origin, const vec3& direction) const override
    // {
    //     return 0;
    // }

    // vec3 randomDir(const point3& origin) const override
    // {
    //     return vec3(0, 0, 0);
    // }

  private:
    shared_ptr<object> boundary;
    double neg_inv_density;
    shared_ptr<material> phase_function;
};

#endif