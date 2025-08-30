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
std::map<std::string, std::string> audioPathMap;

double rad_to_deg(double radians) {
    return radians * 180.0 / PI;
}

std::vector<float> basisToPRS(std::vector<float> basis) {
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
    return std::vector<float> {position[0], position[1], position[2], rotation[0], rotation[1], rotation[2], scale[0], scale[1], scale[2]};
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

            if (line.find("ext_resource type=\"ArrayMesh\"") != std::string::npos) {
                std::string path = (line.substr(line.find("path=\"") + 6, line.find("\" id=") - line.find("path=\"") - 6)).substr(6);
                std::string id = line.substr(line.find("\" id=") + 6, line.length()- line.find("\" id=") - 8);
                idMap[id] = idMap.size() - 1;
                std::string tmpline = "{\"path\":\"" + path + "\",\"index\":" + std::to_string(idMap[id]) +"}";
                std::cout << tmpline << std::endl;
            }

            if (line.find("ext_resource type=\"AudioStream\"") != std::string::npos) {
                std::string path = (line.substr(line.find("path=\"") + 6, line.find("\" id=") - line.find("path=\"") - 6)).substr(6);
                std::string id = line.substr(line.find("\" id=") + 6, line.length() - line.find("\" id=") - 8);
                audioPathMap[id] = path;
            }

            if (line.find("type=\"MeshInstance3D\"") != std::string::npos) {
                std::string positionV3 = "[0,0,0]";
                std::string rotationV3 = "[0,0,0]";
                std::string scaleV3 = "[0,0,0]";
                std::string roughness = "1.00";

                std::string scaleV2 = "[0,0]";
                std::string warpscale_1 = "0";
                std::string warpscale_2 = "0";
                std::string flow_1V2 = "[0,0]";
                std::string flow_2V2 = "[0,0]";
                bool isWater = false;
                int index = 0;
                while (line != "") {
                    if (line.find("transform =") != std::string::npos) {
                        std::vector<float> basis;
                        line = line.substr(line.find("Transform3D(") + 12);
                        line = line.substr(0, line.length() - 1) + ", ";
                        while (line.find(", ") != std::string::npos) {
                            basis.push_back(stof(line.substr(0, line.find(","))));
                            line = line.substr(line.find(",") + 2);
                        }
                        std::vector<float> PRS = basisToPRS(basis);
                        positionV3 = "[" + std::to_string(PRS[0]) + "," + std::to_string(PRS[1]) + "," + std::to_string(PRS[2]) + "]";
                        rotationV3 = "[" + std::to_string(PRS[3]) + "," + std::to_string(PRS[4]) + "," + std::to_string(PRS[5]) + "]";
                        scaleV3 = "[" + std::to_string(PRS[6]) + "," + std::to_string(PRS[7]) + "," + std::to_string(PRS[8]) + "]";

                        scaleV2 = "[" + std::to_string(PRS[6]) + "," + std::to_string(PRS[8]) + "]";
                    }
                    if (line.find("mesh = ExtResource") != std::string::npos) {
                        std::string id = line.substr(line.find("(\"") + 2, line.find("\")") - line.find("(\"") - 2);
                        index = idMap[id];
                    }
                    if (line.find("metadata/roughness =") != std::string::npos) {
                        roughness = line.substr(line.find("= ") + 2);
                    }
                    if (line.find("SubResource(\"PlaneMesh") != std::string::npos) {
                        isWater = true;
                    }
                    if (line.find("metadata/flow_1 =") != std::string::npos) {
                        line = line.substr(line.find("(") + 1);
                        line = line.substr(0, line.length() - 1) + ", ";
                        std::vector<float> flow_1;
                        while (line.find(", ") != std::string::npos) {
                            flow_1.push_back(stof(line.substr(0, line.find(","))));
                            line = line.substr(line.find(",") + 2);
                        }
                        flow_1V2 = "[" + std::to_string(flow_1[0]) + "," + std::to_string(flow_1[1]) + "]";
                    }
                    if (line.find("metadata/flow_2 =") != std::string::npos) {
                        line = line.substr(line.find("(") + 1);
                        line = line.substr(0, line.length() - 1) + ", ";
                        std::vector<float> flow_2;
                        while (line.find(", ") != std::string::npos) {
                            flow_2.push_back(stof(line.substr(0, line.find(","))));
                            line = line.substr(line.find(",") + 2);
                        }
                        flow_2V2 = "[" + std::to_string(flow_2[0]) + "," + std::to_string(flow_2[1]) + "]";
                    }
                    if (line.find("metadata/warpscale_1 =") != std::string::npos) {
                        line = line.substr(line.find("(") + 1);
                        warpscale_1 = line.substr(line.find("metadata/warpscale_1 =") + 23);
                    }
                    if (line.find("metadata/warpscale_2 =") != std::string::npos) {
                        line = line.substr(line.find("(") + 1);
                        warpscale_2 = line.substr(line.find("metadata/warpscale_2 =") + 23);
                    }
                    std::getline(file, line);
                }
                if (!isWater) {
                    std::string tmpLine = "{\"obj\":" + std::to_string(index) + ",\"position\":" + positionV3 + ",\"rotation\":" + rotationV3 + ",\"scale\":" + scaleV3 + ",\"roughness\":" + roughness + "}";
                    std::cout << tmpLine << std::endl;
                }
                else {
                    //{"type":"waterlayer","position":[0,1.5,0],"scale":[200,200],"warpscale_1":2,"warpscale_2":6,"flow_1":[0.5,0.5],"flow_2":[-0.17,-0.17]}
                    std::string tmpLine = "{\"type\":\"waterlayer\",\"position\":" + positionV3 + ",\"scale\":" + scaleV2 + ",\"warpscale_1\":" + warpscale_1 + ",\"warpscale_2\":" + warpscale_2 + ",\"flow_1:\":" + flow_1V2 + ",\"flow_2:\":" + flow_2V2 + "}";
                    std::cout << tmpLine << std::endl;
                }
                
            }

            if (line.find("type=\"OmniLight3D\"") != std::string::npos) {
                std::vector<float> color = { 1,1,1 };
                float energy = 1, range = 1;
                std::vector<float> PRS = { 0,0,0,0,0,0,0,0,0, };
                while (line != "") {
                    if (line.find("Transform3D(") != std::string::npos) {
                        std::vector<float> basis;
                        line = line.substr(line.find("Transform3D(") + 12);
                        line = line.substr(0, line.length() - 1) + ", ";
                        while (line.find(", ") != std::string::npos) {
                            basis.push_back(stof(line.substr(0, line.find(","))));
                            line = line.substr(line.find(",") + 2);
                        }
                        PRS = std::vector<float>();
                        PRS = basisToPRS(basis);
                    }

                    if (line.find("light_color =") != std::string::npos) {
                        color = std::vector<float>();
                        line = line.substr(line.find("Color(") + 6);
                        line = line.substr(0, line.length() - 1) + ", ";
                        while (line.find(", ") != std::string::npos) {
                            color.push_back(stof(line.substr(0, line.find(","))) / 255.0f);
                            line = line.substr(line.find(",") + 2);
                        }
                    }

                    if (line.find("light_energy =") != std::string::npos) {
                        energy = std::stof(line.substr(line.find("= ") + 2));
                    }

                    if (line.find("omni_range =") != std::string::npos) {
                        range = std::stof(line.substr(line.find("= ") + 2));
                    }
                    std::getline(file, line);
                }
                //{"type":"pointlight","lightpos":[-7,14,0],"lightscale":[0.7,0.7,0.7],"lightcolor":[5,5,5],"lightmaxrad":100}
                std::string positionV3 = "[" + std::to_string(PRS[0]) + "," + std::to_string(PRS[1]) + "," + std::to_string(PRS[2]) + "]";
                std::string colorV3 = "[" + std::to_string(color[0] * energy) + "," + std::to_string(color[1] * energy) + "," + std::to_string(color[2] * energy) + "]";
                //lightVolume
                std::string lightscaleV3 = "[" + std::to_string(1) + "," + std::to_string(1) + "," + std::to_string(1) + "]";
                std::string tmpLine = "{\"type\":\"pointlight\",\"lightpos\":" + positionV3 + ",\"lightscale\":" + lightscaleV3 + ",\"lightcolor\":" + colorV3 + ",\"lightmaxrad\":" + std::to_string(range) + "}";
                std::cout << tmpLine << std::endl;
            }


            if (line.find("type=\"DirectionalLight3D\"") != std::string::npos) {
                std::vector<float> color = { 1,1,1 };
                float energy = 1, range = 1;
                std::vector<float> basis = { 1,1,1,1,1,1,1,1,1,1,1,1 };
                while (line != "") {
                    if (line.find("Transform3D(") != std::string::npos) {
                        basis = std::vector<float>();
                        line = line.substr(line.find("Transform3D(") + 12);
                        line = line.substr(0, line.length() - 1) + ", ";
                        while (line.find(", ") != std::string::npos) {
                            basis.push_back(stof(line.substr(0, line.find(","))));
                            line = line.substr(line.find(",") + 2);
                        }
                    }

                    if (line.find("light_color =") != std::string::npos) {
                        color = std::vector<float>();
                        line = line.substr(line.find("Color(") + 6);
                        line = line.substr(0, line.length() - 1) + ", ";
                        while (line.find(", ") != std::string::npos) {
                            color.push_back(stof(line.substr(0, line.find(","))) / 255.0f);
                            line = line.substr(line.find(",") + 2);
                        }
                    }

                    if (line.find("light_energy =") != std::string::npos) {
                        energy = std::stof(line.substr(line.find("= ") + 2));
                    }

                    if (line.find("omni_range =") != std::string::npos) {
                        range = std::stof(line.substr(line.find("= ") + 2));
                    }
                    std::getline(file, line);
                }
                //{"type":"directlight","lightvec":[-1,-1,-1],"lightcolor":[0,0,0]}
                std::string dirV3 = "[" + std::to_string(-basis[2]) + "," + std::to_string(-basis[5]) + "," + std::to_string(-basis[8]) + "]";
                std::string colorV3 = "[" + std::to_string(color[0] * energy) + "," + std::to_string(color[1] * energy) + "," + std::to_string(color[2] * energy) + "]";
                std::string tmpLine = "{\"type\":\"directlight\",\"lightpos\":" + dirV3 + ",\"lightcolor\":" + colorV3 + "}";
                std::cout << tmpLine << std::endl;
            }

            if (line.find("type=\"Camera3D\"") != std::string::npos) {
                std::vector<float> basis = { 1,1,1,1,1,1,1,1,1,1,1,1 };
                while (line != "") {
                    if (line.find("Transform3D(") != std::string::npos) {
                        basis = std::vector<float>();
                        line = line.substr(line.find("Transform3D(") + 12);
                        line = line.substr(0, line.length() - 1) + ", ";
                        while (line.find(", ") != std::string::npos) {
                            basis.push_back(stof(line.substr(0, line.find(","))));
                            line = line.substr(line.find(",") + 2);
                        }
                        std::vector<float> PRS = basisToPRS(basis);
                        std::string positionV3 = "[" + std::to_string(PRS[0]) + "," + std::to_string(PRS[1]) + "," + std::to_string(PRS[2]) + "]";
                        std::string rotationV3 = "[" + std::to_string(PRS[3]) + "," + std::to_string(PRS[4]) + "," + std::to_string(PRS[5]) + "]";
                        std::string tmpLine = "{\"type\":defaultCamera,\"position\":" + positionV3 + ",\"rotation\":" + rotationV3 + "}";
                        std::cout << tmpLine << std::endl;
                    }
                    std::getline(file, line);
                }
            }
            
            if (line.find("type=\"AudioStreamPlayer3D\"") != std::string::npos) {
                std::string path;
                std::string positionV3 = "[0,0,0]";
                std::string axisfollowingV3 = "[0,0,0]";
                std::string looping = "1";
                while (line != "") {
                    if (line.find("stream =") != std::string::npos) {
                        line = line.substr(line.find("ExtResource(\"") + 13);
                        std::string id = line.substr(0, line.length() - 2);
                        path = audioPathMap[id];
                    }

                    if (line.find("transform =") != std::string::npos) {
                        std::vector<float> basis;
                        line = line.substr(line.find("Transform3D(") + 12);
                        line = line.substr(0, line.length() - 1) + ", ";
                        while (line.find(", ") != std::string::npos) {
                            basis.push_back(stof(line.substr(0, line.find(","))));
                            line = line.substr(line.find(",") + 2);
                        }
                        std::vector<float> PRS = basisToPRS(basis);
                        positionV3 = "[" + std::to_string(PRS[0]) + "," + std::to_string(PRS[1]) + "," + std::to_string(PRS[2]) + "]";
                    }

                    if (line.find("metadata/axisfollowing =") != std::string::npos) {
                        line = line.substr(line.find("(") + 1);
                        line = line.substr(0, line.length() - 1) + ", ";
                        std::vector<int> axisfollowing;
                        while (line.find(", ") != std::string::npos) {
                            axisfollowing.push_back(stoi(line.substr(0, line.find(","))));
                            line = line.substr(line.find(",") + 2);
                        }
                        axisfollowingV3 = "[" + std::to_string(axisfollowing[0]) + "," + std::to_string(axisfollowing[1]) + "," + std::to_string(axisfollowing[2]) + "]";
                    }

                    if (line.find("metadata/looping = false") != std::string::npos) {
                        looping = "0";
                    }
                    std::getline(file, line);
                }
                if (path != "") {
                    std::string tmpLine = "{\"type\":audio,\"path\":" + path + ",\"position\":" + positionV3 + ",\"axisfollowing\":" + axisfollowingV3 + ",\"looping\":" + looping + "}";
                    std::cout << tmpLine << std::endl;
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

}
