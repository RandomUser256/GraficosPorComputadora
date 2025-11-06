#ifndef OBJ_H
#define OBJ_H

#include <fstream>
#include <string>
#include <vector>
#include <sstream>
// #include <glm/glm.hpp> // If using GLM for 3D math
#include "glm-master/glm/glm.hpp"

#include "GlassTracer.h"

#include "hittable.h"

class Object3D : public hittable {
    public:
        bool loadOBJ(const char* path, std::vector<glm::vec3>& out_vertices, std::vector<glm::vec2>& out_uvs, std::vector<glm::vec3>& out_normals) {
            std::ifstream file(path, std::ios::in);
            if (!file.is_open()) {
                // Handle error: file not found
                return false;
            }

            std::vector<glm::vec3> temp_vertices;
            std::vector<glm::vec2> temp_uvs;
            std::vector<glm::vec3> temp_normals;
            std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;

            std::string line;
            while (std::getline(file, line)) {
                std::stringstream ss(line);
                std::string prefix;
                ss >> prefix;

                if (prefix == "v") {
                    glm::vec3 vertex;
                    ss >> vertex.x >> vertex.y >> vertex.z;
                    temp_vertices.push_back(vertex);
                    out_vertices.push_back(vertex);
                } else if (prefix == "vt") {
                    glm::vec2 uv;
                    ss >> uv.x >> uv.y;
                    temp_uvs.push_back(uv);
                    out_uvs.push_back(uv);
                } else if (prefix == "vn") {
                    glm::vec3 normal;
                    ss >> normal.x >> normal.y >> normal.z;
                    temp_normals.push_back(normal);
                    out_normals.push_back(normal);
                } else if (prefix == "f") {
                    // Parse face indices (e.g., v/vt/vn v/vt/vn v/vt/vn)
                    // This part can be complex due to varying face formats
                    // and requires careful parsing of '/' delimiters.
                    // Store the parsed indices into vertexIndices, uvIndices, normalIndices.
                }
            }

            // After parsing, use the indices to construct the final interleaved vertex data
            // for out_vertices, out_uvs, out_normals if needed, or directly use the indices
            // with the temporary data for rendering.

            file.close();
            return true;
        }
    private:
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> verticeNormals;
        std::vector<glm::vec2> textureCoordinates;
};

#endif