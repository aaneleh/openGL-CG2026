#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>
using namespace std;

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
int setupShader();
int setupGeometry();
int loadSimpleOBJ(string filePATH, int &nVertices, string &textureFile);
void loadSimpleMTL(string filePATH, string &textureImage);
GLuint loadTexture(string filePath);
void setupLight(string filePATH, GLuint shaderID);

const GLuint WIDTH = 1000, HEIGHT = 1000;

string OBJECT_FILE = "Suzanne.obj";
string ASSETS_DIRECTORY = "../assets/";

float SPEED = 0.1; //o mover e escala vai aumentar nesse valor
int selectedObject = -1;
char selectedAxis = '0';
bool LIGHT = true;

struct Mesh {
    GLuint VAO;
	string meshFile;
	string materialFile;
	string textureFile;
	GLuint textureID;
	int textureWidth;
	int textureHeight;
	glm::vec3 position;
	glm::vec3 rotate;
	glm::vec3 scale;
	int nVertices;
};

struct Light {
	glm::vec3 color;
	glm::vec3 pos;
	bool status;
};

Mesh createMesh(string objPath, glm::vec3 position, float scale);
vector<Mesh> objects;
vector<Light> lights;
void render(glm::mat4 model, GLint modelLoc, Mesh object);

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texc;
layout (location = 3) in vec3 color;

uniform mat4 projection;
uniform mat4 model;

out vec2 texCoords;
out vec3 vNormal;
out vec4 fragPos; 
out vec4 vColor;
void main()
{
   	gl_Position = projection * model * vec4(position.x, position.y, position.z, 1.0);
	fragPos = model * vec4(position.x, position.y, position.z, 1.0);
	texCoords = texc;
	vNormal = normal;
	vColor = vec4(color,1.0);
})";

// Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = R"(
#version 400
in vec2 texCoords;
uniform sampler2D texBuff;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;
uniform float ka;
uniform float kd;
uniform float ks;
uniform float q;
out vec4 color;
in vec4 fragPos;
in vec3 vNormal;
in vec4 vColor;
void main()
{

	//vec3 lightColor = vec3(1.0,1.0,1.0);
	vec4 objectColor = texture(texBuff,texCoords);
	//vec4 objectColor = vColor;

	//Coeficiente de luz ambiente
	vec3 ambient = ka * lightColor;

	//Coeficiente de reflexão difusa
	vec3 N = normalize(vNormal);
	vec3 L = normalize(lightPos - vec3(fragPos));
	float diff = max(dot(N, L),0.0);
	vec3 diffuse = kd * diff * lightColor;

	//Coeficiente de reflexão especular
	vec3 R = normalize(reflect(-L,N));
	vec3 V = normalize(camPos - vec3(fragPos));
	float spec = max(dot(R,V),0.0);
	spec = pow(spec,q);
	vec3 specular = ks * spec * lightColor; 

	vec3 result = (ambient + diffuse) * vec3(objectColor) + specular;
	color = vec4(result,1.0);

})";

int main(){
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Modulo 4 - Three Point Lighting - Helena!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	GLuint shaderID = setupShader();

    objects.push_back(createMesh(ASSETS_DIRECTORY+OBJECT_FILE, glm::vec3(0.5,0.0,0.0), 0.2));
    objects.push_back(createMesh(ASSETS_DIRECTORY+OBJECT_FILE, glm::vec3(-0.5,0.0,0.0), 0.2));

	float ka = 0.1, kd =0.5, ks = 0.5, q = 10.0;

	Light newLight;
	newLight.pos = glm::vec3(0.0, 0.0, 0.0);
	newLight.color = glm::vec3(1.0, 1.0, 1.0);
	newLight.status = true;
	lights.push_back(newLight);

	glm::vec3 lightPos = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 camPos = glm::vec3(0.0,0.0,-3.0);

    glUseProgram(shaderID);

	glActiveTexture(GL_TEXTURE0);

	glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);
	setupLight(ASSETS_DIRECTORY+objects[0].materialFile, shaderID); //Seta os coeficientes KA, KS, etc pegando do arquivo .mtl
	glUniform3f(glGetUniformLocation(shaderID, "lightPos"), lights[0].pos[0],lights[0].pos[1],lights[0].pos[2]);
	glUniform3f(glGetUniformLocation(shaderID, "lightColor"), lights[0].color[0],lights[0].color[1],lights[0].color[2]);
	glUniform3f(glGetUniformLocation(shaderID, "camPos"), camPos.x,camPos.y,camPos.z);

	glm::mat4 projection = glm::ortho(-1.0, 1.0, -1.0, 1.0, -3.0, 3.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	glm::mat4 model = glm::mat4(1);
	GLint modelLoc = glGetUniformLocation(shaderID, "model");
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_BLEND); //Habilita a transparência -- canal alpha
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Seta função de transparência

	while (!glfwWindowShouldClose(window)){
		glfwPollEvents();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(lights[0].status){
			glUniform3f(glGetUniformLocation(shaderID, "lightColor"), lights[0].color[0],lights[0].color[1],lights[0].color[2]);
		} else {
			glUniform3f(glGetUniformLocation(shaderID, "lightColor"), 0.0,0.0,0.0);
		}

        for(int i = 0; i < objects.size(); i ++){
            render(model, modelLoc, objects[i]);
        }

		glfwSwapBuffers(window);

	}
    for(int i = 0; i < objects.size(); i ++){
        glDeleteVertexArrays(1, &objects[i].VAO);
    }
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode){

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	//TESTE LUZ
	if(key == GLFW_KEY_F && action == GLFW_PRESS){
        lights[0].status = !lights[0].status;
        std::cout << "Luz apagada" << std::endl;
    }

	//RESET
	else if(key == GLFW_KEY_0 && action == GLFW_PRESS){
        selectedObject = -1;
		selectedAxis = '0';
        std::cout << "Nenhum objeto ou eixo selecionado" << std::endl;
    }
	
	//SELEÇÃO DE OBJETO
	else if(key == GLFW_KEY_1 && action == GLFW_PRESS){
        selectedObject = 0;
        std::cout << "Objeto selecionado: 1" << std::endl;

    }  else if(key == GLFW_KEY_2 && action == GLFW_PRESS){
        selectedObject = 1;
        std::cout << "Objeto selecionado: 2" << std::endl;
    } 

	//SELEÇÃO DE EIXO
	if (key == GLFW_KEY_X){
		selectedAxis = 'x';
	} else if (key == GLFW_KEY_Y){
		selectedAxis = 'y';
	} else if (key == GLFW_KEY_Z) {
		selectedAxis = 'z';
	} 

    if(selectedObject > -1){
		if (key == GLFW_KEY_R){
			if(selectedAxis == 'x') {
				objects[selectedObject].rotate.x += action != GLFW_RELEASE ? SPEED : 0;
			} else if(selectedAxis == 'y') {
				objects[selectedObject].rotate.y += action != GLFW_RELEASE ? SPEED : 0;
			} else if(selectedAxis == 'z') {
				objects[selectedObject].rotate.z += action != GLFW_RELEASE ? SPEED : 0;
			}
		}

        //Controle de mover: cima/baixo WS, esquerda/direita(x): SD, frente/trás: TG
        if (key == GLFW_KEY_W){
            objects[selectedObject].position.y += action != GLFW_RELEASE ? SPEED : 0;
        } else if (key == GLFW_KEY_S){
            objects[selectedObject].position.y += action != GLFW_RELEASE ? -SPEED : 0;

        } else if (key == GLFW_KEY_D){
            objects[selectedObject].position.x += action != GLFW_RELEASE ? SPEED : 0;
        } else if (key == GLFW_KEY_A){
            objects[selectedObject].position.x += action != GLFW_RELEASE ? -SPEED : 0;

        } else if (key == GLFW_KEY_T){
            objects[selectedObject].position.z += action != GLFW_RELEASE ? SPEED : 0;
        } else if (key == GLFW_KEY_G){
            objects[selectedObject].position.z += action != GLFW_RELEASE ? -SPEED : 0;
        }

        //Controle de escala uniforme: QE
        if (key == GLFW_KEY_Q){
			switch(selectedAxis){
				case 'x':
					objects[selectedObject].scale.x += action != GLFW_RELEASE ? -SPEED : 0;
					break;
				case 'y':
					objects[selectedObject].scale.y += action != GLFW_RELEASE ? -SPEED : 0;
					break;
				case 'z':
					objects[selectedObject].scale.z += action != GLFW_RELEASE ? -SPEED : 0;
					break;
				default:
					objects[selectedObject].scale.x += action != GLFW_RELEASE ? -SPEED : 0;
					objects[selectedObject].scale.y += action != GLFW_RELEASE ? -SPEED : 0;
					objects[selectedObject].scale.z += action != GLFW_RELEASE ? -SPEED : 0;
					break;
			}

        } else if (key == GLFW_KEY_E){
			switch(selectedAxis){
				case 'x':
					objects[selectedObject].scale.x += action != GLFW_RELEASE ? SPEED : 0;
					break;
				case 'y':
					objects[selectedObject].scale.y += action != GLFW_RELEASE ? SPEED : 0;
					break;
				case 'z':
					objects[selectedObject].scale.z += action != GLFW_RELEASE ? SPEED : 0;
					break;
				default:
					objects[selectedObject].scale.x += action != GLFW_RELEASE ? SPEED : 0;
					objects[selectedObject].scale.y += action != GLFW_RELEASE ? SPEED : 0;
					objects[selectedObject].scale.z += action != GLFW_RELEASE ? SPEED : 0;
					break;
			}

        }
	
    }

}

int setupShader(){
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

int loadSimpleOBJ(string filePATH, int &nVertices, string &textureFile) {

	std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<GLfloat> vBuffer;
    glm::vec3 color = glm::vec3(1.0, 0.0, 0.0); //não é mais usado mas sem isso não builda

    std::ifstream arqEntrada(filePATH.c_str());
    if (!arqEntrada.is_open()) {
        std::cerr << "Erro ao tentar ler o arquivo " << filePATH << std::endl;
        return -1;
    }

    std::string line;
    while (std::getline(arqEntrada, line)) {
        std::istringstream ssline(line);
        std::string word;
        ssline >> word;

        if (word == "mtllib") {
            ssline >> textureFile;

		} else if (word == "v") {
            glm::vec3 vertice;
            ssline >> vertice.x >> vertice.y >> vertice.z;
            vertices.push_back(vertice);

        } else if (word == "vn") {
            glm::vec3 normal;
            ssline >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);

		} else if (word == "vt") {
            glm::vec2 vt;
            ssline >> vt.s >> vt.t;
            texCoords.push_back(vt);
			
        } else if (word == "f") {
            while (ssline >> word) {
                int vi = 0, ti = 0, ni = 0;
                std::istringstream ss(word);
                std::string index;

                if (std::getline(ss, index, '/')) vi = !index.empty() ? std::stoi(index) - 1 : 0;
                if (std::getline(ss, index, '/')) ti = !index.empty() ? std::stoi(index) - 1 : 0;
                if (std::getline(ss, index)) ni = !index.empty() ? std::stoi(index) - 1 : 0;

                vBuffer.push_back(vertices[vi].x);
                vBuffer.push_back(vertices[vi].y);
                vBuffer.push_back(vertices[vi].z);
                vBuffer.push_back(normals[ni].x); //normal
                vBuffer.push_back(normals[ni].y); //normal
                vBuffer.push_back(normals[ni].z); //normal
                vBuffer.push_back(texCoords[ti].s); //s
                vBuffer.push_back(texCoords[ti].t); //t
            }
        }
    }

    arqEntrada.close();

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	nVertices = vBuffer.size() / 8;

    return VAO;
}

void loadSimpleMTL(string filePATH, string &textureImage) {

    std::ifstream arqEntrada(filePATH.c_str());

    if (!arqEntrada.is_open()) {
        std::cerr << "Erro ao tentar ler o arquivo " << filePATH << std::endl;
    }

    std::string line;
    while (std::getline(arqEntrada, line)) {
        std::istringstream ssline(line);
        std::string word;
        ssline >> word;

        if (word == "map_Kd") 
            ssline >> textureImage;
	}

    arqEntrada.close();
}

void setupLight(string filePATH, GLuint shaderID) {

	float ka, ks, ke, kd;

    std::ifstream arqEntrada(filePATH.c_str());

    if (!arqEntrada.is_open()) std::cerr << "Erro ao tentar ler o arquivo " << filePATH << std::endl;

    std::string line;
	while (std::getline(arqEntrada, line)) {
        std::istringstream ssline(line);
        std::string word;
        ssline >> word;

        if (word == "Ka") {
            ssline >> ka;

		} else if (word == "Kd") {
            ssline >> kd;

		} else if (word == "Ks") {
            ssline >> ks;

		} else if (word == "Ke") {
            ssline >> ke;

		} 
	}

	glUniform1f(glGetUniformLocation(shaderID, "ka"), ka); //Coeficiente de reflexão ambiente (ka)
	glUniform1f(glGetUniformLocation(shaderID, "kd"), kd); //Coeficiente de reflexão difusa (não tem kd, tentar buscar no .mtl, como não vai ter vai botar 0.0)
	glUniform1f(glGetUniformLocation(shaderID, "ks"), ks); //Coeficiente de reflexão especular (ks)
	glUniform1f(glGetUniformLocation(shaderID, "q"), ke); //Expoente de reflexão especular (ke)

    arqEntrada.close();
}

GLuint loadTexture(string filePath) {
	GLuint texID;

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;

	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data) {
		if (nrChannels == 3) { // jpg, bmp
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		
		} else { // assume que é 4 canais png
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture " << filePath << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);
	
	return texID;
}

Mesh createMesh(string objPath, glm::vec3 position, float scale) {
    Mesh mesh;
	mesh.meshFile = objPath;
	mesh.VAO = loadSimpleOBJ(mesh.meshFile, mesh.nVertices, mesh.materialFile);
	mesh.position = position;
	mesh.rotate = glm::vec3(0.0,0.0,0.0);
	mesh.scale = glm::vec3(scale,scale,scale);
	loadSimpleMTL(ASSETS_DIRECTORY+mesh.materialFile,mesh.textureFile);
	mesh.textureID = loadTexture(ASSETS_DIRECTORY+mesh.textureFile);
    return mesh;
}

void render(glm::mat4 model, GLint modelLoc, Mesh object){

    model = glm::mat4(1); 
    model = glm::translate(model, glm::vec3(object.position.x, object.position.y, object.position.z));
    model = glm::scale(model, glm::vec3(object.scale.x, object.scale.y, object.scale.z));

	model = glm::rotate(model, object.rotate.x, glm::vec3(1.0,0.0,0.0));
	model = glm::rotate(model, object.rotate.y, glm::vec3(0.0,1.0,0.0));
	model = glm::rotate(model, object.rotate.z, glm::vec3(0.0,0.0,1.0));

	glBindVertexArray(object.VAO);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	
	glBindTexture(GL_TEXTURE_2D, object.textureID); //conectando com o buffer de textura que será usado no draw

    glDrawArrays(GL_TRIANGLES, 0, object.nVertices);
}