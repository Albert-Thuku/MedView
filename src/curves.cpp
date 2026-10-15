// Cubic Bezier curves and Catmull-Rom splines.

#include "graphics.h"
#include <algorithm>
#include <stdexcept>

namespace mv {

Point bezierPoint(
    Point a,
    Point b,
    Point c,
    Point d,
    double t
) {
    double u=1-t;

    return {
        u*u*u*a.x+3*u*u*t*b.x+3*u*t*t*c.x+t*t*t*d.x,
        u*u*u*a.y+3*u*u*t*b.y+3*u*t*t*c.y+t*t*t*d.y
    };
}

Polygon bezier(
    Point a,
    Point b,
    Point c,
    Point d,
    int steps
) {
    if (steps<1)
        throw std::invalid_argument("Curve needs at least one step");

    Polygon p;

    for (int i=0; i<=steps; ++i)
        p.push_back(bezierPoint(a,b,c,d,double(i)/steps));

    return p;
}

// Uniform Catmull-Rom interpolation.
// Endpoint controls are duplicated.
Polygon catmullRom(const Polygon& p, int steps) {
    if (steps<1)
        throw std::invalid_argument("Spline needs at least one step");

    if (p.size()<2)
        return p;

    Polygon out;

    for (std::size_t i=0; i+1<p.size(); ++i) {
        Point a=p[i==0 ? 0 : i-1];
        Point b=p[i];
        Point c=p[i+1];
        Point d=p[std::min(i+2,p.size()-1)];

        for (int j=0; j<steps; ++j) {
            double t=double(j)/steps;
            double t2=t*t;
            double t3=t2*t;

            out.push_back({
                0.5*(
                    2*b.x+
                    (-a.x+c.x)*t+
                    (2*a.x-5*b.x+4*c.x-d.x)*t2+
                    (-a.x+3*b.x-3*c.x+d.x)*t3
                ),
                0.5*(
                    2*b.y+
                    (-a.y+c.y)*t+
                    (2*a.y-5*b.y+4*c.y-d.y)*t2+
                    (-a.y+3*b.y-3*c.y+d.y)*t3
                )
            });
        }
    }

    out.push_back(p.back());
    return out;
}

}