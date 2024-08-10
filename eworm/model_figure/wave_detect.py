# cross correlation analysis of muscle activation
import numpy as np
import matplotlib.pyplot as plt
import os


online_input_path = os.path.join("/home/lifesim/MetaWorm/eworm/model_figure/movement", "online_input.txt")
online_motor_neuron_path = os.path.join("/home/lifesim/MetaWorm/eworm/model_figure/movement", "online_motor_neuron.txt")
online_muscle_path = os.path.join("/home/lifesim/MetaWorm/eworm/model_figure/movement", "online_muscle.txt")

# input_trace = np.array([float(x) for x in open(online_input_path, "r").read().split("\n")])

# motor_neuron_trace = [x.split(",") for x in open(online_motor_neuron_path, "r").read().split("\n")]
# for i in range(len(motor_neuron_trace)):
#     for j in range(len(motor_neuron_trace[0])):
#         motor_neuron_trace[i][j] = 0 if motor_neuron_trace[i][j] == '' else float(motor_neuron_trace[i][j])
# motor_neuron_trace = np.array(motor_neuron_trace)[:,:-1]

muscle_trace = [x.split(",") for x in open(online_muscle_path, "r").read().split("\n")]
for i in range(len(muscle_trace)):
    for j in range(len(muscle_trace[0])):
        muscle_trace[i][j] = 0 if muscle_trace[i][j] == '' else float(muscle_trace[i][j])
muscle_trace = np.array(muscle_trace)[:,:-1]

window = [300,601]
# input_trace = input_trace[window[0]:window[1]]
# motor_neuron_trace = motor_neuron_trace[window[0]:window[1]]
muscle_trace = muscle_trace[window[0]:window[1]]
# print(input_trace.shape, motor_neuron_trace.shape, muscle_trace.shape)

traces = muscle_trace.transpose()
new_traces = []
from scipy import interpolate
x = np.arange(0, len(traces[0]))
new_x = np.arange(0,len(traces[0])-1,0.1)
for i, t in enumerate(traces):
    f = interpolate.interp1d(x,traces[i])
    new_y = f(new_x)
    new_traces.append(new_y)
traces = new_traces


# Function to compute cross-correlation and time delay
def compute_cross_correlation(reference_trace, other_trace):
    # Compute cross-correlation
    cross_corr = np.correlate(reference_trace, other_trace, mode='full')
    # Determine the index of the maximum correlation
    max_corr_index = np.argmax(cross_corr)
    # Calculate the time delay
    time_delay = max_corr_index - (len(reference_trace) - 1)
    return cross_corr, time_delay

# Reference trace for cross-correlation
reference_trace = traces[0]

# Plotting setup
# fig, ax = plt.subplots(24, 1, figsize=(5, 30), sharex=True)
# fig.suptitle('Cross-Correlation Analysis')

cross_corrs = []
interval_value = np.zeros(200)
# Compute and plot cross-correlation for each trace
for i, trace in enumerate(traces):
    cross_corr, time_delay = compute_cross_correlation(reference_trace, trace)
    cross_corrs.append(cross_corr)
    # if i < 24:
    #     ax[i].plot(cross_corr)
    #     ax[i].set_title(f'Trace {i+1} Cross-Correlation, Time Delay: {time_delay}')
    #     ax[i].set_ylabel('Cross-Corr')
    #     ax[i].grid(True)
# plt.xlim([2500, 3500])


fig, ax = plt.subplots(1, 1, figsize=(4, 3), sharex=True, dpi=200)
calc_range = [*range(250*10,300*10,1)]
titles = ['DR', 'VR', 'DL', 'VL']

calc_value = []
for k in range(0,60):
    value = 0
    for f in range(4):
        for i, c in enumerate(calc_range):
            for j, cross_corr in enumerate(cross_corrs[0+f*24:24+f*24]):
                value += cross_corr[int(c+j*k)]
    calc_value.append(value)
calc_value = np.array(calc_value)
print(np.argmax(calc_value)*0.01, np.max(calc_value))
ax.plot(np.arange(0,len(calc_value)*0.01,0.01), calc_value)
ax.plot([0,np.argmax(calc_value)*0.01], [np.max(calc_value)]*2, 'r-.')
ax.plot([np.argmax(calc_value)*0.01]*2, [np.min(calc_value), np.max(calc_value)], 'r-.')
# ax.set_title(titles[f])
ax.set_xlabel('Time Delay (s)')
ax.set_ylabel('Sum(Cross Correlation)')
ax.set_xlim([0,0.5])
ax.set_ylim(np.array([4.8,5.4])*1e6)
ax.grid(True)

fig.tight_layout()
fig.savefig('corss_correlation.png')

# for i in range(200):
#     for i, trace in enumerate(muscle_trace):

# plt.tight_layout(rect=[0, 0, 1, 0.95])
# plt.show()

# demo
data = np.zeros(shape=(5,21))
x = np.linspace(0,2*np.pi,21)
for i, d in enumerate(data):
    # import pdb
    # pdb.set_trace()
    data[i] = np.sin(np.concatenate((x[-i:],x[:-i])))
plt.figure(dpi=200)
plt.imshow(data, cmap='hot')
plt.xlabel('Time')
plt.ylabel('Cross Correlation')
plt.xticks([])
plt.yticks([])
plt.savefig('sumcc_demo.png')