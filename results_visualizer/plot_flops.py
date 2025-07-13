import re
import numpy as np
import pandas as pd
import plotly.graph_objects as go

FLOPS_PER_CELL = 23.5


def parse_results(path: str) -> pd.DataFrame:
    rows, res, thr, times = [], None, None, []

    with open(path, encoding="utf-8") as fh:
        for line in map(str.strip, fh):
            if not line:
                continue

            if line.startswith("["):
                if res is not None and times:
                    rows.append(_row(res, thr, times))
                m = re.match(r"\[(\d+),\s*(\d+)\]", line)
                if not m:
                    raise ValueError(f"Bad header: {line}")
                res, thr = map(int, m.groups())
                times = []
            else:
                m = re.match(r"([\d.]+)\s*ms", line)
                if m:
                    times.append(float(m.group(1)))

    if res is not None and times:
        rows.append(_row(res, thr, times))

    return pd.DataFrame(rows)


def _row(resolution: int, threads: int, times_ms: list[float]) -> dict:
    avg_ms = float(np.mean(times_ms))
    cells = (resolution - 1) ** 2
    flops = cells * FLOPS_PER_CELL
    gflops = flops / (avg_ms * 1e-3) / 1e9
    return {
        "resolution": resolution,
        "threads": threads,
        "avg_ms": avg_ms,
        "gflops": gflops,
    }


def build_surface(df: pd.DataFrame):
    df = df.sort_values(["threads", "resolution"])
    piv = df.pivot(index="threads", columns="resolution", values="gflops")
    x, y, z = piv.columns.values, piv.index.values, piv.values

    surf = go.Surface(
        x=x,
        y=y,
        z=z,
        colorscale="Viridis",
        opacity=0.85,
        showscale=True,
        colorbar=dict(title="GFLOP/s"),
    )
    pts = go.Scatter3d(
        x=df["resolution"],
        y=df["threads"],
        z=df["gflops"],
        mode="markers",
        marker=dict(size=3, color="black"),
        name="Mediciones",
    )
    return surf, pts


def main():
    df_opt = parse_results("resultados_opt_new.txt")
    surf, pts = build_surface(df_opt)

    fig = go.Figure(data=[surf, pts])
    fig.update_layout(
        title="Rendimiento de Marching Squares (GFLOP/s)",
        scene=dict(
            xaxis_title="Resolución N", yaxis_title="Hilos", zaxis_title="GFLOP/s"
        ),
        height=700,
        margin=dict(l=0, r=0, b=0, t=50),
    )
    # fig.show()
    fig.write_html("marching_squares_flops.html", include_plotlyjs="cdn")


if __name__ == "__main__":
    main()
