#ifndef MEDIVIEW_EXPLORER_H
#define MEDIVIEW_EXPLORER_H
#include "graphics.h"
namespace mv {
struct Structure {
    std::string name, location, description;
    Color color;
    std::vector<Polygon> parts;
    Point label;
    int motion; // 0 fixed, 1 lung expansion, 2 diaphragm descent
};
struct SystemModel { std::string name, summary; Color color; std::vector<Structure> structures; };
const std::vector<SystemModel>& models(bool female=false);
Polygon bodyOutline();
struct QuizQuestion { int system, structure, correct; std::vector<std::string> options; };
struct Explorer {
    double zoom=1, panX=0, panY=0, phase=0;
    bool playing=false, labels=true, female=false;
    int selected=-1, system=3;
    bool comparing=false, quiz=false;
    int secondary=0, infoPage=0, question=0, score=0, answer=-1;
    double swallowTime=0;
    std::vector<QuizQuestion> questions;
    void toggleCompare();
    void nextSecondary();
    void swapSystems();
    void selectOrgan(int);
    void toggleVariant();
    void toggleAnimation();
    void resetAnimation();
    void startQuiz(unsigned seed);
    void loadQuestion();
    void answerQuiz(int);
    void nextQuestion();
    void exitQuiz();
    bool quizFinished() const;
    void turnPage(int);
    Point toScreen(Point) const;
    Point toModel(Point) const;
    void zoomAt(Point,double);
    void tick(double);
    void resetView();
    void resetBreathing();
    void selectSystem(int);
    void cycleSelection(int);
    const SystemModel& model() const;
};
std::string extendedInfo(const std::string& name);
std::string systemInfo(int system,bool female);
std::vector<std::string> informationLines(const Explorer&);
int informationPages(const Explorer&);
Point swallowPosition(double seconds);
std::string swallowStage(double seconds);
bool inViewport(Point);
bool contains(const Polygon&,Point);
std::vector<Polygon> animatedParts(const Structure&,double);
int hitTest(const Explorer&,Point);
void renderExplorer(Canvas&,const Explorer&);
bool activate(Explorer&,Point);
}
#endif