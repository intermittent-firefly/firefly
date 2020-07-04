import sys
import matplotlib.pyplot as plt

v = []

path = sys.argv[1]
f = open(path, "r")
for line in f:
    v.append(float(line))
f.close()

x = range(1, len(v)+1)
print(x)
plt.plot(x,v)
plt.savefig(sys.argv[2])