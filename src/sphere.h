#ifndef SPHERE_H
#define SPHERE_H

#include "object.h"
#include "utility.h"

class sphere : public object {
  public:
    // stationary sphere
    sphere(point3 _center, double _radius, shared_ptr<material> _material) 
      : center1(_center), radius(_radius), mat(_material), is_moving(false)
      {
          auto rVec = vec3(radius, radius, radius);
          boundingBox = bbox(center1 - rVec, center1 + rVec);
      }

    // dynamic sphere
    sphere(point3 _center1, point3 _center2, double _radius, shared_ptr<material> _material) 
      : center1(_center1), radius(_radius), mat(_material), is_moving(true) 
    {
        moving_dir = _center2 - _center1;
        auto rVec = vec3(radius, radius, radius);
        auto boundingBox_1 = bbox(_center1 - rVec, _center1 + rVec);
        auto boundingBox_2 = bbox(_center2 - rVec, _center2 + rVec);
        boundingBox = bbox(boundingBox_1, boundingBox_2);
    }


    // Method

    bbox get_bbox() const override { return boundingBox; }

    // double get_pdf(const point3& origin, const vec3& direction) const override
    // {
    //     return 0;
    // }

    // vec3 randomDir(const point3& origin) const override
    // {
    //     return vec3(0, 0, 0);
    // }

    bool intersect(const ray& r, interval ray_t, intersect_record& rec) const override 
    {    
        // intersection 
        
        vec3 center = is_moving ? get_current_center(r.time()) : center1;   // if sphere is movable, get current center location
        vec3 oc = r.origin() - center;
        auto a = r.direction().length_squared();
        auto half_b = dot(oc, r.direction());
        auto c = oc.length_squared() - radius*radius;

        // simplified of b^2 - 4ac
        auto discriminant = half_b*half_b - a*c;
        if (discriminant < 0) 
            return false;
        
        // Find the nearest root that lies in the acceptable range.
        auto sqrtd = sqrt(discriminant);
        auto root = (-half_b - sqrtd) / a;  // first root
        if (!ray_t.surrounds(root)) {
            root = (-half_b + sqrtd) / a;   // second root
            if (!ray_t.surrounds(root))
                return false;
        }


        // update intersection record

        rec.t = root;
        rec.p = r.at(rec.t);

        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        get_sphere_uv(outward_normal, rec.u, rec.v);    // update (u,v) for records 
        rec.mat = mat;

        return true;
    }

    void rotate(double degree, int axis) override
    {
        // rotation matrix parameter
        auto radians = degrees_to_radians(degree);
        auto cos_theta = cos(radians);
        auto sin_theta = sin(radians);

        // construct rotation matrix
        vec3 row_x, row_y, row_z;
        switch (axis)
        {
        case 0:
            row_x = vec3(1.0,         0.0,          0.0);
            row_y = vec3(0.0,   cos_theta,   -sin_theta);
            row_z = vec3(0.0,   sin_theta,    cos_theta);
            break;
        case 1:
            row_x = vec3(cos_theta,    0.0,  sin_theta);
            row_y = vec3(0.0,          1.0,        0.0);
            row_z = vec3(-sin_theta,   0.0,  cos_theta);
            break;
        case 2:
            row_x = vec3(cos_theta,  -sin_theta,   0.0);
            row_y = vec3(sin_theta,   cos_theta,   0.0);
            row_z = vec3(      0.0,         0.0,   1.0);
            break;
        default:
            break;
        }

        // update
        auto new_center1_x = dot(row_x, center1);
        auto new_center1_y = dot(row_y, center1);
        auto new_center1_z = dot(row_z, center1);
        center1 = vec3(new_center1_x, new_center1_y, new_center1_z);

        auto new_moveDir_x = dot(row_x, moving_dir);
        auto new_moveDir_y = dot(row_y, moving_dir);
        auto new_moveDir_z = dot(row_z, moving_dir);
        moving_dir = vec3(new_moveDir_x, new_moveDir_y, new_moveDir_z);

        // update bounding box
        if (is_moving)
        {
          auto center2 = moving_dir + center1;
          auto rVec = vec3(radius, radius, radius);
          auto boundingBox_1 = bbox(center1 - rVec, center1 + rVec);
          auto boundingBox_2 = bbox(center2 - rVec, center2 + rVec);
          boundingBox = bbox(boundingBox_1, boundingBox_2);
        }
        else
        {
          auto rVec = vec3(radius, radius, radius);
          boundingBox = bbox(center1 - rVec, center1 + rVec);
        }
    }

    void translate(vec3 dir) override
    {
        // update center
        center1 = vec3(center1.x() + dir.x(), center1.y() + dir.y(), center1.z() + dir.z());
        
        // update bounding box
        if (is_moving)
        {
          auto center2 = moving_dir + center1;
          auto rVec = vec3(radius, radius, radius);
          auto boundingBox_1 = bbox(center1 - rVec, center1 + rVec);
          auto boundingBox_2 = bbox(center2 - rVec, center2 + rVec);
          boundingBox = bbox(boundingBox_1, boundingBox_2);
        }
        else
        {
          auto rVec = vec3(radius, radius, radius);
          boundingBox = bbox(center1 - rVec, center1 + rVec);
        }
    }

  private:
    point3 center1;
    bool is_moving;
    vec3 moving_dir;
    double radius;
    shared_ptr<material> mat;
    bbox boundingBox;

    point3 get_current_center(double time) const
    {
      return center1 + time * moving_dir;
    }
    
    static void get_sphere_uv(const point3& p, double& u, double& v) {
        // p: a given point on the sphere of radius one, centered at the origin.
        // u: returned value [0,1] of angle around the Y axis from X=-1.
        // v: returned value [0,1] of angle from Y=-1 to Y=+1.
        //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
        //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
        //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>

        auto theta = acos(-p.y());
        auto phi = atan2(-p.z(), p.x()) + pi;

        u = phi / (2*pi);
        v = theta / pi;
    }

};

#endif