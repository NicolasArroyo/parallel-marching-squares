set -e

CPP_SOURCE="marching_squares.cpp"
EXECUTABLE_NAME="march"
PYTHON_SCRIPT="visualize.py"
OUTPUT_IMAGE="contour_plot.png"

echo "Compilando el ejecutable: $CPP_SOURCE..."
g++ "$CPP_SOURCE" -o "$EXECUTABLE_NAME" -std=c++17
echo "Compilación exitosa. Ejecutable creado: $EXECUTABLE_NAME"
echo ""

echo "Corriendo el ejecutable..."
./"$EXECUTABLE_NAME" "$1"
echo ""

echo "Corriendo el visualizer: $PYTHON_SCRIPT..."
echo ""
python3 "$PYTHON_SCRIPT"
echo ""

echo "Proceso completado. Imagen creada: $OUTPUT_IMAGE"

