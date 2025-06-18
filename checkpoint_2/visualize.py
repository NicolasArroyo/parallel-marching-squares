import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection
import csv
import sys


def visualize_marching_squares(
    filename="lines.csv", output_filename="contour_plot.png"
):
    segments = []

    with open(filename, "r") as csvfile:
        csv_reader = csv.reader(csvfile)
        header = next(csv_reader)

        for row in csv_reader:
            if len(row) == 4:
                start_x, start_y, end_x, end_y = map(float, row)
                segments.append(((start_x, start_y), (end_x, end_y)))

    fig, ax = plt.subplots(figsize=(10, 10))

    line_collection = LineCollection(segments, colors="white", linewidths=0.1)

    ax.add_collection(line_collection)

    ax.set_title("Marching Squares")
    ax.set_xlabel("X")
    ax.set_ylabel("Y")

    ax.set_facecolor("black")
    fig.set_facecolor("black")

    ax.tick_params(axis="x", colors="white")
    ax.tick_params(axis="y", colors="white")

    for spine in ax.spines.values():
        spine.set_edgecolor("white")

    ax.title.set_color("white")
    ax.xaxis.label.set_color("white")
    ax.yaxis.label.set_color("white")

    ax.autoscale()
    ax.set_aspect("equal", adjustable="box")

    plt.savefig(output_filename, dpi=1000)
    print(f"Plot guardado en '{output_filename}'")


if __name__ == "__main__":
    input_file = sys.argv[1] if len(sys.argv) > 1 else "lines.csv"
    visualize_marching_squares(input_file)
