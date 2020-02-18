import numpy as np
import math
import gzip
import os

n = 1000000 # number of samples
d = 5 # dimensionality of data
sig = 0.2 # (Gaussian) noise rate

'''
Saving the data matrix in vw format
'''
def save_vw_reg_dataset(X, Y, ds_dir):
    n, d = np.shape(X)
    fname = 'ds_synthetic_{}_{}.vw.gz'.format(n, d)
    with gzip.open(os.path.join(ds_dir, fname), 'w') as f:
            for i in range(n):

                st ='{} | {}\n'.format(Y[i], ' '.join(
                    '{}:{:.6f}'.format(j, val) for j, val in zip(range(1,d+1), X[i].data)))

                #print(st)

                f.write(st.encode())

'''
Synthetic dataset 1: w is roughly a unit vector, x_i's are standard normal
(Y is in the range [-10, +10] with high probability)
'''
w = np.random.randn(d) / math.sqrt(d)
X = np.random.randn(n, d) + d
Y = X.dot(w) + sig*np.random.randn(n)
print(Y.min(), Y.max())
#print(X, Y)

'''
Synthetic dataset 2: w is a vector of unit ell_1 norm, x_i's are in the box [-1,+1]^d
'''
#w = np.random.dirichlet(np.ones(d)) # uniform distribution over the simplex
#X = (2*np.random.randint(0,2,size=(n,d)) - 1) (option 1: x_i only lies in the corner)
#X = 2*np.random.rand(n,d) - 1 # (option 2: x_i lies in the interior)
#Y = X.dot(w) + sig*np.random.randn(n)
#print(Y.min(), Y.max())

save_vw_reg_dataset(X, Y, '.')
