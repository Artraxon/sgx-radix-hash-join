import os
from typing import List
from ClusterGraphs import plot_clustered_stacked
import matplotlib.pyplot as plt
import matplotlib as mpl
import pandas as pd

def loadData(identifier: str, keys: List[str], varkeys: List[str]) -> pd.DataFrame:
    df = pd.DataFrame(columns = ["mode"] + varkeys + keys)
    for configuration in next(os.walk(identifier))[1]:
        mode = configuration.split("-")[-1]
        varparams = configuration.split(",")[-len(varkeys):]
        varparams[-1] = varparams[-1].split("-")[1]
        varparams = [it.split("=")[1] for it in varparams]
        varmap = dict(zip(varkeys, varparams))
        varmap["mode"] = mode
        tempdfs: List[pd.DataFrame] = []
        for run in next(os.walk(identifier + "/" + configuration))[1]:
            for file in next(os.walk(identifier + "/" + configuration + "/" + run))[2]:
                if file.endswith("perf"):
                    with open(identifier + "/" + configuration + "/" + run + "/" + file) as f:
                        interestingLines = {}
                        for line in f:
                            for key in keys:
                                if line.startswith(key):
                                    splitted = line.split()
                                    interestingLines[splitted[0]] = [float(splitted[1])]
                        tempdfs.append(pd.DataFrame(interestingLines))
        totalResults = pd.concat(tempdfs)
        means = totalResults.mean()
        ids = pd.Series(varmap)
        df = df.append(pd.DataFrame([ids.append(means)]))
    print("hi")
    return df


df = loadData("TuplesPerNode", ["JTOTAL", "JHIST", "JMPI", "JPROC", "SWINALLOC", "SUNSEAL"], ["Tuples"])
df.sort_values(by=["Tuples"], inplace=True)
nocacheDF = df.loc[df['mode'] == 'nocache']
nativeDF = df.loc[df['mode'] == "native"]
plot_clustered_stacked([nocacheDF, nativeDF], ["nocache", "cache"])
plt.yscale("log")
plt.show()

# fig, ax = plt.subplots()
# ax: plt.Axes = ax
# width = 0.35
# ax.bar(tupleDF.Tuples, tupleDF.SUNSEAL, width, label='Unsealing')
# ax.bar(tupleDF.Tuples, tupleDF.JHIST, width, label='Histogram', bottom=tupleDF.SUNSEAL)
# ax.bar(tupleDF.Tuples, tupleDF.SWINALLOC, width, label='Window Allocation', bottom=tupleDF.JHIST)
# ax.bar(tupleDF.Tuples, tupleDF.JMPI, width, label='Network Partitioning', bottom=tupleDF.SWINALLOC)
# ax.bar(tupleDF.Tuples, tupleDF.JPROC, width, label='Local Partitioning', bottom=tupleDF.JMPI)
# ax.legend()
# plt.yscale("log")
# plt.show()