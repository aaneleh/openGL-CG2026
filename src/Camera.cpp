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

//constantes e seleção
string OBJECT_FILE = "monkey.obj";
string ASSETS_DIRECTORY = "../assets/";
const GLuint WIDTH = 1000, HEIGHT = 1000;
float OBJECT_SPEED = 0.1; //o mover e escala vai aumentar nesse valor
float CAMERA_SPEED = 0.1; //o mover e escala vai aumentar nesse valor
int selectedObject = -1;
char selectedAxis = '0';

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

struct Material {
	float metallic;
	glm::vec3 color;
	float specular_s;
	float specular_e;
};

struct Light {
	glm::vec3 currentColor;
	glm::vec3 color;
	glm::vec3 pos;
};

struct Camera {
	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 front;
	glm::vec3 up;
};

//construtores e objetos
Camera createCamera(glm::vec3 position);
Mesh createMesh(string objPath, glm::vec3 position, float scale);
Material createMaterial(string filePATH);
Light createLight(glm::vec3 position, glm::vec3 color);
vector<Mesh> objects;
vector<Light> lights;
Camera cam;

//utils
int loadSimpleOBJ(string filePATH, int &nVertices, string &textureFile);
string getTextureImage(string filePATH);
GLuint loadTexture(string filePath);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

//setups
int setupShader();
int setupGeometry();
void setupLights(int shaderID);
void render(glm::mat4 model, GLint modelLoc, Mesh object, bool isSelected);

int main(){
	glfwInit();
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Modulo 5 - Camera - Helena!", nullptr, nullptr);
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

    glUseProgram(shaderID);

	glActiveTexture(GL_TEXTURE0);

	Material material = createMaterial(ASSETS_DIRECTORY+objects[0].materialFile);
	glUniform1f(glGetUniformLocation(shaderID, "metallic"), material.metallic);
	glUniform3f(glGetUniformLocation(shaderID, "color"), material.color[0], material.color[1], material.color[2]);
	glUniform1f(glGetUniformLocation(shaderID, "specular_s"), material.specular_s);
	glUniform1f(glGetUniformLocation(shaderID, "specular_e"), material.specular_e);

	setupLights(shaderID);

	glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);


	cam.position = glm::vec3(0.0,0.0,-2.0);
	cam.target = glm::vec3(0.0,0.0,0.0);
	cam.front = glm::vec3(0.0,0.0, -1.0);
	cam.up = glm::vec3(0.0,1.0,0.0);
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::lookAt(cam.position, cam.position + cam.front, cam.up);


	glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, value_ptr(view));

	glUniform3f(glGetUniformLocation(shaderID, "camPos"), cam.position.x,cam.position.y,cam.position.z);


	glm::mat4 projection = glm::ortho(-1.0, 1.0, -1.0, 1.0, -3.0, 3.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	glm::mat4 model = glm::mat4(1);
	GLint modelLoc = glGetUniformLocation(shaderID, "model");
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND); //Habilita a transparência -- canal alpha
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //Seta função de transparência

	while (!glfwWindowShouldClose(window)){
		glfwPollEvents();

		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniform3f(glGetUniformLocation(shaderID, "camPos"), cam.position.x,cam.position.y,cam.position.z);
		glUniform3f(glGetUniformLocation(shaderID, "keyLight_color"), lights[0].currentColor[0],lights[0].currentColor[1],lights[0].currentColor[2]);
		glUniform3f(glGetUniformLocation(shaderID, "fillLight_color"), lights[1].currentColor[0],lights[1].currentColor[1],lights[1].currentColor[2]);
		glUniform3f(glGetUniformLocation(shaderID, "backLight_color"), lights[2].currentColor[0],lights[2].currentColor[1],lights[2].currentColor[2]);

		glm::mat4 view = glm::mat4(1.0f);
		view = glm::lookAt(cam.position, cam.position + cam.front, cam.up);
		glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, value_ptr(view));

        for(int i = 0; i < objects.size(); i ++){
            render(model, modelLoc, objects[i], selectedObject==i);
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

	if (key == GLFW_KEY_W){
		cam.position += CAMERA_SPEED * cam.front;
	}
	if (key == GLFW_KEY_S){
		cam.position -= CAMERA_SPEED * cam.front;

	} 
	if (key == GLFW_KEY_D){
		cam.position -= glm::normalize(glm::cross(cam.front, cam.up)) * CAMERA_SPEED;

	} 
	if (key == GLFW_KEY_A){
		cam.position += glm::normalize(glm::cross(cam.front, cam.up)) * CAMERA_SPEED;

	} 

	if (key == GLFW_KEY_I) {

	}
    if (key == GLFW_KEY_K) {

	}
    if (key == GLFW_KEY_J) {

	}
    if (key == GLFW_KEY_L) {

	}


	//OBJETO
    if(selectedObject > -1){
		if (key == GLFW_KEY_R){
			if(selectedAxis == 'x') {
				objects[selectedObject].rotate.x += action != GLFW_RELEASE ? OBJECT_SPEED : 0;
			} else if(selectedAxis == 'y') {
				objects[selectedObject].rotate.y += action != GLFW_RELEASE ? OBJECT_SPEED : 0;
			} else if(selectedAxis == 'z') {
				objects[selectedObject].rotate.z += action != GLFW_RELEASE ? OBJECT_SPEED : 0;
			}
		}

        //Controle de mover: cima/baixo WS, esquerda/direita(x): SD, frente/trás: TG
        if (key == GLFW_KEY_W){
            objects[selectedObject].position.y += action != GLFW_RELEASE ? OBJECT_SPEED : 0;
        } else if (key == GLFW_KEY_S){
            objects[selectedObject].position.y += action != GLFW_RELEASE ? -OBJECT_SPEED : 0;

        } else if (key == GLFW_KEY_D){
            objects[selectedObject].position.x += action != GLFW_RELEASE ? OBJECT_SPEED : 0;
        } else if (key == GLFW_KEY_A){
            objects[selectedObject].position.x += action != GLFW_RELEASE ? -OBJECT_SPEED : 0;

        } else if (key == GLFW_KEY_T){
            objects[selectedObject].position.z += action != GLFW_RELEASE ? OBJECT_SPEED : 0;
        } else if (key == GLFW_KEY_G){
            objects[selectedObject].position.z += action != GLFW_RELEASE ? -OBJECT_SPEED : 0;
        }

        //Controle de escala uniforme: QE
        if (key == GLFW_KEY_Q){
			switch(selectedAxis){
				case 'x':
					objects[selectedObject].scale.x += action != GLFW_RELEASE ? -OBJECT_SPEED : 0;
					break;
				case 'y':
					objects[selectedObject].scale.y += action != GLFW_RELEASE ? -OBJECT_SPEED : 0;
					break;
				case 'z':
					objects[selectedObject].scale.z += action != GLFW_RELEASE ? -OBJECT_SPEED : 0;
					break;
				default:
					objects[selectedObject].scale.x += action != GLFW_RELEASE ? -OBJECT_SPEED : 0;
					objects[selectedObject].scale.y += action != GLFW_RELEASE ? -OBJECT_SPEED : 0;
					objects[selectedObject].scale.z += action != GLFW_RELEASE ? -OBJECT_SPEED : 0;
					break;
			}

        } else if (key == GLFW_KEY_E){
			switch(selectedAxis){
				case 'x':
					objects[selectedObject].scale.x += action != GLFW_RELEASE ? OBJECT_SPEED : 0;
					break;
				case 'y':
					objects[selectedObject].scale.y += action != GLFW_RELEASE ? OBJECT_SPEED : 0;
					break;
				case 'z':
					objects[selectedObject].scale.z += action != GLFW_RELEASE ? OBJECT_SPEED : 0;
					break;
				default:
					objects[selectedObject].scale.x += action != GLFW_RELEASE ? OBJECT_SPEED : 0;
					objects[selectedObject].scale.y += action != GLFW_RELEASE ? OBJECT_SPEED : 0;
					objects[selectedObject].scale.z += action != GLFW_RELEASE ? OBJECT_SPEED : 0;
					break;
			}

        }

    } 
	
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	//KEY LIGHT
	else if(key == GLFW_KEY_7 && action == GLFW_PRESS){
		lights[0].currentColor = (lights[0].color != lights[0].currentColor) ? lights[0].color : glm::vec3(0.0,0.0,0.0);
		std::cout << "Toggle Key Light" << std::endl;
	//FILL LIGHT
    } else if(key == GLFW_KEY_8 && action == GLFW_PRESS){
		lights[1].currentColor = (lights[1].color != lights[1].currentColor) ? lights[1].color : glm::vec3(0.0,0.0,0.0);
        std::cout << "Toggle Fill Light" << std::endl;
	//BACK LIGHT
	} else if(key == GLFW_KEY_9 && action == GLFW_PRESS){
		lights[2].currentColor = (lights[2].color != lights[2].currentColor) ? lights[2].color : glm::vec3(0.0,0.0,0.0);
        std::cout << "Toggle Back Light" << std::endl;
	}

	//RESET
	else if(key == GLFW_KEY_0){
        selectedObject = -1;
		selectedAxis = '0';
    }
	
	//SELEÇÃO DE OBJETO
	else if(key == GLFW_KEY_1){
        selectedObject = 0;

    }  else if(key == GLFW_KEY_2){
        selectedObject = 1;
    } 

	//SELEÇÃO DE EIXO
	if (key == GLFW_KEY_X){
		selectedAxis = 'x';
	} else if (key == GLFW_KEY_Y){
		selectedAxis = 'y';
	} else if (key == GLFW_KEY_Z) {
		selectedAxis = 'z';
	} 

}

//CRIA KEY LIGHT, FILL LIGHT E BACK LIGHT E ENVIA PARA O SHADER
void setupLights(int shaderID){
	lights.push_back(createLight(glm::vec3(0.0, 0.0, -0.5), glm::vec3(1.0, 1.0, 1.0)));
	lights.push_back(createLight(glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.8, 0.8, 0.8)));
	lights.push_back(createLight(glm::vec3(0.0, 1.0, 0.0), glm::vec3(1.0, 1.0, 1.0)));

	int l = 0;
	glUniform3f(glGetUniformLocation(shaderID, "keyLight_pos"), lights[l].pos[0],lights[l].pos[1],lights[l].pos[2]);
	glUniform3f(glGetUniformLocation(shaderID, "keyLight_color"), lights[l].currentColor[0],lights[l].currentColor[1],lights[l].currentColor[2]);
	l = 1;
	glUniform3f(glGetUniformLocation(shaderID, "fillLight_pos"), lights[l].pos[0],lights[l].pos[1],lights[l].pos[2]);
	glUniform3f(glGetUniformLocation(shaderID, "fillLight_color"), lights[l].currentColor[0],lights[l].currentColor[1],lights[l].currentColor[2]);
	l = 2;
	glUniform3f(glGetUniformLocation(shaderID, "backLight_pos"), lights[l].pos[0],lights[l].pos[1],lights[l].pos[2]);
	glUniform3f(glGetUniformLocation(shaderID, "backLight_color"), lights[l].currentColor[0],lights[l].currentColor[1],lights[l].currentColor[2]);
}

//SETUP SHADER
int setupShader(){
	// Vertex shader
	const GLchar *vertexShaderSource = R"(
		#version 400
		layout (location = 0) in vec3 position;
		layout (location = 1) in vec3 normal;
		layout (location = 2) in vec2 texc;

		uniform mat4 model;
		uniform mat4 view;
		uniform mat4 projection;

		out vec2 vTexc;
		out vec3 vNormal;
		out vec3 vPosition; 
		void main() {
			vec4 worldPos = model * vec4(position, 1.0);
			gl_Position = projection * worldPos * view;
			vPosition = worldPos.xyz;
			vTexc = texc;
			vNormal = normal;
		})"
	;

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "[setupShader] ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	// Fragment shader
	const GLchar *fragmentShaderSource = R"(
		#version 400

		in vec2 vTexc;
		in vec3 vNormal;
		in vec3 vPosition;

		uniform sampler2D texBuff;

		uniform vec3 camPos;
		uniform vec3 keyLight_pos;
		uniform vec3 keyLight_color;
		uniform vec3 fillLight_pos;
		uniform vec3 fillLight_color;
		uniform vec3 backLight_pos;
		uniform vec3 backLight_color;

		uniform float metallic; //ka
		uniform vec3 color; 	//kd
		uniform float specular_s; //ks
		uniform float specular_e; //ke

		out vec4 finalColor;

		vec3 CalcPointLight(vec3 light_pos, vec3 light_color, vec3 N, vec3 V, vec3 objectPosition, vec4 objectColor){

			vec3 L = normalize(light_pos - vec3(objectPosition));
			float distance = length(L);

			vec3 diff = max(dot(N, L), 0.0)*light_pos;
			
			float attenuation = 1.0 / (distance * distance * distance);

			vec3 diffuse = color * diff * light_color * attenuation;

			vec3 R = normalize(reflect(-L,N));
			float spec = max(dot(R,V),0.0);
			spec = pow(spec,1.0);

			vec3 specular = specular_s * spec * light_color; 

			return (diffuse) * vec3(objectColor) + specular;

		} 

		void main(){

			vec3 N = normalize(vNormal);
			vec3 V = normalize(camPos - vec3(vPosition));

			vec4 objectColor = texture(texBuff,vTexc);

			//vec3 ambient = metallic * vec3(1.0, 1.0, 1.0) * vec3(objectColor);

			vec3 result = CalcPointLight(keyLight_pos, keyLight_color, N, V, vPosition, objectColor);
			result += CalcPointLight(fillLight_color, fillLight_color, N, V, vPosition, objectColor);
			result += CalcPointLight(backLight_color, backLight_color, N, V, vPosition, objectColor);

			finalColor = vec4((result * vec3(objectColor)),1.0);

		})"
	;

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "[setupShader] ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
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
		std::cout << "[setupShader] ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

// CARREGA UM OBJETO OBJ E RETORNA O SEU VAO, NUMERO DE VERTICES E QUAL O NOME DO ARQUIVO DE TEXTURA
int loadSimpleOBJ(string filePATH, int &nVertices, string &textureFile) {

	//CARREGA OS VERTICES, NORMAIS E COORDENADAS DE TEXTURA DO ARQUIVO OBJ
	std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<GLfloat> vBuffer;
    glm::vec3 color = glm::vec3(1.0, 0.0, 0.0); //não é mais usado mas sem isso não builda

    std::ifstream fileOBJ(filePATH.c_str());
    if (!fileOBJ.is_open()) std::cerr << "[loadSimpleOBJ] Erro ao tentar ler o arquivo .obj: " << filePATH << std::endl;

    std::string line;
    while (std::getline(fileOBJ, line)) {
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

    fileOBJ.close();

	//CRIA E RETORNA O VBO E VAO DO OBJETO IMPORTADO
    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
	//POSIÇÃO
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

	//NORMAIS
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

	//TEXTURA
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	nVertices = vBuffer.size() / 8;
    return VAO;
}

//CARREGA UM ARQUIVO MTL E RETORNA O NOME DA IMAGEM DE TEXTURA
string getTextureImage(string filePATH) {

	string textureImage; 
    std::ifstream arqEntrada(filePATH.c_str());

    if (!arqEntrada.is_open()) std::cerr << "[getTextureImage] Erro ao tentar ler o arquivo " << filePATH << std::endl;

    std::string line;
    while (std::getline(arqEntrada, line)) {
        std::istringstream ssline(line);
        std::string word;
        ssline >> word;

        if (word == "map_Kd") 
            ssline >> textureImage;
	}

    arqEntrada.close();
	return textureImage;
}

//CARREGA UMA IMAGEM DE TEXTURA E RETORNA O TEXTURE ID CRIADO
GLuint loadTexture(string filePath) {
	std::cout << "[loadTexture] Carregando textura " << filePath << std::endl;
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
	else std::cout << "[loadTexture] Failed to load texture " << filePath << std::endl;

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);
	
	std::cout << "[loadTexture] Textura " << filePath << " carregada" << std::endl;
	return texID;
}

//RETORNA UMA MESH COM UM VAO DO OBJ ESCOLHIDO, POSIÇÃO ESCOLHIDO, ROTAÇÃO DEFAULT, ESCALA ESCOLHIDA E TEXTURA DO OBJ
Mesh createMesh(string objPath, glm::vec3 position, float scale) {
    Mesh mesh;
	mesh.meshFile = objPath;
	mesh.VAO = loadSimpleOBJ(mesh.meshFile, mesh.nVertices, mesh.materialFile);
	mesh.position = position;
	mesh.rotate = glm::vec3(0.0,0.0,0.0);
	mesh.scale = glm::vec3(scale,scale,scale);
	mesh.textureFile = getTextureImage(ASSETS_DIRECTORY+mesh.materialFile);
	if(mesh.textureFile != "")	mesh.textureID = loadTexture(ASSETS_DIRECTORY+mesh.textureFile);
    return mesh;
}

//RETORNA UMA CAMERA COM POSIÇÃO ESCOLHIDA E ROTAÇÃO DEFAULT
Camera createCamera(glm::vec3 position){
    Camera cam;
	cam.position = position;
    return cam;
}

//RETORNA UM MATERIAL COM AS PROPRIEDAS DE MATERIAL DO ARQUIVO MTL ESCOLHIDO
Material createMaterial(string filePATH) {
	Material material;
    std::ifstream arqEntrada(filePATH.c_str());

    if (!arqEntrada.is_open()) std::cerr << "Erro ao tentar ler o arquivo " << filePATH << std::endl;

    std::string line;
	while (std::getline(arqEntrada, line)) {
        std::istringstream ssline(line);
        std::string word;
        ssline >> word;

        if (word == "Ka") {
            ssline >> material.metallic;

		} else if (word == "Kd") {
            ssline >> material.color[0] >> material.color[1] >> material.color[2];

		} else if (word == "Ks") {
            ssline >> material.specular_s;

		} else if (word == "Ke") {
            ssline >> material.specular_e;

		} 
	}

    arqEntrada.close();
	return material;
}

//RETORNA UMA LIGHT NA POSIÇÃO E COR ESCOLHIDAS
Light createLight(glm::vec3 position, glm::vec3 color){
	Light light;
	light.pos = position;
	light.color = color;
	light.currentColor = color;
	return light;
}

//RENDERIZA UMA MESH
void render(glm::mat4 model, GLint modelLoc, Mesh object, bool isSelected){

    model = glm::mat4(1); 
    model = glm::translate(model, glm::vec3(object.position.x, object.position.y, object.position.z));
    model = glm::scale(model, glm::vec3(object.scale.x, object.scale.y, object.scale.z));

	model = glm::rotate(model, object.rotate.x, glm::vec3(1.0,0.0,0.0));
	model = glm::rotate(model, object.rotate.y, glm::vec3(0.0,1.0,0.0));
	model = glm::rotate(model, object.rotate.z, glm::vec3(0.0,0.0,1.0));

	glBindVertexArray(object.VAO);

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
	
	glBindTexture(GL_TEXTURE_2D, object.textureID); 

    glDrawArrays(GL_TRIANGLES, 0, object.nVertices);

	if(isSelected)	{
		glPointSize(4);
		glDrawArrays(GL_POINTS, 0, object.nVertices);
		glBindVertexArray(0);
	}
}