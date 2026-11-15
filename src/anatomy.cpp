#include "explorer.h"
#include <cmath>
namespace mv {
namespace {
const Color bone={225,219,193}, muscle={211,111,123}, digest={232,170,102}, lungColor={224,133,157}, artery={231,99,112}, nerve={238,202,101}, urinary={160,155,223}, repro={219,143,195};
// Corner-cutting subdivision preserves the outline while removing angular joins.
Polygon rounded(Polygon p,int passes=2){for(int n=0;n<passes;++n){Polygon q;for(std::size_t i=0;i<p.size();++i){Point a=p[i],b=p[(i+1)%p.size()];q.push_back({.75*a.x+.25*b.x,.75*a.y+.25*b.y});q.push_back({.25*a.x+.75*b.x,.25*a.y+.75*b.y});}p=q;}return p;}
Polygon oval(double x,double y,double rx,double ry){Polygon p;for(int i=0;i<64;++i){double a=i*6.283185307179586/64;p.push_back({x+rx*std::cos(a),y+ry*std::sin(a)});}return p;}
Polygon ribbon(const Polygon& path,double radius){
 Polygon a,b;
 for(std::size_t i=0;i<path.size();++i){Point prev=path[i?i-1:0],next=path[i+1<path.size()?i+1:i];double dx=next.x-prev.x,dy=next.y-prev.y,len=std::hypot(dx,dy);if(len<1e-8)len=1;
 a.push_back({path[i].x-dy/len*radius,path[i].y+dx/len*radius});b.push_back({path[i].x+dy/len*radius,path[i].y-dx/len*radius});}
 for(auto it=b.rbegin();it!=b.rend();++it)a.push_back(*it);
 return a;
}
Polygon tube(Point a,Point b,double r){return ribbon({a,b},r);}
Polygon curve(Point a,Point b,Point c,Point d,double r){return ribbon(bezier(a,b,c,d,28),r);}
Polygon mirror(Polygon p){for(auto& v:p)v.x=-v.x;return p;}
Structure item(const char* name,const char* location,const char* info,Color c,std::vector<Polygon> p,Point label,int motion=0){for(auto& shape:p)if(shape.size()<20)shape=rounded(shape);return {name,location,info,c,p,label,motion};}
Polygon kidney(double x,double y,bool flip){
 Polygon p=bezier({-4,-18},{-26,-25},{-30,22},{-5,20},24);
 auto q=bezier({-5,20},{10,16},{-9,4},{-3,-1},18);p.insert(p.end(),q.begin(),q.end());q=bezier({-3,-1},{8,-8},{9,-16},{-4,-18},18);p.insert(p.end(),q.begin(),q.end());
 if(flip)p=mirror(p);
 return transform(p,{0,0},1,1,{x,y});
}
std::vector<SystemModel> build(bool female){
 std::vector<SystemModel> m;
 SystemModel s={"SKELETAL","SUPPORT, PROTECTION AND MOVEMENT",bone,{}};
 s.structures.push_back(item("SKULL","HEAD","PROTECTS THE BRAIN AND SUPPORTS THE STRUCTURES OF THE FACE.",bone,{oval(0,40,27,32)}, {0,40}));
 std::vector<Polygon> spine;for(int y=81;y<320;y+=12)spine.push_back(oval(0,y,6,5));
 s.structures.push_back(item("VERTEBRAL COLUMN","MIDLINE OF THE BACK","SUPPORTS THE TRUNK AND PROTECTS THE SPINAL CORD. THE SEGMENTS HERE ARE SIMPLIFIED.",bone,spine,{0,220}));
 std::vector<Polygon> ribs;for(int i=0;i<7;++i){double y=121+i*12,wide=48-std::abs(i-3)*3;ribs.push_back(curve({-3,y},{-wide,y-12},{-wide-9,y+22},{-4,y+14},2));ribs.push_back(mirror(ribs.back()));}
 ribs.push_back(tube({0,118},{0,197},4));s.structures.push_back(item("RIB CAGE","CHEST","RIBS AND STERNUM HELP PROTECT THE HEART AND LUNGS. ONLY SELECTED RIBS ARE SHOWN.",bone,ribs,{35,166}));
 s.structures.push_back(item("PELVIS","LOWER TRUNK","TRANSFERS WEIGHT TO THE LOWER LIMBS AND PROTECTS PELVIC ORGANS.",bone,{{{-4,303},{-40,293},{-48,319},{-33,345},{-8,332},{0,320}},{{4,303},{40,293},{48,319},{33,345},{8,332},{0,320}}},{25,318}));
 std::vector<Polygon> arms;for(int side:{-1,1}){arms.push_back(tube({side*59.0,115},{side*84.0,206},5));arms.push_back(tube({side*84.0,214},{side*108.0,296},3));arms.push_back(tube({side*92.0,213},{side*115.0,294},3));arms.push_back(oval(side*114,310,8,14));}
 s.structures.push_back(item("ARM BONES","UPPER LIMBS","THE HUMERUS, RADIUS AND ULNA SUPPORT THE ARMS AND PROVIDE LEVERS FOR MOVEMENT.",bone,arms,{-85,206}));
 std::vector<Polygon> legs;for(int side:{-1,1}){legs.push_back(tube({side*26.0,339},{side*32.0,424},6));legs.push_back(oval(side*32,432,7,7));legs.push_back(tube({side*32.0,441},{side*32.0,525},5));legs.push_back(tube({side*42.0,441},{side*40.0,525},2));legs.push_back(oval(side*39,538,16,5));}
 s.structures.push_back(item("LEG BONES","LOWER LIMBS","THE FEMUR, PATELLA, TIBIA AND FIBULA SUPPORT WEIGHT AND HELP THE BODY MOVE.",bone,legs,{33,462}));m.push_back(s);
 s={"MUSCULAR","MOVEMENT, POSTURE AND HEAT",muscle,{}};
 s.structures.push_back(item("DELTOIDS","SHOULDERS","HELP LIFT AND ROTATE THE UPPER ARMS.",muscle,{oval(-59,116,17,23),oval(59,116,17,23)},{-59,116}));
 s.structures.push_back(item("PECTORALS","FRONT OF THE CHEST","HELP MOVE THE UPPER ARMS ACROSS THE BODY.",muscle,{{{-3,116},{-40,110},{-53,130},{-41,154},{-4,155}},{{3,116},{40,110},{53,130},{41,154},{4,155}}},{27,133}));
 s.structures.push_back(item("BICEPS","FRONT OF UPPER ARMS","HELP BEND THE ELBOW AND TURN THE FOREARM.",muscle,{ribbon({{-68,145},{-77,172},{-82,197}},9),ribbon({{68,145},{77,172},{82,197}},9)},{-76,170}));
 std::vector<Polygon> abs;for(int y=179;y<272;y+=28){abs.push_back(oval(-13,y,10,12));abs.push_back(oval(13,y,10,12));}
 s.structures.push_back(item("ABDOMINAL MUSCLES","FRONT OF THE ABDOMEN","SUPPORT THE TRUNK AND HELP FLEX THE SPINE AND COMPRESS THE ABDOMEN.",muscle,abs,{13,220}));
 s.structures.push_back(item("QUADRICEPS","FRONT OF THE THIGHS","EXTEND THE KNEE. THIS GROUP CONTAINS FOUR MUSCLES IN EACH THIGH.",muscle,{oval(-29,377,17,43),oval(29,377,17,43)},{29,380}));
 s.structures.push_back(item("TIBIALIS ANTERIOR","FRONT OF LOWER LEGS","LIFTS THE FOOT TOWARD THE SHIN AND HELPS CONTROL THE FOOT WHILE WALKING.",muscle,{oval(-29,478,9,36),oval(29,478,9,36)},{-29,478}));m.push_back(s);
 s={"DIGESTIVE","BREAKDOWN AND ABSORPTION",digest,{}};
 s.structures.push_back(item("MOUTH","LOWER FACE","TAKES IN FOOD. CHEWING AND SALIVA BEGIN ITS BREAKDOWN.",digest,{oval(0,58,10,4)},{0,58}));
 s.structures.push_back(item("OESOPHAGUS","NECK AND CHEST","MOVES SWALLOWED FOOD TO THE STOMACH USING MUSCULAR CONTRACTIONS.",digest,{curve({0,64},{-4,110},{2,160},{13,196},4)},{0,140}));
 s.structures.push_back(item("LIVER","UPPER RIGHT ABDOMEN","PRODUCES BILE AND PROCESSES NUTRIENTS. ALSO HAS MANY METABOLIC FUNCTIONS.",{182,108,105},{{{-49,202},{-30,184},{9,189},{23,203},{-7,219},{-46,221}}},{-28,202}));
 Polygon stomach=bezier({15,191},{45,180},{48,225},{29,237},28);auto q=bezier({29,237},{0,245},{-6,219},{15,212},20);stomach.insert(stomach.end(),q.begin(),q.end());
 s.structures.push_back(item("STOMACH","UPPER LEFT ABDOMEN","STORES AND MIXES FOOD WITH ACID AND ENZYMES BEFORE PASSING IT TO THE SMALL INTESTINE.",digest,{stomach},{29,214}));
 s.structures.push_back(item("PANCREAS","BEHIND THE STOMACH","RELEASES DIGESTIVE ENZYMES INTO THE SMALL INTESTINE. ALSO MAKES HORMONES THAT REGULATE BLOOD GLUCOSE.",{238,203,123},{oval(6,239,28,6)},{6,239}));
 std::vector<Polygon> small;for(int i=0;i<4;++i)small.push_back(curve({-24.0,260.0+i*11},{30,246.0+i*11},{35,277.0+i*11},{-22,271.0+i*11},3));
 s.structures.push_back(item("SMALL INTESTINE","CENTRAL ABDOMEN","COMPLETES MUCH OF DIGESTION AND ABSORBS MOST NUTRIENTS.",{236,170,133},small,{0,283}));
 s.structures.push_back(item("LARGE INTESTINE","AROUND SMALL INTESTINE","ABSORBS WATER AND ELECTROLYTES AND FORMS FAECES. ENDS AT THE RECTUM AND ANUS.",{203,139,112},{ribbon({{-39,307},{-39,254},{-30,248},{32,248},{40,258},{40,303},{17,315},{0,317},{0,337}},5)},{39,282}));m.push_back(s);
 s={"RESPIRATORY","AIRFLOW AND GAS EXCHANGE",lungColor,{}};
 Polygon right=bezier({-10,117},{-26,105},{-47,127},{-52,163},36);
 auto qlung=bezier({-52,163},{-57,184},{-54,200},{-50,207},30);right.insert(right.end(),qlung.begin(),qlung.end());
 qlung=bezier({-50,207},{-35,202},{-16,211},{-9,198},30);right.insert(right.end(),qlung.begin(),qlung.end());
 qlung=bezier({-9,198},{-6,177},{-11,144},{-10,117},30);right.insert(right.end(),qlung.begin(),qlung.end());
 Polygon left=bezier({10,117},{29,109},{49,134},{52,166},36);
 qlung=bezier({52,166},{57,191},{51,203},{45,207},30);left.insert(left.end(),qlung.begin(),qlung.end());
 qlung=bezier({45,207},{36,204},{27,207},{24,199},28);left.insert(left.end(),qlung.begin(),qlung.end());
 qlung=bezier({24,199},{24,186},{7,182},{12,166},28);left.insert(left.end(),qlung.begin(),qlung.end());
 qlung=bezier({12,166},{16,146},{9,132},{10,117},28);left.insert(left.end(),qlung.begin(),qlung.end());
 s.structures.push_back(item("RIGHT LUNG","RIGHT SIDE OF THE CHEST","HAS THREE LOBES. TINY AIR SACS EXCHANGE OXYGEN AND CARBON DIOXIDE WITH BLOOD.",lungColor,{right},{-31,160},1));
 s.structures.push_back(item("LEFT LUNG","LEFT SIDE OF THE CHEST","HAS TWO LOBES AND A CARDIAC NOTCH THAT ACCOMMODATES THE HEART. AIR SACS ENABLE GAS EXCHANGE.",lungColor,{left},{32,160},1));
 s.structures.push_back(item("TRACHEA","NECK AND UPPER CHEST","THE WINDPIPE CARRIES AIR TOWARD THE BRONCHI. CARTILAGE RINGS HELP KEEP IT OPEN.",{164,204,212},{tube({0,76},{0,145},5)},{0,100}));
 s.structures.push_back(item("BRONCHI","WITHIN THE CHEST","THE TWO MAIN BRONCHI CARRY AIR FROM THE TRACHEA INTO THE LUNGS AND DIVIDE INTO SMALLER AIRWAYS.",{209,214,191},{curve({0,143},{-7,150},{-15,151},{-25,165},3),curve({0,143},{7,150},{15,151},{25,165},3)},{-18,155}));
 Polygon diaphragm=bezier({-52,216},{-27,201},{27,201},{52,216},30);q=bezier({52,216},{24,215},{-24,215},{-52,227},30);diaphragm.insert(diaphragm.end(),q.begin(),q.end());
 s.structures.push_back(item("DIAPHRAGM","BELOW THE LUNGS","CONTRACTS AND DESCENDS DURING INHALATION. RELAXATION HELPS AIR LEAVE DURING QUIET EXHALATION.",{181,111,151},{diaphragm},{0,216},2));m.push_back(s);
 s={"CIRCULATORY","TRANSPORT THROUGH BLOOD",artery,{}};
 s.structures.push_back(item("MAJOR ARTERIES","TRUNK AND LIMBS","CARRY BLOOD AWAY FROM THE HEART. THESE ARE SELECTED SCHEMATIC ROUTES, NOT A COMPLETE VASCULAR MAP.",artery,{ribbon({{7,165},{-5,145},{-9,200},{-9,310},{-28,355},{-32,525}},3),ribbon({{-9,310},{28,355},{32,525}},3),ribbon({{-5,145},{-50,116},{-82,210},{-110,299}},2),ribbon({{-5,145},{50,116},{82,210},{110,299}},2),tube({-5,145},{-7,63},2)},{-9,271}));
 s.structures.push_back(item("MAJOR VEINS","TRUNK AND LIMBS","RETURN BLOOD TOWARD THE HEART. BLUE IS A DIAGRAM CONVENTION; BLOOD IS NOT BLUE.",{105,164,226},{ribbon({{17,173},{9,211},{9,308},{-20,356},{-24,525}},3),ribbon({{9,308},{36,356},{40,525}},3),ribbon({{10,145},{-44,124},{-75,212},{-103,300}},2),ribbon({{10,145},{44,124},{75,212},{103,300}},2),tube({9,145},{7,63},2)},{9,270}));
 // An asymmetric ventricular silhouette, rather than a decorative heart symbol.
 Polygon heart=bezier({-7,152},{-22,157},{-18,180},{4,194},32);
 q=bezier({4,194},{11,202},{26,185},{29,172},32);heart.insert(heart.end(),q.begin(),q.end());
 q=bezier({29,172},{30,152},{17,146},{7,151},32);heart.insert(heart.end(),q.begin(),q.end());
 q=bezier({7,151},{3,146},{-3,148},{-7,152},20);heart.insert(heart.end(),q.begin(),q.end());
 s.structures.push_back(item("HEART","CHEST, SLIGHTLY LEFT OF MIDLINE","PUMPS BLOOD THROUGH THE PULMONARY AND SYSTEMIC CIRCULATIONS. THE RIGHT SIDE SENDS BLOOD TO THE LUNGS; THE LEFT TO THE BODY.",artery,{heart},{10,171}));m.push_back(s);
 s={"NERVOUS","CONTROL AND COMMUNICATION",nerve,{}};
 std::vector<Polygon> nerves;
 for(int side:{-1,1}){nerves.push_back(ribbon({{0,104},{side*48.0,116},{side*82.0,210},{side*112.0,305}},2));nerves.push_back(ribbon({{0,285},{side*26.0,348},{side*33.0,525}},2));for(int y=134;y<275;y+=24)nerves.push_back(curve({0,double(y)},{side*20.0,double(y)},{side*35.0,y+9.0},{side*46.0,y+13.0},1.4));}
 s.structures.push_back(item("PERIPHERAL NERVES","TRUNK AND LIMBS","CONNECT THE CENTRAL NERVOUS SYSTEM WITH THE REST OF THE BODY, CARRYING SENSORY AND MOTOR SIGNALS.",nerve,nerves,{33,375}));
 s.structures.push_back(item("SPINAL CORD","WITHIN THE VERTEBRAL CANAL","RELAYS SIGNALS BETWEEN THE BRAIN AND BODY AND COORDINATES MANY REFLEXES. ITS END IS SIMPLIFIED HERE.",nerve,{tube({0,62},{0,272},4)},{0,211}));
 s.structures.push_back(item("BRAIN","INSIDE THE SKULL","INTEGRATES SENSORY INFORMATION AND COORDINATES MOVEMENT, THOUGHT, MEMORY AND MANY BODY FUNCTIONS.",{233,188,213},{oval(-10,34,14,20),oval(10,34,14,20),oval(0,55,12,8)},{0,35}));m.push_back(s);
 s={"URINARY","FILTRATION AND FLUID BALANCE",urinary,{}};
 s.structures.push_back(item("RIGHT KIDNEY","POSTERIOR RIGHT ABDOMEN","FILTERS BLOOD TO FORM URINE AND HELPS REGULATE FLUIDS, ELECTROLYTES AND ACID BASE BALANCE.",urinary,{kidney(-23,235,false)},{-35,235}));
 s.structures.push_back(item("LEFT KIDNEY","POSTERIOR LEFT ABDOMEN","FILTERS BLOOD TO FORM URINE. IT IS NORMALLY SLIGHTLY HIGHER THAN THE RIGHT KIDNEY.",urinary,{kidney(23,227,true)},{35,227}));
 s.structures.push_back(item("URETERS","BETWEEN KIDNEYS AND BLADDER","TWO MUSCULAR TUBES TRANSPORT URINE FROM THE KIDNEYS TO THE BLADDER.",urinary,{curve({-25,243},{-22,270},{-15,282},{-9,303},2),curve({25,235},{22,267},{15,282},{9,303},2)},{-20,273}));
 s.structures.push_back(item("BLADDER","PELVIS","STORES URINE UNTIL IT IS RELEASED DURING URINATION.",urinary,{oval(0,313,18,16)},{0,313}));
 s.structures.push_back(item("URETHRA","BELOW THE BLADDER","CARRIES URINE FROM THE BLADDER TO THE OUTSIDE OF THE BODY. LENGTH AND COURSE VARY WITH ANATOMY.",urinary,{tube({0,328},{0,350},2)},{0,341}));m.push_back(s);
 s={"REPRODUCTIVE",female?"FEMALE REPRODUCTIVE STRUCTURES":"MALE REPRODUCTIVE STRUCTURES",repro,{}};
 if(female){
 s.structures.push_back(item("OVARIES","EITHER SIDE OF THE UTERUS","PRODUCE OOCYTES AND HORMONES INCLUDING OESTROGEN AND PROGESTERONE.",repro,{oval(-36,295,9,6),oval(36,295,9,6)},{-36,295}));
 s.structures.push_back(item("UTERINE TUBES","UPPER PELVIS","TRANSPORT THE OOCYTE TOWARD THE UTERUS. FERTILISATION USUALLY OCCURS WITHIN A UTERINE TUBE.",repro,{curve({-12,295},{-19,280},{-33,278},{-36,290},2),curve({12,295},{19,280},{33,278},{36,290},2)},{26,285}));
 s.structures.push_back(item("UTERUS","CENTRAL PELVIS","A MUSCULAR ORGAN WHERE AN EMBRYO CAN IMPLANT AND A FETUS CAN DEVELOP.",repro,{{{-16,295},{-10,288},{10,288},{16,295},{10,312},{4,320},{-4,320},{-10,312}}},{0,304}));
 s.structures.push_back(item("CERVIX AND VAGINA","LOWER PELVIS","THE CERVIX IS THE LOWER PART OF THE UTERUS. THE VAGINA CONNECTS IT TO THE EXTERIOR.",repro,{ribbon({{0,319},{0,339},{3,350}},4)},{0,335}));
 }else{
 s.structures.push_back(item("TESTES","SCROTUM","PRODUCE SPERM AND TESTOSTERONE.",repro,{oval(-12,357,7,11),oval(12,357,7,11)},{-12,357}));
 s.structures.push_back(item("DUCTUS DEFERENS","FROM SCROTUM INTO PELVIS","TRANSPORT SPERM FROM THE EPIDIDYMIDES TOWARD THE EJACULATORY DUCTS.",repro,{curve({-15,352},{-40,288},{-15,278},{-5,316},2),curve({15,352},{40,288},{15,278},{5,316},2)},{-26,314}));
 s.structures.push_back(item("SEMINAL VESICLES","BEHIND THE BLADDER","PRODUCE FLUID THAT CONTRIBUTES TO SEMEN AND SUPPORTS SPERM.",repro,{oval(-12,302,6,9),oval(12,302,6,9)},{12,302}));
 s.structures.push_back(item("PROSTATE","BELOW THE BLADDER","ADDS FLUID TO SEMEN AND SURROUNDS THE UPPER PART OF THE URETHRA.",repro,{oval(0,321,11,7)},{0,321}));
 s.structures.push_back(item("PENIS","EXTERNAL PELVIC REGION","CONTAINS THE URETHRA, THROUGH WHICH URINE OR SEMEN CAN LEAVE THE BODY.",repro,{ribbon({{0,330},{0,356}},5),oval(0,358,5,4)},{0,342}));
 }m.push_back(s);return m;
}
}
const std::vector<SystemModel>& models(bool female){static const auto male=build(false);static const auto fem=build(true);return female?fem:male;}
Polygon bodyOutline(){
 return rounded({{-12,70},{-14,92},{-52,103},{-66,109},{-78,135},{-93,199},{-101,229},{-118,292},{-124,313},{-117,329},{-108,324},{-103,302},{-83,251},{-69,215},{-55,165},{-46,216},{-44,270},{-53,312},{-49,353},{-48,413},{-44,445},{-45,503},{-48,531},{-58,543},{-55,550},{-24,550},{-20,538},{-20,488},{-18,442},{-13,397},{-7,352},{0,340},{7,352},{13,397},{18,442},{20,488},{20,538},{24,550},{55,550},{58,543},{48,531},{45,503},{44,445},{48,413},{49,353},{53,312},{44,270},{46,216},{55,165},{69,215},{83,251},{103,302},{108,324},{117,329},{124,313},{118,292},{101,229},{93,199},{78,135},{66,109},{52,103},{14,92},{12,70}},2);
}
}