#include <iostream>
#include <vector>
#include <string>
#include "model.h"


struct Satellite{
  std::string name;
  float distFromPlanet;
  float orbitSpeed;
  float diameter;
  Model moon;

};

struct Planet{
    std::string name;
    float orbitSpeed;
    float orbitAngle;
    float orbitRadius;
    float diameter;
    float mass;
    float rotSpeed;
    float rotAngle;
    Model object;
    std::vector<Satellite> satellites;

};
