import torch
import torch.nn as nn

input = torch.rand(64, 64).half().cuda()
mask = torch.Tensor([0, 0, 1, 1]).tile((64, 16)).cuda().bool()
linear = nn.Linear(64, 64).half().cuda()
linear.weight = nn.Parameter(nn.utils.to_sparse_coo(linear.weight.masked_fill(~mask, 0)))