from cProfile import label
import numpy as np
import array
import matplotlib.pyplot as plt

pos_file = "/media/ningwang/D/ReinforcementLearning/AI4Animation/AI4Animation/SIGGRAPH_2022/PyTorch/Dataset/WormMonoPos2_.bin"
vel_file = "/media/ningwang/D/ReinforcementLearning/AI4Animation/AI4Animation/SIGGRAPH_2022/PyTorch/Dataset/WormMonoVel2_.bin"
frames = 121
feature_dim = 51*121
yRange = 0.5

def FunctionsPos(ax, values, xmin, xmax, ymin, ymax, colors=None, title=None):
    ax.cla()
    idx = 0
    args = np.linspace(xmin, xmax, values.shape[1])
    for i in range(values.shape[0]):
        if colors != None:
            ax.plot(args, values[i,:], color=colors[idx])
        else:
            p = values[i,:]+(i*(ymax-ymin)/values.shape[0]+ymin)*0.8
            ax.plot(p)
            # ax.annotate('%d'%(i+1), xy=(xmin, p[0]), xycoords='data', xytext=(-40, 0), textcoords='offset points', fontsize=10, arrowprops=dict(arrowstyle="->", connectionstyle="arc3,rad=.2"))
    idx += 1
    ax.set_ylim(-0.7, 0.7)
    if title != None:
        ax.set_ylabel(title, fontsize=16)

    ax.axes.xaxis.set_visible(True)
    ax.axes.yaxis.set_visible(True)
    ax.set_yticks([])
    if "Position" in title:
        ax.set_xlabel("Time Step", fontsize=18)
        ax.set_xticks(ticks=np.arange(0,121,20))
        ax.set_xticklabels([str(int(x)) for x in np.arange(0,121,20)], fontsize=15)
    else:
        ax.set_xticks([])
    

def FunctionsVel(ax, values, xmin, xmax, ymin, ymax, colors=None, title=None):
    ax.cla()
    idx = 0
    args = np.linspace(xmin, xmax, values.shape[1])
    for i in range(values.shape[0]):
        if colors != None:
            ax.plot(args, values[i,:], color=colors[idx])
        else:
            ax.plot(args, values[i,:])
    idx += 1
    ax.set_ylim(-1.7, 1.7)
    ax.set_ylabel(title, fontsize=16)

    ax.axes.xaxis.set_visible(True)
    ax.axes.yaxis.set_visible(True)
    ax.set_yticks([])
    if "Velocity" in title:
        ax.set_xlabel("Time Step", fontsize=18)
        ax.set_xticks(ticks=np.arange(0,121,20))
        ax.set_xticklabels([str(int(x)) for x in np.arange(0,121,20)], fontsize=15)
    else:
        ax.set_xticks([])

#binaryFile = .bin data matrix of shape samples x features
#sampleIndices = list of sample indices from 0
#featureCount = number of features per sample
def ReadBatch(binaryFile, sampleIndices, featureCount):
    bytesPerLine = featureCount*4
    data = []
    with open(binaryFile, "rb") as f:
        for i in sampleIndices:
            f.seek(i*bytesPerLine)
            bytes = f.read(bytesPerLine)
            data.append(np.float32(array.array('f', bytes)))
    return np.concatenate(data).reshape(len(sampleIndices), -1)
    # Example:
    # batchSize = 32
    # samples = 100
    # features = 2904
    # batch = ReadBatch("Input.bin", np.random.randint(samples, size=batchSize), features)

# pos = ReadBatch(pos_file, np.array([0]), feature_dim)
# vel = ReadBatch(vel_file, np.array([0]), feature_dim)


# _, ax_pos = plt.subplots(3,1,figsize=(13,5), dpi=200)
# FunctionsPos(ax_pos[0], pos.reshape(51, frames)[0::3,:], -1.0, 1.0, -yRange, yRange, title="Position X")
# FunctionsPos(ax_pos[1], pos.reshape(51, frames)[1::3,:], -1.0, 1.0, -yRange, yRange, title="Position Y")
# FunctionsPos(ax_pos[2], pos.reshape(51, frames)[2::3,:], -1.0, 1.0, -yRange, yRange, title="Position Z")
# plt.tight_layout()
# for ax in ax_pos:
#     for key, spine in ax.spines.items():
#         if key in ['left', 'right', 'bottom', 'top']:
#             spine.set_visible(False)
# plt.savefig("/media/ningwang/D/ReinforcementLearning/AI4Animation/AI4Animation/SIGGRAPH_2022/PyTorch/Dataset/pos.png")


# _, ax_vel = plt.subplots(3,1,figsize=(13,5), dpi=200)
# FunctionsVel(ax_vel[0], vel.reshape(51, frames)[0::3,:], 0, 121, -yRange, yRange, title="Velocity X")
# FunctionsVel(ax_vel[1], vel.reshape(51, frames)[1::3,:], 0, 121, -yRange, yRange, title="Velocity Y")
# FunctionsVel(ax_vel[2], vel.reshape(51, frames)[2::3,:], 0, 121, -yRange, yRange, title="Velocity Z")
# plt.tight_layout()
# for ax in ax_vel:
#     for key, spine in ax.spines.items():
#         if key in ['left', 'right', 'bottom', 'top']:
#             spine.set_visible(False)
# plt.savefig("/media/ningwang/D/ReinforcementLearning/AI4Animation/AI4Animation/SIGGRAPH_2022/PyTorch/Dataset/vel.png")

# jjj = 0