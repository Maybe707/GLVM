#include <bits/types/FILE.h>
#include <iostream>
#include <fstream>
#include <sstream>

void ReadFile(const char* _filePath) {
	const char* _pWavefrontObjFile = _filePath;
	std::ifstream WavefrontObjFileInputStream;
	std::stringstream WavefrontObjFileOutputStream;
	std::string sWavefrontObjFileData;
	const char* pWavefrontObjFileData;
        
	WavefrontObjFileInputStream.open(_pWavefrontObjFile);
	if(WavefrontObjFileInputStream.good()) {

		WavefrontObjFileOutputStream << WavefrontObjFileInputStream.rdbuf();
		WavefrontObjFileInputStream.close();
		sWavefrontObjFileData = WavefrontObjFileOutputStream.str();
	} else {
		std::cout << "Error of reading wavefront.obj file" << std::endl;
		return;
	}

	pWavefrontObjFileData = sWavefrontObjFileData.c_str();

	for (unsigned int i = 0; i < sWavefrontObjFileData.size(); ++i) {
		if (sWavefrontObjFileData[i] == '\n')
			std::cout << sWavefrontObjFileData[i] << std::endl;
		else
			std::cout << sWavefrontObjFileData[i];
	}
}

void readFile(const char* _filePath) {
	char buffer[840];

	FILE* ptr;

	ptr = fopen("cube.bin", "rb");
	fread(buffer, 840, 1, ptr);

	for(unsigned int i = 0; i < 840; i += 2)
		printf("%hu ", buffer[i]);
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
	readFile("cube.bin");
	
	return 0;
}
