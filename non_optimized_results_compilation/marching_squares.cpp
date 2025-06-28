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

std::vector<LineSegment> marchSquare(Point corners[4],
                                     float values[4],
                                     float isolevel)
{
    int caseIdx = 0;
    for (int i = 0; i < 4; ++i)
        if (values[i] >= isolevel)
            caseIdx |= (1 << i);

    if (caseIdx == 0 || caseIdx == 15)
        return {};

    Point edgePt[4];

    auto getEdgePoint = [&](int e) -> Point
    {
        int c0 = edgeCorners[e][0], c1 = edgeCorners[e][1];
        return lerp(corners[c0], corners[c1],
                    values[c0], values[c1],
                    isolevel);
    };

    std::vector<LineSegment> segs;
    int *pair = edgePairs[caseIdx];

    for (int i = 0; i < 4 && pair[i] != -1; i += 2)
    {
        int eA = pair[i], eB = pair[i + 1];

        segs.push_back({getEdgePoint(eA), getEdgePoint(eB)});
    }
    return segs;
}

int main(int argc, char *argv[])
{
    int gridResolution = 100;

    if (argc > 1)
        gridResolution = std::stoi(argv[1]);

    const int gridWidth = gridResolution;
    const int gridHeight = gridResolution;

    const float isolevel = 0.5f;

    std::vector<std::vector<float>> scalarField(gridHeight, std::vector<float>(gridWidth));

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (int y = 0; y < gridHeight; ++y)
    {
        for (int x = 0; x < gridWidth; ++x)
        {
            scalarField[y][x] = std::rand() % 2;
        }
    }


    for (int i = 0; i < 5; ++i) 
    {
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
                    
                    if (!cellSegments.empty())
                    {
                        privateSegments.insert(privateSegments.end(), cellSegments.begin(), cellSegments.end());
                    }
                }
            }

            #pragma omp critical
            allSegments.insert(allSegments.end(), privateSegments.begin(), privateSegments.end());
        }

        double endTime = omp_get_wtime();
        double elapsedTimeMs = (endTime - startTime) * 1000.0;

        std::cout << elapsedTimeMs << " ms." << std::endl;
    }
    return 0;
}