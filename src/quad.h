#ifndef QUAD_H
#define QUAD_H

#include "utility.h"
#include "object.h"
#include "scene.h"

class quad : public object
{
private:
    point3 Q;
    vec3 u;
    vec3 v;
    std::shared_ptr<material> mat;
    vec3 normal;
    double area;
    double D;
    vec3 w;
    bbox bounding_box;

public:
    
    // initialization
    
    quad(const point3& _Q, const vec3 _u, const vec3 _v, std::shared_ptr<material> _material) : Q(_Q), u(_u), v(_v), mat(_material)
    {
        set_bbox();

        auto n = cross(u, v);
        normal = unit_vector(n);
        D = dot(normal, Q);
        w = n / dot(n, n);

        area = n.length();
    }

    virtual void set_bbox()
    {
        bounding_box = bbox(Q, Q + u + v).pad();
    }


    // Method
    
    bbox get_bbox() const override { return bounding_box; }

    double get_pdf(const point3& origin, const vec3& direction) const override
    {
        intersect_record rec;

        // test if light's ray intersect with this quad
        if (!this->intersect(ray(origin, direction), interval(0.001, infinity), rec))
            return 0;

        auto distance_squared = rec.t * rec.t * direction.length_squared();
        auto cosine = fabs(dot(direction, rec.normal) / direction.length());

        return distance_squared / (cosine * area);
    }

    vec3 randomDir(const point3& origin) const override
    {
        auto p = Q + (random_double() * u) + (random_double() * v);
        return p - origin;
    }


    bool intersect(const ray& r, interval ray_t, intersect_record& rec) const override
    {
        // ray-plane intersection
        
        auto denominator = dot(normal, r.direction());
        auto numerator = D - dot(normal, r.origin());
        // test if denominator is too small for precision
        if (fabs(denominator) < 1e-8)
        {
            return false;
        }
        
        auto t = numerator / denominator;
        // test if t is in valid interval
        if (!ray_t.contains(t))
        {
            return false;
        }
        
        auto p_intersect = r.at(t);
        // test if intersection is inside or outside the quad
        auto p_vec = p_intersect - Q;
        auto alpha = dot(w, cross(p_vec, v));
        auto beta = dot(w, cross(u, p_vec));
        if (!is_interior(alpha, beta))
        {
            return false;
        }
        
        // update record
        rec.p = p_intersect;
        rec.t = t;
        rec.mat = mat;
        rec.set_face_normal(r, normal);
        rec.u = alpha;
        rec.v = beta;

        return true;
    }

    virtual bool is_interior(double a, double b) const {
        // Given the hit point in plane coordinates, return false if it is outside the
        // primitive, otherwise set the hit record UV coordinates and return true.

        if ((a < 0) || (1 < a) || (b < 0) || (1 < b))
            return false;
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

        // rotate Q, u, v
        // these three are the parameters defining a quad
        auto new_Q_x = dot(row_x, Q);
        auto new_Q_y = dot(row_y, Q);
        auto new_Q_z = dot(row_z, Q);
        Q = vec3(new_Q_x, new_Q_y, new_Q_z);
        
        auto new_u_x = dot(row_x, u);
        auto new_u_y = dot(row_y, u);
        auto new_u_z = dot(row_z, u);
        u = vec3(new_u_x, new_u_y, new_u_z);
        
        auto new_v_x = dot(row_x, v);
        auto new_v_y = dot(row_y, v);
        auto new_v_z = dot(row_z, v);
        v = vec3(new_v_x, new_v_y, new_v_z);

        // update related params like initialization
        set_bbox();
        auto n = cross(u, v);
        normal = unit_vector(n);
        D = dot(normal, Q);
        w = n / dot(n, n);
    }

    void translate(vec3 dir) override
    {
        Q = vec3(Q.x() + dir.x(), Q.y() + dir.y(), Q.z() + dir.z());
        // u = vec3(u.x() + dir.x(), u.y() + dir.y(), u.z() + dir.z());
        // v = vec3(v.x() + dir.x(), v.y() + dir.y(), v.z() + dir.z());

        // update related params like initialization
        set_bbox();
        auto n = cross(u, v);
        normal = unit_vector(n);
        D = dot(normal, Q);
        w = n / dot(n, n);
    }
    
};

// box
inline shared_ptr<scene> box(const point3& a, const point3& b, shared_ptr<material> mat)
{
    // Returns the 3D box (six sides) that contains the two opposite vertices a & b.

    auto sides = make_shared<scene>();

    // Construct the two opposite vertices with the minimum and maximum coordinates.
    auto min = point3(fmin(a.x(), b.x()), fmin(a.y(), b.y()), fmin(a.z(), b.z()));
    auto max = point3(fmax(a.x(), b.x()), fmax(a.y(), b.y()), fmax(a.z(), b.z()));

    auto dx = vec3(max.x() - min.x(), 0, 0);
    auto dy = vec3(0, max.y() - min.y(), 0);
    auto dz = vec3(0, 0, max.z() - min.z());

    sides->add(make_shared<quad>(point3(min.x(), min.y(), max.z()),  dx,  dy, mat)); // front
    sides->add(make_shared<quad>(point3(max.x(), min.y(), max.z()), -dz,  dy, mat)); // right
    sides->add(make_shared<quad>(point3(max.x(), min.y(), min.z()), -dx,  dy, mat)); // back
    sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()),  dz,  dy, mat)); // left
    sides->add(make_shared<quad>(point3(min.x(), max.y(), max.z()),  dx, -dz, mat)); // top
    sides->add(make_shared<quad>(point3(min.x(), min.y(), min.z()),  dx,  dz, mat)); // bottom

    return sides;
}

#endif //QUAD_H