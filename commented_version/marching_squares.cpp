#include <string>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cmath>
#include <array>
#include <iostream>
#include <chrono>
#include <fstream>
#include <omp.h>

/*
    PUNTO EN UN ESPACIO 2D
    
    - USADO COMO BASE EN EL STRUCT LINESEGMENT
*/
struct Point
{
    float x, y;
};

/*
    SEGMENTO DE LÍNEA EN UN ESPACIO 2D

    - USADO PARA REPRESENTAR LAS LÍNEAS QUE FORMARÁN LA
      ISOCURVA RESULTANTE LUEGO DE APLICAR EL ALGORITMO
*/
struct LineSegment
{
    Point start, end;
};

/*
    FUNCIÓN PARA CALCULAR UNA INTERPOLACIÓÑ LINEAR

    - SI SABEMOS QUE UN SEGMENTO DE LA ISOCURVA RESULTANTE PASA EN LA
      ARISTA ENTRE DOS ISOVALORES, UNA SOLUCIÓN SIMPLE SERÍA DECIR QUE
      ATRAVIESA EN EL MEDIO DE ESTOS

    - SIN EMBARGO, UNA MEJOR APROXIMACIÓN PODRÍA REALIZARSE CON UNA
      INTERPOLACIÓN LINEAR, PUES SABEMOS EL VALOR DE AMBOS VÉRTICES
      Y EL VALOR DETERMINADO COMO ISOVALOR

    - EL EPSILON NOS PERMITE DESCARTAR LOS CASOS EN DONDE LA DIFERENCIA
      ENTRE LOS ISOVALORES DE LOS DOS VÉRTICES ES MUY PEQUEÑA
*/
float EPS = 1e-6f;
Point lerp(Point p1, Point p2, float v1, float v2, float iso)
{
    float denom = v2 - v1;

    if (std::fabs(denom) < EPS)
        return p1;

    float t = (iso - v1) / denom;

    return {p1.x + t * (p2.x - p1.x),
            p1.y + t * (p2.y - p1.y)};
}

/*
    ARRAY 2D PARA ENUMERAR LOS BORDES DE UN CUADRADO

    - EXISTEN 4 BORDES
    - EXISTEN 2 VÉRTICES POR BORDE
    - RECORDEMOS QUE NO TODOS LOS BORDES
      TRABAJAN CON LOS MISMOS VÉRTICES
*/
int edgeCorners[4][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}};

/*
    ARRAY 2D PARA ENUMERAR LOS POSIBLES CASOS DE SEGMENTOS

    - RECORDEMOS QUE LA COMBINACIÓN GENERADA AL COMPARAR LOS
      ISOVALORES EN LOS 4 VÉRTICES DE UN CUADRADO CON EL
      ISOVALOR UMBRAL GENERA UN NÚMERO DE 4 BITS

    - EXISTEN 16 POSIBLES COMBINACIONES (2^4 = 16)
    - EN CADA CASO, PUEDEN HABER HASTA 2 SEGMENTOS QUE
      PASAN POR EL CUADRADO, RESULTANDO EN 4 VÉRTICES

    - SEGÚN EL NÚMERO DE 4 BITS VAMOS A UN ITEM ESPECÍFICO
      DE NUESTRO ARRAY 2D Y SABREMOS POR QUÉ ARISTAS PASARÁ
      NUESTRA ISOCURVA RESULTANTE

    - HAY 3 CASOS POSIBLES:
        - {-1, -1, -1, -1} : NO ES NECESARIO NINGÚN SEGMENTO DE LÍNEA
        - { A,  B, -1, -1} : SOLO ES NECESARIO UN SEGMENTO DE LÍNEA
        - { A,  B,  C,  D} : SON NECESARIOS DOS SEGMENTOS DE LÍNEA
*/
int edgePairs[16][4] = {
    {-1, -1, -1, -1},
    {3, 0, -1, -1},  
    {0, 1, -1, -1},  
    {3, 1, -1, -1},  
    {1, 2, -1, -1},  
    {0, 1, 3, 2},    
    {0, 2, -1, -1},  
    {3, 2, -1, -1},   
    {2, 3, -1, -1},   
    {0, 2, -1, -1},   
    {0, 3, 1, 2},     
    {1, 2, -1, -1},   
    {3, 1, -1, -1},   
    {0, 1, -1, -1},   
    {3, 0, -1, -1},   
    {-1, -1, -1, -1}  
};

/*
    FUNCIÓN PRINCIPAL DEL ALGORITMO

    - CALCULA LOS SEGMENTOS DE LÍNEA QUE DEBEN SER GENERADOS
      PARA UNA CASILLA EN ESPECÍFICO (CASILLA == SQUARE)

    - RECIBE LA POSICIÓN (X, Y) DE UNA CASILLA
        - PARA EL CASO QUE ESTAMOS MANEJANDO, ESTO ES SUFICIENTE,
          PUES ASUMIMOS QUE SIEMPRE HAY UNA DISTANCIA DE 1 UNIDAD
          ENTRE ISOVALORES VECINOS EN LA ISOMALLA

    - RECIBE LOS 4 ISOVALORES DE LAS 4 ESQUINAS DE LA CASILLA

    - RECIBE EL ISOVALOR QUE SERVIRÁ DE UMBRAL (ISOLEVEL)

    - RECIBE LA REFERENCIA A UN VECTOR DONDE COLOCARÁ LOS SEGMENTOS
      DE LÍNEA GENERADOS, PARA LUEGO SER JUNTADOS EN LA ISOCURVA FINAL
*/
void marchSquare(float cell_x, float cell_y,
                 float values[4],
                 float isolevel,
                 std::vector<LineSegment>& outSegments)
{

    int caseIdx = 0;

    // SE CALCULA EL CASO EN PARTICULAR
    // COMPARAMOS LOS ISOVALORES CON EL UMBRAL
    if (values[0] >= isolevel) caseIdx |= 1;
    if (values[1] >= isolevel) caseIdx |= 2;
    if (values[2] >= isolevel) caseIdx |= 4;
    if (values[3] >= isolevel) caseIdx |= 8;


    // UN EARLY RETURN
    // SI ES EL PRIMER O ÚLTIMO CASO, RETORNAMOS
    // ESTOS DOS CASOS NO GENERAN SEGMENTOS DE LÍNEA
    if (caseIdx == 0 || caseIdx == 15)
        return;

    // CALCULAMOS LAS POSICIONES DE TODAS LAS ESQUINAS
    // RECORDEMOS QUE ASUMIMOS UNA DISTANCIA DE 1 ENTRE VECINOS
    Point corners[4] = {
        {cell_x, cell_y},
        {cell_x + 1, cell_y},
        {cell_x + 1, cell_y + 1},
        {cell_x, cell_y + 1}
    };
    
    // FUNCIÓN PARA OBTENER LA POSICIÓN POR DONDE ATRAVIESA LA ISOCURVA A UNA ARISTA
    auto getEdgePoint = [&](int e) -> Point
    {
        int c0 = edgeCorners[e][0], c1 = edgeCorners[e][1];
        return lerp(corners[c0], corners[c1],
                    values[c0], values[c1],
                    isolevel);
    };

    // OBTENEMOS LAS ARISTAS POR DONDE PASARÁ LA ISOCURVA
    // USAMOS EL ARREGLO EDGEPAIRS CON EL CASO ENCONTRADO
    int *pair = edgePairs[caseIdx];

    // POR CADA POSIBLE SEGMENTO DE LÍNEA, CALCULAMOS SUS PUNTOS DE CRUCE
    // SE USA UN FOR LOOP PORQUE PUEDEN LLEGAR A ENCONTRASE 2 SEGMENTOS
    // EL LOOP PARARÁ SI NO EXISTE EL SEGUNDO CASO
    for (int i = 0; i < 4 && pair[i] != -1; i += 2)
    {
        outSegments.push_back({getEdgePoint(pair[i]), getEdgePoint(pair[i + 1])});
    }
}

/*
    FUNCIÓN PRINCIPAL DEL PROGRAMA

    - CREA LA MALLA DE ISOVALORES
    - LLAMA A MARCHSQUARE POR CADA UNA DE LAS CASILLAS DISPONIBLES
    - OBTENEMOS TODOS LOS SEGMENTOS DE LÍNEA DE LA ISOCURVA RESULTANTE
*/
int main(int argc, char *argv[])
{
    int gridResolution = 100;

    // EL PRIMER ARGUMENTO SERÁ LA DIMENSIÓN DE LA MALLA
    // DIMENSIÓN: ARGV[1] X ARGV[1]
    if (argc > 1)
        gridResolution = std::stoi(argv[1]);

    /*
        NOTA: DECIMOS QUE LA MALLA ES CUADRADA, PERO REALMENTE
        LA HEMOS APLANADO A 1 DIMENSIÓN, Y ACCEDEMOS A SUS
        CASILLAS CON UN OFFSET
    */

    const int gridWidth = gridResolution;
    const int gridHeight = gridResolution;

    // EL ISOVALOR DE UMBRAL ESTÁ DEFINIDO COMO 0.5
    const float isolevel = 0.5f;

    std::cout << "\nResolución de la malla: " << gridWidth << "x" << gridHeight << std::endl;

    std::vector<float> scalarField(gridWidth * gridHeight);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // SE CREA UNA MALLA CON LAS DIMENSIONES DADAS EN EL ARGUMENTO
    // LA MALLA ESTÁ COMPUESTA POR 0S Y 1S
    for (int y = 0; y < gridHeight; ++y)
    {
        for (int x = 0; x < gridWidth; ++x)
        {
            scalarField[y * gridWidth + x] = std::rand() % 2;
        }
    }

    /*
        ¿POR QUÉ ESTAMOS DECIDIENDO QUE EL 
        ISOVALOR DE UMBRAL ESTÁ DEFINIDO COMO 0.5?

        DEPENDIENDO DE LA MALLA DE ISOVALORES QUE USEMOS COMO INPUT, EL
        ALGORITMO HARÁ MÁS O MENOS TRABAJO. POR EJEMPLO, SI LE DAMOS UNA
        MALLA EN DONDE VARIAS CASILLAS CAEN EN EL PRIMER O ÚLTIMO CASO,
        ENTONCES EL ALGORITMO RETORNARÁ RÁPIDAMENTE DE VARIAS ITERACIONES
        PUES ESTAS NO ESTÁN GENERANDO SEGMENTOS DE LÍNEA.

        SIN EMBARGO, SI LE DAMOS UNA MALLA BINARIA Y UN ISOVALOR AL MEDIO
        DE AMBOS POSIBLES VALORES, ESTARÍAMOS ACERCÁNDONOS AL MÁXIMO DEL
        TRABAJO QUE PODRÍA REALIZAR EL ALGORITMO, PUES CADA UNA DE LAS
        CASILLAS GENERARÍA EN PROMEDIO 1 SEGMENTO DE LÍNEA, A LA VEZ QUE
        MINIMIZAMOS LAS CASILLAS EN DONDE NO SE GENERA SEGMENTO ALGUNO.
    */

    // VECTOR DONDE SE GUARDARÁN TODOS LOS SEGMENTOS DE LÍNEA RESULTANTE
    std::vector<LineSegment> allSegments;

    double startTime = omp_get_wtime();

    // INICIO DE LA SECCIÓN PARALELA
    #pragma omp parallel
    {
        // CADA THREAD DEFINIRÁ SU PROPIO VECTOR PARA SUS SEGMENTOS GENERADOS
        std::vector<LineSegment> privateSegments;

        // SE PARALELIZA EL FOR QUE ITERA LAS POSIBLES CASILLAS EN LA MALLA
        #pragma omp for nowait
        for (int y = 0; y < gridHeight - 1; ++y)
        {
            // PRIMERO OBTENEMOS LOS VALORES EL LADO IZQUIERDO DE LA CASILLA
            float left_top_val = scalarField[y * gridWidth];
            float left_bottom_val = scalarField[(y + 1) * gridWidth];

            for (int x = 0; x < gridWidth - 1; ++x)
            {
                // LUEGO OBTENEMOS LOS VALORES DEL LADO DERECHO DE LA CASILLA
                float right_top_val = scalarField[y * gridWidth + (x + 1)];
                float right_bottom_val = scalarField[(y + 1) * gridWidth + (x + 1)];

                float values[4] = {
                    left_top_val,
                    right_top_val,
                    right_bottom_val,
                    left_bottom_val
                };

                // LLAMAMOS A MARCHSQUARE AHORA QUE TENEMOS TODOS LOS DATOS
                marchSquare((float)x, (float)y, values, isolevel, privateSegments);

                /*
                    PASANDO A LA SIGUIENTE CASILLA, AHORA LOS VALORES
                    QUE ESTABAN EN LA DERECHA PASARÁN A LA IZQUIERDA.
                
                    ESTA ESTRATEGIA DE SLIDING WINDOW PERMITE REDUCIR EL NÚMERO DE ACCESOS
                    A A LA MALLA ORIGINAL EN LA MITAD. EN LUGAR DE ACCEDER CUATRO VECES POR
                    ITERACIÓN, AHORA SOLO SE ACCEDE DOS VECES.
                */
                left_top_val = right_top_val;
                left_bottom_val = right_bottom_val;
            }
        }

        // AHORA, SE NECESITA UNA SECCIÓN CRÍTICA PARA AÑADIR LOS SEGMENTOS PRIVADOS AL GLOBAL
        #pragma omp critical
        allSegments.insert(allSegments.end(), privateSegments.begin(), privateSegments.end());
    }

    // TERMINAMOS DE MEDIR EL TIEMPO E IMPRIMIMOS RESULTADOS
    double endTime = omp_get_wtime();
    double elapsedTimeMs = (endTime - startTime) * 1000.0;

    std::cout << "Marching squares tomó " << elapsedTimeMs << " ms." << std::endl;
    std::cout << "Se generó " << allSegments.size() << " segmentos de línea." << std::endl;

    return 0;
}