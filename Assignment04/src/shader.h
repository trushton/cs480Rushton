#include <stdexcept>
#include <iostream>
#include <string> //added for read in
#include <fstream> //added for read in
#include <streambuf> //added for read in
using namespace std;

class shader
{
  public:
  void readIn(string);
  char* get();
  private:
  char* shader;
};