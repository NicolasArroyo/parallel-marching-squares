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
std::vector<LineSegment> marchSquare(Point corners[4],
                                     float values[4],
                                     float isolevel)
{
    // Calculamos la representación en bits del caso actual
    int caseIdx = 0;
    for (int i = 0; i < 4; ++i)
        if (values[i] >= isolevel)
            caseIdx |= (1 << i);

    // Si son los casos con 0 segmentos, no tenemos que hacer nada
    if (caseIdx == 0 || caseIdx == 15)
        return {};

    Point edgePt[4];

    // Función lambda para calcular por dónde en los bordes atraviesa nuestro segmento
    auto getEdgePoint = [&](int e) -> Point
    {
        int c0 = edgeCorners[e][0], c1 = edgeCorners[e][1];
        return lerp(corners[c0], corners[c1],
                    values[c0], values[c1],
                    isolevel);
    };

    std::vector<LineSegment> segs;
    int *pair = edgePairs[caseIdx];

    // Iteramos los pares de bordes por donde pasará el segmento
    // Como mencionamos, mejor caso: 1 segmento, peor caso: 2 segemtos
    for (int i = 0; i < 4 && pair[i] != -1; i += 2)
    {
        int eA = pair[i], eB = pair[i + 1];

        // Agregamos los dos puntos, el inicial y final del segmento
        segs.push_back({getEdgePoint(eA), getEdgePoint(eB)});
    }
    return segs;
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

    std::vector<std::vector<float>> scalarField(gridHeight, std::vector<float>(gridWidth));

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Ahora nuestra malla está compuesta por 0s y 1s
    // Tomamos esta decisión para que no hayan muchas celdas vacías y estresemos más los recursos
    for (int y = 0; y < gridHeight; ++y)
    {
        for (int x = 0; x < gridWidth; ++x)
        {
            scalarField[y][x] = std::rand() % 2;
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
            for (int x = 0; x < gridWidth - 1; ++x)
            {
                Point corners[4] = {
                    {(float)x, (float)y},
                    {(float)x + 1, (float)y},
                    {(float)x + 1, (float)y + 1},
                    {(float)x, (float)y + 1}};

                float values[4] = {
                    scalarField[y][x], scalarField[y][x + 1],
                    scalarField[y + 1][x + 1], scalarField[y + 1][x]};
                
                std::vector<LineSegment> cellSegments = marchSquare(corners, values, isolevel);
                
                // A diferencia de la forma secuencial, ya no insertamos los segmentos de frente en el vector global
                // Sino, los colocamos en un vector local del thread llamado privateSegments
                // NOTA: Sí podríamos insertarlos en el vector global usando un critical section.
                //       Sin embargo, sería ineficiente con la gran cantidad de segmentos que manejamos.
                //       Por eso, solo haremos el insert en el vector global cuando este thread acabe su fila.
                if (!cellSegments.empty())
                {
                    privateSegments.insert(privateSegments.end(), cellSegments.begin(), cellSegments.end());
                }
            }
        }

        // Ahora que el thread terminó, agregará sus segmentos privados al vector global de segmentos
        #pragma omp critical
        allSegments.insert(allSegments.end(), privateSegments.begin(), privateSegments.end());
    }

    double endTime = omp_get_wtime();
    double elapsedTimeMs = (endTime - startTime) * 1000.0;

    std::cout << "Marching squares tomó " << elapsedTimeMs << " ms." << std::endl;
    std::cout << "Se generó " << allSegments.size() << " segmentos de línea." << std::endl;

    std::ofstream outputFile(outputFilename);

    outputFile << "start_x,start_y,end_x,end_y\n";
    for (const auto &segment : allSegments)
    {
        outputFile << segment.start.x << "," << segment.start.y << ","
                   << segment.end.x << "," << segment.end.y << "\n";
    }

    outputFile.close();
    std::cout << "Se escribieron los segmentos correctamente en " << outputFilename << std::endl;

    return 0;
}