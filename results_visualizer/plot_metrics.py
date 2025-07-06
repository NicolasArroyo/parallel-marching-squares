import re
import numpy as np
import pandas as pd
import plotly.graph_objects as go


def parse_metric(path: str, colname: str) -> pd.DataFrame:
    rows, res = [], None
    with open(path, encoding="utf-8") as fh:
        for raw in fh:
            line = raw.strip()
            if not line:
                continue
            if line.startswith("["):
                res = int(re.match(r"\[(\d+)\]", line).group(1))
            else:
                m = re.match(r"(\d+):\s*([\d.]+)", line)
                if m:
                    thr, val = int(m.group(1)), float(m.group(2))
                    rows.append({"resolution": res, "threads": thr, colname: val})
    return pd.DataFrame(rows)


def make_surface(
    df: pd.DataFrame, value_col: str, name: str, colorscale: str, legendgroup: str
):
    df = df.sort_values(["threads", "resolution"])
    grid = df.pivot(index="threads", columns="resolution", values=value_col)
    x, y, z = grid.columns.values, grid.index.values, grid.values

    surf = go.Surface(
        x=x,
        y=y,
        z=z,
        name=name,
        colorscale=colorscale,
        opacity=0.85,
        showscale=False,
        legendgroup=legendgroup,
    )

    pts = go.Scatter3d(
        x=df["resolution"],
        y=df["threads"],
        z=df[value_col],
        mode="markers",
        marker=dict(size=2, color="black"),
        name=f"{name} (pts)",
        legendgroup=legendgroup,
        showlegend=False,
    )
    return surf, pts


def build_figure(df_su_opt, df_su_no, df_eff_opt, df_eff_no):

    traces = []
    traces += make_surface(df_su_opt, "speedup", "Speed-up Opt", "Viridis", "Opt")
    traces += make_surface(df_su_no, "speedup", "Speed-up No", "Reds", "No")
    traces += make_surface(df_eff_opt, "efficiency", "Eficiencia Opt", "Viridis", "Opt")
    traces += make_surface(df_eff_no, "efficiency", "Eficiencia No", "Reds", "No")

    vis_speed_both = [True, True, True, True] + [False] * 4
    for tr, v in zip(traces, vis_speed_both):
        tr.visible = v

    fig = go.Figure(data=traces)

    masks = {
        "SU_BOTH": [True, True, True, True] + [False] * 4,
        "SU_OPT": [True, True, False, False] + [False] * 4,
        "SU_NO": [False, False, True, True] + [False] * 4,
        "EF_BOTH": [False] * 4 + [True, True, True, True],
        "EF_OPT": [False] * 4 + [True, True, False, False],
        "EF_NO": [False] * 4 + [False, False, True, True],
    }

    updatemenu = dict(
        type="buttons",
        direction="down",
        x=0.02,
        y=1.05,
        buttons=[
            dict(
                label="Speed-up  •  ambos",
                method="update",
                args=[
                    {"visible": masks["SU_BOTH"]},
                    {
                        "scene": {
                            "zaxis": {
                                "title": "Speed-up",
                                "range": [
                                    0,
                                    1.05
                                    * max(
                                        df_su_opt["speedup"].max(),
                                        df_su_no["speedup"].max(),
                                    ),
                                ],
                            }
                        }
                    },
                ],
            ),
            dict(
                label="Speed-up  •  solo optimizado",
                method="update",
                args=[
                    {"visible": masks["SU_OPT"]},
                    {
                        "scene": {
                            "zaxis": {
                                "title": "Speed-up",
                                "range": [
                                    0,
                                    1.05
                                    * max(
                                        df_su_opt["speedup"].max(),
                                        df_su_no["speedup"].max(),
                                    ),
                                ],
                            }
                        }
                    },
                ],
            ),
            dict(
                label="Speed-up  •  solo no optimizado",
                method="update",
                args=[
                    {"visible": masks["SU_NO"]},
                    {
                        "scene": {
                            "zaxis": {
                                "title": "Speed-up",
                                "range": [
                                    0,
                                    1.05
                                    * max(
                                        df_su_opt["speedup"].max(),
                                        df_su_no["speedup"].max(),
                                    ),
                                ],
                            }
                        }
                    },
                ],
            ),
            dict(
                label="Eficiencia • ambos",
                method="update",
                args=[
                    {"visible": masks["EF_BOTH"]},
                    {"scene": {"zaxis": {"title": "Eficiencia", "range": [0, 1.1]}}},
                ],
            ),
            dict(
                label="Eficiencia • solo optimizado",
                method="update",
                args=[
                    {"visible": masks["EF_OPT"]},
                    {"scene": {"zaxis": {"title": "Eficiencia", "range": [0, 1.1]}}},
                ],
            ),
            dict(
                label="Eficiencia • solo no optimizado",
                method="update",
                args=[
                    {"visible": masks["EF_NO"]},
                    {"scene": {"zaxis": {"title": "Eficiencia", "range": [0, 1.1]}}},
                ],
            ),
        ],
    )

    fig.update_layout(
        title="Marching Squares (OMP) - Speed-up & Eficiencia",
        scene=dict(
            xaxis_title="Resolución (N)",
            yaxis_title="Hilos",
            zaxis_title="Speed-up",
            yaxis=dict(range=[1, 20]),
        ),
        updatemenus=[updatemenu],
        margin=dict(l=0, r=0, b=0, t=80),
        height=700,
    )

    return fig


def main():
    df_su_opt = parse_metric("speedup_opt.txt", "speedup")
    df_su_no = parse_metric("speedup_non_opt.txt", "speedup")
    df_eff_opt = parse_metric("efficiency_opt.txt", "efficiency")
    df_eff_no = parse_metric("efficiency_non_opt.txt", "efficiency")

    fig = build_figure(df_su_opt, df_su_no, df_eff_opt, df_eff_no)
    fig.show()


if __name__ == "__main__":
    main()
