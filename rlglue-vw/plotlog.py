import csv,numpy,os
import matplotlib.pyplot as plt

os.system('tail +4 log.dat > locations.dat')
#os.system('awk \'BEGIN{FS="|"}; {print $1":"$2}\' log.dat | awk \'{print $1":"$2":"$3}\' | awk \'BEGIN{FS=":"} ; {if($4 != "") print $4","$6","$1}\' > locations.dat')
data = []
with open('locations.dat', 'rb') as f:
    csvr = csv.reader(f)
    for row in csvr:
        try:
            data += [map(float, row)]
        except:
            pass

data = numpy.array(data)
#plt.scatter(data[:,0], data[:,1])
plt.hexbin(data[:,1], data[:,2], bins='log')
plt.savefig("locations.png")

plt.clf()
index = 10000
plt.hexbin(data[index:,1], data[index:,2], C=data[index:,0])
plt.savefig("values.png")

