# Computação Gráfica - Híbrido

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

![Tarefa 3 - Mesh.cpp](Mesh.png)
  
  
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
