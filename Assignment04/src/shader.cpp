#include "shader.h"

using namespace std;

void shader::readIn(string filename)
{
    // Initialize variables, fstream, std string

    std::ifstream fin;
    std::string temp;
    std::string shaderStr = "";

    // open file, clear stream, read in shader to string
    fin.clear();
    fin.open(filename);
    if(fin.is_open()){
      while (getline(fin, temp)){
        shaderStr += temp;
      }
      fin.close();
      shader = shaderStr.c_str();
    }

}

const char* shader::get()
{
 return shader;
}
