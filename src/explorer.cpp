#include "explorer.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <sstream>

namespace mv {
namespace {
const Color bg = {9, 23, 39}, panel = {15, 33, 50}, teal = {94, 226, 206},
            ink = {234, 243, 249}, muted = {143, 169, 189},
            gold = {255, 219, 138};
const Rect viewport = {218, 118, 818, 706};
void button(Canvas &c, int x, int y, int w, const std::string &s,
            bool active = false, bool enabled = true) {
  rectangle(c, {double(x), double(y), double(x + w), double(y + 38)},
            active ? Color{28, 78, 87} : Color{25, 43, 60}, true);
  if (active)
    rectangle(c, {double(x), double(y), double(x + 3), double(y + 38)}, teal,
              true);
  text(c, x + 12, y + 12, s,
       enabled ? (active ? teal : ink) : Color{77, 97, 114}, 2);
}
int wrapped(Canvas &c, int x, int y, const std::string &s, Color color = muted,
            int chars = 25, int scale = 2) {
  std::istringstream words(s);
  std::string word, line;
  while (words >> word) {
    if (!line.empty() && int(line.size() + word.size() + 1) > chars) {
      text(c, x, y, line, color, scale);
      y += scale * 10 + 4;
      line.clear();
    }
    if (!line.empty())
      line += ' ';
    line += word;
  }
  if (!line.empty()) {
    text(c, x, y, line, color, scale);
    y += scale * 10 + 4;
  }
  return y;
}
Rect scaledViewport(double scale) {
  return {viewport.left * scale, viewport.top * scale, viewport.right * scale,
          viewport.bottom * scale};
}
Point screenPoint(const Explorer &e, Point p, double scale) {
  p = e.toScreen(p);
  return {p.x * scale, p.y * scale};
}
Polygon screenPoly(const Explorer &e, const Polygon &p, double scale = 1) {
  Polygon q;
  for (Point v : p)
    q.push_back(screenPoint(e, v, scale));
  return clipPolygon(q, scaledViewport(scale));
}
Color tone(Color base, double light) {
  return {static_cast<unsigned char>(std::min(255.0, base.r * light)),
          static_cast<unsigned char>(std::min(255.0, base.g * light)),
          static_cast<unsigned char>(std::min(255.0, base.b * light))};
}
void shaded(Canvas &c, const Explorer &e, const Polygon &p, Color base,
            Color edge, double scale = 1) {
  auto q = screenPoly(e, p, scale);
  if (q.size() < 3)
    return;
  double minX = q[0].x, maxX = minX, minY = q[0].y, maxY = minY;
  for (Point v : q) {
    minX = std::min(minX, v.x);
    maxX = std::max(maxX, v.x);
    minY = std::min(minY, v.y);
    maxY = std::max(maxY, v.y);
  }
  // Lighting is anchored in model space, so panning/clipping cannot move the
  // highlight.
  double l = p[0].x, r = l, t = p[0].y, b = t;
  for (Point v : p) {
    l = std::min(l, v.x);
    r = std::max(r, v.x);
    t = std::min(t, v.y);
    b = std::max(b, v.y);
  }
  int left = int(std::floor(minX)), top = int(std::floor(minY));
  int w = int(std::ceil(maxX)) - left + 1, h = int(std::ceil(maxY)) - top + 1;
  Canvas mask(w, h, {0, 0, 0});
  fillPolygon(mask, transform(q, {0, 0}, 1, 1, {-double(left), -double(top)}),
              {255, 255, 255});
  for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x)
      if (mask.get(x, y).r) {
        Point v = e.toModel({(left + x) / scale, (top + y) / scale});
        double u = (v.x - l) / std::max(1.0, r - l),
               z = (v.y - t) / std::max(1.0, b - t);
        double roundness = std::sqrt(std::max(0.0, 1 - std::pow(2 * u - 1, 2)));
        double highlight = std::exp(
            -((u - .32) * (u - .32) * 28 + (z - .23) * (z - .23) * 14));
        double light = .56 + .38 * roundness + .16 * (1 - z) + .17 * highlight;
        c.pixel(left + x, top + y, tone(base, light));
      }
  outline(c, q, edge);
}
void path(Canvas &c, const Explorer &e, const Polygon &p, Color color,
          double scale = 1) {
  for (std::size_t i = 1; i < p.size(); ++i) {
    Point a = screenPoint(e, p[i - 1], scale), b = screenPoint(e, p[i], scale);
    if (clipLine(a, b, scaledViewport(scale)))
      line(c, a, b, color);
  }
}
Point motionPoint(Point p, int motion, double phase) {
  double b = (1 - std::cos(phase)) * 0.5;
  if (motion == 1) {
    p.x *= 1 + 0.04 * b;
    p.y = 117 + (p.y - 117) * (1 + 0.06 * b);
  }
  if (motion == 2)
    p.y += 8 * b;
  return p;
}
} // namespace
const SystemModel &Explorer::model() const { return models(female)[system]; }
bool inViewport(Point p) {
  return p.x >= viewport.left && p.x <= viewport.right && p.y >= viewport.top &&
         p.y <= viewport.bottom;
}
Point Explorer::toScreen(Point p) const {
  return {518 + p.x * zoom + panX, 410 + (p.y - 280) * zoom + panY};
}
Point Explorer::toModel(Point p) const {
  return {(p.x - 518 - panX) / zoom, 280 + (p.y - 410 - panY) / zoom};
}
void Explorer::zoomAt(Point p, double factor) {
  Point m = toModel(p);
  zoom = std::max(0.6, std::min(4.0, zoom * factor));
  panX = p.x - 518 - m.x * zoom;
  panY = p.y - 410 - (m.y - 280) * zoom;
}
void Explorer::tick(double dt) {
  if (!playing || quiz || dt <= 0)
    return;
  if (system == 3)
    phase = std::fmod(phase + dt * 6.283185307179586 / 5.0, 6.283185307179586);
  else if (system == 2) {
    swallowTime = std::min(8.0, swallowTime + dt);
    if (swallowTime >= 8)
      playing = false;
  }
}
void Explorer::resetView() {
  zoom = 1;
  panX = panY = 0;
}
void Explorer::resetBreathing() {
  phase = 0;
  playing = false;
}
void Explorer::selectSystem(int next) {
  if (quiz || next < 0 || next >= 8)
    return;
  system = next;
  selected = -1;
  infoPage = 0;
  if (secondary == system)
    nextSecondary();
  resetAnimation();
  resetView();
}
void Explorer::cycleSelection(int direction) {
  if (quiz)
    return;
  infoPage = 0;
  int n = int(model().structures.size());
  selected = selected < 0 ? (direction > 0 ? 0 : n - 1)
                          : (selected + direction + n) % n;
}
bool contains(const Polygon &p, Point q) {
  if (p.size() < 3)
    return false;
  bool inside = false;
  for (std::size_t i = 0, j = p.size() - 1; i < p.size(); j = i++) {
    Point a = p[j], b = p[i];
    double cross = (q.x - a.x) * (b.y - a.y) - (q.y - a.y) * (b.x - a.x);
    if (std::abs(cross) < 1e-7 && q.x >= std::min(a.x, b.x) &&
        q.x <= std::max(a.x, b.x) && q.y >= std::min(a.y, b.y) &&
        q.y <= std::max(a.y, b.y))
      return true;
    if ((a.y > q.y) != (b.y > q.y) &&
        q.x < (b.x - a.x) * (q.y - a.y) / (b.y - a.y) + a.x)
      inside = !inside;
  }
  return inside;
}
std::vector<Polygon> animatedParts(const Structure &organ, double phase) {
  auto parts = organ.parts;
  for (auto &p : parts)
    for (auto &v : p)
      v = motionPoint(v, organ.motion, phase);
  return parts;
}
int hitTest(const Explorer &e, Point p) {
  if (!inViewport(p))
    return -1;
  Point m = e.toModel(p);
  const auto &organs = e.model().structures;
  for (int i = int(organs.size()) - 1; i >= 0; --i)
    for (const auto &part : animatedParts(organs[i], e.phase))
      if (contains(part, m))
        return i;
  return -1;
}
bool activate(Explorer &e, Point p) {
  auto within = [&](double x, double y, double w, double h) {
    return p.x >= x && p.x <= x + w && p.y >= y && p.y <= y + h;
  };
  if (within(16, 563, 178, 38)) {
    e.toggleCompare();
    return true;
  }
  if (within(16, 612, 178, 38)) {
    if (e.quiz)
      e.exitQuiz();
    else
      e.startQuiz(std::random_device{}());
    return true;
  }
  if (e.quiz) {
    if (e.quizFinished()) {
      if (within(858, 736, 145, 38))
        e.startQuiz(std::random_device{}());
    } else {
      for (int i = 0; i < 4; ++i)
        if (within(850, 225 + i * 50, 320, 44)) {
          e.answerQuiz(i);
          return true;
        }
      if (within(858, 736, 145, 38)) {
        e.nextQuestion();
        return true;
      }
    }
    if (within(1012, 736, 168, 38)) {
      e.exitQuiz();
      return true;
    }
  } else {
    for (int i = 0; i < 8; ++i)
      if (within(16, 155 + i * 49, 178, 38)) {
        e.selectSystem(i);
        return true;
      }
    if (within(850, 179, 320, int(e.model().structures.size()) * 29)) {
      int i = int((p.y - 179) / 29);
      if (p.y <= 179 + i * 29 + 24) {
        e.selectOrgan(i);
        return true;
      }
    }
    if (within(1040, 138, 130, 30)) {
      e.selectOrgan(-1);
      return true;
    }
    if (e.comparing) {
      if (within(16, 660, 178, 38)) {
        e.swapSystems();
        return true;
      }
      if (within(16, 709, 178, 38)) {
        e.nextSecondary();
        return true;
      }
    } else if (e.system == 7 && within(16, 660, 178, 38)) {
      e.toggleVariant();
      return true;
    }
    if ((e.system == 2 || e.system == 3) && within(238, 736, 124, 38)) {
      e.toggleAnimation();
      return true;
    }
    if ((e.system == 2 || e.system == 3) && within(374, 736, 126, 38)) {
      e.resetAnimation();
      return true;
    }
    if (within(520, 736, 130, 38)) {
      e.labels = !e.labels;
      return true;
    }
  }
  if ((!e.quiz || e.answer >= 0) && !e.quizFinished()) {
    if (within(858, 657, 80, 38)) {
      e.turnPage(-1);
      return true;
    }
    if (within(1085, 657, 85, 38)) {
      e.turnPage(1);
      return true;
    }
  }
  if (within(670, 736, 140, 38)) {
    e.resetView();
    return true;
  }
  if (within(690, 72, 46, 38)) {
    e.zoomAt({518, 410}, 1.2);
    return true;
  }
  if (within(748, 72, 46, 38)) {
    e.zoomAt({518, 410}, 1 / 1.2);
    return true;
  }
  return false;
}
namespace {
void details(Canvas &c, const Explorer &e, const Structure &organ,
             double scale) {
  Color shadow = tone(organ.color, .57), light = tone(organ.color, 1.22);
  // Decorative strokes are restricted to the parent structure and deform with
  // it.
  auto stroke = [&](Polygon points, Color color) {
    for (std::size_t i = 1; i < points.size(); ++i) {
      Point mid = {(points[i - 1].x + points[i].x) / 2,
                   (points[i - 1].y + points[i].y) / 2};
      bool inside = false;
      for (const auto &part : organ.parts)
        if (contains(part, mid)) {
          inside = true;
          break;
        }
      if (inside)
        path(c, e,
             {motionPoint(points[i - 1], organ.motion, e.phase),
              motionPoint(points[i], organ.motion, e.phase)},
             color, scale);
    }
  };
  auto curve = [&](Point a, Point b, Point d, Point f, Color color) {
    stroke(bezier(a, b, d, f, 40), color);
  };
  const std::string &name = organ.name;
  if (name == "RIGHT LUNG" || name == "LEFT LUNG") {
    int side = name == "RIGHT LUNG" ? -1 : 1;
    for (int i = 0; i < 6; ++i) {
      double y = 136 + i * 11;
      curve({side * 15.0, 149}, {side * 25.0, y}, {side * 35.0, y - 3},
            {side * 47.0, y + 5}, light);
      for (int j = 0; j < 3; ++j) {
        double x = 27 + j * 6;
        curve({side * x, y + 1}, {side * (x + 3), y - 4},
              {side * (x + 5), y - 8}, {side * (x + 6), y - 10},
              tone(organ.color, 1.08));
      }
    }
    curve({side * 17.0, 148}, {side * 26.0, 169}, {side * 35.0, 185},
          {side * 47.0, 197}, shadow);
    if (side < 0)
      curve({-13, 166}, {-26, 162}, {-39, 163}, {-52, 166}, shadow);
  } else if (name == "TRACHEA") {
    for (int y = 81; y < 145; y += 6)
      curve({-5.0, double(y)}, {-2.0, y + 2.0}, {2.0, y + 2.0},
            {5.0, double(y)}, light);
  } else if (name == "BRAIN") {
    for (int side : {-1, 1})
      for (int i = 0; i < 5; ++i) {
        double y = 19 + i * 7;
        curve({side * 4.0, y}, {side * 24.0, y - 8}, {side * 7.0, y + 12},
              {side * 21.0, y + 5}, shadow);
      }
    curve({0, 15}, {-3, 28}, {3, 41}, {0, 53}, shadow);
  } else if (name == "HEART") {
    curve({5, 151}, {-6, 164}, {14, 180}, {6, 193}, shadow);
    curve({4, 160}, {11, 161}, {19, 167}, {25, 174}, light);
    for (int i = 0; i < 3; ++i)
      curve({3.0 + i, 165.0 + i * 7}, {-7, 164.0 + i * 7}, {-9, 169.0 + i * 7},
            {-12, 171.0 + i * 7}, shadow);
  } else if (name == "RIGHT KIDNEY" || name == "LEFT KIDNEY") {
    int side = name == "RIGHT KIDNEY" ? -1 : 1;
    double y = side < 0 ? 235 : 227;
    curve({side * 29.0, y - 14}, {side * 46.0, y - 20}, {side * 52.0, y + 13},
          {side * 32.0, y + 15}, light);
    curve({side * 25.0, y - 7}, {side * 36.0, y - 3}, {side * 35.0, y + 6},
          {side * 25.0, y + 9}, shadow);
  } else if (e.system == 1) {
    for (const auto &part : organ.parts) {
      double l = part[0].x, r = l, t = part[0].y, b = t;
      for (Point v : part) {
        l = std::min(l, v.x);
        r = std::max(r, v.x);
        t = std::min(t, v.y);
        b = std::max(b, v.y);
      }
      for (int i = 1; i < 7; ++i) {
        double x = l + (r - l) * i / 7;
        curve({x, t}, {x - 2, t + (b - t) * .35}, {x + 2, t + (b - t) * .65},
              {x, b}, i % 2 ? shadow : light);
      }
    }
  } else if (name == "STOMACH") {
    for (int i = 0; i < 4; ++i)
      curve({19.0 + i * 4, 197}, {37.0 + i, 209}, {7.0 + i * 4, 223},
            {24.0 + i * 2, 233}, shadow);
  } else if (name == "LIVER") {
    curve({-8, 188}, {-4, 197}, {-10, 207}, {-5, 216}, shadow);
    curve({-45, 205}, {-30, 192}, {-17, 195}, {-12, 197}, light);
  } else if (name == "LARGE INTESTINE") {
    for (int y = 261; y < 305; y += 8)
      for (int side : {-1, 1})
        curve({side * 35.0, double(y)}, {side * 38.0, y + 3.0},
              {side * 41.0, y + 3.0}, {side * 44.0, double(y)}, shadow);
    for (int x = -27; x < 34; x += 9)
      curve({double(x), 243}, {x - 2.0, 247}, {x - 2.0, 250}, {double(x), 253},
            shadow);
  } else if (name == "SKULL") {
    for (int side : {-1, 1}) {
      Polygon eye;
      for (int i = 0; i <= 48; ++i) {
        double a = i * 6.283185307 / 48;
        eye.push_back({side * 10.0 + 6 * std::cos(a), 35 + 7 * std::sin(a)});
      }
      stroke(eye, shadow);
    }
    stroke({{-3, 45}, {0, 39}, {3, 45}, {-3, 45}}, shadow);
    curve({-13, 53}, {-5, 58}, {5, 58}, {13, 53}, shadow);
    for (int x = -9; x <= 9; x += 3)
      stroke({{double(x), 54}, {double(x), 59}}, shadow);
  } else if (name == "VERTEBRAL COLUMN") {
    for (int y = 81; y < 320; y += 12)
      curve({-5.0, double(y)}, {-2.0, y + 2.0}, {2.0, y + 2.0},
            {5.0, double(y)}, shadow);
  } else if (name == "PANCREAS") {
    for (int x = -16; x < 31; x += 6)
      curve({double(x), 234}, {x - 3.0, 238}, {x + 3.0, 241}, {double(x), 244},
            shadow);
  } else if (name == "UTERUS") {
    curve({-9, 295}, {0, 299}, {0, 307}, {0, 315}, shadow);
    curve({9, 295}, {0, 299}, {0, 307}, {0, 315}, shadow);
  } else if (name == "TESTES" || name == "OVARIES") {
    for (const auto &part : organ.parts) {
      double x = 0, y = 0;
      for (Point v : part) {
        x += v.x;
        y += v.y;
      }
      x /= part.size();
      y /= part.size();
      for (int i = -2; i <= 2; ++i)
        curve({x - 4, y + i * 2}, {x - 2, y - 3 + i * 2},
              {x + 2, y + 3 + i * 2}, {x + 4, y + i * 2}, shadow);
    }
  } else if (name == "BLADDER") {
    curve({-13, 308}, {-11, 298}, {11, 298}, {13, 308}, light);
    curve({-9, 315}, {0, 325}, {0, 325}, {9, 315}, shadow);
  }
}
void renderAnatomy(Canvas &c, const Explorer &e, double scale) {
  shaded(c, e, bodyOutline(), {49, 70, 87}, {80, 108, 129}, scale);
  Polygon head;
  for (int i = 0; i < 96; ++i) {
    double a = i * 6.283185307 / 96;
    head.push_back({27 * std::cos(a), 40 + 33 * std::sin(a)});
  }
  shaded(c, e, head, {49, 70, 87}, {80, 108, 129}, scale);
  for (const auto &organ : e.model().structures) {
    for (const auto &p : animatedParts(organ, e.phase))
      shaded(c, e, p, organ.color, tone(organ.color, .77), scale);
    details(c, e, organ, scale);
  }
  // The comparison layer is an amber outline. Filled anatomy and selection
  // remain primary.
  if (e.comparing && !e.quiz) {
    Explorer other = e;
    other.system = e.secondary;
    other.phase = 0;
    for (const auto &organ : other.model().structures)
      for (const auto &p : organ.parts) {
        auto q = screenPoly(e, p, scale);
        outline(c, q, {239, 173, 83});
      }
  }
  if (e.system == 2 && !e.quiz && (e.swallowTime > 0 || e.playing)) {
    Point bolus = swallowPosition(e.swallowTime);
    Polygon food;
    for (int i = 0; i < 48; ++i) {
      double a = i * 6.283185307 / 48;
      food.push_back(
          {bolus.x + 3.8 * std::cos(a), bolus.y + 5.5 * std::sin(a)});
    }
    shaded(c, e, food, {255, 222, 100}, {255, 245, 193}, scale);
    if (e.swallowTime > 1.8 && e.swallowTime < 6.8) {
      Point wave = swallowPosition(std::max(1.8, e.swallowTime - .32));
      path(c, e,
           {{wave.x - 5, wave.y - 2},
            {wave.x - 2, wave.y},
            {wave.x + 2, wave.y},
            {wave.x + 5, wave.y - 2}},
           {255, 238, 171}, scale);
    }
  }
  if (e.selected >= 0 && e.selected < int(e.model().structures.size()))
    for (const auto &p :
         animatedParts(e.model().structures[e.selected], e.phase)) {
      auto q = screenPoly(e, p, scale);
      outline(c, q, gold);
      for (int dx = -2; dx <= 2; ++dx)
        for (int dy = -2; dy <= 2; ++dy)
          if (dx * dx + dy * dy <= 4)
            outline(c,
                    clipPolygon(
                        transform(q, {0, 0}, 1, 1, {dx * scale, dy * scale}),
                        scaledViewport(scale)),
                    gold);
    }
}
} // namespace
namespace {
void informationPanel(Canvas &c, const Explorer &e, int y = 470) {
  const auto lines = informationLines(e);
  int pages = informationPages(e),
      page = std::max(0, std::min(pages - 1, e.infoPage));
  for (int i = 0; i < 8 && page * 8 + i < int(lines.size()); ++i)
    text(c, 858, y + i * 22, lines[page * 8 + i], muted, 2);
  button(c, 858, 657, 80, "PREV", false, page > 0);
  button(c, 1085, 657, 85, "NEXT", false, page + 1 < pages);
  text(c, 964, 672, std::to_string(page + 1) + " / " + std::to_string(pages),
       teal, 1);
}
void quizPanel(Canvas &c, const Explorer &e) {
  if (e.quizFinished()) {
    text(c, 858, 145, "ROUND COMPLETE", teal, 2);
    text(c, 858, 207, "SCORE " + std::to_string(e.score) + " / 10", ink, 3);
    wrapped(c, 858, 280,
            "REPLAY FOR A NEW MIX OF STRUCTURES, OR RETURN TO EXPLORE TO "
            "REVIEW EACH SYSTEM.",
            muted, 25, 2);
    wrapped(c, 858, 430,
            "THIS IS A PRACTICE QUIZ. THE ANATOMY DRAWINGS ARE SCHEMATIC.",
            muted, 25, 2);
    button(c, 858, 736, 145, "REPLAY");
    button(c, 1012, 736, 168, "EXPLORE");
    return;
  }
  const auto &q = e.questions[e.question];
  text(c, 858, 143, "QUIZ " + std::to_string(e.question + 1) + " / 10", teal,
       2);
  text(c, 858, 180, "NAME THE GOLD", ink, 2);
  text(c, 858, 201, "HIGHLIGHTED STRUCTURE", ink, 2);
  for (int i = 0; i < 4; ++i) {
    int y = 225 + i * 50;
    Color fill = {25, 43, 60};
    if (e.answer >= 0 && i == q.correct)
      fill = {28, 78, 66};
    else if (e.answer == i)
      fill = {99, 44, 51};
    rectangle(c, {850.0, double(y), 1170.0, double(y + 44)}, fill, true);
    wrapped(c, 860, y + 7, std::to_string(i + 1) + " " + q.options[i], ink, 25,
            2);
  }
  if (e.answer < 0) {
    wrapped(c, 858, 459,
            "CHOOSE 1-4. ZOOM OR PAN TO INSPECT THE HIGHLIGHT. LABELS AND "
            "NOTES ARE HIDDEN UNTIL YOU ANSWER.",
            muted, 25, 2);
  } else {
    text(c, 858, 440,
         e.answer == q.correct ? "CORRECT!" : "REVIEW THE GREEN ANSWER",
         e.answer == q.correct ? teal : gold, 1);
    informationPanel(c, e, 470);
  }
  button(c, 858, 736, 145, e.question == 9 ? "RESULTS" : "CONTINUE", false,
         e.answer >= 0);
  button(c, 1012, 736, 168, "EXPLORE");
}
} // namespace
void renderExplorer(Canvas &c, const Explorer &e) {
  const auto &model = e.model();
  rectangle(c, {0, 0, 205, 810}, panel, true);
  text(c, 22, 30, "MEDIVIEW", teal, 3);
  text(c, 22, 76, "ANATOMY LAB", muted, 1);
  text(c, 22, 121, e.quiz ? "QUIZ: SYSTEMS LOCKED" : "CHOOSE A SYSTEM", muted,
       1);
  for (int i = 0; i < 8; ++i)
    button(c, 16, 155 + 49 * i, 178, models()[i].name, e.system == i, !e.quiz);
  button(c, 16, 563, 178, e.comparing ? "END COMPARE" : "COMPARE", e.comparing);
  button(c, 16, 612, 178, e.quiz ? "EXIT QUIZ" : "QUIZ", e.quiz);
  if (e.comparing) {
    button(c, 16, 660, 178, "SWAP SYSTEMS");
    button(c, 16, 709, 178, "NEXT LAYER");
    text(c, 22, 765, "C LAYER / X SWAP", muted, 1);
  } else if (e.quiz) {
    text(c, 22, 675, "SCORE " + std::to_string(e.score) + " / 10", teal, 2);
    wrapped(c, 22, 717, "1-4 ANSWER / ENTER NEXT / ESC EXIT", muted, 25, 1);
  } else if (e.system == 7) {
    button(c, 16, 660, 178, e.female ? "VIEW MALE" : "VIEW FEMALE");
    text(c, 22, 715, e.female ? "FEMALE VIEW" : "MALE VIEW", teal, 2);
  } else {
    wrapped(c, 22, 672, "BODY RIGHT IS ON YOUR LEFT. KEYS 1-8 SWITCH SYSTEM.",
            muted, 25, 1);
  }
  text(c, 22, 789, "EDUCATIONAL / SCHEMATIC", muted, 1);
  text(c, 238, 28, model.name + " SYSTEM", ink, 3);
  text(c, 238, 69, model.summary, model.color, 1);
  text(c, 238, 92,
       e.comparing ? "AMBER OUTLINE: " + models(e.female)[e.secondary].name
                   : "SCROLL TO ZOOM / DRAG TO PAN",
       e.comparing ? Color{239, 173, 83} : muted, 1);
  button(c, 690, 72, 46, "+");
  button(c, 748, 72, 46, "-");
  rectangle(c, viewport, Color{12, 29, 44}, true);
  Canvas scene(2400, 1620, {12, 29, 44});
  renderAnatomy(scene, e, 2);
  for (int y = int(viewport.top); y <= int(viewport.bottom); ++y)
    for (int x = int(viewport.left); x <= int(viewport.right); ++x) {
      int r = 0, g = 0, b = 0;
      for (int dy = 0; dy < 2; ++dy)
        for (int dx = 0; dx < 2; ++dx) {
          Color v = scene.get(x * 2 + dx, y * 2 + dy);
          r += v.r;
          g += v.g;
          b += v.b;
        }
      c.pixel(x, y,
              {static_cast<unsigned char>((r + 2) / 4),
               static_cast<unsigned char>((g + 2) / 4),
               static_cast<unsigned char>((b + 2) / 4)});
    }
  if (e.labels && !e.quiz) {
    for (std::size_t i = 0; i < model.structures.size(); ++i) {
      const auto &organ = model.structures[i];
      Point anchor =
          e.toScreen(motionPoint(organ.label, organ.motion, e.phase));
      if (!inViewport(anchor))
        continue;
      bool left = i % 2 == 0;
      int row = int(i / 2), y = 180 + row * 115;
      int x = left ? 230 : 659;
      int width = std::min(25, int(organ.name.size())) * 6;
      Point end = {left ? double(x + width + 4) : double(x - 6), double(y + 4)};
      Point a = anchor, b = end;
      if (clipLine(a, b, viewport))
        line(c, a, b, {78, 109, 128});
      rectangle(
          c,
          {double(x - 3), double(y - 3), double(x + width + 3), double(y + 12)},
          Color{12, 29, 44}, true);
      text(c, x, y, organ.name.substr(0, 25),
           e.selected == int(i) ? gold : muted, 1);
    }
  }
  rectangle(c, {840, 118, 1180, 706}, panel, true);
  if (e.quiz)
    quizPanel(c, e);
  else {
    text(c, 858, 143, "STRUCTURES", teal, 2);
    rectangle(c, {1040, 138, 1170, 168}, {25, 43, 60}, true);
    text(c, 1050, 149, "SYSTEM INFO", ink, 1);
    for (std::size_t i = 0; i < model.structures.size(); ++i) {
      int y = 179 + int(i) * 29;
      rectangle(c, {850.0, double(y), 1170.0, double(y + 24)},
                e.selected == int(i) ? Color{35, 67, 78} : Color{22, 42, 58},
                true);
      text(c, 860, y + 8,
           std::to_string(i + 1) + "  " + model.structures[i].name,
           e.selected == int(i) ? gold : ink, 1);
    }
    line(c, {858, 396}, {1163, 396}, {44, 66, 83});
    wrapped(c, 858, 418,
            e.selected >= 0 && e.selected < int(model.structures.size())
                ? model.structures[e.selected].name
                : "SYSTEM OVERVIEW",
            ink, 25, 2);
    informationPanel(c, e);
  }
  if (!e.quiz && (e.system == 2 || e.system == 3)) {
    std::string stage =
        e.system == 2
            ? swallowStage(e.swallowTime)
            : (e.playing ? (e.phase < 3.14159265 ? "INHALING" : "EXHALING")
                         : "BREATHING PAUSED");
    text(c, 238, 716, stage, teal, 1);
    double progress =
        e.system == 2 ? e.swallowTime / 8 : (1 - std::cos(e.phase)) * .5;
    rectangle(c, {620, 715, 810, 720}, {37, 61, 79}, true);
    rectangle(c, {620, 715, 620 + 190 * progress, 720}, teal, true);
    button(c, 238, 736, 124,
           e.playing ? "PAUSE" : (e.system == 2 ? "SWALLOW" : "BREATHE"),
           e.playing);
    button(c, 374, 736, 126, "RESET");
  } else if (!e.quiz)
    text(c, 238, 716,
         e.comparing ? "FILLED: PRIMARY / OUTLINE: COMPARISON"
                     : "SELECT A STRUCTURE OR READ SYSTEM INFO",
         muted, 1);
  if (!e.quiz)
    button(c, 520, 736, 130, "LABELS", e.labels);
  button(c, 670, 736, 140, "RESET VIEW");
  text(c, 238, 792,
       e.quiz ? "1-4 ANSWER / ENTER CONTINUE / Q OR ESC EXIT / PGUP PGDN NOTES"
              : "SPACE ANIMATE / B RESET / L LABELS / R VIEW / Q QUIZ / C "
                "COMPARE / PGUP PGDN NOTES",
       muted, 1);
}
} // namespace mv