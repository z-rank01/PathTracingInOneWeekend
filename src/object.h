#ifndef OBJECT_H
#define OBJECT_H

#include "ray.h"
#include "utility.h"
#include "bbox.h"

class material;

// record of newest intersection points
class intersect_record {
  public:
    point3 p;
    vec3 normal;
    double t;
    bool front_face;
    shared_ptr<material> mat;
    double u;
    double v;

    void set_face_normal(const ray& r, const vec3& outward_normal) {
        // Sets the intersect record normal vector.
        // NOTE: the parameter `outward_normal` is assumed to have unit length.

        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : - outward_normal;
    }
};

// define a virtual hittable object class

class object
{
    public:
        virtual ~object() = default;

        // when recursively tracing ray, we monitor:  
        // 1. ray
        // 2. intersection time of going in and out an object.
        // 3. update record with nearest intersection one.

        virtual bbox get_bbox() const = 0;
        virtual bool intersect(const ray& r, interval ray_t, intersect_record& rec) const = 0; // the passed-in tmin and tmax are orignially 0 and infinity.
        
        // not pure virtual function
        virtual void rotate(double degree, int axis) {}
        virtual void translate(vec3 dir) {}
        virtual double get_pdf(const point3& origin, const vec3& direction) const { return 0; }
        virtual vec3 randomDir(const point3& origin) const { return vec3(1, 0, 0); }
};


#endif //OBJECT_H