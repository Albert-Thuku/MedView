
// Polygon outlines, filling, clipping and transformations.

#include "graphics.h"
#include <algorithm>
#include <cmath>

namespace mv {

void outline(
    Canvas& out,
    const Polygon& p,
    Color c,
    bool closed
) {
    if (p.size()<2) return;

    for (std::size_t i=1; i<p.size(); ++i)
        line(out,p[i-1],p[i],c);

    if (closed)
        line(out,p.back(),p.front(),c);
}

// Even-odd scanline filling using pixel centres.
void fillPolygon(Canvas& out, const Polygon& p, Color c) {
    if (p.size()<3) return;

    double low=p[0].y;
    double high=p[0].y;

    for (Point v:p) {
        low=std::min(low,v.y);
        high=std::max(high,v.y);
    }

    const int start=std::max(0,int(std::ceil(low-0.5)));
    const int end=std::min(
        out.height()-1,
        int(std::ceil(high-0.5))-1
    );

    for (int y=start; y<=end; ++y) {
        const double scan=y+0.5;
        std::vector<double> xs;

        for (std::size_t i=0,j=p.size()-1; i<p.size(); j=i++) {
            Point a=p[j];
            Point b=p[i];

            if (
                (a.y<=scan && b.y>scan) ||
                (b.y<=scan && a.y>scan)
            ) {
                xs.push_back(
                    a.x+(scan-a.y)*(b.x-a.x)/(b.y-a.y)
                );
            }
        }

        std::sort(xs.begin(),xs.end());

        for (std::size_t i=0; i+1<xs.size(); i+=2) {
            int left=std::max(0,int(std::ceil(xs[i]-0.5)));
            int right=std::min(
                out.width()-1,
                int(std::ceil(xs[i+1]-0.5))-1
            );

            for (int x=left; x<=right; ++x)
                out.pixel(x,y,c);
        }
    }
}

// Sutherland-Hodgman polygon clipping.
Polygon clipPolygon(const Polygon& input, Rect r) {
    Polygon result=input;

    for (int edge=0; edge<4; ++edge) {
        Polygon source=result;
        result.clear();

        if (source.empty()) break;

        const auto inside=[edge,r](Point p) {
            if (edge==0) return p.x>=r.left;
            if (edge==1) return p.x<=r.right;
            if (edge==2) return p.y>=r.top;
            return p.y<=r.bottom;
        };

        const auto intersection=[edge,r](Point a,Point b) {
            if (edge<2) {
                double x=edge==0 ? r.left : r.right;

                return Point{
                    x,
                    a.y+(b.y-a.y)*(x-a.x)/(b.x-a.x)
                };
            }

            double y=edge==2 ? r.top : r.bottom;

            return Point{
                a.x+(b.x-a.x)*(y-a.y)/(b.y-a.y),
                y
            };
        };

        Point previous=source.back();

        for (Point current:source) {
            bool pin=inside(previous);
            bool cin=inside(current);

            if (pin!=cin)
                result.push_back(intersection(previous,current));

            if (cin)
                result.push_back(current);

            previous=current;
        }
    }

    return result;
}

Polygon transform(
    const Polygon& p,
    Point origin,
    double sx,
    double sy,
    Point offset
) {
    Polygon result;
    result.reserve(p.size());

    for (Point v:p) {
        result.push_back({
            origin.x+(v.x-origin.x)*sx+offset.x,
            origin.y+(v.y-origin.y)*sy+offset.y
        });
    }

    return result;
}

void rectangle(Canvas& c, Rect r, Color color, bool filled) {
    Polygon p={
        {r.left,r.top},
        {r.right,r.top},
        {r.right,r.bottom},
        {r.left,r.bottom}
    };

    if (filled)
        fillPolygon(c,p,color);
    else
        outline(c,p,color);
}

}