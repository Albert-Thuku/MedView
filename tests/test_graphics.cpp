#include "graphics.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <algorithm>
using namespace mv;
const Color black={0,0,0}, white={255,255,255};
int checks=0;
void require(bool ok,const char* message) {++checks;if(!ok)throw std::runtime_error(message);}
bool near(double a,double b) {return std::abs(a-b)<1e-7;}
bool lit(const Canvas& c,int x,int y) {return c.get(x,y).r==255;}
double area(const Polygon& p) {
    double a=0;for(std::size_t i=0;i<p.size();++i){Point u=p[i],v=p[(i+1)%p.size()];a+=u.x*v.y-u.y*v.x;}return std::abs(a)/2;
}
int main() {
    try {
        // Endpoint inclusion, exact dominant-axis count and connectivity in 8 octants.
        for(Point end:Polygon{{29,23},{23,29},{17,29},{11,23},{11,17},{17,11},{23,11},{29,17},{20,20},{20,30},{30,20}}) {
            Canvas c(41,41,black);line(c,{20,20},end,white);
            require(lit(c,20,20)&&lit(c,int(end.x),int(end.y)),"Line missing endpoint");
            int count=0;
            for(int y=0;y<41;++y)for(int x=0;x<41;++x)if(lit(c,x,y)) {
                ++count;int neighbors=0;
                for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx)
                    if((dx||dy)&&x+dx>=0&&x+dx<41&&y+dy>=0&&y+dy<41&&lit(c,x+dx,y+dy))++neighbors;
                require(neighbors>0||(end.x==20&&end.y==20),"Disconnected line");
            }
            require(count==int(std::max(std::abs(end.x-20),std::abs(end.y-20)))+1,"Line pixel count");
        }
        Rect viewport={10,10,30,30};Point a={0,20},b={40,20};
        require(clipLine(a,b,viewport)&&near(a.x,10)&&near(b.x,30),"Horizontal clip");
        a={20,0};b={20,40};require(clipLine(a,b,viewport)&&near(a.y,10)&&near(b.y,30),"Vertical clip");
        a={0,0};b={40,40};require(clipLine(a,b,viewport)&&near(a.x,10)&&near(b.x,30),"Diagonal clip");
        a={0,0};b={5,0};require(!clipLine(a,b,viewport),"Outside line rejected");
        a={10,10};b=a;require(clipLine(a,b,viewport),"Boundary point accepted");
        a={9,9};b=a;require(!clipLine(a,b,viewport),"Outside point rejected");
        Polygon clipped=clipPolygon({{0,0},{40,0},{40,40},{0,40}},viewport);
        require(near(area(clipped),400),"Polygon clip area");
        for(Point p:clipped)require(p.x>=10&&p.x<=30&&p.y>=10&&p.y<=30,"Clip containment");
        require(clipPolygon({{0,0},{4,0},{4,4}},viewport).empty(),"Outside polygon");
        require(clipPolygon({},viewport).empty(),"Empty polygon");
        Polygon concave={{2,2},{12,2},{12,6},{6,6},{6,12},{2,12}};
        Canvas fill(20,20,black);fillPolygon(fill,concave,white);
        int filled=0;for(int y=0;y<20;++y)for(int x=0;x<20;++x)filled+=lit(fill,x,y)?1:0;
        require(filled==64,"Concave fill area");require(!lit(fill,8,8)&&lit(fill,3,8),"Concave notch");
        std::reverse(concave.begin(),concave.end());Canvas reverse(20,20,black);fillPolygon(reverse,concave,white);
        for(int y=0;y<20;++y)for(int x=0;x<20;++x)require(lit(fill,x,y)==lit(reverse,x,y),"Winding independence");
        Canvas circ(41,41,black);circle(circ,{20,20},9,white);
        require(lit(circ,29,20)&&lit(circ,20,29),"Circle cardinal points");
        for(int y=0;y<41;++y)for(int x=0;x<41;++x)if(lit(circ,x,y)) {
            require(lit(circ,40-x,y)&&lit(circ,x,40-y)&&lit(circ,y,x),"Circle symmetry");
            require(std::abs(std::hypot(x-20,y-20)-9)<0.8,"Circle radial error");
        }
        Canvas ell(61,61,black);ellipse(ell,{30,30},18,10,white);
        require(lit(ell,48,30)&&lit(ell,30,40),"Ellipse cardinal points");
        for(int y=0;y<61;++y)for(int x=0;x<61;++x)if(lit(ell,x,y))require(lit(ell,60-x,y)&&lit(ell,x,60-y),"Ellipse symmetry");
        Canvas degenerate(11,11,black);ellipse(degenerate,{5,5},0,3,white);
        require(lit(degenerate,5,2)&&lit(degenerate,5,8),"Degenerate ellipse");
        circle(degenerate,{1,1},0,white);require(lit(degenerate,1,1),"Zero circle");
        auto curve=bezier({0,0},{0,10},{10,10},{10,0},10);
        require(near(curve.front().x,0)&&near(curve.back().x,10),"Bezier endpoints");
        require(near(curve[5].x,5)&&near(curve[5].y,7.5),"Bezier midpoint");
        auto spline=catmullRom({{0,0},{10,20},{30,0}},8);
        require(near(spline[8].x,10)&&near(spline[8].y,20)&&near(spline.back().x,30),"Spline knot interpolation");
        auto moved=transform({{4,5}},{2,3},2,3,{10,20});
        require(near(moved[0].x,16)&&near(moved[0].y,29),"Transform pivot and translation");
        bool rejected=false;try{bezier({0,0},{0,0},{0,0},{0,0},0);}catch(const std::invalid_argument&){rejected=true;}
        require(rejected,"Reject invalid sampling count");
        std::cout<<"PASS: "<<checks<<" checks (lines, clipping, filling, circles, ellipses, curves, transforms).\n";
        return 0;
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<"\n";return 1;}
}
