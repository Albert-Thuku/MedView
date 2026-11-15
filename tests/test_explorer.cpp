#include "explorer.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <set>
int checks=0;
void check(bool b,const char* m){++checks;if(!b)throw std::runtime_error(m);}
bool near(double a,double b){return std::abs(a-b)<1e-7;}
int main(int argc,char** argv){try{
 mv::Explorer e;
 check(mv::models().size()==8,"Eight systems");
 check(mv::hitTest(e,e.toScreen({-30,180}))==0,"Right lung pick");
 check(mv::hitTest(e,e.toScreen({32,180}))==1,"Left lung pick");
 check(mv::hitTest(e,e.toScreen({0,100}))==2,"Trachea pick");
 check(mv::hitTest(e,{850,300})==-1,"Panel excluded");
 auto anchor=mv::Point{470,360};auto before=e.toModel(anchor);e.zoomAt(anchor,1.8);auto after=e.toModel(anchor);
 check(near(before.x,after.x)&&near(before.y,after.y),"Zoom anchor");
 e.panX+=25;e.panY-=30;check(mv::hitTest(e,e.toScreen({-30,180}))==0,"Transformed pick");
 e.tick(1);check(e.phase==0,"Paused phase");e.playing=true;e.tick(1.25);check(near(e.phase,3.141592653589793/2),"Time based cycle");
 double phase=e.phase;e.playing=false;e.tick(5);check(e.phase==phase,"Pause holds pose");
 e.resetBreathing();check(!e.playing&&e.phase==0,"Reset animation");
 e.zoomAt(anchor,100);check(e.zoom==4,"Max zoom");e.zoomAt(anchor,0.001);check(e.zoom==0.6,"Min zoom");e.resetView();
 check(e.zoom==1&&e.panX==0&&e.panY==0,"Reset view");
 for(int variant=0;variant<2;++variant){e.female=variant!=0;for(int sys=0;sys<8;++sys){e.selectSystem(sys);const auto& model=e.model();
 check(!model.structures.empty(),"Model empty");check(e.selected==-1&&!e.playing&&e.zoom==1,"Switch reset");
 for(std::size_t i=0;i<model.structures.size();++i){const auto& organ=model.structures[i];check(!organ.description.empty()&&!organ.location.empty(),"Organ info");bool hit=false;
 for(const auto& part:organ.parts){check(part.size()>=3,"Polygon size");for(auto p:part)check(std::isfinite(p.x)&&std::isfinite(p.y),"Finite geometry");}
 // Find a visible selectable point, independently of label placement.
 for(int y=0;y<=560&&!hit;y+=2)for(int x=-130;x<=130&&!hit;x+=2)if(mv::hitTest(e,e.toScreen({double(x),double(y)}))==int(i))hit=true;
 check(hit,"Structure must have selectable visible area");
 check(mv::activate(e,{860.0,187.0+29*i})&&e.selected==int(i),"List selection");
 }
 e.selected=-1;e.cycleSelection(1);check(e.selected==0,"Keyboard first");e.cycleSelection(-1);check(e.selected==int(model.structures.size())-1,"Keyboard wrap");
 e.selected=0;mv::Canvas frame(1200,810,{9,23,39});mv::renderExplorer(frame,e);
 check(frame.get(210,400).r==9,"Viewport gutter");
 if(argc>1&&(variant==0||sys==7))frame.saveBMP(std::string(argv[1])+"/"+std::to_string(sys+1)+(variant?"_female":"")+".bmp");
 }}
 for(int sys=0;sys<8;++sys){e.selectSystem(sys);for(double zoom:{0.6,4.0})for(double pan:{-1000.0,0.0,1000.0}){e.zoom=zoom;e.panX=pan;e.panY=-pan;mv::Canvas frame(1200,810,{9,23,39});mv::renderExplorer(frame,e);check(frame.get(210,400).r==9,"Clipping under transforms");}}
 e.selectSystem(3);e.playing=true;e.tick(2.5);check(near(e.phase,3.141592653589793),"Full inhalation");
 check(mv::hitTest(e,e.toScreen({-30,180}))==0,"Animated selection");
 if(argc>1){e.zoom=2;e.panY=160;e.selected=0;mv::Canvas frame(1200,810,{9,23,39});mv::renderExplorer(frame,e);frame.saveBMP(std::string(argv[1])+"/breathing.bmp");}
 e.selectSystem(0);mv::activate(e,{260,750});check(!e.playing,"Non respiratory play disabled");
 e.selectSystem(7);bool f=e.female;mv::activate(e,{60,675});check(e.female!=f&&e.selected==-1,"Variant switch");
 check(mv::contains({{0,0},{10,0},{10,10},{0,10}},{10,5}),"Boundary pick");

 // Learning panel: every system and every structure has substantial, bounded pages.
 for(bool female:{false,true}){e.female=female;for(int sys=0;sys<8;++sys){e.selectSystem(sys);check(mv::systemInfo(sys,female).size()>300,"System overview depth");for(int i=-1;i<int(e.model().structures.size());++i){e.selectOrgan(i);auto lines=mv::informationLines(e);check(lines.size()>8,"Paginated information");for(const auto& line:lines)check(line.size()<=25,"Information line fits panel");check(mv::informationPages(e)*8>=int(lines.size()),"All text reachable");e.turnPage(1000);check(e.infoPage==mv::informationPages(e)-1,"Page upper bound");e.turnPage(-1000);check(e.infoPage==0,"Page lower bound");if(i>=0)check(mv::extendedInfo(e.model().structures[i].name).size()>200,"Per-organ expanded content");}}}
 // Swallow follows the oesophageal Bezier, holds on pause and completes once.
 e.selectSystem(2);e.toggleAnimation();e.tick(3.5);double swallowed=e.swallowTime;check(e.playing&&near(swallowed,3.5),"Swallow advances");e.toggleAnimation();e.tick(2);check(near(e.swallowTime,swallowed),"Swallow pause holds");e.toggleAnimation();e.tick(20);check(!e.playing&&e.swallowTime==8,"Swallow stops at stomach");e.toggleAnimation();check(e.playing&&e.swallowTime==0,"Replay starts at mouth");e.resetAnimation();check(!e.playing&&e.swallowTime==0,"Swallow reset");
 auto start=mv::swallowPosition(0),end=mv::swallowPosition(8);check(near(start.x,0)&&near(start.y,58),"Bolus starts in mouth");check(near(end.x,28)&&near(end.y,219),"Bolus reaches stomach");
 for(int i=0;i<=100;++i){double t=1.8+5.0*i/100;auto bolus=mv::swallowPosition(t);check(mv::contains(e.model().structures[1].parts[0],bolus),"Bolus stays in oesophagus");}
 check(mv::swallowStage(2)!=mv::swallowStage(7),"Stage captions change");
 // Hidden animation controls must neither draw nor respond on other systems.
 for(int sys:{0,1,4,5,6,7}){e.selectSystem(sys);check(!mv::activate(e,{250,750})&&!e.playing,"Hidden play is inactive");check(!mv::activate(e,{400,750}),"Hidden reset is inactive");mv::Canvas c(1200,810,{9,23,39});mv::renderExplorer(c,e);for(int x:{240,300,390,490}){auto color=c.get(x,750);check(color.r==9&&color.g==23&&color.b==39,"No hidden animation button pixels");}}
 e.selectSystem(3);check(mv::activate(e,{250,750})&&e.playing,"Respiratory button visible and active");e.selectSystem(2);check(!e.playing&&e.phase==0,"Switch clears previous animation");
 // All ordered system pairs are distinct and can be swapped without moving the camera.
 e.comparing=true;for(int sys=0;sys<8;++sys){e.selectSystem(sys);std::set<int> visited;for(int i=0;i<7;++i){e.nextSecondary();check(e.secondary!=e.system,"No self comparison");visited.insert(e.secondary);}check(visited.size()==7,"All comparison layers reachable");for(int second=0;second<8;++second)if(second!=sys){e.secondary=second;e.zoom=1.7;e.panX=13;double zoom=e.zoom,pan=e.panX;e.swapSystems();check(e.system==second&&e.secondary==sys,"Comparison swap");check(e.zoom==zoom&&e.panX==pan,"Swap preserves alignment");e.swapSystems();}}
 e.comparing=false;
 // Quiz rounds have distinct targets, cover every system and never award twice.
 for(unsigned seed=1;seed<=30;++seed){e.female=seed%2==0;e.startQuiz(seed);check(e.quiz&&!e.comparing&&!e.playing&&e.questions.size()==10,"Quiz start");std::set<int> systems;std::set<std::pair<int,int>> targets;
 for(const auto& q:e.questions){systems.insert(q.system);targets.insert({q.system,q.structure});std::set<std::string> choices(q.options.begin(),q.options.end());check(choices.size()==4,"Unique quiz choices");check(q.options[q.correct]==mv::models(e.female)[q.system].structures[q.structure].name,"Answer matches target");}
 check(systems.size()==8&&targets.size()==10,"Quiz coverage without repeated targets");int originalSystem=e.system,originalSelection=e.selected;e.selectSystem((e.system+1)%8);e.selectOrgan(-1);e.cycleSelection(1);e.toggleVariant();e.toggleAnimation();check(e.system==originalSystem&&e.selected==originalSelection&&!e.playing,"Quiz blocks answer leaks and animation");
 e.nextQuestion();check(e.question==0,"Cannot skip unanswered question");for(int i=0;i<10;++i){int correct=e.questions[i].correct;int choice=i%2?correct:(correct+1)%4;e.answerQuiz(choice);int score=e.score;e.answerQuiz(correct);check(e.score==score,"No second score");e.nextQuestion();}check(e.quizFinished()&&e.score==5,"Quiz result count");e.answerQuiz(0);e.nextQuestion();check(e.score==5&&e.question==10,"Completed quiz immutable");e.exitQuiz();check(!e.quiz&&e.selected==-1,"Quiz exit restores exploration");}
 if(argc>1){auto shot=[&](const std::string& name){mv::Canvas c(1200,810,{9,23,39});mv::renderExplorer(c,e);c.saveBMP(std::string(argv[1])+"/"+name+".bmp");};
 e.selectSystem(2);e.zoom=2;e.panY=200;e.labels=false;e.toggleAnimation();e.tick(4.3);e.selectOrgan(1);shot("swallow");e.resetAnimation();
 e.selectSystem(3);e.secondary=4;e.comparing=true;e.zoom=1.7;e.panY=130;shot("compare");e.comparing=false;
 e.startQuiz(23);shot("quiz_question");e.answerQuiz((e.questions[0].correct+1)%4);shot("quiz_feedback");while(!e.quizFinished()){if(e.answer<0)e.answerQuiz(e.questions[e.question].correct);e.nextQuestion();}shot("quiz_results");e.exitQuiz();
 e.selectSystem(6);e.selectOrgan(0);e.turnPage(2);shot("organ_notes");e.selectSystem(0);shot("system_notes");}
 std::cout<<"PASS: "<<checks<<" assertions across 8 systems, both reproductive variants, interaction, clipping, animation, comparison, quizzes and paginated learning.\n";return 0;
 }catch(const std::exception& ex){std::cerr<<"FAIL after "<<checks<<": "<<ex.what()<<'\n';return 1;}}
