
from sys import argv
import matplotlib.pyplot as plt
import matplotlib.patches as ptch
import numpy as np

colors = ["blue", "orange", "green", "yellow", "purple"]

def np_array(*vals):
    ret = np.zeros([len(vals)])
    ret[:] = vals
    return ret

def set_legend(*labels):
    plt.legend(handles=[ptch.Patch(label=item, color=colors[i]) for i, item in enumerate(labels, 0)])

def stacked_chart(width, xticks, *category_vals):
    bottom = np.zeros([len(xticks)])
    for i, vals in enumerate(category_vals):
        plt.bar(xticks, vals, width=width, color=colors[i], bottom=bottom)
        bottom += vals
        

if argv[1] == "mem":

    width = 0.4
    hwidth = width / 2 + 0.025
    
    # x-axis tick values
    xticks = np.ndarray([2,2])
    xticks[:,0] = np.arange(2) - hwidth
    xticks[:,1] = np.arange(2) + hwidth
    xticks = xticks.reshape(4)
    
    # legend
    set_legend("VCs")
    
    # parameters
    vc_entry_size_base = 64 + (1 + 1 + 1 + 32 + 32) + 4 # 64b data + 35b link ctrl + 4 output directions (one-hot)
    vc_entry_size_redu = 64 + (1 + 1 + 1 + 32 + 32 + 1) + 4 # 64b data + 36b link_ctrl + 4 output directions (one-hot)
    vc_n_entries = 2 ** 3 # 8 entries in a VC
    router_dir_n_vc = 4 # 4 VCs per router direction
    router_n_dirs_base = 5 + 5 # 5 RW ports
    router_n_dirs_redu = 5 + 7 # 5 RW ports, 2 RO ports
    
    # values to plot
    vc_size = np_array(router_n_dirs_base * router_dir_n_vc * vc_n_entries * vc_entry_size_base, router_n_dirs_redu * router_dir_n_vc * vc_n_entries * vc_entry_size_redu, 0, 0)
    stacked_chart(width, xticks, vc_size)
    
    # extended x-axis tick values
    xticks = np.ndarray([2,3])
    xticks[:,0] = np.arange(2) - hwidth
    xticks[:,1] = np.arange(2)
    xticks[:,2] = np.arange(2) + hwidth
    xticks = xticks.reshape(6)
    
    # labels
    tick_labels = ["typical", "\nrouter", "redundant", "typical", "\nadapter", "redundant"]
    plt.xticks(xticks, tick_labels)
    plt.xlabel("Component and mode")
    plt.ylabel("Memory size (bits)")
    plt.title("Memory size for different components")
    
elif argv[1] == "latency":

    width = 0.4
    hwidth = width / 2 + 0.025
    
    # x-axis tick values
    xticks = np.ndarray([3,2])
    xticks[:,0] = np.arange(3) - hwidth
    xticks[:,1] = np.arange(3) + hwidth
    xticks = xticks.reshape(6)
    
    # legend
    set_legend("Typical NoC", "Custom NoC")
    
    # values to plot
    latency_base = np_array(3583.33, 0.0, 64625.0, 0.0, 180000.0, 0.0) / 1000.0
    latency_redu = np_array(0.0, 3583.33, 0.0, 10000.0, 0.0, 58000.0) / 1000.0
    stacked_chart(width, xticks, latency_base, latency_redu)
    
    # extended x-axis tick values
    xticks = np.arange(3)
    
    # labels
    tick_labels = ["command packets", "response packets", "total"]
    plt.xticks(xticks, tick_labels)
    plt.yticks(np.arange(0, 201, 20))
    plt.xlabel("Latency measure by mode")
    plt.ylabel("Average latency (clock cycles)")
    plt.title("Latency of each system")

    pass
    
elif argv[1] == "functionality":

    # plot injected faults vs first non-majority detected and error comparisons

    pass
    
else:
    print("Invalid mode")
    
plt.show()
