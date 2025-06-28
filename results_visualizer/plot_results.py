import re
import sys
import numpy as np
import pandas as pd
import plotly.graph_objects as go


def parse_results(path: str) -> pd.DataFrame:
    rows = []
    res = thr = None
    times = []

    with open(path, encoding="utf-8") as fh:
        for line in fh:
            line = line.strip()
            if not line:
                continue

            if line.startswith("["):
                if res is not None and times:
                    rows.append({
                        "resolution": res,
                        "threads": thr,
                        "elements": res * res,
                        "avg_ms": np.mean(times),
                    })
                m = re.match(r"\[(\d+),\s*(\d+)\]", line)
                if not m:
                    raise ValueError(f"Formato de cabecera desconocido: {line}")
                res, thr = map(int, m.groups())
                times = []
            else:
                m = re.match(r"([\d.]+)\s*ms", line)
                if m:
                    times.append(float(m.group(1)))

    if res is not None and times:
        rows.append({
            "resolution": res,
            "threads": thr,
            "elements": res * res,
            "avg_ms": np.mean(times),
        })

    return pd.DataFrame(rows)


def build_surface(df: pd.DataFrame, name: str, color_scale: str):
    df_sorted = df.sort_values(by=["threads", "resolution"])
    pivot = df_sorted.pivot(index="threads", columns="resolution", values="avg_ms")
    x = pivot.columns.values
    y = pivot.index.values
    z = pivot.values

    surface = go.Surface(
        x=x, y=y, z=z,
        colorscale=color_scale,
        opacity=0.8,
        showscale=False,
        name=name,
        visible=True
    )

    points = go.Scatter3d(
        x=df_sorted["resolution"],
        y=df_sorted["threads"],
        z=df_sorted["avg_ms"],
        mode='markers',
        marker=dict(size=4, color='black'),
        name=f'{name} (puntos)',
        visible=True
    )

    return surface, points


def plot_surfaces(df_opt, df_not_opt):
    surf_opt, pts_opt = build_surface(df_opt, "Optimizado", "Viridis")
    surf_no, pts_no = build_surface(df_not_opt, "No optimizado", "Reds")

    fig = go.Figure(data=[surf_opt, pts_opt, surf_no, pts_no])

    fig.update_layout(
        scene=dict(
            xaxis_title="Resolución (N)",
            yaxis_title="Hilos",
            zaxis_title="Tiempo medio (ms)",
            zaxis=dict(range=[0, 20000]),
            yaxis=dict(range=[1, 20])
        ),
        updatemenus=[{
            "type": "buttons",
            "direction": "left",
            "x": 0.5,
            "y": 1.15,
            "buttons": [
                {
                    "label": "Mostrar ambos",
                    "method": "update",
                    "args": [{"visible": [True, True, True, True]}]
                },
                {
                    "label": "Solo optimizado",
                    "method": "update",
                    "args": [{"visible": [True, True, False, False]}]
                },
                {
                    "label": "Solo no optimizado",
                    "method": "update",
                    "args": [{"visible": [False, False, True, True]}]
                }
            ],
        }],
        margin=dict(l=0, r=0, b=0, t=80),
        height=700,
    )

    fig.show()


def main():
    df_opt = parse_results("resultados_opt.txt")
    df_no_opt = parse_results("resultados_non_opt.txt")
    plot_surfaces(df_opt, df_no_opt)


if __name__ == "__main__":
    main()
