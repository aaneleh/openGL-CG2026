# Computação Gráfica - Híbrido

## Setup

1. Compilador C/C++
    - msys64 https://github.com/msys2/msys2-installer/releases/download/2025-12-13/msys2-x86_64-20251213.exe
2. Bibliotecas para desenvolvimento
    - msys2 → ```sh pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain``` 
3. Compilador e configurador de projeto
    - CMake https://github.com/Kitware/CMake/releases/download/v4.0.2/cmake-4.0.2-windows-x86_64.zip
    - E extensão VS Code Name https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools
4. Ferramenta para importar bibliotecas do Git
    - Git portable https://github.com/git-for-windows/git/releases/download/v2.49.0.windows.1/PortableGit-2.49.0-64-bit.7z.exe
5. Configuração CMAKE  
    - /.vscode/settings.json
    ```
    { 
        "cmake.cmakePath": "C:/Users/NOME_DO_USUARIO/Documents/cmake-4.0.2-windows-x86_64/bin/cmake.exe", 
        "cmake.environment": { 
            "PATH": "C:\\msys64\\ucrt64\\bin;C:\\Users\\NOME_DO_USUARIO\\Documents\\cmake-4.0.2-windows-x86_64\\bin;C:\\Users\\NOME_DO_USUARIO\\Documents\\PortableGit\\bin;${env:PATH}" 
        },
        "cmake.buildEnvironment": {
            "PATH": "C:\\msys64\\ucrt64\\bin;C:\\Users\\NOME_DO_USUARIO\\Documents\\cmake-4.0.2-windows-x86_64\\bin;C:\\Users\\NOME_DO_USUARIO\\Documents\\PortableGit\\bin;${env:PATH}"
        }
    }
    ```
    - Rodar "CMake: Delete Cache and Reconfigure"
    - Rodar "CMake: Select a Kit"
    - Rodar "CMake: Build"

## Assets 

Todos os assets e texturas foram criados no https://www.blender.org/

## Referências

Iluminação https://learnopengl.com/Lighting/Multiple-lights
Camera https://learnopengl.com/Getting-Started/Camera


## Grau B - `Keyframes.cpp`

#### Seleção e Transformação: 
Selecionar um objeto da cena e aplicar operações de translação, rotação e escala uniforme.
    1: Seleciona o objeto 1
    2: Seleciona o objeto 2 (...)
    x: Seleciona o eixo
    y: Seleciona o eixo Y
    z: Seleciona o eixo Z
    0: Desseleciona eixo e objeto
    W: Move objeto para cima
    S: Move objeto para baixo
    D: Move objeto para a direita
    A: Move objeto para a esquerda
    T: Move objeto para frente
    G: Move objeto para tras
	Q: Diminui a escala no eixo selecionado (ou uniforme se nao tem eixo)
    E: Aumenta a escala no eixo selecionado (ou uniforme se nao tem eixo)
    R: Rotaciona no eixo selecionado
#### Materiais e Texturas
Alternar a exibição para comprovar a leitura correta dos coeficientes do material (ka, ks, kd do arquivo .mtl) e o mapeamento de textura.
#### Iluminação (Phong):
Modificar parâmetros de luz e ligar/desligar individualmente as fontes de luz da cena (incluindo a lógica de 3 pontos), evidenciando o modelo de iluminação implementado.
    7: Desliga/Liga key light
    8: Desliga/Liga fill light
    9: Desliga/Liga back light
#### Câmera: 
Navegar ativamente pela cena utilizando a câmera (controle via teclado/mouse).
    UP: Move camera para cima
    DOWN: Move camera para baixo
    RIGHT: Move camera para a direita
    LEFT: Move camera para a esquerda
    MOUSE (PRESSIONANDO BOTAO DIREITO): Olha ao redor  
#### Animação: 
Iniciar e pausar a trajetória de objetos definidos com curvas paramétricas (Bézier).
    MOUSE (CLIQUE DO BOTAO ESQUERDO): Cria novo keyframe no objeto selecionado
    P: Pausa/Continua as animações



## Tarefa 1  `Hello3D.cpp`

![Tarefa 1 - Hello3D.cpp](Hello3D.png)


## Tarefa 2  `Cube.cpp`  

- Alterado a geometria para um cubo
- Alterado cores: cada face tem uma cor e cada triangulo uma variação de tom
- Alterada rotação: trocado para float para poder rotacionar até nos 3 eixo ao mesmo tempo
- Adicionado translação: A e D move no eixo X, W e S no eixo Y e I e J no eixo Z
- Adicionado controle de escala: Q diminui a escala e E aumenta
- Adicionado outro cubo na cena: criado um Struct Cube, cada cubo tem sua instância com VAO, posição e escala inicial

![Tarefa 2 - Cube.cpp](Cube.png)


## Vivencial 1 `MeshTransform.cpp`

- Leitura de arquivos .OBJ
- Exibir mais de um objeto na tela
- Seleção dos objetos, a partir de uma tecla (1 e 2) (0 remove a seleção)
- Aplicação de transformações no objeto selecionado:
    - Rotacionar (R) nos eixos x, y e z: Rotaciona em enquanto pressionando X, Y e Z
    - Transladar (T) nos eixos x, y e z: Move ao clicar em WASD e TG
    - Aplicar escala (S): uniforme usando Q e E e por eixo usando UI, JK e NM para os eixos x, y e z, respectivamente

![Vivencial 1 - MeshTransform.cpp](MeshTransform.png)
  
  
## Tarefa 3  `Mesh.cpp` 

- Carregado arquivo .obj pelo loadSimpleOBJ
- Adicionado atributos de normais e coordenadas de textura (s t) ao VAO
- Renderizado objeto carregado
- Lido nome do arquivo de textura do material (.mtl) do objeto

  
## Tarefa 4  `Phong.cpp` 

- Carregado as informações dos vetores normais dos vértices no arquivo .OBJ (vn). 
- Recuperar os coeficientes de iluminação ambiente, difusa e especular do arquivo de materiais (.mtl), que serão enviados pela aplicação para o fragment shader, onde calcularemos sua contribuição para a cor do pixel.

![Tarefa 4 - Phong.cpp](Phong.png)
  
  
## Vivencial 2 `Lighting.cpp`

- Alterado sistema de transformações: ao clicar em XYZ é selecionado o eixo, então R rotaciona e QE alteram a escala; 0 remove a seleção
- Criado Struct para controlar as luzes
- Posicionar 3 luzes na cena, de acordo com a técnica de iluminação de 3 pontos de forma automática, a partir da posição e escala do objeto principal da cena.
    - key light: É a fonte de luz mais intensa, que ilumina o objeto principal e define o tom geral da cena.
    - fill light: Suaviza as sombras criadas pela luz principal, equilibrando a iluminação e evitando que a imagem fique muito contrastada.
    - back light: Ilumina o fundo do objeto, criando profundidade e separando-o do fundo.
- Implementar um fator de atenuação na parcela de reflexão difusa
- Permitir, através de teclas, desabilitar e habilitar cada uma das 3 fontes de luz. 
    - 7 8 e 9 para keyLight, fillLight e backLight respectivamente


## Tarefa 5  `Classes.cpp` 

- Implemente uma câmera em primeira pessoa. 
- Implemente a câmera como um objeto de uma classe Câmera, de maneira a agrupar todos os seus atributos e encapsular as 
- Ações de Mover e Rotacionar.
  
- Refatoração função de teclado
- Encapsulamento em classes Camera, Mesh, Light e Material

> Para usar objetos do blender, tem que ser marcado para exportar UV coordenates, normals e trianguled mesh


## Tarefa 6 `Keyframes.cpp`

- Adicione trajetórias para objetos
- Permitir a adição de alguns pontos no espaço e realizar a translação do objeto por estes pontos de maneira cíclica  
