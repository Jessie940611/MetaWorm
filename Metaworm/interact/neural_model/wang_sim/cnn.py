import torch
import torch.nn as nn


class CNN(nn.Module):
    def __init__(self):
        super(CNN, self).__init__()
        self.conv1 = nn.Conv1d(
                in_channels=80,              # input height
                out_channels=96,             # n_filters
                kernel_size=480,             # filter size
                stride=20,                   # filter movement/step
                padding=230
        ) # output shape (bs, 96, 300)

    def forward(self, x):
        output = self.conv1(x)
        return output


class CNN2(nn.Module):
    def __init__(self):
        super(CNN2, self).__init__()
        self.conv1 = nn.Conv1d(
                in_channels=80,              # input height
                out_channels=96,             # n_filters
                kernel_size=21,              # filter size
        ) # output shape (bs, 96, 300)

    def forward(self, x):
        output = self.conv1(x)
        return output

