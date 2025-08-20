#define PI 3.14159
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

std::string readFileToString(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + filename);
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::map<std::string, int> idMap;
std::vector<std::string> pathArray;

double rad_to_deg(double radians) {
    return radians * 180.0 / PI;
}

int main()
{
    try {
        std::string filename = "node_3d.tscn";
        //std::string content = readFileToString(filename);
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("Failed to open file: " + filename);
        }
        std::string line;
        while (std::getline(file, line)) {
            //std::cout << line << std::endl;
            if (line.find("ext_resource type=\"ArrayMesh\"") != std::string::npos) {
                std::string path = (line.substr(line.find("path=\"") + 6, line.find("\" id=") - line.find("path=\"") - 6)).substr(6);
                std::string id = line.substr(line.find("\" id=") + 6, line.length()- line.find("\" id=") - 8);
                idMap[id] = idMap.size();
                std::string tmpline = "{\"path\":\"" + path + "\",\"index\":" + std::to_string(idMap[id] - 1) +"}";
                std::cout << tmpline << std::endl;
            }
            if (line.find("type=\"MeshInstance3D\"") != std::string::npos) {
                //std::string tmpLine = 
                std::getline(file, line);
                line = line.substr(line.find("Transform3D(") + 12);
                line = line.substr(0, line.length() - 1) + ", ";
                std::vector<float> basis;
                while (line.find(", ") != std::string::npos) {
                    basis.push_back(stof(line.substr(0, line.find(","))));
                    line = line.substr(line.find(",") + 2);
                }
                std::vector<float> position = { basis[9],basis[10],basis[11] };
                float scaleX = sqrt(basis[0] * basis[0] + basis[1] * basis[1] + basis[2] * basis[2]);
                float scaleY = sqrt(basis[3] * basis[3] + basis[4] * basis[4] + basis[5] * basis[5]);
                float scaleZ = sqrt(basis[6] * basis[6] + basis[7] * basis[7] + basis[8] * basis[8]);
                std::vector<float> scale = { scaleX,scaleY,scaleZ };
                std::vector<float> normBasisX = { basis[0] / scaleX,basis[1] / scaleX,basis[2] / scaleX };
                std::vector<float> normBasisY = { basis[3] / scaleY,basis[4] / scaleY,basis[5] / scaleY };
                std::vector<float> normBasisZ = { basis[6] / scaleZ,basis[7] / scaleZ,basis[8] / scaleZ };
                std::vector<float> rotation = { 0,0,0 };
                float sin_pitch = -normBasisZ[1];
                if (std::abs(sin_pitch) > 0.99999) {
                    rotation[2] = 0.0;
                    rotation[1] = std::atan2(
                        -normBasisX[2], // -m20
                        normBasisX[0]  //  m00
                    );

                    rotation[0] = (sin_pitch > 0) ? PI / 2.0 : -PI / 2.0;

                }
                else {
                    rotation[0] = std::asin(sin_pitch);

                    rotation[1] = std::atan2(
                        normBasisZ[0], // m02
                        normBasisZ[2]  // m22
                    );

                    rotation[2] = std::atan2(
                        normBasisX[1], // m10
                        normBasisY[1]  // m11
                    );
                }
                rotation[0] = rad_to_deg(rotation[0]);
                rotation[1] = rad_to_deg(rotation[1]);
                rotation[2] = rad_to_deg(rotation[2]);
                std::getline(file, line);
                std::string id = line.substr(line.find("(\"") + 2, line.find("\")") - line.find("(\"") - 2);
                int index = idMap[id];
                //{"obj":1,"position":[-41.000000,-58.000000,-34.000000],"rotate":[0.000000,0.000000,0.000000],"scale":[3.819163,3.819163,3.819163],"roughness":1.000000}
                std::string positionV3 = "[" + std::to_string(position[0]) + "," + std::to_string(position[1]) + "," + std::to_string(position[2]) + "]";
                std::string scaleV3 = "[" + std::to_string(scale[0]) + "," + std::to_string(scale[1]) + "," + std::to_string(scale[2]) + "]";
                std::string rotationV3 = "[" + std::to_string(rotation[0]) + "," + std::to_string(rotation[1]) + "," + std::to_string(rotation[2]) + "]";
                std::string roughness = "1.00";
                std::string tmpLine = "{\"obj\":" + std::to_string(index) + ",\"position\":" + positionV3 + ",\"rotate\":" + rotationV3 + ",\"scale\":" + scaleV3 + ",\"roughness\":" + roughness + "}";
                std::cout << tmpLine << std::endl;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

}
