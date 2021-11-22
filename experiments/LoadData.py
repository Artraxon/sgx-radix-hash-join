import os
import re
from typing import List
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
