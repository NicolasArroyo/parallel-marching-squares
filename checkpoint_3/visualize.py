import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection
import csv
import sys

def visualize_marching_squares(filename="lines.csv", output_filename="contour_plot.png"):
    """
    Reads line segments from a CSV file and plots them efficiently
    using a LineCollection.

    Args:
        filename (str): The path to the CSV file containing the line segments.
        output_filename (str): The name of the image file to save the plot to.
    """
    segments = []
    
    # --- 1. Read the Data File ---
    try:
        with open(filename, 'r') as csvfile:
            csv_reader = csv.reader(csvfile)
            header = next(csv_reader)
            
            # Read all rows into a list of segments
            for row in csv_reader:
                if len(row) == 4:
                    start_x, start_y, end_x, end_y = map(float, row)
                    segments.append(((start_x, start_y), (end_x, end_y)))
                    
    except FileNotFoundError:
        print(f"Error: The file '{filename}' was not found.", file=sys.stderr)
        return
    except (ValueError, IndexError) as e:
        print(f"Error reading the file: {e}", file=sys.stderr)
        return

    if not segments:
        print("No line segments found in the file. The plot will be empty.")
        return
    
    # --- 2. Plot the Line Segments Efficiently ---
    fig, ax = plt.subplots(figsize=(10, 8)) # Adjusted for better viewing
    
    # Create a LineCollection object. This is MUCH faster than plotting in a loop.
    line_collection = LineCollection(segments, colors='white', linewidths=0.5)
    
    # Add the entire collection to the axes at once
    ax.add_collection(line_collection)

    # --- 3. Customize and Save Plot ---
    ax.set_title("Marching Squares Contour Plot")
    ax.set_xlabel("X")
    ax.set_ylabel("Y")
    
    # Set the background color to black to match the example image
    ax.set_facecolor('black')
    fig.set_facecolor('black')

    # Customize tick colors to be visible on a black background
    ax.tick_params(axis='x', colors='white')
    ax.tick_params(axis='y', colors='white')

    # Customize spine colors
    for spine in ax.spines.values():
        spine.set_edgecolor('white')
    
    # Set title and label colors
    ax.title.set_color('white')
    ax.xaxis.label.set_color('white')
    ax.yaxis.label.set_color('white')

    # Automatically adjust the plot limits and maintain aspect ratio
    ax.autoscale()
    ax.set_aspect('equal', adjustable='box')
    
    plt.savefig(output_filename, dpi=150) # Set DPI for better image quality
    print(f"Plot successfully saved to '{output_filename}'")


if __name__ == "__main__":
    # Check for a custom filename from command-line arguments, if needed
    input_file = sys.argv[1] if len(sys.argv) > 1 else "lines.csv"
    visualize_marching_squares(input_file)
