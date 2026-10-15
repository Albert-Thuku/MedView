#include "graphics.h"
#include <algorithm>
#include <cmath>

using namespace mv;

namespace {

const int S = 3;

const Color white = {234,243,249};
const Color teal = {94,226,206};
const Color muted = {143,169,189};

Point scaled(Point p) {
    return {p.x * S, p.y * S};
}

Polygon scaled(const Polygon& p) {
    return transform(p, {0,0}, S, S, {0,0});
}

Color blend(Color a, Color b, double t) {
    t = std::max(0.0, std::min(1.0, t));

    return {
        static_cast<unsigned char>(a.r + (b.r - a.r) * t),
        static_cast<unsigned char>(a.g + (b.g - a.g) * t),
        static_cast<unsigned char>(a.b + (b.b - a.b) * t)
    };
}

void label(
    Canvas& c,
    int x,
    int y,
    const std::string& s,
    Color color,
    int size = 2
) {
    text(c, x*S, y*S, s, color, size*S);
}

void box(Canvas& c, Rect r, Color color) {
    rectangle(
        c,
        {r.left*S, r.top*S, r.right*S, r.bottom*S},
        color,
        true
    );
}

void stroke(
    Canvas& c,
    const Polygon& p,
    Color color,
    double width = 1,
    bool closed = false
) {
    Polygon q = scaled(p);
    int radius = int(width*S/2);

    // Circular brush made from translated Bresenham strokes.
    for (int y = -radius; y <= radius; ++y) {
        for (int x = -radius; x <= radius; ++x) {
            if (x*x + y*y <= radius*radius) {
                outline(
                    c,
                    transform(q, {0,0}, 1, 1, {double(x),double(y)}),
                    color,
                    closed
                );
            }
        }
    }
}

void curve(
    Canvas& c,
    Point a,
    Point b,
    Point d,
    Point e,
    Color color,
    double width = 1
) {
    stroke(c, bezier(a,b,d,e,90), color, width);
}

void add(Polygon& p, Point a, Point b, Point c, Point d) {
    Polygon q = bezier(a,b,c,d,70);
    p.insert(p.end(), q.begin(), q.end());
}

void shade(
    Canvas& c,
    const Polygon& p,
    Color dark,
    Color light,
    Point focal,
    double spread
) {
    Canvas mask(c.width(), c.height(), {0,0,0});
    fillPolygon(mask, scaled(p), {255,255,255});

    for (int y = 0; y < c.height(); ++y) {
        for (int x = 0; x < c.width(); ++x) {
            if (mask.get(x,y).r) {
                double dx = (double(x)/S-focal.x)/spread;
                double dy = (double(y)/S-focal.y)/(spread*1.7);
                double t = std::max(0.0, 1-std::sqrt(dx*dx+dy*dy));

                c.pixel(x,y,blend(dark,light,0.15+0.85*t));
            }
        }
    }
}

Polygon lung(bool leftOnScreen) {
    Polygon p;

    if (leftOnScreen) {
        add(p,{499,275},{497,230},{465,244},{432,285});
        add(p,{432,285},{385,337},{355,454},{366,552});
        add(p,{366,552},{370,581},{393,548},{432,536});
        add(p,{432,536},{462,526},{490,537},{500,502});
        add(p,{500,502},{512,450},{499,386},{501,352});
        add(p,{501,352},{505,326},{500,302},{499,275});
    } else {
        add(p,{548,274},{551,232},{579,247},{614,287});
        add(p,{614,287},{662,343},{692,452},{683,553});
        add(p,{683,553},{682,580},{656,550},{620,539});
        add(p,{620,539},{587,530},{559,535},{553,507});
        add(p,{553,507},{548,479},{570,461},{567,427});
        add(p,{567,427},{565,402},{541,394},{543,363});
        add(p,{543,363},{548,332},{545,305},{548,274});
    }

    return p;
}

void branches(
    Canvas& c,
    Point a,
    double angle,
    double length,
    int depth,
    double width
) {
    const double dx = std::cos(angle);
    const double dy = std::sin(angle);

    Point b = {a.x+dx*length, a.y+dy*length};

    curve(
        c,
        a,
        {a.x+dx*length*0.3, a.y+dy*length*0.10},
        {b.x-dx*length*0.2, b.y-dy*length*0.4},
        b,
        {239,187,176},
        width
    );

    if (depth > 0) {
        branches(c,b,angle-0.5,length*0.65,depth-1,width*0.63);
        branches(c,b,angle+0.5,length*0.62,depth-1,width*0.63);
    }
}

void detailedLung(Canvas& c, bool left) {
    Polygon p = lung(left);

    stroke(c,p,{22,67,79},11,true);
    stroke(c,p,{36,110,115},7,true);

    shade(
        c,p,{119,53,71},{231,145,149},
        left ? Point{434,358} : Point{608,350},
        155
    );

    Canvas vessels(c.width(),c.height(),{0,0,0});

    if (left) {
        branches(vessels,{487,382},-2.13,48,4,5);
        branches(vessels,{479,391},2.6,41,4,5);
        branches(vessels,{483,397},1.83,56,4,6);
    } else {
        branches(vessels,{566,382},-1.02,49,4,5);
        branches(vessels,{575,391},0.65,42,4,5);
        branches(vessels,{580,408},1.18,48,4,5);
    }

    Canvas mask(c.width(),c.height(),{0,0,0});
    fillPolygon(mask,scaled(p),{255,255,255});

    for (int y=0; y<c.height(); ++y) {
        for (int x=0; x<c.width(); ++x) {
            if (mask.get(x,y).r && vessels.get(x,y).r)
                c.pixel(x,y,vessels.get(x,y));
        }
    }

    if (left) {
        curve(
            c,{387,418},{425,430},{460,417},{494,389},
            {143,73,90},2.5
        );
        curve(
            c,{381,514},{425,474},{464,450},{494,392},
            {143,73,90},3
        );
    } else {
        curve(
            c,{557,395},{588,427},{626,477},{676,514},
            {143,73,90},3
        );
    }

    stroke(c,p,teal,2.3,true);
}

}

void detailedOverview(const std::string& dir) {
    Canvas c(1200*S,810*S,{9,23,39});

    // Radial background computed directly in C++.
    for (int y=0; y<c.height(); ++y) {
        for (int x=0; x<c.width(); ++x) {
            double dx = (double(x)/S-520)/650;
            double dy = (double(y)/S-370)/650;

            c.pixel(
                x,y,
                blend(
                    {7,20,35},
                    {19,43,64},
                    std::max(0.0,1-std::sqrt(dx*dx+dy*dy))
                )
            );
        }
    }

    box(c,{0,0,205,810},{12,29,47});
    stroke(c,{{205,0},{205,810}},{37,61,79});

    label(c,23,32,"MEDIVIEW",teal,3);
    label(c,24,71,"LEARN ANATOMY",muted,1);

    const std::string systems[] = {
        "SKELETAL","MUSCULAR","DIGESTIVE","RESPIRATORY",
        "CIRCULATORY","NERVOUS","URINARY","REPRODUCTIVE"
    };

    label(c,24,134,"SYSTEM LAYERS",muted,1);

    for (int i=0; i<8; ++i) {
        int y = 176+i*51;

        if (i==3) {
            box(c,{12,double(y-12),194,double(y+30)},{27,75,85});
            box(c,{12,double(y-12),16,double(y+30)},teal);
        }

        label(c,26,y,systems[i],i==3 ? teal : muted,2);
    }

    label(c,24,663,"CP1 FOUNDATION",teal,1);
    label(c,24,689,"STATIC VIEW",white,2);
    label(c,24,727,"OTHER LAYERS",muted,1);
    label(c,24,746,"PLANNED",muted,1);

    label(c,238,31,"RESPIRATORY EXPLORER",white,3);
    label(c,240,72,"DETAILED COMPONENT STUDY / FRONT VIEW",muted,2);

    // Torso constructed from connected cubic Bezier segments.
    Polygon body;
    add(body,{474,143},{470,185},{471,208},{419,227});
    add(body,{419,227},{361,245},{316,241},{294,307});
    add(body,{294,307},{275,352},{297,401},{278,478});
    add(body,{278,478},{265,536},{249,598},{241,652});
    add(body,{241,652},{250,669},{281,674},{301,661});
    add(body,{301,661},{327,610},{328,553},{344,488});
    add(body,{344,488},{348,547},{359,603},{349,675});
    add(body,{349,675},{444,690},{597,690},{699,675});
    add(body,{699,675},{689,602},{702,542},{707,488});
    add(body,{707,488},{724,553},{724,610},{746,661});
    add(body,{746,661},{762,674},{793,669},{804,652});
    add(body,{804,652},{796,594},{780,532},{770,478});
    add(body,{770,478},{751,400},{774,351},{753,307});
    add(body,{753,307},{733,245},{685,245},{629,227});
    add(body,{629,227},{577,208},{578,185},{576,143});

    shade(c,body,{93,73,77},{200,154,137},{523,330},310);
    stroke(c,body,{171,138,127},1.2,true);

    curve(c,{477,159},{475,201},{493,231},{502,254},{222,178,153},7);
    curve(c,{573,159},{572,201},{554,231},{550,254},{222,178,153},7);
    curve(c,{334,268},{398,253},{454,259},{498,271},{224,178,156},5);
    curve(c,{552,271},{600,259},{656,253},{716,268},{224,178,156},5);
    curve(c,{322,311},{338,377},{327,431},{310,472},{155,113,104},6);
    curve(c,{727,311},{710,377},{721,431},{737,472},{155,113,104},6);

    // Diaphragm.
    Polygon diaphragm;
    add(diaphragm,{364,581},{421,520},{617,518},{684,581});
    add(diaphragm,{684,581},{681,612},{678,631},{670,643});
    add(diaphragm,{670,643},{628,583},{555,562},{529,590});
    add(diaphragm,{529,590},{492,562},{414,589},{378,643});
    add(diaphragm,{378,643},{369,630},{364,604},{364,581});

    shade(c,diaphragm,{102,49,61},{180,103,104},{525,564},190);

    Canvas fibres(c.width(),c.height(),{0,0,0});

    for (int i=0; i<15; ++i) {
        double x=382+i*20;

        curve(
            fibres,
            {x,605+18*std::abs(x-525)/150},
            {x,576},
            {(x+525)/2,549},
            {525,548},
            {191,116,115},
            1
        );
    }

    Canvas diaphragmMask(c.width(),c.height(),{0,0,0});
    fillPolygon(diaphragmMask,scaled(diaphragm),{255,255,255});

    for (int y=0; y<c.height(); ++y) {
        for (int x=0; x<c.width(); ++x) {
            if (diaphragmMask.get(x,y).r && fibres.get(x,y).r)
                c.pixel(x,y,fibres.get(x,y));
        }
    }

    stroke(c,diaphragm,{210,144,135},1.3,true);

    detailedLung(c,true);
    detailedLung(c,false);

    // Trachea, rings and main bronchi.
    curve(c,{525,204},{525,255},{525,325},{524,348},{76,110,125},25);
    curve(c,{525,204},{525,255},{525,325},{524,348},{157,188,196},18);

    for (int y=212; y<340; y+=13)
        ellipse(c,scaled({525,double(y)}),10*S,3*S,{207,221,222});

    curve(c,{525,340},{522,359},{505,367},{485,386},{89,123,137},16);
    curve(c,{525,340},{532,360},{550,370},{569,386},{89,123,137},16);
    curve(c,{525,340},{522,359},{505,367},{485,386},{198,210,207},9);
    curve(c,{525,340},{532,360},{550,370},{569,386},{198,210,207},9);

    label(c,325,706,"RIGHT LUNG",muted,1);
    label(c,620,706,"LEFT LUNG",muted,1);
    label(c,367,734,"ANATOMICAL SIDES / FRONT VIEW",muted,1);

    // Static information panel.
    box(c,{835,136,1170,692},{13,32,51});
    stroke(
        c,
        {{835,136},{1170,136},{1170,692},{835,692}},
        {40,69,89},1,true
    );

    label(c,858,162,"STRUCTURE STUDY",teal,1);
    label(c,858,198,"LUNGS",white,5);
    label(c,858,251,"RESPIRATORY SYSTEM",teal,2);
    stroke(c,{{858,293},{1146,293}},{41,72,91});

    label(c,858,318,"01  CONTOURS",white,2);
    label(c,858,348,"ASYMMETRIC BEZIER SHAPES",muted,1);
    label(c,858,367,"LOBAR DIVISION GUIDES",muted,1);

    label(c,858,412,"02  AIRWAYS",white,2);
    label(c,858,442,"TRACHEAL RINGS AND",muted,1);
    label(c,858,461,"BRANCHING BRONCHIAL PATHS",muted,1);

    label(c,858,506,"03  DIAPHRAGM",white,2);
    label(c,858,536,"CURVED DOME AND",muted,1);
    label(c,858,555,"SCHEMATIC MUSCLE FIBRES",muted,1);

    box(c,{857,605,1148,657},{25,66,76});
    label(c,875,619,"STATIC CP1 RENDER",teal,2);

    stroke(c,{{238,774},{1170,774}},{39,65,83});
    label(
        c,239,791,
        "PURE C++ / SCANLINE FILL / BEZIER / BRESENHAM / 3X SUPERSAMPLING",
        muted,1
    );

    label(c,858,724,"SCHEMATIC ANATOMY",muted,1);
    label(c,858,743,"NOT AN INTERACTIVE SCREEN",muted,1);

    // Average each 3x3 block to produce antialiased output.
    Canvas result(1200,810,{0,0,0});

    for (int y=0; y<810; ++y) {
        for (int x=0; x<1200; ++x) {
            int r=0,g=0,b=0;

            for (int j=0; j<S; ++j) {
                for (int i=0; i<S; ++i) {
                    Color v=c.get(x*S+i,y*S+j);
                    r+=v.r;
                    g+=v.g;
                    b+=v.b;
                }
            }

            result.pixel(x,y,{
                static_cast<unsigned char>(r/(S*S)),
                static_cast<unsigned char>(g/(S*S)),
                static_cast<unsigned char>(b/(S*S))
            });
        }
    }

    result.saveBMP(dir+"/01_respiratory.bmp");
}