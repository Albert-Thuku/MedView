// Assigned module: AJ
// Application entry point, demonstration scenes and bitmap labels.

#include "graphics.h"
#include <array>
#include <cctype>
#include <cmath>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>


using namespace mv;

void detailedOverview(const std::string &dir);

namespace {

const Color BG = {12, 23, 38};
const Color PANEL = {19, 36, 53};
const Color INK = {226, 238, 245};
const Color MUTED = {136, 164, 184};
const Color TEAL = {57, 205, 181};
const Color PINK = {245, 130, 151};
const Color GOLD = {252, 196, 100};
const Color GRID = {37, 61, 79};

void title(Canvas &c, const std::string &name, const std::string &subtitle,
           int page) {
  text(c, 42, 30, "MEDIVIEW", TEAL, 3);
  text(c, 925, 37, "CP1 / 0" + std::to_string(page), MUTED, 2);

  text(c, 42, 82, name, INK, 4);
  text(c, 44, 132, subtitle, MUTED, 2);

  line(c, {42, 168}, {1158, 168}, GRID);
  line(c, {42, 750}, {1158, 750}, GRID);

  text(c, 44, 772, "PURE C++ / SOFTWARE RASTERIZER", MUTED, 2);
  text(c, 680, 772, "SCHEMATIC / EDUCATIONAL DEMO", MUTED, 2);
}

void grid(Canvas &c, Rect r) {
  rectangle(c, r, PANEL, true);

  for (int x = int(r.left) + 30; x < r.right; x += 30)
    line(c, {double(x), r.top}, {double(x), r.bottom}, GRID);

  for (int y = int(r.top) + 30; y < r.bottom; y += 30)
    line(c, {r.left, double(y)}, {r.right, double(y)}, GRID);
}

void append(Polygon &p, const Polygon &q) {
  p.insert(p.end(), q.begin(), q.end());
}

Polygon lung(bool right) {
  Polygon p = bezier({-14, -102}, {-55, -116}, {-91, -55}, {-94, 42});

  append(p, bezier({-94, 42}, {-103, 106}, {-58, 121}, {-15, 91}));
  append(p, bezier({-15, 91}, {-1, 40}, {-7, -62}, {-14, -102}));

  if (right)
    for (Point &v : p)
      v.x = -v.x;

  return p;
}

Polygon placedLung(bool right, Point center, double sx = 1, double sy = 1) {
  return transform(lung(right), {0, 0}, sx, sy, center);
}

void organ(Canvas &c, const Polygon &p, Color color) {
  fillPolygon(c, p, color);
  outline(c, p, INK);
}

void airways(Canvas &c, Point center, double s) {
  const auto at = [center, s](double x, double y) {
    return Point{center.x + x * s, center.y + y * s};
  };

  for (int offset = -3; offset <= 3; ++offset) {
    line(c, at(offset, -132), at(offset, -18), GOLD);
    line(c, at(offset, -18), at(-48 + offset, 30), GOLD);
    line(c, at(offset, -18), at(48 + offset, 30), GOLD);
  }

  line(c, at(-45, 27), at(-70, 8), GOLD);
  line(c, at(-45, 27), at(-62, 63), GOLD);
  line(c, at(45, 27), at(70, 8), GOLD);
  line(c, at(45, 27), at(62, 63), GOLD);
}

void primitives(const std::string &dir) {
  Canvas c(1200, 810, BG);

  title(c, "GRAPHICS ALGORITHM ATLAS",
        "02 / PRIMITIVES USED TO CONSTRUCT THE MEDIVIEW MODEL", 2);

  line(c, {600, 190}, {600, 730}, GRID);
  line(c, {42, 459}, {1158, 459}, GRID);

  text(c, 55, 194, "BRESENHAM LINES", TEAL, 2);

  Point root = {177, 268};

  for (Point q : Polygon{{82, 244},
                         {126, 228},
                         {209, 214},
                         {276, 233},
                         {287, 292},
                         {236, 335},
                         {150, 345},
                         {86, 303}}) {
    line(c, root, q, GOLD);
  }

  text(c, 325, 257, "8 OCTANTS", INK, 2);
  text(c, 325, 288, "AIRWAY BRANCHES", MUTED, 2);
  text(c, 55, 402, "INTEGER ERROR / BOTH ENDPOINTS", MUTED, 2);

  text(c, 631, 194, "MIDPOINT CIRCLE + ELLIPSE", TEAL, 2);
  circle(c, {738, 307}, 65, PINK);
  ellipse(c, {936, 307}, 104, 66, TEAL);

  line(c, {730, 307}, {746, 307}, MUTED);
  line(c, {738, 299}, {738, 315}, MUTED);

  text(c, 648, 402, "ROUNDED DETAILS / HEAD OUTLINE", MUTED, 2);

  text(c, 55, 483, "CUBIC BEZIER", TEAL, 2);

  Point a = {82, 648};
  Point b = {182, 487};
  Point d = {500, 637};
  Point cc = {377, 486};

  outline(c, {a, b, cc, d}, GRID, false);
  outline(c, bezier(a, b, cc, d), GOLD, false);

  for (Point v : Polygon{a, b, cc, d})
    circle(c, v, 5, INK);

  text(c, 55, 702, "4 CONTROLS / DIAPHRAGM CONTOUR", MUTED, 2);
  text(c, 631, 483, "CATMULL-ROM SPLINE", TEAL, 2);

  Polygon knots = {{659, 630}, {745, 552}, {844, 625}, {936, 554}, {1100, 625}};

  outline(c, knots, GRID, false);
  outline(c, catmullRom(knots), PINK, false);

  for (Point v : knots)
    circle(c, v, 5, INK);

  text(c, 631, 702, "INTERPOLATED PATH / VESSEL STUDY", MUTED, 2);

  c.saveBMP(dir + "/02_algorithms.bmp");
}

void clipping(const std::string &dir) {
  Canvas c(1200, 810, BG);

  title(c, "VIEWPORT CLIPPING",
        "03 / SAME GEOMETRY BEFORE AND AFTER WINDOW INTERSECTION", 3);

  text(c, 54, 203, "BEFORE", PINK, 3);
  text(c, 653, 203, "AFTER", TEAL, 3);

  const Rect left = {100, 303, 500, 577};
  const Rect right = {700, 303, 1100, 577};

  grid(c, {54, 251, 548, 663});
  grid(c, {652, 251, 1146, 663});

  Polygon p = placedLung(false, {513, 393}, 3.2, 1.75);
  fillPolygon(c, p, PINK);
  outline(c, p, INK);

  Polygon moved = transform(p, {0, 0}, 1, 1, {600, 0});
  Polygon clipped = clipPolygon(moved, right);

  fillPolygon(c, clipped, TEAL);
  outline(c, clipped, INK);

  Point a = {63, 608};
  Point b = {539, 266};

  line(c, a, b, GOLD);

  a.x += 600;
  b.x += 600;

  if (clipLine(a, b, right))
    line(c, a, b, GOLD);

  rectangle(c, left, GOLD);
  rectangle(c, right, GOLD);

  text(c, 111, 319, "WINDOW", GOLD, 2);
  text(c, 711, 319, "WINDOW", GOLD, 2);

  text(c, 56, 687, "PINK: ORIGINAL LUNG POLYGON", MUTED, 2);
  text(c, 654, 687, "TEAL: SUTHERLAND-HODGMAN", MUTED, 2);
  text(c, 56, 719, "GOLD: WINDOW + TEST LINE", MUTED, 2);
  text(c, 654, 719, "LINE: COHEN-SUTHERLAND", MUTED, 2);

  c.saveBMP(dir + "/03_clipping.bmp");
}

void transforms(const std::string &dir) {
  Canvas c(1200, 810, BG);

  title(c, "TRANSFORMING ORGAN GEOMETRY",
        "04 / LOCAL COORDINATES + SCALING + TRANSLATION", 4);

  const Point centers[] = {{217, 410}, {572, 410}, {951, 410}};

  const double sx[] = {0.9, 0.9, 1.18};
  const double sy[] = {0.9, 0.9, 1.12};

  const std::string labels[] = {"ORIGINAL", "TRANSLATED", "SCALED + MOVED"};

  for (int i = 0; i < 3; ++i) {
    text(c, 58 + 385 * i, 202, labels[i], i == 2 ? PINK : TEAL, 2);

    grid(c, {double(53 + 385 * i), 247, double(377 + 385 * i), 584});

    Polygon l = placedLung(false, centers[i], sx[i], sy[i]);
    Polygon r = placedLung(true, centers[i], sx[i], sy[i]);

    organ(c, l, TEAL);
    organ(c, r, PINK);
    airways(c, centers[i], 0.8);
    circle(c, centers[i], 5, GOLD);
  }

  text(c, 58, 614, "SCALE: 0.90 / 0.90", INK, 2);
  text(c, 443, 614, "MOVE: +355 / 0", INK, 2);
  text(c, 828, 614, "SCALE: 1.18 / 1.12", INK, 2);

  text(c, 58, 663, "X = ORIGIN.X + (X - ORIGIN.X) * SX + TX", MUTED, 2);
  text(c, 58, 699, "Y = ORIGIN.Y + (Y - ORIGIN.Y) * SY + TY", MUTED, 2);

  c.saveBMP(dir + "/04_transforms.bmp");
}

} // namespace

int main(int argc, char **argv) {
  if (argc > 1 && std::string(argv[1]) == "--help") {
    std::cout << "MediView CP1\n"
              << "Usage: mediview [existing-output-directory]\n"
              << "Default: evidence. Creates four 24-bit BMP images "
              << "using standard C++ only.\n";

    return 0;
  }

  if (argc > 2) {
    std::cerr << "Expected at most one output directory. Use --help.\n";
    return 1;
  }

  const std::string dir = argc == 2 ? argv[1] : "evidence";

  try {
    detailedOverview(dir);
    primitives(dir);
    clipping(dir);
    transforms(dir);

    std::cout << "MediView CP1: generated 4 BMP images in " << dir << "\n"
              << "Open the BMP files to view the results.\n";

    return 0;
  } catch (const std::exception &error) {
    std::cerr << error.what() << "\nCreate the output directory first.\n";

    return 1;
  }
}

namespace mv {

// Bitmap font drawn directly into the pixel buffer.
void text(Canvas &canvas, int x, int y, const std::string &value, Color c,
          int scale) {
  static const std::map<char, std::array<unsigned char, 7>> font = {
      {'A', {{14, 17, 17, 31, 17, 17, 17}}},
      {'B', {{30, 17, 17, 30, 17, 17, 30}}},
      {'C', {{14, 17, 16, 16, 16, 17, 14}}},
      {'D', {{30, 17, 17, 17, 17, 17, 30}}},
      {'E', {{31, 16, 16, 30, 16, 16, 31}}},
      {'F', {{31, 16, 16, 30, 16, 16, 16}}},
      {'G', {{14, 17, 16, 23, 17, 17, 15}}},
      {'H', {{17, 17, 17, 31, 17, 17, 17}}},
      {'I', {{31, 4, 4, 4, 4, 4, 31}}},
      {'J', {{7, 2, 2, 2, 18, 18, 12}}},
      {'K', {{17, 18, 20, 24, 20, 18, 17}}},
      {'L', {{16, 16, 16, 16, 16, 16, 31}}},
      {'M', {{17, 27, 21, 21, 17, 17, 17}}},
      {'N', {{17, 25, 21, 19, 17, 17, 17}}},
      {'O', {{14, 17, 17, 17, 17, 17, 14}}},
      {'P', {{30, 17, 17, 30, 16, 16, 16}}},
      {'Q', {{14, 17, 17, 17, 21, 18, 13}}},
      {'R', {{30, 17, 17, 30, 20, 18, 17}}},
      {'S', {{15, 16, 16, 14, 1, 1, 30}}},
      {'T', {{31, 4, 4, 4, 4, 4, 4}}},
      {'U', {{17, 17, 17, 17, 17, 17, 14}}},
      {'V', {{17, 17, 17, 17, 17, 10, 4}}},
      {'W', {{17, 17, 17, 21, 21, 21, 10}}},
      {'X', {{17, 17, 10, 4, 10, 17, 17}}},
      {'Y', {{17, 17, 10, 4, 4, 4, 4}}},
      {'Z', {{31, 1, 2, 4, 8, 16, 31}}},
      {'0', {{14, 17, 19, 21, 25, 17, 14}}},
      {'1', {{4, 12, 4, 4, 4, 4, 14}}},
      {'2', {{14, 17, 1, 2, 4, 8, 31}}},
      {'3', {{30, 1, 1, 14, 1, 1, 30}}},
      {'4', {{2, 6, 10, 18, 31, 2, 2}}},
      {'5', {{31, 16, 16, 30, 1, 1, 30}}},
      {'6', {{14, 16, 16, 30, 17, 17, 14}}},
      {'7', {{31, 1, 2, 4, 8, 8, 8}}},
      {'8', {{14, 17, 17, 14, 17, 17, 14}}},
      {'9', {{14, 17, 17, 15, 1, 1, 14}}},
      {'.', {{0, 0, 0, 0, 0, 6, 6}}},
      {':', {{0, 6, 6, 0, 6, 6, 0}}},
      {'-', {{0, 0, 0, 31, 0, 0, 0}}},
      {'/', {{1, 2, 2, 4, 8, 8, 16}}},
      {'+', {{0, 4, 4, 31, 4, 4, 0}}},
      {'(', {{2, 4, 8, 8, 8, 4, 2}}},
      {')', {{8, 4, 2, 2, 2, 4, 8}}},
      {'=', {{0, 0, 31, 0, 31, 0, 0}}}};

  for (char letter : value) {
    auto found =
        font.find(char(std::toupper(static_cast<unsigned char>(letter))));

    if (found != font.end()) {
      for (int row = 0; row < 7; ++row) {
        for (int col = 0; col < 5; ++col) {
          if (found->second[row] & (1 << (4 - col))) {
            for (int a = 0; a < scale; ++a) {
              for (int b = 0; b < scale; ++b) {
                canvas.pixel(x + col * scale + a, y + row * scale + b, c);
              }
            }
          }
        }
      }
    }

    x += 6 * scale;
  }
}

} // namespace mv