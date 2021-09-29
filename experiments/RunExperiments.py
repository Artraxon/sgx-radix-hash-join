import string
import os
import shutil
import subprocess
from typing import Dict, List

#hostlist: List[str] = ["sr630-wn-a-" + (str(i) if i >= 10 else "0" + str(i)) for i in range(1, 61)]
hostlist: List[str] = ["sr250-wn-b-" + (str(i) if i >= 10 else "0" + str(i)) for i in range(1, 16)]
maxHost = 15
perNode = 16
times = 5

def run(name: str, hosts: int, nodesPerHost: int, params: Dict[str, str], times: int, native: bool, suffix: str = ""):
    if name not in next(os.walk("."))[1]:
        os.mkdir(name)

    def execute(mode: str, executable: str, packageSize: int = 0):
        modeName = baseName + "-" + mode
        if modeName.split("/", 1)[1] in next(os.walk(name))[1]:
            shutil.rmtree(modeName)
        os.mkdir(modeName)

        print("Running " + modeName + " next " + str(times) + " times. Done ", end="", flush=True)
        for run in range(1, times + 1):
            previousDirs: List[str] = next(os.walk("."))[1]
            paramsToUse = params.copy()
            if mode == "caching":
                paramsToUse["-p"] = str(packageSize)
            executedProcess = subprocess.run(["/usr/mpi/gcc/openmpi-4.0.3rc4/bin/mpiexec" +
                                              " -H " + ",".join(listTimes(hostlist[:hosts], nodesPerHost)) +
                                              " /home/larose-ldap/sgx/artifacts/" + executable + " " +
                                              " ".join([key + " " + value for key, value in paramsToUse.items()])],
                                             shell=True,
                                             #stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT
                                             )
            newDirs = substractList(next(os.walk("."))[1], previousDirs)
            os.rename(newDirs[0], modeName + "/" + suffix + "Run" + str(run))
            print(str(run) + (", " if run < times else ""), end="", flush=True)
        print("\n", end="")

    baseName = name + "/" + str(hosts) + "x" + str(nodesPerHost) + "-" + ",".join([key + "=" + value for key, value in params.items()])
    execute("nocache", "hashj-app")
    execute("caching", "hashj-app", 64)
    if native:
        execute("native", "cahj-bin")

def substractList(list1: List, list2: List)-> List:
    result = []
    for it in list1:
        if it not in list2:
            result.append(it)
    return result

def runVaried(name: str, hosts: int, nodesPerHost: int, fixedParams: Dict[str, str], key: str, args: List, times, native: bool):
    for arg in args:
        newParams = fixedParams.copy()
        newParams.update({key: str(arg)})
        run(name, hosts, nodesPerHost, newParams, times, native)

def listTimes(list: List, times: int)-> List:
    result = []
    for i in range(0, times):
        result += list
    return result

#Vary Tuples per Node
runVaried("TuplesPerNode", maxHost, 1, {'-p': '128'}, '-t', [1000, 10000, 100000, 1000*1000, 5*1000*10000], times, True)

#Vary Nodes Per Host, with same amount of data per Host
for perHost in range(1,perNode):
    run("NodesPerHostConstant", perHost, {'-p': '128', '-t': str(5*1000*1000//perHost)}, times, True)

#Vary Nodes per Host with increasing amount of data per Host
for perHost in range(1,perNode):
    run("NodesPerHostIncreasing", maxHost, perHost, {'-p': '128', '-t': str(5*1000*1000)}, times, True)

#Vary Hosts with fixed amount of data
for hosts in range(2, maxHost + 1):
    run("HostsFixedData", hosts, 1, {'-p': '128', '-t': str(10*1000*1000//hosts)}, times, True)

#Vary Network Partitioning Fanout
runVaried("NetworkPart", maxHost, 1, {'-p': '128', '-t': str(1000*1000*5)}, '-n', [5, 6, 7, 8, 9, 10, 11, 12], times, True)

#Vary Local Partitioning Fanout
runVaried("LocalPart", maxHost, 1, {'-p': '128', '-t': str(1000*1000*5)}, '-l', [5, 6, 7, 8, 9, 10, 11, 12], times, True)

#Vary Package Size
runVaried("PackageSize", maxHost, 1, {'-t': str(1000*1000*5)}, '-p', [32, 64, 128, 256, 512, 1024, 2048], times, False)

#Vary Data Skew
runVaried("DataSkew", maxHost, 1, {'-t': str(1000*1000*5), '-p': '128', '-s': str(5*1000*1000)}, '-z', [1, 2, 3, 4, 5], times, False)