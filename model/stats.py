
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
    return np.amax(bottom)

if argv[1] == "mem":

    width = 0.4
    hwidth = width / 2 + 0.025

    # x-axis tick values
    xticks = np.ndarray([3,2])
    xticks[:,0] = np.arange(3) - hwidth
    xticks[:,1] = np.arange(3) + hwidth
    xticks = xticks.reshape(6)

    # legend
    set_legend("VCs*", "CRC synchronizer**")

    # VC parameters
    vc_entry_size_base = 64 + (1 + 1 + 1 + 32 + 32) + 4 # 64b data + 35b link ctrl + 4 output directions (one-hot)
    vc_entry_size_redu = 64 + (1 + 1 + 1 + 32 + 32 + 1) + 4 # 64b data + 36b link_ctrl + 4 output directions (one-hot)
    vc_n_entries = 2 ** 3 # 8 entries in a VC
    router_dir_n_vc = 3 # 4 VCs per router direction
    router_n_dirs_base = 5 + 5 # 5 RW ports
    router_n_dirs_redu = 5 + 7 # 5 RW ports, 2 RO ports
    vc_size = np_array(router_n_dirs_base * router_dir_n_vc * vc_n_entries * vc_entry_size_base, router_n_dirs_redu * router_dir_n_vc * vc_n_entries * vc_entry_size_redu, 0, 0, 0, 0)

    # CRC parameters
    chkpt_size_pkts = 4 # num packets/checkpoint
    chkpt_size_bits = chkpt_size_pkts * 64 # num bits/checkpoint
    n_chkpts = 128 * 8 / chkpt_size_bits # num checkpoints
    crc_size_bits = 3 * (32 * n_chkpts + 32 + chkpt_size_bits)
    crc_size = np_array(0, 0, 0, crc_size_bits, crc_size_bits, 0.0)

    # values to plot
    max_val = stacked_chart(width, xticks, vc_size, crc_size)

    # extended x-axis tick values
    xticks = np.ndarray([3,3])
    xticks[:,0] = np.arange(3) - hwidth
    xticks[:,1] = np.arange(3)
    xticks[:,2] = np.arange(3) + hwidth
    xticks = xticks.reshape(9)

    # labels
    tick_labels = ["typical", "\nrouter", "redundant", "typical", "\nadapter", "redundant", "typical", "\napplication", "redundant"]
    plt.xticks(xticks, tick_labels)
    plt.yticks(np.arange(0, max_val+4999, 5000))
    plt.xlabel("Component and mode")
    plt.ylabel("Memory size (bits)")
    plt.title(f"*4 VCs per router, each with 8 entries\n**For {int(n_chkpts)} checkpoints, each with {chkpt_size_pkts} packets ({chkpt_size_bits}b)")
    plt.suptitle("Memory size for different components",fontsize=16, y=1)

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
    factors = latency_base[::2] / latency_redu[1::2]
    stacked_chart(width, xticks, latency_base, latency_redu)
    print(factors)

    # extended x-axis tick values
    xticks = np.arange(3)

    # labels
    tick_labels = ["command packets", "response packets", "total"]
    plt.xticks(xticks, tick_labels)
    plt.yticks(np.arange(0, 201, 20))
    plt.xlabel("Latency measure by mode")
    plt.ylabel("Latency (clock cycles)")
    plt.title("Latency of each system")

    pass

elif argv[1] == "functionality":

    # plot injected faults vs first non-majority detected and error comparisons

    pass

else:
    print("Invalid mode")

plt.show()
