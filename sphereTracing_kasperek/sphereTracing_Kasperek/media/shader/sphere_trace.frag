#version 150

// @author F. Aubert (Univ. Lille)

const int iMax=150; // max loop
const int nbNodeMax=20; // max node

// uniforms
uniform int nbNode;
uniform int nbLeaf;
uniform int tree[nbNodeMax*2];
uniform mat4 leafEyeMatrix[nbNodeMax];
uniform vec4 colorLeaf[nbNodeMax];

// in out
out vec4 fragColor;
in vec2 fVertex;

// globals
float distanceNode[nbNodeMax];
bool isSet[nbNodeMax];
vec4 colorNode[nbNodeMax];

// sphere distance
float dSphere(vec3 p, vec3 centre, float rayon) {

    float dist =  length (centre - p);

    return dist - rayon;
}

float dSphereRepere(vec3 p, int i){
    vec3 centre = vec3(0, 0, 0);
    float rayon = 1;

    vec4 p_sphere = leafEyeMatrix[i] * vec4(p, 1.0);
    return dSphere(p_sphere.xyz, centre, rayon);
}


// distance from p to an union of primitives
float dList(vec3 p) {

/*
  // Le point est dans le repère de l'oeil
  // On veut le mettre dans le repère de la sphere
  vec4 p_sphere = leafEyeMatrix[0] * vec4(p, 1.0); //On multiplie le point eye par la matrice de transformation SPHERE -> EYE
  return  dSphere(p_sphere.xyz, vec3(0, 0, 0), 1);
*/


    // Une sphere centrée en (0,0,0) et de rayon 1
    vec3 centre1 = vec3(0, 0, 0);
    float rayon1 = 1;

    // Une sphere centrée en (2,0,0) et de rayon 0.5
    vec3 centre2 = vec3(1, 0, 0);
    float rayon2 = 0.5;

    vec4 p_sphere_1 = leafEyeMatrix[0] * vec4(p, 1.0); //On multiplie le point eye par la matrice de transformation SPHERE -> EYE
    vec4 p_sphere_2 = leafEyeMatrix[1] * vec4(p, 1.0); //On multiplie le point eye par la matrice de transformation SPHERE -> EYE

    float d1 =dSphere(p_sphere_1.xyz, centre1, rayon1);
    float d2 =dSphere(p_sphere_2.xyz, centre2, rayon2);

    return min(d1, d2);

}


// set distanceNode[indexNode] if indexNode is a primitive
void setDistanceLeaf(vec3 p,int indexNode,int primitive) {
    if(primitive == 0){
        distanceNode[indexNode] = dSphereRepere(p, indexNode);
    }

}


void updateParent(int indexNode) {
  int parent = tree[indexNode*2+1];
  float d_current = distanceNode[indexNode];

  if(isSet[parent]){
      int op_parent = tree[parent*2];
      float d_parent = distanceNode[parent];

      if (op_parent == 0){
          distanceNode[parent] = min(d_current, d_parent);
          if(distanceNode[parent] < d_current){
              colorNode[parent] = colorNode[indexNode];
          }

      }

      if(op_parent == 1){
          distanceNode[parent] = max(d_current, d_parent);
          if(distanceNode[parent] > d_current){
              colorNode[parent] = colorNode[indexNode];
          }
      }

      if(op_parent == 2){
        distanceNode[parent] = max(d_current, -d_parent);
      }

  }
  else{
      distanceNode[parent] = d_current;
      isSet[parent] = true;
      colorNode[parent] = colorNode[indexNode];
  }

}

float distanceTree(vec3 p) {
  bool done=false;

  int operation;
  int parent;

  int i;

  for(i=0;i<nbNode;++i) {
    isSet[i]=false; // a child already updated the node i ?
  }

  for(i=0;i<nbNode-1;++i) {
    operation=tree[i*2];
    parent=tree[i*2+1];
    if (operation>=4) {
      setDistanceLeaf(p,i,operation-4);
    }
    updateParent(i);
  }

  return distanceNode[nbNode-1];
}

void main() {
  // Le point de vue se trouve en z = 2
  vec3 p=vec3(0,0,2); // ray origin (eye)
  // Direction du rayon allant de la camera jusqu'au point de l'écran
  vec3 u=vec3(fVertex.x, fVertex.y, 0) - p; // ray direction
  u = normalize(u);

  fragColor = vec4(0.2,0.2,0.2,1); // default color of the pixel

  // init color of the nodes for leaves
  for(int i=0;i<nbLeaf;++i) {
    colorNode[i]=colorLeaf[i];
  }

  // init of the main loop for sphere tracing
  bool done=false;
  bool far=false;
  float d;
  int iter=iMax;


  // main loop of the sphere tracing
  while(!done) {
    // Distance à une sphère de rayon 1
    //d=dSphere(p, vec3(0, 0, 0), 1);

    //Changement de repère
//    /d = dList(p);

    d=distanceTree(p);

    //On avance dans la direction du rayon u de la distance d
    p = p+u*d;

    --iter;
    if (d<0.00001) done=true;
    else if (d>10.0) {done=true;far=true;}
    else if (iter==0) {far=true;done=true;}
  }

  // set the fragment color
  if (!far) {
     float e = 0.001;

     float dx = (distanceTree(vec3(p.x + e , p.y , p.z)) - distanceTree(vec3(p.x - e , p.y , p.z)))/ (e * 2);
     float dy = (distanceTree(vec3(p.x , p.y + e, p.z)) - distanceTree(vec3(p.x , p.y - e, p.z)))/ (e * 2);
     float dz = (distanceTree(vec3(p.x , p.y , p.z + e)) - distanceTree(vec3(p.x , p.y , p.z - e)))/ (e * 2);

     vec3 N = vec3 (dx, dy, dz);

     //Vecteur d'éclairement
     vec3 L = vec3(0,0,1);


     //Intensité de réflexion (N · L) voir cours M3DS Eclairement Phong
     float a = dot(L, N);

     if( a < 0 ){
         //la zone ne doit pas être éclairée
         a = 0;
     }
    //Couleur diffuse = Kd ( N · L )
    //Kd = (rouge,vert,bleu) couleur souhaitée pour le matériel.

    fragColor = colorNode[nbNode - 1] * a;

    //fragColor=vec4(1.0,0,0,1.0);
    //fragColor=vec4(1,0,0,1);
  }
}
