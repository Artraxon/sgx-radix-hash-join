import os
import shutil
from typing import List
from ClusterGraphs import plot_clustered_stacked
from GeneratePerNodeGraphs import genGraph as genGraphPerNode
import GenerateSplitGraphs as gsg
import matplotlib.pyplot as plt
import pandas as pd
from LoadData import loadData

dirs = ["TuplesPerNode", "NodesPerHostConstant", "NodesPerHostIncreasing", "HostsFixedData", "NetworkPart", "LocalPart",
        "PackageSize", "DataSkew"]

if "graphs" in next(os.walk("."))[1]:
    shutil.rmtree("graphs")
os.mkdir("graphs")

def rindex(index, label):
    if label is not None:
        index.rename(label, inplace=True)

def set_size(w,h, ax=None):
    """ w, h: width, height in inches """
    if not ax: ax=plt.gca()
    l = ax.figure.subplotpars.left
    r = ax.figure.subplotpars.right
    t = ax.figure.subplotpars.top
    b = ax.figure.subplotpars.bottom
    figw = float(w)/(r-l)
    figh = float(h)/(t-b)
    ax.figure.set_size_inches(figw, figh)

def genGraph(dir: str, colgroups: List, rows: List, varkeys: List[str] = ["Tuples"], filterBy: str = "Tuples",
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
    df = loadData(dir, cols.copy(), varkeys, maxBy=maxBy)
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
    dfs = []
    plt.figure(figsize=(10,7))
    if 'caching' in df.index:
        cacheDF: pd.DataFrame = df.loc['caching']#.drop("mode", axis=1)#.set_index(filterBy)
        rindex(cacheDF.index, xlabel)
        dfs.append(cacheDF)
    axe = None
    if nocache and 'caching' in df.index:
        nocacheDF: pd.DataFrame = df.loc['nocache']#.drop("mode", axis=1)#.set_index(filterBy)
        rindex(nocacheDF.index, xlabel)
        dfs.append(nocacheDF)
        plot_clustered_stacked(dfs, ["caching", "noncaching"], title="", colors=colors, legend=legend)
        legend=False
    elif not nocache:
        axe = cacheDF.plot(kind="bar", stacked=True, edgecolor="black", linewidth=0.3)
    else:
        nocacheDF: pd.DataFrame = df.loc['nocache']#.drop("mode", axis=1)#.set_index(filterBy)
        rindex(nocacheDF.index, xlabel)
        axe = nocacheDF.plot(kind="bar", stacked=True, edgecolor="black", linewidth=0.3)
    # plt.yscale("log")
    # nocacheDF.plot(kind="bar", stacked=True)
    # plt.show()
    # cacheDF.plot(kind="bar", stacked=True)

    drawingDf = df.copy()# df.drop(["mode"], axis=1)
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
        #plt.tight_layout(pad=1)
        plt.tight_layout(rect=(0,0,1,0.75))
        axe.get_figure().savefig("graphs/" + name + ".png", bbox_extra_artists=(l1,), bbox_inches="tight")
    else:
        #plt.tight_layout(pad=1)
        plt.tight_layout(rect=(0,0,1,0.75))
        #plt.tight_layout("")
        plt.savefig("graphs/" + name + ".png")
    #nativeDF.set_index(filterBy, inplace=True)
    if 'native' in df.index:
        nativeDF: pd.DataFrame = df.loc["native"]  # .drop(["mode"], axis=1)
        if "SUNSEAL" in nativeDF.columns:
            nativeDF.drop(["SUNSEAL"], axis=1, inplace=True)
        if not yscaleSecure:
            groupedCols = [g[0] for g in colgroups if g[0] != "SUNSEAL"]
            nativeDF[groupedCols] = nativeDF[groupedCols] * yfactor
        if colors is not None:
            #colors = colors[:6] + colors[7:]
            #nativeDF[nativeDF < 0] = 0
            rindex(nativeDF.index, xlabel)
            axe: plt.Axes = nativeDF.plot(kind="bar", stacked=True, edgecolor="black", linewidth=0.3, legend=None)
            axe.set_ylabel(nativeYLabel)
            h, l = axe.get_legend_handles_labels()  # get the handles we want to modify
            cols = len(nativeDF.columns)
            for i in range(0, cols, cols):  # len(h) = n_col * n_df
                for j, pa in enumerate(h[i:i + cols]):
                    for rect in pa.patches:  # for each index
                            rect.set_facecolor(colors[j])
        else:
            nativeDF.plot(kind="bar", stacked=True, edgecolor="black", linewidth=0.3, legend=None)
        #plt.legend(loc="center left", bbox_to_anchor=(1, 0.5))
        axe.figure.set_size_inches(5, 5)
        plt.xticks(rotation=0)
        plt.tight_layout(rect=(0,0,1,0.75))
        #plt.tight_layout(pad=2)
        plt.savefig("graphs/" + name + "-native.png")
    print("generated " + name)
    plt.clf()

columns = ["JHIST", "JMPI", "JPROC", "SWINALLOC", "SUNSEAL"]
localColumns = ["LPHISTCOMP", "LPPART", "BPMEMALLOC", "BPBUILD", "BPPROBE"]
networkColumns = [("NALLOC",  ["MIMEMALLOC", "MOMEMALLOC"]), ("NMAIN", ["MIMAINPART", "MOMAINPART"]), ("NFLUSH", ["MIFLUSHPART", "MOFLUSHPART"]),
                  ("MWINPUT", ["MWINPUT"], "NMAIN"), ("MWINWAIT", ["MWINWAIT"], "NMAIN"), ("MWSEAL", ["MWSEAL"], "NMAIN")]

allPlusNetwork = [columns[0]] + networkColumns + columns[2:]
allPlusLocal = columns[:2] + localColumns + columns[3:]
allColumns = [columns[0]] + networkColumns + localColumns + columns[3:]

defaultColors = ["tab:blue", "tab:orange", "tab:olive", "mediumaquamarine", "tab:purple"]
networkColors = ["palegreen", "pink", "silver", "gold", "darkturquoise", "cornflowerblue"]
localColors = ["midnightblue", "plum", "forestgreen", "darkkhaki", "azure"]
networkMixedColors = [defaultColors[0]] + networkColors + defaultColors[2:]
localMixedColors = defaultColors[:2] + localColors + defaultColors[3:]
allMixedColors = [defaultColors[0]] + networkColors + localColors + defaultColors[3:]

gsg.genGraphSplit("NodesPerHostIncreasing", "../oldArtifacts/NodesPerHostIncreasing", allPlusNetwork, list(range(14, 17, 2)), ["Hosts", "PerHost", "Tuples"], "PerHost",
                  colors=networkMixedColors, name="NodesPerHostIncreasingExtreme",
                  xlabel="Nodes per Host")

genGraph("LocalPart", allPlusLocal, range(5, 11), ["Hosts", "PerHost", "Tuples", "LPart"], "LPart", colors=localMixedColors,
         ylabel="ns/Tuple", yfactor=1000, xlabel="radix bits")
genGraph("LocalPart", allColumns, range(5, 11), ["Hosts", "PerHost", "Tuples", "LPart"], "LPart", colors=allMixedColors,
         ylabel="ns/Tuple", yfactor=1000, name="LocalPartAllColumns")
genGraphPerNode("DataSkew", allColumns, [1.0, 2],
         ["Hosts", "PerHost", "Tuples", "ZipfSize", "ZipfFactor"], "ZipfFactor", maxBy="JTOTAL", normalize=False, colors=allMixedColors,
         selection=["nocache,uniform","nocache,1","nocache,2","caching,1","caching,2","caching,uniform"],
         yfactor=1/1000000, ylabel="Runtime in seconds", xlabel="Node")

genGraph("TuplesPerNode", allColumns, [1000 * 1000, 5 * 1000 * 1000, 10*1000*1000, 15*1000*1000,
                                       20*1000*1000, 30*1000*1000, 40*1000*1000, 50*1000*1000, 60*1000*1000],
         ["Hosts", "PerHost", "Tuples"],
         "Tuples",
         colors=allMixedColors,
         xfactor=1/1000000,
         xlabel="number of tuples per node [M]",
         yfactor=1000,
         ylabel="ns/Tuple",
         name="Tuples per node")


genGraph("NetworkPart", allColumns, range(5, 11), ["Hosts", "PerHost", "Tuples", "NPart"], "NPart", colors=allMixedColors,
         ylabel="ns/Tuple", yfactor=1000,
         xlabel="Radix Bits")
genGraph("NodesPerHostIncreasing",
         allColumns,
         [1] + list(range(2, 13, 2)),
         ["Hosts", "PerHost", "Tuples"],
         "PerHost",
         colors=allMixedColors,
         yfactor=1000,
         ylabel="ns/Tuple",
         xlabel="Nodes per Host")


genGraph("NodesPerHostConstant", allColumns, [1] + list(range(2, 13, 2)), ["Hosts", "PerHost", "Tuples"], "PerHost",
         colors=allMixedColors, name="NodesPerHostConstantLong", yscaleSecure=True, ylabel="ns/Tuple", yfactor=1000,
         xlabel="Nodes per Host")


genGraph("TuplesPerNode", allColumns, [100 * 1000, 1000 * 1000],
         ["Hosts", "PerHost", "Tuples"],
         "Tuples",
         name="Tuples Small", colors=allMixedColors)



genGraph("HostsIncreasingData", allColumns, range(1, 6, 1), ["Hosts", "PerHost", "Tuples"], "Hosts", True,
         name="HostsIncreasingData 1-6", colors=allMixedColors, legend=False, yfactor=1000, nativeYLabel="ns/Tuple", yscaleSecure=False)

genGraph("HostsIncreasingData", allColumns, range(5, 16, 2), ["Hosts", "PerHost", "Tuples"], "Hosts", True,
         name="HostsIncreasingData 6-16", colors=allMixedColors, yfactor=1000, nativeYLabel="ns/Tuple", yscaleSecure=True)

genGraph("HostsFixedData", allColumns, range(2, 16, 2), ["Hosts", "PerHost", "Tuples"], "Hosts", True, colors=allMixedColors,
         yfactor=1000, ylabel="ns/Tuple", yscaleSecure=True)


gsg.genGraphSplit("NodesPerHostConstant", "../oldArtifacts/NodesPerHostConstant", allPlusNetwork, list(range(14, 17, 2)), ["Hosts", "PerHost", "Tuples"], "PerHost",
                  colors=networkMixedColors, name="NodesPerHostConstantExtreme",
                  xlabel="Nodes per Host")

genGraph("NodesPerHostConstant", localColumns, [1] + list(range(2, 13, 2)), ["Hosts", "PerHost", "Tuples"], "PerHost",
         colors=localColors, name="NodesPerHostConstantLocal", yfactor=1000, ylabel="ns/Tuple", yscaleSecure=True,
         xlabel="Nodes per Host")

genGraph("TuplesPerNode",
         ["LPHISTCOMP", "LPPART", "BPMEMALLOC", "BPBUILD", "BPPROBE"],
         [1000 * 1000, 5 * 1000 * 1000, 10*1000*1000, 15*1000*1000
             #, 20*1000*1000
             ],
         ["Hosts", "PerHost", "Tuples"],
         "Tuples",
         name="Tuples Per Node Local Processing",
         nocache=False,
         colors=localColors)

genGraph("PackageSize", allPlusNetwork, [64, 128, 256, 512, 1024, 2048], ["Hosts", "PerHost", "Tuples", "packageSize"],
         "packageSize", colors=networkMixedColors,
         xlabel="Package and buffer size in Tuples/8",
         yfactor=1000,
         ylabel="ns/Tuple")



genGraph("LocalPart",
         ["LPHISTCOMP", "PART", "BPMEMALLOC", "BPBUILD", "BPPROBE"],
         range(5, 11),
         ["Hosts", "PerHost", "Tuples", "LPart"],
         "LPart",
         name="Local Partitioning Local Processing",
         nocache=False,
         colors=localColors)

#genGraph("NodesPerHostConstant", allPlusNetwork, [1, 2, 4, 8, 16], ["Hosts", "PerHost", "Tuples"], "PerHost", colors=mixedColors)
#genGraph("HostsIncreasingData", allPlusLocal, range(1, 7, 1), ["Hosts", "PerHost", "Tuples"], "Hosts", True,
#         name="HostsIncreasingData 1-6", colors=localMixedColors, legend=False)





#genGraph("HostsFixedData", ["LPHISTCOMP", "LPPART", "BPMEMALLOC", "BPBUILD", "BPPROBE"], range(2, 16, 2), ["Hosts", "PerHost", "Tuples"], "Hosts", True, name="HostsFixedData Local")
#




#genGraph("DataSkewSize", columns, [5*1000, 50*1000, 500*1000, 5*1000*1000], ["Hosts", "PerHost", "Tuples", "packageSize", "ZipfFactor", "ZipfSize"], "ZipfSize",)