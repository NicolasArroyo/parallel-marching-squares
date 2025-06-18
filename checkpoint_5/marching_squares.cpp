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

// Struct para puntos en 2D
struct Point
{
    float x, y;
};

// Struct para segmentos de línea
// Consiste de 2 puntos en 2D
struct LineSegment
{
    Point start, end;
};

// Se usa solo para el linear interpolation
float EPS = 1e-6f;

// Usamos linear interpolation
// Calcula en qué parte del borde entre dos puntos cae el isovalue
Point lerp(Point p1, Point p2, float v1, float v2, float iso)
{
    float denom = v2 - v1;

    // Por seguridad, en caso no haya diferencia grande entre los valores de ambos puntos
    // Ayuda a que no creemos bordes demasiado pequeños
    if (std::fabs(denom) < EPS)
        return p1;

    // Acá sigue el cálculo normal de una linear interpolation
    float t = (iso - v1) / denom;

    return {p1.x + t * (p2.x - p1.x),
            p1.y + t * (p2.y - p1.y)};
}

// Usado en conjunto a edgePairs
// Se encarga de enumerar los bordes
// TOP -> RIGHT -> BOTTOM -> LEFT
int edgeCorners[4][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}};

// Usado para los posibles casos de segmentos
// Recordemos que se generará un número de 4 bits dependiendo de la combinación de values
// Ese número binario representa la combinación de segmentos de línea que habrá
// Está dividido en parejas:
//  - Como mínimo 0 segmentos (Caso 0 y 15)
//  - En promedio 1 segmento (Caso 1, 2, 3, 4, 6, 7, 8, 9, 11, 12, 13 y 14)
//  - Como máximo 2 segmentos (Caso 5 y 10)
int edgePairs[16][4] = {
    {-1, -1, -1, -1}, // 0   0000
    {3, 0, -1, -1},   // 1   0001
    {0, 1, -1, -1},   // 2   0010
    {3, 1, -1, -1},   // 3   0011
    {1, 2, -1, -1},   // 4   0100
    {0, 1, 3, 2},     // 5   0101
    {0, 2, -1, -1},   // 6   0110
    {3, 2, -1, -1},   // 7   0111
    {2, 3, -1, -1},   // 8   1000
    {0, 2, -1, -1},   // 9   1001
    {0, 3, 1, 2},     // 10  1010
    {1, 2, -1, -1},   // 11  1011
    {3, 1, -1, -1},   // 12  1100
    {0, 1, -1, -1},   // 13  1101
    {3, 0, -1, -1},   // 14  1110
    {-1, -1, -1, -1}  // 15  1111
};

// Función principal
// Calcula los segmentos de línea para una casilla 2x2 (un square)
// Ahora no devuelve nada, recibe el vector como referencia
void marchSquare(float cell_x, float cell_y,
                 float values[4],
                 float isolevel,
                 std::vector<LineSegment>& outSegments)
{
    int caseIdx = 0;

    if (values[0] >= isolevel) caseIdx |= 1;
    if (values[1] >= isolevel) caseIdx |= 2;
    if (values[2] >= isolevel) caseIdx |= 4;
    if (values[3] >= isolevel) caseIdx |= 8;


    if (caseIdx == 0 || caseIdx == 15)
        return;

    // Ahora construimos el vector de esquinas aquí
    Point corners[4] = {
        {cell_x, cell_y},
        {cell_x + 1, cell_y},
        {cell_x + 1, cell_y + 1},
        {cell_x, cell_y + 1}
    };
    
    auto getEdgePoint = [&](int e) -> Point
    {
        int c0 = edgeCorners[e][0], c1 = edgeCorners[e][1];
        return lerp(corners[c0], corners[c1],
                    values[c0], values[c1],
                    isolevel);
    };

    int *pair = edgePairs[caseIdx];

    for (int i = 0; i < 4 && pair[i] != -1; i += 2)
    {
        outSegments.push_back({getEdgePoint(pair[i]), getEdgePoint(pair[i + 1])});
    }
}

int main(int argc, char *argv[])
{
    int gridResolution = 100;

    // Primer argumento será qué tamaño será nuestra grid
    if (argc > 1)
        gridResolution = std::stoi(argv[1]);

    const int gridWidth = gridResolution;
    const int gridHeight = gridResolution;
    const std::string outputFilename = "lines.csv";

    const float isolevel = 0.5f;

    std::cout << "\nResolución de la malla: " << gridWidth << "x" << gridHeight << std::endl;

    // Ahora ya no es una matriz (vector de vectores)
    // Sino, lo hemos linealizado
    // Se cambia un poco cómo accedemos a las coordenadas, pero no es un gran cambio
    std::vector<float> scalarField(gridWidth * gridHeight);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Ahora nuestra malla está compuesta por 0s y 1s
    // Tomamos esta decisión para que no hayan muchas celdas vacías y estresemos más los recursos
    for (int y = 0; y < gridHeight; ++y)
    {
        for (int x = 0; x < gridWidth; ++x)
        {
            scalarField[y * gridWidth + x] = std::rand() % 2;
        }
    }

    std::vector<LineSegment> allSegments;
    double startTime = omp_get_wtime();

    #pragma omp parallel
    {
        std::vector<LineSegment> privateSegments;

        #pragma omp for nowait
        for (int y = 0; y < gridHeight - 1; ++y)
        {
            // Ahora aplicamos un sliding window
            // Esto debería reducir el acceso a memoria en un 50%
            float left_top_val = scalarField[y * gridWidth];
            float left_bottom_val = scalarField[(y + 1) * gridWidth];

            for (int x = 0; x < gridWidth - 1; ++x)
            {
                float right_top_val = scalarField[y * gridWidth + (x + 1)];
                float right_bottom_val = scalarField[(y + 1) * gridWidth + (x + 1)];

                float values[4] = {
                    left_top_val,
                    right_top_val,
                    right_bottom_val,
                    left_bottom_val
                };

                marchSquare((float)x, (float)y, values, isolevel, privateSegments);

                left_top_val = right_top_val;
                left_bottom_val = right_bottom_val;
            }
        }

        #pragma omp critical
        allSegments.insert(allSegments.end(), privateSegments.begin(), privateSegments.end());
    }

    double endTime = omp_get_wtime();
    double elapsedTimeMs = (endTime - startTime) * 1000.0;

    std::cout << "Marching squares tomó " << elapsedTimeMs << " ms." << std::endl;
    std::cout << "Se generó " << allSegments.size() << " segmentos de línea." << std::endl;

    return 0;
}