set -e 

CPP_SOURCE="marching_squares.cpp"
EXECUTABLE="march"

g++ -O3 -std=c++17 -fopenmp "$CPP_SOURCE" -o "$EXECUTABLE"

for res in $(seq 2000 2000 20000); do
  for thr in $(seq 1 20); do
    printf "[%d, %d]\n" "$res" "$thr"   
    OMP_NUM_THREADS=$thr "./$EXECUTABLE" "$res"
    printf "\n"
  done
done
