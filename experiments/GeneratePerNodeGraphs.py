import re
import os
import shutil
from typing import List
from ClusterTripleGraphs import plot_clustered_stacked
import matplotlib.pyplot as plt
import pandas as pd
from LoadData import loadData

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

def loadData(identifier: str, keys: List[str], varkeys: List[str], uniformFilter: str = None) -> dict[dict, pd.DataFrame]:
    resultMap = {}
    for configuration in next(os.walk(identifier))[1]:
        if uniformFilter is not None and configuration != uniformFilter:
            continue
        #df = pd.DataFrame(columns=["mode"] + varkeys + keys)
        mode = configuration.split("-")[-1]
        varparams = re.split(",|--", configuration)[-len(varkeys):]
        section = 1 if len(varparams) >= 3 else 0
        varparams[-1] = varparams[-1].split("-")[section]
        varparams = [it.split("=")[1] if len(it.split("=")) > 1 else it for it in varparams]
        varparams = varparams[0].split("x") + varparams[1:]
        varmap = dict(zip(varkeys, varparams))
        varmap["mode"] = mode
        tempdfs: List[List[pd.DataFrame]] = []
        for run in next(os.walk(identifier + "/" + configuration))[1]:
            nodes: List[pd.DataFrame] = []
            for file in next(os.walk(identifier + "/" + configuration + "/" + run))[2]:
                if file.endswith("perf"):
                    with open(identifier + "/" + configuration + "/" + run + "/" + file) as f:
                        interestingLines = {}
                        for line in f:
                            for key in keys:
                                if line.startswith(key):
                                    splitted = line.split()
                                    interestingLines[splitted[0]] = [float(splitted[1])]
                        nodes.append(pd.DataFrame(interestingLines, index=[int(file.split(".")[0])]))
            tempdfs.append(nodes)
        result: pd.Series
        totalResults: pd.DataFrame
        files = [pd.concat(files) for files in tempdfs]
        totalResults = pd.concat(files)
        totalResultsByIndex = totalResults.groupby(totalResults.index)
        result = totalResultsByIndex.mean()
        if uniformFilter is None:
            resultMap[varmap['mode'] + "," + varmap["ZipfFactor"]] = result
        else:
            resultMap[varmap['mode'] + "," + "uniform"] = result
    return resultMap


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
             legend: bool = True,
             selection: List[str] = None):
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
    loadedDfs = loadData(dir, cols.copy(), varkeys.copy(), uniformFilter=None)
    loadedDfs.update(loadData("NodesPerHostIncreasing", cols.copy(), varkeys.copy(), uniformFilter="15x1--t=5000000-caching"))
    loadedDfs.update(loadData("NodesPerHostIncreasing", cols.copy(), varkeys.copy(), uniformFilter="15x1--t=5000000-nocache"))
    dfs: dict[str, pd.DataFrame] = {}
    for key, value in loadedDfs.items():
        if key in selection:
            dfs[key] = value
    transformedDfs: dict[str, pd.DataFrame] = {}
    for key, df in dfs.items():
        #df = df[df[filterBy].isin(rows)]
        #df.sort_values(by=[filterBy], inplace=True)
        df.sort_index(inplace=True)
        if yscaleSecure:
            df[cols] = df[cols] * yfactor
        #df[filterBy] = df[filterBy] * xfactor
        #df.set_index(["mode", filterBy], inplace=True)
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
        transformedDfs[key] = df

    plt.figure(figsize=(10,20))
    plot_clustered_stacked([transformedDfs["caching,uniform"], transformedDfs["caching,1"], transformedDfs["caching,2"]],
                           ["uniform", "z=1", "z=2"], title="", colors=colors)
    drawingDf = df.copy()# df.drop(["mode"], axis=1)
    cols = len(drawingDf.columns)
    plt.ylabel(ylabel)
    plt.xticks(rotation=0)
    #plt.tight_layout(pad=1)
    plt.tight_layout(rect=(0,0,1,0.75))
    #plt.tight_layout("")
    plt.savefig("graphs/caching" + name + ".png")
    #nativeDF.set_index(filterBy, inplace=True)
    plt.clf()

    plot_clustered_stacked([transformedDfs["nocache,uniform"], transformedDfs["nocache,1"], transformedDfs["nocache,2"]],
                           ["uniform", "z=1", "z=2"], title="", colors=colors)
    drawingDf = df.copy()# df.drop(["mode"], axis=1)
    cols = len(drawingDf.columns)
    plt.ylabel(ylabel)
    plt.xticks(rotation=0)
    #plt.tight_layout(pad=1)
    plt.tight_layout(rect=(0,0,1,0.75))
    #plt.tight_layout("")
    plt.savefig("graphs/noncaching" + name + ".png")
    #nativeDF.set_index(filterBy, inplace=True)
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

