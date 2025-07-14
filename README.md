# Marching Squares Paralelo

Una implementación paralela de alto rendimiento del algoritmo Marching Squares para visualizar contornos 2D usando OpenMP.nmj

## 🎯 Descripción general

El algoritmo Marching Squares es una técnica de gráficos por computadora que genera líneas de contorno (isocurvas) a partir de un campo escalar 2D. Este proyecto ofrece varias implementaciones optimizadas del algoritmo y demuestra mejoras de rendimiento significativas mediante paralelización y técnicas de optimización.

## 🚀 Características

- **Varias implementaciones**: desde la versión secuencial base hasta versiones paralelas altamente optimizadas  
- **Paralelización con OpenMP**: multihilo eficiente usando directivas OpenMP  
- **Benchmark de rendimiento**: análisis exhaustivo con métricas de speedup y eficiencia  
- **Herramientas de visualización**: scripts en Python para analizar y visualizar resultados de rendimiento  
- **Compatibilidad multiplataforma**: funciona en Linux, macOS y Windows

## 📁 Estructura del proyecto

```
parallel-marching-squares/
├── checkpoint_1/              # Implementación secuencial inicial
├── checkpoint_2/              # Implementación paralela básica
├── checkpoint_3/              # Optimización de memoria
├── checkpoint_4/              # Optimización de caché
├── checkpoint_5/              # Versión final optimizada
├── optimized_results_compilation/     # Versión optimizada con resultados
├── non_optimized_results_compilation/ # Línea base sin optimizar
├── alternative_optimizations/         # Enfoque de optimización reservando memoria
├── commented_version/                 # Implementación comentada
├── results_visualizer/                # Análisis y visualización de rendimiento
└── README.md                          # Este archivo
```

## 🧮 Descripción del algoritmo

El algoritmo Marching Squares funciona del siguiente modo:

1. **Generación de la cuadrícula**: crear un campo escalar 2D (matriz de valores).  
2. **Procesamiento de celdas**: para cada celda 2×2 de la cuadrícula:  
   - Comparar los valores de las esquinas con un umbral de nivel iso.  
   - Generar un índice de caso de 4 bits según qué esquinas están por encima/debajo del umbral.  
   - Usar tablas de consulta para determinar qué aristas cruza el contorno.  
   - Calcular los puntos de intersección precisos mediante interpolación lineal.  
3. **Construcción del contorno**: combinar todos los segmentos de línea para formar el contorno final.

### Estructuras de datos clave

- **Point**: estructura de coordenadas 2D `{x, y}`  
- **LineSegment**: segmento de línea con punto inicial y final  
- **Tablas de aristas**: configuraciones pre‑calculadas para los 16 casos posibles  

## 🏗️ Versiones de implementación

### 1. Secuencial básica (`checkpoint_1/`)
- Implementación básica con múltiples niveles iso.  
- Genera contornos circulares a partir de una función de distancia radial.  
- Exporta resultados a un archivo CSV.  

### 2. Paralela optimizada (`optimized_results_compilation/`)
- **Paralelización OpenMP**: usa `#pragma omp parallel for`.  
- **Optimización de memoria**: matriz 2D aplanada a 1D para mejorar la caché.  
- **Campo escalar aleatorio**: valores binarios aleatorios para benchmarking.  
- **Almacenamiento privado por hilo**: cada hilo mantiene su propia lista de segmentos.  

### 3. Optimización alternativa (`alternative_optimizations/`)
- **Reserva de memoria optimizada**: Utiliza `mySegs.reserve(maxPerThread * 2)` para pre-asignar memoria y evitar realocaciones dinámicas durante la ejecución.  
- **Planificación estática**: `schedule(static)` para mejor balance de carga.  
- **Sincronización reducida**: minimiza secciones críticas usando vectores separados por hilo.

### 4. Versión comentada (`commented_version/`)
- Implementación totalmente documentada en español.  
- Explicaciones detalladas de cada paso del algoritmo.  
- Recurso educativo para comprender el algoritmo.  

## 🔧 Compilación y ejecución

### Requisitos previos
- Compilador C++ con soporte C++17  
- Biblioteca OpenMP  
- Make (opcional)  

### Configuración en macOS
```bash
# Instalar OpenMP con Homebrew
brew install libomp

# Compilar con las banderas adecuadas
g++ -Xpreprocessor -fopenmp -lomp -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib marching_squares.cpp -o march -std=c++17
```

### Configuración en Linux
```bash
# Compilar con GCC
g++ -fopenmp marching_squares.cpp -o march -std=c++17
```

### Run.sh

En cada carpeta hay un archivo run.sh, en el cual automatizamos la ejecución de cada checkpoint del proyecto.
```bash
set -e

CPP_SOURCE="marching_squares.cpp"
EXECUTABLE_NAME="march"

if [ -n "$2" ]; then
  export OMP_NUM_THREADS=$2
fi

echo "Compilando el ejecutable: $CPP_SOURCE con OpenMP support..."
g++ -fopenmp "$CPP_SOURCE" -o "$EXECUTABLE_NAME" -std=c++17
echo "Compilación exitosa. Ejecutable creado: $EXECUTABLE_NAME"
echo ""

echo "Corriendo el ejecutable..."
./"$EXECUTABLE_NAME" "$1"
echo ""

echo "Proceso completado."
```

### Ejecución
```bash
# Uso básico
./march [grid_size] [num_threads]

# Ejemplo: cuadrícula 2000x2000 con 4 hilos
./march 2000 4

# Usar script de ejecución
./run.sh 2000 4
```

## 📊 Resultados de rendimiento

El proyecto incluye un análisis exhaustivo que muestra mejoras significativas:

### Resultados de speedup
- **Tamaño de cuadrícula 2000**: hasta 2.8× de speedup con el número óptimo de hilos  
- **Tamaño de cuadrícula 4000**: hasta 3.4×  
- **Tamaño de cuadrícula 8000**: hasta 3.0×  
- **Tamaño de cuadrícula 12000**: hasta 4.0×  
- **Tamaño de cuadrícula 16000**: hasta 3.3×  

### Principales optimizaciones
1. **Disposición de memoria**: matrices 2D aplanadas mejoran la localidad de caché.  
2. **Optimización de bucles**: cálculos redundantes reducidos en los bucles internos.  
3. **Almacenamiento local por hilo**: minimiza la sincronización.  
4. **Balance de carga**: la planificación estática asegura distribución uniforme del trabajo.  

## 📈 Visualización y análisis

El directorio `results_visualizer/` contiene scripts en Python para analizar el rendimiento:

- `plot_results.py`: gráficos de superficie 3D comparando la versión optimizada vs. la no optimizada.  
- `plot_metrics.py`: análisis de speedup y eficiencia.  
- `get_speedup_efficiency.py`: cálculo de métricas de rendimiento.  

### Ejecución de la visualización
```bash
cd results_visualizer/
python plot_results.py
```

## 🔍 Análisis de rendimiento

### Metodología
- **Benchmarking**: 10 ejecuciones por configuración para significancia estadística.  
- **Tamaños de cuadrícula**: de 2000×2000 a 20000×20000 elementos.  
- **Número de hilos**: de 1 a 20 hilos.  
- **Métricas**: tiempo de ejecución, speedup y eficiencia.  

### Hallazgos clave
1. **Número óptimo de hilos**: generalmente 8‑12 hilos para el mejor rendimiento.  
2. **Escalabilidad**: el algoritmo escala bien con el tamaño del problema.  
3. **Cuello de botella de memoria**: rendimiento limitado por el ancho de banda de memoria a altos conteos de hilos.  
4. **Efectos de caché**: ganancias significativas gracias a patrones de acceso a memoria mejorados.  

## 🛠️ Detalles técnicos

### Implementación OpenMP al checkpoint 5
```cpp
#pragma omp parallel
{
    std::vector<LineSegment> privateSegments;
    
    #pragma omp for nowait
    for (int y = 0; y < gridHeight - 1; ++y) {
        // Procesar celdas de la cuadrícula
    }
    
    #pragma omp critical
    allSegments.insert(allSegments.end(), 
                      privateSegments.begin(), 
                      privateSegments.end());
}
```

### Optimizaciones clave en `alternative_optimizations`
1. **Evitar asignaciones de memoria dinámicas** en los bucles internos.  
2. **Usar almacenamiento local por hilo** para minimizar la sincronización.  
3. **Optimizar patrones de acceso a memoria** para eficiencia de caché.  
4. **Reducir operaciones en punto flotante** cuando sea posible.  

## 📋 Ejemplos de uso

### Generación básica de contornos
```cpp
// Generar contornos para un campo escalar
std::vector<LineSegment> contours;
marchSquare(x, y, values, isolevel, contours);
```

### Benchmark de rendimiento
```bash
# Probar diferentes tamaños de cuadrícula
for size in 2000 4000 6000 8000; do
    ./march $size
done
```

## 🔗 Referencias

- [Algoritmo Marching Squares](https://es.wikipedia.org/wiki/Marching_squares)  
- [Documentación de OpenMP](https://www.openmp.org/)  

## 👥 Autores

- Nicolás Arroyo
- Gabriel Espinoza
- Josué Arriaga
