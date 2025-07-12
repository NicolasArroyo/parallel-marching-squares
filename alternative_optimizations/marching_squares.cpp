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

struct Point
{
    float x, y;
};

struct LineSegment
{
    Point start, end;
};

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

int edgeCorners[4][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0}};

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

    if (argc > 1)
        gridResolution = std::stoi(argv[1]);


    const int gridWidth = gridResolution;
    const int gridHeight = gridResolution;

    const float isolevel = 0.5f;

    std::vector<float> scalarField(gridWidth * gridHeight);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (int y = 0; y < gridHeight; ++y)
    {
        for (int x = 0; x < gridWidth; ++x)
        {
            scalarField[y * gridWidth + x] = std::rand() % 2;
        }
    }

    for (int i = 0; i < 10; ++i) 
    {
        int numThreads = omp_get_max_threads();
        std::vector<std::vector<LineSegment>> threadSegments(numThreads);

        double startTime = omp_get_wtime();

        #pragma omp parallel
        {
            int tid = omp_get_thread_num();

            auto &mySegs = threadSegments[tid];
            
            std::size_t totalCells   = std::size_t(gridHeight - 1) * (gridWidth - 1);
            std::size_t maxPerThread = (totalCells + numThreads - 1) / numThreads;
            
            mySegs.reserve(maxPerThread * 2);

            #pragma omp for nowait schedule(static)
            for (int y = 0; y < gridHeight - 1; ++y)
            {
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

                    marchSquare((float)x, (float)y, values, isolevel, mySegs);

                    left_top_val = right_top_val;
                    left_bottom_val = right_bottom_val;
                }
            }
        }

        double endTime = omp_get_wtime();
        double elapsedTimeMs = (endTime - startTime) * 1000.0;

        std::cout << elapsedTimeMs << " ms." << std::endl;
    }

    return 0;
}