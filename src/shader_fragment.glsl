#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define CAR  0
#define PLANE  2
#define SPHERE 1
#define FUSCA  4
uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;
uniform sampler2D TextureImage3;
uniform sampler2D TextureImage4;
uniform sampler2D TextureImage5;
uniform sampler2D TextureImage6;

// Novo atributo in para Gouraud Shading da esfera
in vec4 cor_v;
in vec2 sphere_texcoords;

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void main()
{
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

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;

    // Vetor que define o sentido da reflexão especular ideal.
    vec4 r = -l + 2*n*(dot(n, l));// PREENCHA AQUI o vetor de reflexão especular ideal

    // Parâmetros que definem as propriedades espectrais da superfície
    vec3 Kd; // Refletância difusa
    vec3 Ks; // Refletância especular
    vec3 Ka; // Refletância ambiente
    float q; // Expoente especular para o modelo de iluminação de Phong


    if ( object_id == PLANE )
    {

        // O plane.obj não tem texcoords. Geramos proceduralmente.
        // Usamos position_model.x e position_model.z como UVs.
        // Multiplicamos por 50.0 para repetir a textura (tiling).
        vec2 plane_uv = vec2(position_model.x, position_model.z) * 100.0;
        vec3 texColor = texture(TextureImage2, plane_uv).rgb;

        // Coeficientes espectrais derivados da textura
        Kd = texColor;                  // Difusa baseada na textura
        Ka = texColor * 0.3;            // Ambiente mais fraco
        Ks = vec3(0.1, 0.1, 0.1);       // Asfalto não brilha muito
        q  = 2.0;                      // Brilho bem suave
    }
    else if ( object_id == FUSCA)
    {

        // PREENCHA AQUI as coordenadas de textura do coelho, computadas com
        // projeção planar XY em COORDENADAS DO MODELO. Utilize como referência
        // o slides 99-104 do documento Aula_20_Mapeamento_de_Texturas.pdf,
        // e também use as variáveis min*/max* definidas abaixo para normalizar
        // as coordenadas de textura U e V dentro do intervalo [0,1]. Para
        // tanto, veja por exemplo o mapeamento da variável 'p_v' utilizando
        // 'h' no slides 158-160 do documento Aula_20_Mapeamento_de_Texturas.pdf.
        // Veja também a Questão 4 do Questionário 4 no Moodle.

        float minx = bbox_min.x;
        float maxx = bbox_max.x;

        float miny = bbox_min.y;
        float maxy = bbox_max.y;

        float minz = bbox_min.z;
        float maxz = bbox_max.z;

        //Mudançcas que não foi possível testar no lab

        U = position_model[0]/(maxx-minx);
        V = position_model[1]/(maxy-miny);
    }
    else if ( object_id == CAR )
    {
        vec3 texColor = texture(TextureImage3, texcoords).rgb;

        // Coeficientes espectrais derivados da textura
        Kd = texColor;                  // Difusa baseada na textura
        Ka = texColor * 0.3;            // Ambiente mais fraco
        Ks = vec3(0.3, 0.3, 0.3);       // Reflexão especular leve
        q  = 64.0;                      // Brilho moderado
    }

    // Esfera utiliza Gouraud Shading
    if (object_id == SPHERE){
        // Obtemos a refletância difusa a partir da leitura da imagem TextureImage0
        vec3 Kd0 = texture(TextureImage0, sphere_texcoords).rgb;
        color.rgb = Kd0 * cor_v.rgb;
    }
    else if (object_id == FUSCA){
        vec3 Kd0 = texture(TextureImage3, vec2(U, V)).rgb; // difusa
        vec3 Ks0 = texture(TextureImage4, vec2(U, V)).rgb; // especular
        float gloss = texture(TextureImage5, vec2(U, V)).r; // gloss

        float lambert = max(dot(n, l), 0.0);
        vec3 diffuse = Kd0 * lambert;
        vec3 specular = Ks0 * pow(max(dot(r, v), 0.0), gloss * 64.0);
        vec3 ambient = Kd0 * 0.2;

        color.rgb = diffuse + specular + ambient;
    }
    else{
        // Espectro da fonte de iluminação
        vec3 I = vec3(1.0,1.0,1.0); // PREENCH AQUI o espectro da fonte de luz

        // Espectro da luz ambiente
        vec3 Ia = vec3(0.2,0.2,0.2); // PREENCHA AQUI o espectro da luz ambiente

        // Termo difuso utilizando a lei dos cossenos de Lambert
        vec3 lambert_diffuse_term = Kd*I*max(0, dot(n, l)); // PREENCHA AQUI o termo difuso de Lambert

        // Termo ambiente
        vec3 ambient_term = Ka*Ia; // PREENCHA AQUI o termo ambiente

        // Termo especular utilizando o modelo de iluminação de Phong
        vec3 phong_specular_term  = Ks*I*max(0, pow(dot(r, v), q)); // PREENCH AQUI o termo especular de Phong

        color.rgb = lambert_diffuse_term + phong_specular_term + ambient_term;
    }

    color.a = 1;
    color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
}