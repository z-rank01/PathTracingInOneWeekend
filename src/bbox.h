#ifndef BBOX_H
#define BBOX_H

#include "utility.h"

class bbox
{
public:
    interval x, y, z;  // the axis-aligned "slab"


    // initialization

    bbox() {};  // default empty bbox
    bbox(const interval& ix, const interval& iy, const interval& iz) : x(ix), y(iy), z(iz) {}
    bbox(const point3& p1, const point3& p2)
    {
        // two point define a AABB
        x = interval(fmin(p1[0], p2[0]), fmax(p1[0], p2[0]));
        y = interval(fmin(p1[1], p2[1]), fmax(p1[1], p2[1]));
        z = interval(fmin(p1[2], p2[2]), fmax(p1[2], p2[2]));
    }
    bbox(const bbox& b1, const bbox& b2)
    {
        x = interval(b1.x, b2.x);
        y = interval(b1.y, b2.y);
        z = interval(b1.z, b2.z);
    }

    bbox pad() {
        // Return an AABB that has no side narrower than some delta, padding if necessary.
        double delta = 0.0001;
        interval new_x = (x.size() >= delta) ? x : x.expand(delta);
        interval new_y = (y.size() >= delta) ? y : y.expand(delta);
        interval new_z = (z.size() >= delta) ? z : z.expand(delta);

        return bbox(new_x, new_y, new_z);
    }
    

    // method
    const interval& axis(int n) const {
        if (n == 1) return y;
        if (n == 2) return z;
        return x;
    }

    bool intersect(const ray& r, interval t) const
    {
        for(int i = 0; i < 3; ++i)
        {
            auto invDir = 1 / r.direction()[i];
            auto orig = r.origin()[i];

            auto t0 = (axis(i).min - orig) * invDir;
            auto t1 = (axis(i).max - orig) * invDir;

            if (invDir < 0)
            {
                std::swap(t0, t1);
            }

            if(t0 > t.min) t.min = t0;
            if(t1 < t.max) t.max = t1;

            if (t.min >= t.max)
            {
                return false;
            }
            
        }

        return true;
    }
};



#endif // BBOX_H