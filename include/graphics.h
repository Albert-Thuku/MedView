#ifndef MEDIVIEW_GRAPHICS_H
#define MEDIVIEW_GRAPHICS_H

#include <cstdint>
#include <string>
#include <vector>

namespace mv {

struct Point { double x, y; };
struct Color { std::uint8_t r, g, b; };
struct Rect { double left, top, right, bottom; };

using Polygon = std::vector<Point>;

class Canvas {
public:
    Canvas(int width, int height, Color background);

    int width() const { return w_; }
    int height() const { return h_; }

    void pixel(int x, int y, Color color);
    Color get(int x, int y) const;
    void saveBMP(const std::string& path) const;

private:
    int w_, h_;
    std::vector<Color> pixels_;
};

void line(Canvas&, Point a, Point b, Color);
bool clipLine(Point& a, Point& b, Rect viewport);

void outline(Canvas&, const Polygon&, Color, bool closed = true);
void fillPolygon(Canvas&, const Polygon&, Color);
Polygon clipPolygon(const Polygon&, Rect viewport);

Polygon transform(
    const Polygon&,
    Point origin,
    double sx,
    double sy,
    Point offset
);

void circle(Canvas&, Point center, int radius, Color);
void ellipse(Canvas&, Point center, int rx, int ry, Color);

Point bezierPoint(Point a, Point b, Point c, Point d, double t);
Polygon bezier(Point a, Point b, Point c, Point d, int steps = 60);
Polygon catmullRom(const Polygon& controls, int stepsPerSegment = 24);

void text(
    Canvas&,
    int x,
    int y,
    const std::string&,
    Color,
    int scale = 2
);

void rectangle(Canvas&, Rect, Color, bool filled = false);

}

#endif