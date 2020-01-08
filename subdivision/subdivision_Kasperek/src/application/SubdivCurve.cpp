#include "SubdivCurve.h"
#include <cmath>
#include <iostream>

#include "Vector3.h"
#include "Matrix4.h"

using namespace std;
using namespace p3d;

SubdivCurve::~SubdivCurve() {
}

SubdivCurve::SubdivCurve() {
  _nbIteration=1;
  _source.clear();
  _result.clear();

}


void SubdivCurve::addPoint(const p3d::Vector3 &p) {
  _source.push_back(p);
}

void SubdivCurve::point(int i,const p3d::Vector3 &p) {
  _source[i]=p;
}


void SubdivCurve::chaikinIter(const vector<Vector3> &p) {
  /* TODO : one iteration of Chaikin : input = p, output = you must set the vector _result (vector of Vector3)
   */
  _result.clear();

  Vector3 start = p[0];
  Vector3 pPlusUn;
  Vector3 q;

  for(int i = 1; i < p.size(); i++){
    pPlusUn = p[i];
    q = (start * 3/4) + (pPlusUn * 1/4);
    _result.push_back(q);
    q = (start * 1/4) + (pPlusUn * 3/4);
    _result.push_back(q);
    start = p[i];
  }
  if(isClosed()){
      pPlusUn = p[0];
      q = (start * 3/4) + (pPlusUn * 1/4);
      _result.push_back(q);
      q = (start * 1/4) + (pPlusUn * 3/4);
      _result.push_back(q);
  }

}

void SubdivCurve::dynLevinIter(const vector<Vector3> &p) {
  /* TODO : one iteration of DynLevin : input = p, output = you must set the vector _result (vector of Vector3)
   */
    _result.clear();
    //Vector3 pMoinsUn,pI,pPlusUn,pPlusDeux;

  int size = p.size();

   _result.resize(size*2);
  int i;
  for(i =0 ; i< size; i++){
      _result[2*i] = p[i];
      _result[2*i+1] = -(p[(i+2)%size]+p[(size+i-1)%size])/16.0 + (9.0/16.0)*(p[(i+1)%size]+p[i]);
  }
}


void SubdivCurve::chaikin() {
  if (_source.size()<2) return;
  vector<Vector3> current;
  _result=_source;
  for(int i=0;i<_nbIteration;++i) {
    current=_result;
    chaikinIter(current);
  }
}

void SubdivCurve::dynLevin() {
  if (_source.size()<2) return;
  if (!isClosed()) return;
  vector<Vector3> current;
  _result=_source;
  for(int i=0;i<_nbIteration;++i) {
    current=_result;
    dynLevinIter(current);
  }
}


