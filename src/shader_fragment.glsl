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
#define SPHERE1 1
#define PLANE  2
#define SPHERE2 3
#define BARRIER  4

#define STRAIGHT 5
#define RAMP 6
#define TURN 7
#define SPHEREBEZIER 8

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
uniform sampler2D TextureImage7;
uniform sampler2D TextureImage8;
uniform sampler2D TextureImage9;

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


    if (object_id == STRAIGHT || object_id == TURN)
    {

        // O plane.obj não tem texcoords. Geramos proceduralmente.
        // Usamos position_model.x e position_model.z como UVs.
        // Multiplicamos por 50.0 para repetir a textura (tiling).
        //vec2 plane_uv = vec2(position_model.x, position_model.z) * 100.0;
        //vec3 texColor = texture(TextureImage2, plane_uv).rgb;
        vec3 texColor = texture(TextureImage2, (texcoords * 5.0f)).rgb ; //diminui o * 25.0 do texcoords, isso diminuia o tamanho das imagens e fazia precisar repetir muitas mais vezes
        // Coeficientes espectrais derivados da textura
        Kd = texColor;                  // Difusa baseada na textura
        Ka = texColor * 0.3;            // Ambiente mais fraco
        Ks = vec3(0.1, 0.1, 0.1);       // Asfalto não brilha muito
        q  = 2.0;                      // Brilho bem suave
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
    else if (object_id == PLANE) {
        vec2 plane_uv = vec2(position_model.x, position_model.z) * 2000.0;
        vec3 texColor = texture(TextureImage9, plane_uv).rgb;

        Kd = texColor;                  // Difusa baseada na textura
        Ka = texColor * 0.3;            // Ambiente mais fraco
        Ks = vec3(0.1, 0.1, 0.1);       // Asfalto não brilha muito
        q  = 2.0;                      // Brilho bem suave
    }

    
    if (object_id == SPHERE1 || object_id == SPHERE2 || object_id == SPHEREBEZIER){

        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;

        float raio = 1.0;

        vec4 pc = normalize(position_model - bbox_center);

        vec4 pl = bbox_center + raio*pc;

        vec4 vecp = pl - bbox_center;

        float theta = atan(pl[0], pl[2]);
        float phi = asin(pl[1]/raio);

        U = (theta + M_PI) / (2.0*M_PI);
        V = (phi + M_PI/2.0) / M_PI;

        vec2 sphereUV = vec2(U, V);

        vec3 Kd0;
        float ao;
        vec3 texColor;

        if (object_id == SPHERE1 || object_id == SPHEREBEZIER){

            Kd0 = texture(TextureImage0, sphereUV).rgb;
            ao = texture(TextureImage1, sphereUV).r;

            texColor = texture(TextureImage1, (texcoords* 25.0)).rgb ;
        }
        else {
            Kd0 = texture(TextureImage7, sphereUV).rgb;
            ao = texture(TextureImage8, sphereUV).r;

            texColor = texture(TextureImage7, (texcoords* 25.0)).rgb ;
        }


        float lambert = max(0, dot(n,l));

        Ka = texColor * 0.3;            // Ambiente mais fraco
        vec3 Ia = vec3(0.2, 0.2, 0.2);

        vec3 ambient_term = Ka * Ia;

        color.rgb = Kd0 * (lambert * ao + 0.1 * ao) + ambient_term;
    }
    else if (object_id == BARRIER){

    // Simply output the interpolated color
    color.rgb = cor_v.rgb;
    color.a = 1.0;
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