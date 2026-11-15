#include "graphics.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace mv {
Canvas::Canvas(int w, int h, Color bg) : w_(w), h_(h) {
    if (w <= 0 || h <= 0) throw std::invalid_argument("Invalid canvas dimensions");
    pixels_.assign(static_cast<std::size_t>(w) * h, bg);
}
void Canvas::pixel(int x, int y, Color c) {
    if (x >= 0 && x < w_ && y >= 0 && y < h_) pixels_[static_cast<std::size_t>(y)*w_+x] = c;
}
Color Canvas::get(int x, int y) const {
    if (x < 0 || y < 0 || x >= w_ || y >= h_) throw std::out_of_range("Pixel outside canvas");
    return pixels_[static_cast<std::size_t>(y)*w_+x];
}
void Canvas::saveBMP(const std::string& path) const {
    std::ofstream out(path.c_str(), std::ios::binary);
    if (!out) throw std::runtime_error("Cannot write " + path);
    const std::uint32_t stride = (static_cast<std::uint32_t>(w_)*3+3)&~3u;
    const std::uint32_t size = stride*static_cast<std::uint32_t>(h_);
    const auto u16 = [&out](std::uint16_t v) { out.put(char(v&255)); out.put(char((v>>8)&255)); };
    const auto u32 = [&out](std::uint32_t v) { for(int i=0;i<4;++i) out.put(char((v>>(8*i))&255)); };
    out.put('B'); out.put('M'); u32(54+size); u16(0); u16(0); u32(54);
    u32(40); u32(w_); u32(h_); u16(1); u16(24); u32(0); u32(size);
    u32(2835); u32(2835); u32(0); u32(0);
    for (int y=h_-1;y>=0;--y) {
        for (int x=0;x<w_;++x) { Color c=get(x,y); out.put(char(c.b)); out.put(char(c.g)); out.put(char(c.r)); }
        for(std::uint32_t p=static_cast<std::uint32_t>(w_)*3;p<stride;++p) out.put(0);
    }
    if (!out) throw std::runtime_error("Failed writing " + path);
}
// Bresenham: integer error accumulation works in all eight octants.
void line(Canvas& out, Point a, Point b, Color c) {
    int x=int(std::lround(a.x)), y=int(std::lround(a.y));
    const int x1=int(std::lround(b.x)), y1=int(std::lround(b.y));
    const int dx=std::abs(x1-x), sx=x<x1?1:-1;
    const int dy=-std::abs(y1-y), sy=y<y1?1:-1;
    int error=dx+dy;
    for (;;) {
        out.pixel(x,y,c); if (x==x1 && y==y1) break;
        const int twice=2*error;
        if (twice>=dy) { error+=dy; x+=sx; }
        if (twice<=dx) { error+=dx; y+=sy; }
    }
}
// Cohen-Sutherland clips geometry before rasterization (not just pixel bounds).
bool clipLine(Point& a, Point& b, Rect r) {
    const auto code=[r](Point p) { return (p.x<r.left?1:0)|(p.x>r.right?2:0)|(p.y<r.top?4:0)|(p.y>r.bottom?8:0); };
    for (;;) {
        const int ca=code(a), cb=code(b);
        if (!(ca|cb)) return true;
        if (ca&cb) return false;
        const int co=ca?ca:cb;
        Point p;
        if(co&8) p={a.x+(b.x-a.x)*(r.bottom-a.y)/(b.y-a.y),r.bottom};
        else if(co&4) p={a.x+(b.x-a.x)*(r.top-a.y)/(b.y-a.y),r.top};
        else if(co&2) p={r.right,a.y+(b.y-a.y)*(r.right-a.x)/(b.x-a.x)};
        else p={r.left,a.y+(b.y-a.y)*(r.left-a.x)/(b.x-a.x)};
        if(co==ca) a=p; else b=p;
    }
}
void outline(Canvas& out, const Polygon& p, Color c, bool closed) {
    if(p.size()<2) return;
    for(std::size_t i=1;i<p.size();++i) line(out,p[i-1],p[i],c);
    if(closed) line(out,p.back(),p.front(),c);
}
// Even-odd scanline filling. Sample pixel centers and use half-open edge ranges
// so shared vertices are counted once. Horizontal edges do not intersect.
void fillPolygon(Canvas& out, const Polygon& p, Color c) {
    if(p.size()<3) return;
    double low=p[0].y, high=p[0].y;
    for(Point v:p) { low=std::min(low,v.y); high=std::max(high,v.y); }
    const int start=std::max(0,int(std::ceil(low-0.5)));
    const int end=std::min(out.height()-1,int(std::ceil(high-0.5))-1);
    for(int y=start;y<=end;++y) {
        const double scan=y+0.5;
        std::vector<double> xs;
        for(std::size_t i=0,j=p.size()-1;i<p.size();j=i++) {
            Point a=p[j],b=p[i];
            if((a.y<=scan && b.y>scan)||(b.y<=scan && a.y>scan))
                xs.push_back(a.x+(scan-a.y)*(b.x-a.x)/(b.y-a.y));
        }
        std::sort(xs.begin(),xs.end());
        for(std::size_t i=0;i+1<xs.size();i+=2) {
            int left=std::max(0,int(std::ceil(xs[i]-0.5)));
            int right=std::min(out.width()-1,int(std::ceil(xs[i+1]-0.5))-1);
            for(int x=left;x<=right;++x) out.pixel(x,y,c);
        }
    }
}
// Sutherland-Hodgman: successively clip against four convex window edges.
Polygon clipPolygon(const Polygon& input, Rect r) {
    Polygon result=input;
    for(int edge=0;edge<4;++edge) {
        Polygon source=result; result.clear(); if(source.empty()) break;
        const auto inside=[edge,r](Point p) { if(edge==0)return p.x>=r.left; if(edge==1)return p.x<=r.right; if(edge==2)return p.y>=r.top; return p.y<=r.bottom; };
        const auto intersection=[edge,r](Point a,Point b) {
            if(edge<2) { double x=edge==0?r.left:r.right; return Point{x,a.y+(b.y-a.y)*(x-a.x)/(b.x-a.x)}; }
            double y=edge==2?r.top:r.bottom; return Point{a.x+(b.x-a.x)*(y-a.y)/(b.y-a.y),y};
        };
        Point previous=source.back();
        for(Point current:source) {
            bool pin=inside(previous), cin=inside(current);
            if(pin!=cin) result.push_back(intersection(previous,current));
            if(cin) result.push_back(current);
            previous=current;
        }
    }
    return result;
}
Polygon transform(const Polygon& p, Point origin, double sx, double sy, Point offset) {
    Polygon result; result.reserve(p.size());
    for(Point v:p) result.push_back({origin.x+(v.x-origin.x)*sx+offset.x,origin.y+(v.y-origin.y)*sy+offset.y});
    return result;
}
void circle(Canvas& out, Point center, int r, Color c) {
    if(r<0) throw std::invalid_argument("Negative circle radius");
    int cx=int(std::lround(center.x)),cy=int(std::lround(center.y));
    int x=0,y=r,d=1-r;
    while(x<=y) {
        out.pixel(cx+x,cy+y,c);out.pixel(cx-x,cy+y,c);out.pixel(cx+x,cy-y,c);out.pixel(cx-x,cy-y,c);
        out.pixel(cx+y,cy+x,c);out.pixel(cx-y,cy+x,c);out.pixel(cx+y,cy-x,c);out.pixel(cx-y,cy-x,c);
        ++x; if(d<0)d+=2*x+1; else {--y;d+=2*(x-y)+1;}
    }
}
// Midpoint ellipse: use separate decision parameters for shallow/steep regions.
void ellipse(Canvas& out, Point center, int rx, int ry, Color c) {
    if(rx<0||ry<0) throw std::invalid_argument("Negative ellipse radius");
    if(rx==0) {line(out,{center.x,center.y-ry},{center.x,center.y+ry},c);return;}
    if(ry==0) {line(out,{center.x-rx,center.y},{center.x+rx,center.y},c);return;}
    int cx=int(std::lround(center.x)),cy=int(std::lround(center.y));
    int x=0,y=ry; double rx2=double(rx)*rx,ry2=double(ry)*ry,dx=0,dy=2*rx2*y;
    const auto plot=[&]() {out.pixel(cx+x,cy+y,c);out.pixel(cx-x,cy+y,c);out.pixel(cx+x,cy-y,c);out.pixel(cx-x,cy-y,c);};
    double d1=ry2-rx2*ry+0.25*rx2;
    while(dx<dy) {
        plot();++x;dx+=2*ry2;
        if(d1<0)d1+=dx+ry2;else{--y;dy-=2*rx2;d1+=dx-dy+ry2;}
    }
    double d2=ry2*(x+0.5)*(x+0.5)+rx2*(y-1.0)*(y-1.0)-rx2*ry2;
    while(y>=0) {
        plot();--y;dy-=2*rx2;
        if(d2>0)d2+=rx2-dy;else{++x;dx+=2*ry2;d2+=dx-dy+rx2;}
    }
}
Point bezierPoint(Point a, Point b, Point c, Point d, double t) {
    double u=1-t;
    return {u*u*u*a.x+3*u*u*t*b.x+3*u*t*t*c.x+t*t*t*d.x,
            u*u*u*a.y+3*u*u*t*b.y+3*u*t*t*c.y+t*t*t*d.y};
}
Polygon bezier(Point a,Point b,Point c,Point d,int steps) {
    if(steps<1) throw std::invalid_argument("Curve needs at least one step");
    Polygon p; for(int i=0;i<=steps;++i)p.push_back(bezierPoint(a,b,c,d,double(i)/steps)); return p;
}
// Uniform Catmull-Rom with duplicated endpoint controls: interpolates each knot.
Polygon catmullRom(const Polygon& p,int steps) {
    if(steps<1) throw std::invalid_argument("Spline needs at least one step");
    if(p.empty())return {};
    if(p.size()==1)return Polygon{p.front()};
    Polygon out;
    for(std::size_t i=0;i+1<p.size();++i) {
        Point a=p[i==0?0:i-1],b=p[i],c=p[i+1],d=p[std::min(i+2,p.size()-1)];
        for(int j=0;j<steps;++j) {
            double t=double(j)/steps,t2=t*t,t3=t2*t;
            out.push_back({0.5*((2*b.x)+(-a.x+c.x)*t+(2*a.x-5*b.x+4*c.x-d.x)*t2+(-a.x+3*b.x-3*c.x+d.x)*t3),
                           0.5*((2*b.y)+(-a.y+c.y)*t+(2*a.y-5*b.y+4*c.y-d.y)*t2+(-a.y+3*b.y-3*c.y+d.y)*t3)});
        }
    }
    out.push_back(p.back());return out;
}
void rectangle(Canvas& c,Rect r,Color color,bool filled) {
    Polygon p={{r.left,r.top},{r.right,r.top},{r.right,r.bottom},{r.left,r.bottom}};
    if(filled)fillPolygon(c,p,color);else outline(c,p,color);
}
}