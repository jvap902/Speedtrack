#version 330 core

// Atributos de vértice recebidos como entrada ("in") pelo Vertex Shader.
// Veja a função BuildTrianglesAndAddToVirtualScene() em "main.cpp".
layout (location = 0) in vec4 model_coefficients;
layout (location = 1) in vec4 normal_coefficients;
layout (location = 2) in vec2 texture_coefficients;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Precisamos que o Vertex Shader também conheça esses dados
uniform int object_id;
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Samplers de textura (apenas os que precisamos aqui)
uniform sampler2D TextureImage0; // Textura da esfera

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

#define SPHERE 0

// Atributos de vértice que serão gerados como saída ("out") pelo Vertex Shader.
// ** Estes serão interpolados pelo rasterizador! ** gerando, assim, valores
// para cada fragmento, os quais serão recebidos como entrada pelo Fragment
// Shader. Veja o arquivo "shader_fragment.glsl".
out vec4 position_world;
out vec4 position_model;
out vec4 normal;
out vec2 texcoords;

// Novo atributo out para Gouraud Shading da esfera
out vec4 cor_v;

void main()
{
    // A variável gl_Position define a posição final de cada vértice
    // OBRIGATORIAMENTE em "normalized device coordinates" (NDC), onde cada
    // coeficiente estará entre -1 e 1 após divisão por w.
    // Veja {+NDC2+}.
    //
    // O código em "main.cpp" define os vértices dos modelos em coordenadas
    // locais de cada modelo (array model_coefficients). Abaixo, utilizamos
    // operações de modelagem, definição da câmera, e projeção, para computar
    // as coordenadas finais em NDC (variável gl_Position). Após a execução
    // deste Vertex Shader, a placa de vídeo (GPU) fará a divisão por W. Veja
    // slides 41-67 e 69-86 do documento Aula_09_Projecoes.pdf.

    gl_Position = projection * view * model * model_coefficients;

    // Como as variáveis acima  (tipo vec4) são vetores com 4 coeficientes,
    // também é possível acessar e modificar cada coeficiente de maneira
    // independente. Esses são indexados pelos nomes x, y, z, e w (nessa
    // ordem, isto é, 'x' é o primeiro coeficiente, 'y' é o segundo, ...):
    //
    //     gl_Position.x = model_coefficients.x;
    //     gl_Position.y = model_coefficients.y;
    //     gl_Position.z = model_coefficients.z;
    //     gl_Position.w = model_coefficients.w;
    //

    // Agora definimos outros atributos dos vértices que serão interpolados pelo
    // rasterizador para gerar atributos únicos para cada fragmento gerado.

    // Posição do vértice atual no sistema de coordenadas global (World).
    position_world = model * model_coefficients;

    // Posição do vértice atual no sistema de coordenadas local do modelo.
    position_model = model_coefficients;

    // Normal do vértice atual no sistema de coordenadas global (World).
    // Veja slides 123-151 do documento Aula_07_Transformacoes_Geometricas_3D.pdf.
    normal = inverse(transpose(model)) * normal_coefficients;
    normal.w = 0.0;

    // Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
    texcoords = texture_coefficients;

    // Inicializar cor_v em -1 pois isso vai determinar se será usado Gouraud Shading ou Phong Shading
    // no fragment_shader
    cor_v = vec4(-1.0, -1.0, -1.0, 0.0);
    //Cálculo do Gouraud Shading
    if(object_id == SPHERE) {
        // Obtemos a posição da câmera utilizando a inversa da matriz que define o
        // sistema de coordenadas da câmera.
        vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
        vec4 camera_position = inverse(view) * origin;

        // O fragmento atual é coberto por um ponto que percente à superfície de um
        // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
        // sistema de coordenadas global (World coordinates). Esta posição é obtida
        // através da interpolação, feita pelo rasterizador, da posição de cada
        // vértice.
        vec4 p = position_world;

        // Normal do fragmento atual, interpolada pelo rasterizador a partir das
        // normais de cada vértice.
        vec4 n = normalize(normal);

        vec4 light_source_position = vec4(0.0, 2.0, 1.0, 1.0);
        vec4 light_direction = normalize(vec4(0.0, 1.0, 0.0, 0.0));

        float alpha = radians(30.0);

        float intensity = 1;

        // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
        vec4 l = normalize(light_source_position-p);

        // Vetor que define o sentido da câmera em relação ao ponto atual.
        vec4 v = normalize(camera_position - p);

        // Vetor que define o sentido da reflexão especular ideal.
        vec4 r = -l + 2*n*(dot(n, l));// PREENCHA AQUI o vetor de reflexão especular ideal

        // 2. Calcular Coordenadas UV da Esfera (EXATAMENTE como no fragment shader)
        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;
        float raio = 1.0;
        vec4 pc = normalize(position_model - bbox_center);
        vec4 pl = bbox_center + raio*pc;
        vec4 vecp = pl - bbox_center;
        float theta = atan(pl[0], pl[2]);
        float phi = asin(pl[1]/raio);

        float U = (theta + M_PI) / (2.0*M_PI);
        float V = (phi + M_PI/2.0) / M_PI;

        // 3. Obter materiais da Esfera (Kd da textura)
        vec3 Kd = texture(TextureImage0, vec2(U,V)).rgb;

        /*
        // Equação de Iluminação
        float lambert = max(0,dot(n,l));

        cor_v.rgb = Kd * (lambert + 0.01);
        */
        //Kd = vec3(0.8f, 0.4f, 0.08f);

        vec3 Ks = vec3(0.1f,0.1f,0.1f);
        vec3 Ka = Kd/2;
        float q = 1.0;

        vec3 I = vec3(1.0f,1.0f,1.0f); // PREENCH AQUI o espectro da fonte de luz

        // Espectro da luz ambiente
        vec3 Ia = vec3(0.2f, 0.2f, 0.2f); // PREENCHA AQUI o espectro da luz ambiente

        // Termo difuso utilizando a lei dos cossenos de Lambert
        vec3 lambert_diffuse_term = Kd*I*max(0, dot(n, l)); // PREENCHA AQUI o termo difuso de Lambert

        // Termo ambiente
        vec3 ambient_term = Ka*Ia; // PREENCHA AQUI o termo ambiente

        // Termo especular utilizando o modelo de iluminação de Phong
        vec3 phong_specular_term  = Ks*I*max(0, pow(dot(r, v), q)); // PREENCH AQUI o termo especular de Phong

        // NOTE: Se você quiser fazer o rendering de objetos transparentes, é
        // necessário:
        // 1) Habilitar a operação de "blending" de OpenGL logo antes de realizar o
        //    desenho dos objetos transparentes, com os comandos abaixo no código C++:
        //      glEnable(GL_BLEND);
        //      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // 2) Realizar o desenho de todos objetos transparentes *após* ter desenhado
        //    todos os objetos opacos; e
        // 3) Realizar o desenho de objetos transparentes ordenados de acordo com
        //    suas distâncias para a câmera (desenhando primeiro objetos
        //    transparentes que estão mais longe da câmera).
        // Alpha default = 1 = 100% opaco = 0% transparente
        cor_v.a = 1;

        // Cor final do fragmento calculada com uma combinação dos termos difuso,
        // especular, e ambiente. Veja slide 48 do documento Aula_17_e_18_Modelos_de_Iluminacao - RESUMO.pdf e slide 129 do documento Aula_17_e_18_Modelos_de_Iluminacao.pdf.
        cor_v.rgb = lambert_diffuse_term + ambient_term + phong_specular_term;

        // Cor final com correção gamma, considerando monitor sRGB.
        // Veja https://en.wikipedia.org/w/index.php?title=Gamma_correction&oldid=751281772#Windows.2C_Mac.2C_sRGB_and_TV.2Fvideo_standard_gammas
        //cor_v.rgb = pow(cor_v.rgb, vec3(1.0,1.0,1.0)/2.2);
    }

}

