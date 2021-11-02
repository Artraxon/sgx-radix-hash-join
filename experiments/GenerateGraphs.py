import os
import shutil
import re
from typing import List
from ClusterGraphs import plot_clustered_stacked
import matplotlib.pyplot as plt
import pandas as pd


def loadData(identifier: str, keys: List[str], varkeys: List[str], maxBy: str = None) -> pd.DataFrame:
    if maxBy is not None and maxBy not in keys:
        keys.append(maxBy)
    df = pd.DataFrame(columns=["mode"] + varkeys + keys)
    for configuration in next(os.walk(identifier))[1]:
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
                        nodes.append(pd.DataFrame(interestingLines, index=[file.split(".")[0]]))
            tempdfs.append(nodes)
        result: pd.Series
        totalResults: pd.DataFrame
        if maxBy is None:
            totalResults = pd.concat([frame for sublist in tempdfs for frame in sublist])
        else:
            files = [pd.concat(files) for files in tempdfs]
            maxValues = [frame.loc[frame[maxBy].idxmax()] for frame in files]
            totalResults = pd.DataFrame(maxValues)
        result = totalResults.mean()
        ids = pd.Series(varmap)
        df = df.append(pd.DataFrame([ids.append(result)]))
    return df


dirs = ["TuplesPerNode", "NodesPerHostConstant", "NodesPerHostIncreasing", "HostsFixedData", "NetworkPart", "LocalPart",
        "PackageSize", "DataSkew"]
columns = ["JHIST", "JMPI", "JPROC", "SWINALLOC", "SUNSEAL"]

if "graphs" in next(os.walk("."))[1]:
    shutil.rmtree("graphs")
os.mkdir("graphs")


def genGraph(dir: str, cols: List[str], rows: List, varkeys: List[str] = ["Tuples"], filterBy: str = "Tuples",
             nocache: bool = True,
             name: str = None,
             maxBy: str = None,
             normalize: bool = True):
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
    if normalize:
        for column in cols:
            df[column] = df[column] / (df['Tuples'] * df["Hosts"] * df["PerHost"])
    df.set_index(filterBy)
    for column in df.columns:
        if column not in cols + ["mode", filterBy]:
            df.drop(column, axis=1, inplace=True)

    dfs = []
    cacheDF: pd.DataFrame = df.loc[df['mode'] == 'caching'].drop("mode", axis=1).set_index(filterBy)
    dfs.append(cacheDF)
    legend = True
    if nocache and cacheDF.size > 0:
        nocacheDF: pd.DataFrame = df.loc[df['mode'] == 'nocache'].drop("mode", axis=1).set_index(filterBy)
        dfs.append(nocacheDF)
        plot_clustered_stacked(dfs, ["caching", "noncaching"], title="")
        legend = False
    elif not nocache:
        cacheDF.plot(kind="bar", stacked=True, edgecolor="black", linewidth=0.3)
        plt.legend(loc="center left", bbox_to_anchor=(1, 0.5))
    else:
        nocacheDF: pd.DataFrame = df.loc[df['mode'] == 'nocache'].drop("mode", axis=1).set_index(filterBy)
        nocacheDF.plot(kind="bar", stacked=True, edgecolor="black", linewidth=0.3)
        plt.legend(loc="center left", bbox_to_anchor=(1, 0.5))

    # plt.yscale("log")
    # nocacheDF.plot(kind="bar", stacked=True)
    # plt.show()
    # cacheDF.plot(kind="bar", stacked=True)
    plt.tight_layout(pad=2)
    plt.savefig("graphs/" + name + ".png")
    nativeDF: pd.DataFrame = df.loc[df['mode'] == "native"].drop(["mode"], axis=1)
    if "SUNSEAL" in nativeDF.columns:
        nativeDF.drop(["SUNSEAL"], axis=1, inplace=True)
    nativeDF.set_index(filterBy, inplace=True)
    if nativeDF.size > 0:
        nativeDF.plot(kind="bar", stacked=True, edgecolor="black", linewidth=0.3, legend=None)
        #plt.legend(loc="center left", bbox_to_anchor=(1, 0.5))
        plt.tight_layout(pad=2)
        plt.savefig("graphs/" + name + "-native.png")
    print("generated " + name)
    plt.clf()


genGraph("HostsIncreasingData", columns, range(1, 7, 1), ["Hosts", "PerHost", "Tuples"], "Hosts", True, name="HostsIncreasingData 1-6")
genGraph("HostsIncreasingData", columns, range(6, 16, 2), ["Hosts", "PerHost", "Tuples"], "Hosts", True, name="HostsIncreasingData 6-16")
genGraph("TuplesPerNode",
         ["LPHISTCOMP", "LPPART", "BPMEMALLOC", "BPBUILD", "BPPROBE"],
         [1000 * 1000, 5 * 1000 * 1000, 10*1000*1000, 15*1000*1000, 20*1000*1000],
         ["Hosts", "PerHost", "Tuples"],
         "Tuples",
         name="Tuples Per Node Local Processing",
         nocache=False)
genGraph("TuplesPerNode", columns, [1000, 10000, 100 * 1000, 1000 * 1000],
         ["Hosts", "PerHost", "Tuples"],
        "Tuples",
         name="Tuples Small")

genGraph("TuplesPerNode", columns, [1000 * 1000, 5 * 1000 * 1000, 10*1000*1000, 15*1000*1000, 20*1000*1000],
         ["Hosts", "PerHost", "Tuples"],
         "Tuples")
genGraph("NodesPerHostConstant", columns, [1, 2, 4, 8, 16], ["Hosts", "PerHost", "Tuples"], "PerHost")
genGraph("NodesPerHostIncreasing",
         columns,
         [1, 2, 4, 8, 16],
         ["Hosts", "PerHost", "Tuples"],
         "PerHost")

genGraph("PackageSize", columns, [64, 128, 256, 512, 1024, 2048], ["Hosts", "PerHost", "Tuples", "packageSize"],
         "packageSize", False)
genGraph("HostsFixedData", columns, range(2, 16, 3), ["Hosts", "PerHost", "Tuples"], "Hosts", True)
#

genGraph("NetworkPart", columns, range(5, 11), ["Hosts", "PerHost", "Tuples", "NPart"], "NPart")
genGraph("LocalPart", columns, range(5, 11), ["Hosts", "PerHost", "Tuples", "LPart"], "LPart")

genGraph("DataSkew", columns, [1, 2, 3, 4, 5],
         ["Hosts", "PerHost", "Tuples", "ZipfSize", "ZipfFactor"], "ZipfFactor", maxBy="JTOTAL", normalize=False)

genGraph("LocalPart",
         ["LPHISTCOMP", "LPPART", "BPMEMALLOC", "BPBUILD", "BPPROBE"],
         range(5, 11),
         ["Hosts", "PerHost", "Tuples", "LPart"],
         "LPart",
         name="Local Partitioning Local Processing",
         nocache=False)



#genGraph("DataSkewSize", columns, [5*1000, 50*1000, 500*1000, 5*1000*1000], ["Hosts", "PerHost", "Tuples", "packageSize", "ZipfFactor", "ZipfSize"], "ZipfSize",)