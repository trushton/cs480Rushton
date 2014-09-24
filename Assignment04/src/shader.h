#ifndef SHADER_HPP
#define SHADER_HPP

#include <iostream>
#include <fstream>
#include <string>

class shader{
	private:
		const char* vertex;
		const char* fragment;

	public:
		const char* getVertex(){ return vertex; }
		const char* getFragment(){ return fragment; }

		void readVertex(){
			std::ifstream in("vertex.txt");
			std::string content( (std::istreambuf_iterator<char>(in) ),
				std::istreambuf_iterator<char>() );
			vertex = content.c_str();
		}

		void readFragment(){
				std::ifstream in("fragment.txt");
				std::string content( (std::istreambuf_iterator<char>(in) ),
					std::istreambuf_iterator<char>() );
				fragment = content.c_str();
		}
};

#endif
