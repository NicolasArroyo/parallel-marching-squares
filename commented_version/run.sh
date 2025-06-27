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
