from LoadData import loadData
from typing import List
from ClusterGraphs import plot_clustered_stacked
import matplotlib.pyplot as plt
import pandas as pd


def rindex(index, label):
    if label is not None:
        index.rename(label, inplace=True)

def genGraphSplit(dir: str, dir2: str, colgroups: List, rows: List, varkeys: List[str] = ["Tuples"], filterBy: str = "Tuples",
             nocache: bool = True,
             name: str = None,
             maxBy: str = None,
             normalize: bool = True,
             ylabel: str = "us/Tuple",
             xlabel: str = None,
             yfactor: float = 1,
             yscaleSecure: bool = True,
             nativeYLabel: str = None,
             xfactor: float = 1,
             colors: List = None,
             legend: bool = True):
    colgroups = colgroups.copy()
    if nativeYLabel is None:
        nativeYLabel = ylabel
    for i, group in enumerate(colgroups):
        if isinstance(group, str):
            colgroups[i] = ((group, [group]))
    colgroups = [group for group in colgroups if isinstance(group, tuple)]
    cols = [col for tuple in colgroups for col in tuple[1]]
    if name is None:
        name = dir
    print("Loading data for " + dir)
    df1 = loadData(dir, cols.copy(), varkeys, maxBy=maxBy)
    df2 = loadData(dir2, cols.copy(), varkeys, maxBy=maxBy)
    def transformDF(df: pd.DataFrame)-> pd.DataFrame:
        for col in varkeys:
            if col is filterBy:
                df[col] = df[col].astype(type(rows[0]))
            else:
                df[col] = df[col].astype(int)
        df = df[df[filterBy].isin(rows)]
        df.sort_values(by=[filterBy], inplace=True)
        if yscaleSecure:
            df[cols] = df[cols] * yfactor
        if normalize:
            for column in cols:
                df[column] = df[column] / (df['Tuples'] * df["Hosts"] * df["PerHost"])
        df[filterBy] = df[filterBy] * xfactor
        df.set_index(["mode", filterBy], inplace=True)
        for column in df.columns:
            if column not in cols:
                df.drop(column, axis=1, inplace=True)
        for group in colgroups:
            df[group[0]] = df[group[1]].sum(axis=1)
            toDrop = [col for col in group[1] if col != group[0]]
            df.drop(toDrop, axis=1, inplace=True)
        for group in colgroups:
            if len(group) > 2:
                idxs = []
                if "caching" in df.index:
                    idxs.append("caching")
                if "native" in df.index:
                    idxs.append("native")
                df.loc[idxs][group[2]] -= df[group[0]]
        df = df[[group[0] for group in colgroups]]
        return df

    dfs = []
    df1 = transformDF(df1)
    df2 = transformDF(df2)
    cacheDF1: pd.DataFrame = df1.loc['caching']
    cacheDF2: pd.DataFrame = df2.loc['caching']
    rindex(cacheDF1.index, xlabel)
    rindex(cacheDF2.index, xlabel)
    axe = None
    nocacheDF1: pd.DataFrame = df1.loc['nocache']
    nocacheDF2: pd.DataFrame = df2.loc['nocache']
    rindex(nocacheDF1.index, xlabel)
    rindex(nocacheDF2.index, xlabel)
    plot_clustered_stacked([cacheDF1, nocacheDF1], ["caching", "noncaching"], title="", colors=colors, legend=legend, subplotParam=121, l1x=0.8, l2y=-0.15)
    plot_clustered_stacked([cacheDF2, nocacheDF2], ["caching", "noncaching"], title="", colors=colors, legend=False, subplotParam=122)
    legend=False
    # plt.yscale("log")
    # nocacheDF.plot(kind="bar", stacked=True)
    # plt.show()
    # cacheDF.plot(kind="bar", stacked=True)

    drawingDf = df1.copy()# df.drop(["mode"], axis=1)
    cols = len(drawingDf.columns)
    if axe is not None and colors is not None:
        h, l = axe.get_legend_handles_labels()  # get the handles we want to modify
        for i in range(0, cols, cols):  # len(h) = n_col * n_df
            for j, pa in enumerate(h[i:i + cols]):
                for rect in pa.patches:  # for each index
                    rect.set_facecolor(colors[j])

    if legend:
        h, l = axe.get_legend_handles_labels()  # get the handles we want to modify
        l1 = axe.legend(h[::-1], l[::-1],
                        bbox_to_anchor=(0.5, 1.00),
                        loc="lower center",
                        ncol=4
                        #bbox_to_anchor=(1.05, 1),
                        #borderaxespad=0.)
                        )
        axe.add_artist(l1)
        #plt.legend(loc="center left", bbox_to_anchor=(1, 0.3))
    plt.ylabel(ylabel)
    plt.xticks(rotation=0)
    if axe is not None:
        plt.tight_layout(w_pad=3, rect=(0,0,1,0.75))
        axe.get_figure().savefig("graphs/" + name + ".png", bbox_extra_artists=(l1,), bbox_inches="tight")
    else:
        plt.tight_layout(w_pad=3, rect=(0,0,1,0.75))
        #plt.tight_layout("")
        plt.savefig("graphs/" + name + ".png")
    #nativeDF.set_index(filterBy, inplace=True)
    print("generated " + name)
    plt.clf()
