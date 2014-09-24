#include "shader.h"

using namespace std;

void shader::readIn(string filename)
{
    // Initialize variables, fstream, std string

    std::ifstream fin;
    std::string temp;

    // open file, clear stream, read in shader to string

    fin.clear();
    fin.open(filename);
    while( fin.good() )
    {
     temp += fin.get();
    }
    temp += '\0';
    // convert string into c_style string

    int y = temp.length()-1;
    shader = new char[temp.length()];
    for(int x=0;x<y;x++)
    {
     shader[x] = temp[x];
    }
    fin.close();
}

char* shader::get()
{
 return shader;
}
