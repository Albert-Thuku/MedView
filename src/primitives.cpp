// Lines, line clipping, circles and ellipses.

#include "graphics.h"
#include <cmath>
#include <stdexcept>

namespace mv {

// Bresenham line algorithm: all eight octants.
void line(Canvas& out, Point a, Point b, Color c) {
    int x=int(std::lround(a.x));
    int y=int(std::lround(a.y));

    const int x1=int(std::lround(b.x));
    const int y1=int(std::lround(b.y));

    const int dx=std::abs(x1-x);
    const int sx=x<x1 ? 1 : -1;
    const int dy=-std::abs(y1-y);
    const int sy=y<y1 ? 1 : -1;

    int error=dx+dy;

    for (;;) {
        out.pixel(x,y,c);

        if (x==x1 && y==y1)
            break;

        const int twice=2*error;

        if (twice>=dy) {
            error+=dy;
            x+=sx;
        }

        if (twice<=dx) {
            error+=dx;
            y+=sy;
        }
    }
}

// Cohen-Sutherland line clipping.
bool clipLine(Point& a, Point& b, Rect r) {
    const auto code=[r](Point p) {
        return
            (p.x<r.left ? 1 : 0) |
            (p.x>r.right ? 2 : 0) |
            (p.y<r.top ? 4 : 0) |
            (p.y>r.bottom ? 8 : 0);
    };

    for (;;) {
        const int ca=code(a);
        const int cb=code(b);

        if (!(ca|cb))
            return true;

        if (ca&cb)
            return false;

        const int co=ca ? ca : cb;
        Point p;

        if (co&8) {
            p={
                a.x+(b.x-a.x)*(r.bottom-a.y)/(b.y-a.y),
                r.bottom
            };
        } else if (co&4) {
            p={
                a.x+(b.x-a.x)*(r.top-a.y)/(b.y-a.y),
                r.top
            };
        } else if (co&2) {
            p={
                r.right,
                a.y+(b.y-a.y)*(r.right-a.x)/(b.x-a.x)
            };
        } else {
            p={
                r.left,
                a.y+(b.y-a.y)*(r.left-a.x)/(b.x-a.x)
            };
        }

        if (co==ca)
            a=p;
        else
            b=p;
    }
}

// Midpoint circle algorithm.
void circle(Canvas& out, Point center, int r, Color c) {
    if (r<0)
        throw std::invalid_argument("Negative circle radius");

    int cx=int(std::lround(center.x));
    int cy=int(std::lround(center.y));

    int x=0;
    int y=r;
    int d=1-r;

    while (x<=y) {
        out.pixel(cx+x,cy+y,c);
        out.pixel(cx-x,cy+y,c);
        out.pixel(cx+x,cy-y,c);
        out.pixel(cx-x,cy-y,c);

        out.pixel(cx+y,cy+x,c);
        out.pixel(cx-y,cy+x,c);
        out.pixel(cx+y,cy-x,c);
        out.pixel(cx-y,cy-x,c);

        ++x;

        if (d<0) {
            d+=2*x+1;
        } else {
            --y;
            d+=2*(x-y)+1;
        }
    }
}

// Midpoint ellipse algorithm.
void ellipse(
    Canvas& out,
    Point center,
    int rx,
    int ry,
    Color c
) {
    if (rx<0 || ry<0)
        throw std::invalid_argument("Negative ellipse radius");

    if (rx==0) {
        line(
            out,
            {center.x,center.y-ry},
            {center.x,center.y+ry},
            c
        );
        return;
    }

    if (ry==0) {
        line(
            out,
            {center.x-rx,center.y},
            {center.x+rx,center.y},
            c
        );
        return;
    }

    int cx=int(std::lround(center.x));
    int cy=int(std::lround(center.y));

    int x=0;
    int y=ry;

    double rx2=double(rx)*rx;
    double ry2=double(ry)*ry;
    double dx=0;
    double dy=2*rx2*y;

    const auto plot=[&]() {
        out.pixel(cx+x,cy+y,c);
        out.pixel(cx-x,cy+y,c);
        out.pixel(cx+x,cy-y,c);
        out.pixel(cx-x,cy-y,c);
    };

    double d1=ry2-rx2*ry+0.25*rx2;

    while (dx<dy) {
        plot();
        ++x;
        dx+=2*ry2;

        if (d1<0) {
            d1+=dx+ry2;
        } else {
            --y;
            dy-=2*rx2;
            d1+=dx-dy+ry2;
        }
    }

    double d2=
        ry2*(x+0.5)*(x+0.5)+
        rx2*(y-1.0)*(y-1.0)-
        rx2*ry2;

    while (y>=0) {
        plot();
        --y;
        dy-=2*rx2;

        if (d2>0) {
            d2+=rx2-dy;
        } else {
            ++x;
            dx+=2*ry2;
            d2+=dx-dy+rx2;
        }
    }
}

}