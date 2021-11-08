from typing import List

import pandas as pd
import matplotlib.cm as cm
import numpy as np
import matplotlib.pyplot as plt

def plot_clustered_stacked(dfall,
                           labels=None,
                           title="multiple stacked bar plot",
                           H="//",
                           colors: List = None,
                           legend: bool = True,
                           ylabel: str = None,
                           **kwargs):
    """Given a list of dataframes, with identical columns and index, create a clustered stacked bar plot. 
labels is a list of the names of the dataframe, used for the legend
title is a string for the title of the plot
H is the hatch used for identification of the different dataframe"""

    n_df = len(dfall)
    n_col = len(dfall[0].columns) 
    n_ind = len(dfall[0].index)
    axe: plt.Axes = plt.subplot(111)
    x = np.arange(1, n_col + 1)
    width = 0.35

    axe = dfall[0].plot(kind="bar",
                        linewidth=0.3,
                        stacked=True,
                        ax=axe,
                        legend=False,
                        grid=False,
                        position=-width,
                        edgecolor="black",
                        **kwargs)  # make bar plots

    axe = dfall[1].plot(kind="bar",
                        linewidth=0.3,
                        stacked=True,
                        ax=axe,
                        legend=False,
                        grid=False,
                        position=width,
                        edgecolor="black",
                        **kwargs)  # make bar plots

    h,l = axe.get_legend_handles_labels() # get the handles we want to modify
    for i in range(0, n_df * n_col, n_col): # len(h) = n_col * n_df
        for j, pa in enumerate(h[i:i+n_col]):
            for rect in pa.patches: # for each index
                if colors is not None:
                    rect.set_facecolor(colors[j])
                #rect.set_x(rect.get_x() + 1 / float(n_df + 1) * i / float(n_col))
                rect.set_hatch(H * int(i / n_col)) #edited part     
                #rect.set_width(1 / float(n_df + 1))
                rect.set_width(width)

    axe.set_xticks((np.arange(0, 2 * n_ind, 2) + 1 / float(n_df + 1)) / 2.)
    axe.set_xticklabels(dfall[0].index, rotation = 0)
    axe.set_title(title)
    if ylabel is not None:
        axe.set_ylabel(ylabel)

    # Add invisible data to add another legend

    if legend:
        n = []
        for i in range(n_df):
            n.append(axe.bar(0, 0, color="gray", hatch=H * i))
        l1 = axe.legend(h[n_col - 1::-1], l[n_col - 1::-1], loc=[1.01, 0.25])
        if labels is not None:
            l2 = plt.legend(n[::-1], labels[::-1], loc=[1.01, 0.1])
        axe.add_artist(l1)

    return axe