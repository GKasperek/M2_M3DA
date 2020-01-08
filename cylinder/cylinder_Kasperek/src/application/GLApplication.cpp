#include "GLApplication.h"
#include "GLTool.h"

#include "Vector3.h"
#include "Vector2.h"


#include <iostream>

/*!
*
* @file
*
* @brief
* @author F. Aubert
*
*/


using namespace std;
using namespace p3d;

GLApplication::~GLApplication() {
}

enum EMenu {M_Draw_Square, M_Set_Section_Square, M_Set_Section_Circle,M_Draw_Section,M_Draw_Path,M_Spline_Line,M_Build_Extrusion,M_Build_Revolution};
enum EDraw {D_Square,D_Path,D_Section,D_Extrusion};
enum EPath {Path_Line,Path_Spline};
EMenu _activeMenu;
EDraw _activeDraw;
EPath _activePath;

GLApplication::GLApplication() {
  //
  _leftPanelMenu << "Draw square (drawing example)" << "Input section (= square)" << "Input section (= circle)" << "Input section" << "Input Path" << "Switch Linear/Spline Path";
  _leftPanelMenu  << "Draw extrusion" << "Draw revolution (Path = circle)";
  _activeMenu=M_Draw_Square;
  _activeDraw=D_Square;
  _activePath=Path_Line;

  pathDefault();
  sectionCircle();

  _cameraSection.ortho(-2.5,2.5,-2.5,2.5,0,2);
  _cameraPath.ortho(-2.5,2.5,-2.5,2.5,0,2);
  _cameraPath.position(0,1,0);
  _cameraPath.lookAt(Vector3(0,0,0));
  _cameraExtrusion.frustum(-_frustum,_frustum,-_frustum,_frustum,0.03,1000);
  _cameraExtrusion.position(0,0,10);
  _cameraExtrusion.lookAt(Vector3(0,0,0));

}


/** ********************************************************************** **/
void GLApplication::initialize() {
  // appelée 1 seule fois à l'initialisation du contexte
  // => initialisations OpenGL
  glClearColor(1,1,1,1);

  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glClearDepth(1);

  p3d::initGLTool();

  // ...
}

void GLApplication::resize(int width,int height) {
  // appelée à chaque dimensionnement du widget OpenGL
  // (inclus l'ouverture de la fenêtre)
  // => réglages liés à la taille de la fenêtre
  _cameraPath.viewport(0,0,width,height);
  _cameraSection.viewport(0,0,width,height);
  _cameraExtrusion.viewport(0,0,width,height);
  // ...
}

void GLApplication::update() {
  // appelée toutes les 20ms (60Hz)
  // => mettre à jour les données de l'application
  // avant l'affichage de la prochaine image (animation)
  // ...

  if (_activeDraw==D_Path) {
    if (mouseLeftPressed()) {
      _path.push_back(_cameraPath.windowToWorld(mouseX(),mouseY()));
    }
    if (keyPressed(Qt::Key_X)) _path.clear();
  }
  if (_activeDraw==D_Section) {
    if (mouseLeft()) {
      _section.push_back(_cameraSection.windowToWorld(mouseX(),mouseY()).xy());
    }
    if (keyPressed(Qt::Key_X)) _section.clear();
  }
  if (_activeDraw==D_Extrusion) {
    updateCameraExtrusion();
  }
}

/* ************************************************************ */

void GLApplication::updateCameraExtrusion() {
  if (mouseLeft()) {
    Vector3 center=_cameraExtrusion.pointTo(Coordinate_Local,Vector3(0,0,0));
    Vector3 vertical=Vector3(0,1,0);
    _cameraExtrusion.translate(center,Coordinate_Local);
    _cameraExtrusion.rotate(-deltaMouseX()/2.0,vertical,Coordinate_Local);
    _cameraExtrusion.rotate(deltaMouseY()/2.0,Vector3(1,0,0),Coordinate_Local);
    _cameraExtrusion.translate(-center,Coordinate_Local);
  }
  if (left()) _cameraExtrusion.left(0.3);
  if (right()) _cameraExtrusion.right(0.3);
  if (forward()) _cameraExtrusion.forward(0.3);
  if (backward()) _cameraExtrusion.backward(0.3);
  if (accelerateWheel()) {
    _frustum*=1.05;
    _cameraExtrusion.frustum(-_frustum,_frustum,-_frustum,_frustum,0.03,1000);
  }
  if (decelerateWheel()) {
    _frustum/=1.05;
    _cameraExtrusion.frustum(-_frustum,_frustum,-_frustum,_frustum,0.03,1000);
  }
}


void GLApplication::draw() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  switch(_activeDraw) {
  case D_Square: p3d::apply(_cameraSection);drawSquare();break;
  case D_Section: p3d::apply(_cameraSection);drawSection();break;
  case D_Extrusion: p3d::apply(_cameraExtrusion);drawExtrusion();break;
  case D_Path:
    apply(_cameraPath);    if (_activePath==Path_Line) {
      drawPathLine();
    }
    else if (_activePath==Path_Spline) {
      drawPathSpline();
    }
  default:break;
  }
}


/** ************************************************************************ **/

/**
 * @brief GLApplication::drawSquare
 * Example of drawing primitives : p3d::drawPoints and p3d::drawLineStrip and p3d:draw for text. Notice shader activation before the p3d::draw... calls
 */
void GLApplication::drawSquare() {



  // compute points of the square in array of Vector2
  vector<Vector2> pts;
  pts.resize(5);
  pts[0]=Vector2(-0.5,-0.5);
  pts[1]=Vector2(0.5,-0.5);
  pts[2]=Vector2(0.5,0.5);
  pts[3]=Vector2(-0.5,0.5);
  pts[4]=pts[0];

  // draw square (line strip)
  p3d::ambientColor=Vector4(1,0,0,1); // the drawing color for the shader : must be set before the call p3d::shaderVertexAmbient
  p3d::shaderVertexAmbient(); // enable a simple shader.
  p3d::drawLineStrip(pts,5); // draw the array of Vector2D with the actual shader (you must set the number of points to draw : 5 here for the square).

  // draw vertices (points)
  glPointSize(10);
  p3d::ambientColor=Vector4(0,0,1,1);
  p3d::shaderVertexAmbient();
  p3d::drawPoints(pts);

  // draw text
  p3d::ambientColor=Vector4(1,0,1,1);
  p3d::draw("V0",Vector3(pts[0],0));


}


/** ************************************************************************ **/

/**
 * @brief GLApplication::sectionSquare
 * Example of cross section setup (here a square : notice the repeat of the first vertex to close the cross section).
 */

void GLApplication::sectionSquare() {
  _section.clear();
  _section.push_back(Vector2(-1,-1));
  _section.push_back(Vector2(1,-1));
  _section.push_back(Vector2(1,1));
  _section.push_back(Vector2(-1,1));
  _section.push_back(Vector2(-1,-1));
}


/**
 * @brief GLApplication::crossSectionCircle
 * set the cross section (i.e. set _inputCrossSection) as a circle
 */
void GLApplication::sectionCircle() {
  _section.clear();
  double r = 1.5;
  double nb = 20;
  double pas = M_PI/nb;
  double teta = 0;
  for(int i = 0; i < nb; i++){
      _section.push_back(Vector2(r*cos(teta),r*sin(teta)));
      teta += pas;
  }
  for(int i = 0; i < nb; i++){
      _section.push_back(Vector2(r*cos(-teta),r*sin(-teta)));
      teta -= pas;
  }
  _section.push_back(Vector2(r*cos(0),r*sin(0)));
}


/** ************************************************************************ **/

void GLApplication::pathDefault() {

  /*_path.clear();
  _path.push_back(Vector3(0,0,-2));
  _path.push_back(Vector3(0,0,2));*/
   _path.clear();
   _path.push_back(Vector3(-2,0,-2));
   _path.push_back(Vector3(2,0,2));

   _path.clear();
     _path.push_back(Vector3(-2,0,-2));
     _path.push_back(Vector3(0,0,2));
     _path.push_back(Vector3(2,0,-1));

}

void GLApplication::pathCircle() {
    /*
    _section.clear();
    double r = 1.5;
    double nb = 20;
    double pas = M_PI/nb;
    double teta = 0;
    for(int i = 0; i < nb; i++){
        _section.push_back(Vector2(r*cos(teta),r*sin(teta)));
        teta += pas;
    }
    for(int i = 0; i < nb; i++){
        _section.push_back(Vector2(r*cos(-teta),r*sin(-teta)));
        teta -= pas;
    }
    _section.push_back(Vector2(r*cos(0),r*sin(0)));*/
    _path.clear();
    float nbPoints = 20;

    float coordX, coordZ;

    for(float t = -1; t <= 2*M_PI; t+=(2*M_PI)/nbPoints){
        coordX = cos(t);
        coordZ = sin(t);
        _path.push_back(Vector3(coordX, 0, coordZ));
    }
}

/** ************************************************************************ **/

void GLApplication::drawSection() {
  p3d::ambientColor=Vector4(1,0,0,1);
  p3d::shaderVertexAmbient();
  p3d::drawPoints(_section);
  p3d::drawLineStrip(_section);
}

void GLApplication::drawPathLine() {
  if (_path.size()<1) return;

  p3d::ambientColor=Vector4(0,0,1,1);
  p3d::shaderVertexAmbient();
  p3d::drawPoints(_path);
  p3d::drawLineStrip(_path);
}


void GLApplication::drawExtrusion() {
  if (_extrusion.size()<4) return;
  int nbSlice=_section.size();


  p3d::ambientColor=Vector4(1,0,0,1);
  p3d::shaderVertexAmbient();


  drawGrid(_extrusion,nbSlice); // comment this once last question done


  //  uncomment once normals computed (last question)
  /*p3d::lightPosition[0]=Vector4(0,0,10,1);
  p3d::lightIntensity[0]=1.0;
  p3d::material(Vector4(0,0,0.3,1),Vector3(0,0.2,0.8),Vector3(0,0.8,0.3),100);
  p3d::diffuseBackColor=Vector3(0.8,0,0);
  p3d::shaderLightPhong();
  fillGrid(_extrusion,_normalExtrusion,nbSlice);*/




  drawPath();

}



void GLApplication::drawPathSpline() {
  if (_path.size()>=2) {
    vector<Vector3> toDraw;
    toDraw.clear();
    int nbPts=100;
    double step=1.0/(nbPts-1);
    double t=0;
    for(int i=0;i<nbPts;++i) {
      toDraw.push_back(pointSpline(t));
      t+=step;
    }

    p3d::ambientColor=Vector4(0,0,1,1);
    p3d::shaderVertexAmbient();
    drawLineStrip(toDraw,toDraw.size());
  }
  if (_path.size()>0) {
    p3d::ambientColor=Vector4(0,0,1,1);
    p3d::shaderVertexAmbient();
    p3d::drawPoints(_path);
  }
}

/** ************************************************************************* **/

/**
 * @brief transform the point p in the plane (x,y,0) to the point in the plane with the normal n (i.e. rotation of the plane (x,y,0))
 * @param p : the point expressed in the plane (x,y,0)
 * @param n : the normal of the plane
 * @return the transformation of the point p
 */

Vector3 GLApplication::rotatePlane(const Vector3 &p,const Vector3 &n) {
  Vector3 result;
  //double teta = acos((p.dot(n))/(p.length()*n.length()));

  Matrix4 rotation;
  rotation.setRotation(Vector3(0,0,1),n);

  result = rotation.transformPoint(p);
  return result;

}



Vector3 GLApplication::pointSpline(double tNormalized) {
  Vector3 result;

  // Calcul de l'intervalle
  unsigned int i = tNormalized*(_path.size() - 1);
  double t = tNormalized * (_path.size() - 1) - i;

  Vector3 p0 = _path[i];
  Vector3 p1 = _path[i + 1];

  Vector3 t0 = tangentPathLine(i);
  Vector3 t1 = tangentPathLine(i+1);

  // Calcul de la dérivé
  result = pow(t,3) * (2*p0 - 2*p1 + t0 + t1) + pow(t,2) * (-3*p0 + 3*p1 - 2*t0 - t1) + (t * t0) + p0;

  return result;
}


Vector3 GLApplication::tangentPathSpline(double tNormalized) {
    Vector3 result;

    // Calcul de l'intervalle
    unsigned int i = tNormalized*(_path.size() - 1);
    double t = tNormalized * (_path.size() - 1) - i;

    Vector3 p0 = _path[i];
    Vector3 p1 = _path[i + 1];

    Vector3 t0 = tangentPathLine(i);
    Vector3 t1 = tangentPathLine(i+1);

    // Calcul de la dérivé
    result = 3* (2*p0 - 2*p1 + t0 + t1) * pow(t,2) + 2*t* (-3*p0 + 3*p1 - 2*t0 - t1) + t0+ p0;

    return result;
}



Vector3 GLApplication::tangentPathLine(unsigned int i) {
  Vector3 result;

  if(i == 0){
      result = _path[1] - _path[0];
  }
  else if(i == _path.size() - 1){
    result = _path[_path.size() - 1] - _path[_path.size() - 2];
  }else{
      result = _path[i+1] - _path[i-1];
  }

  return result;
}

/** ************************************************************************* **/

void GLApplication::normalSection() {
  _normalSection.clear();
  std::vector<Vector2> norm;
  for(unsigned int i = 0; i < _section.size() -1; i++){
      Vector2 s = Vector2(_section[i], _section[i+1]);

      Vector2 n = Vector2(-s.y(), s.x());
      norm.push_back(n);
  }

  Vector2 moy = Vector2(norm[0]);
  moy.normalize();
  _normalSection.push_back(norm[0]);

  for(unsigned int i = 0; i < _section.size()-1; i++){
    moy = (norm[i] + norm[i+1])/2;
    moy.normalize();
    _normalSection.push_back(moy);
  }
}


void GLApplication::extrudeLine() {
  if (_path.size()<1 || _section.size()<1) return;

  _extrusion.clear();
  _normalExtrusion.clear(); // for lighting (last question)

  float x;
  float y;
  float z;

  for(int i=0 ; i<_path.size() ; i++){
      for(int j=0 ; j <_section.size(); j++){
          Vector3 a = Vector3(_section[j].x(),_section[j].y(),0);

          Vector3 r = rotatePlane(a,tangentPathLine(i));
          _extrusion.push_back(r+ _path[i]);
      }
  }


}

void GLApplication::extrudeSpline() {
  if (_path.size()<1 || _section.size()<1) return;

  _extrusion.clear();

  _normalExtrusion.clear(); // for lighting (last question)

  normalSection();
  unsigned int n = 100;
  double pas = 1.0/n-1;
  double t = 0;
  Vector3 normale;

  for(int i = 0;i<n;i++){
      /*for(int j = 0; j < _section.size(); j++){
        Vector3 a = Vector3(_section[j].x(),_section[j].y(),0);

        Vector3 r = pointSpline(i * pas) + rotatePlane(a, tangentPathSpline(i *pas));
        _extrusion.push_back(r);*/

      Vector3 spline = pointSpline(t);
      Vector3 tangentSpline = tangentPathSpline(t);

      int j = 0;

      for(Vector2 pointSection : _section){
        float coordX = pointSection.x();
        float coordY = pointSection.y();

        Vector3 point = Vector3(coordX,coordY,0);
        Vector3 extrusion = rotatePlane(point, tangentSpline);
        _extrusion.push_back(spline + extrusion);

        normale = Vector3(_section[j],0);
        _normalExtrusion.push_back(normale);
        j++;
      }
      t = t + pas;
  }

}


/** ************************************************************************* **/


double GLApplication::scale(double tNormalized) {
  return 1.0;

}

void GLApplication::buildExtrusion() {
  if (_activePath==Path_Line)
    extrudeLine();
  else if (_activePath==Path_Spline) {
    extrudeSpline();
  }
}

void GLApplication::drawPath() {
  if (_activePath==Path_Line) {
    drawPathLine();
  }
  else if (_activePath==Path_Spline) {
    drawPathSpline();
  }
}


/** ********************************************************************** **/
/** i = button number, s = button text
 */
void GLApplication::leftPanel(int i,const std::string &) {
  _activeMenu=static_cast<EMenu>(i);

  switch (_activeMenu) {
  case M_Draw_Square:
    _activeDraw=D_Square;
    break;
  case M_Set_Section_Square:
    sectionSquare();
    _activeDraw=D_Section;
    break;
  case M_Set_Section_Circle:
    sectionCircle();
    _activeDraw=D_Section;
    break;
  case M_Build_Extrusion:
    buildExtrusion();
    _activeDraw=D_Extrusion;
    break;
  case M_Build_Revolution:
    pathCircle();
    buildExtrusion();
    _activeDraw=D_Extrusion;
    break;
  case M_Draw_Path:
    _activeDraw=D_Path;
    break;
  case M_Draw_Section:
    _activeDraw=D_Section;
    break;
  case M_Spline_Line:
    if (_activePath==Path_Line) _activePath=Path_Spline;
    else _activePath=Path_Line;
    if (_activeDraw==D_Extrusion) {
      buildExtrusion();
    }
    break;
  default:break;
  }

  /*
  switch (i) {
    case 0:...;break;
    case 1:...;break;
    ...
  }
  */
}



