#ifndef MODEL_H
#define MODEL_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>


#include <glew.h> 
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <SOIL.h>
#include <Importer.hpp>
#include <scene.h>


#include "Mesh.h"
#include"Texture.h"
#include"../DISPLAY/Camera.h"


using std::string;
using std::vector;
using glm::mat4;
using glm::vec3;

class Model
{
public:
	


	Model(GLchar* path);

	// Draws the model, and thus all its meshes
	void Draw(Display* window, Camera camera, Shader shader,
			  vec3 translate = vec3(0.0, 0.0, 0.0),
			  vec3 rotate =    vec3(1.0, 1.0, 1.0),
			  float angle =    0.0,
		      vec3 scale =     vec3(1.0, 1.0, 1.0));

private:
	/*  Model Data  */

	mat4 modelMatrix;
	GLuint modelMatrixUniformLocation;

	GLuint programID;
	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded;	// Stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.

										/*  Functions   */
										// Loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void LoadModel(string path);
	// Processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void ProcessNode(aiNode* node, const aiScene* scene);

	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);


	// Checks all material textures of a given type and loads the textures if they're not loaded yet.
	// The required info is returned as a Texture struct.
	vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName);
};




#endif