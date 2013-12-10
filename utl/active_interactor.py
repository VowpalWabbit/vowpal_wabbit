import socket
import sys
import argparse

#readline is under GPL licensing
#import readline

def recvall(s, n):
    buf=s.recv(n)
    ret=len(buf)
    while ret>0 and len(buf)<n:
        if buf[-1]=='\n': break
        tmp=s.recv(n)
        ret=len(tmp)
        buf=buf+tmp
    return buf

def get_label(example,minus1,i,tag,pred):
    print '\nrequest for example %d: tag="%s", prediction=%g: %s'%(i,tag,pred,example)
    while True:
        label=raw_input('Provide? [0/1/skip]: ')
        if label == '1':
            break
        if label == '0':
            if minus1: label='-1'
            break
        if 0 < len(label) <= len('skip') and 'skip'.startswith(label):
            label = None
            break
    return label

parser = argparse.ArgumentParser(description="interact with VW in active learning mode")
parser.add_argument("-s","--seed", help="seed labeled dataset")
parser.add_argument("-o","--output", help="output file")
parser.add_argument("-v","--verbose", action="store_true", help="show example (in addition to tag)")
parser.add_argument("-m","--minus1", action="store_true", help="interpret 0 as -1")
parser.add_argument("host", help="the machine VW is running on")
parser.add_argument("port", type=int, help="the port VW is listening on")
parser.add_argument("unlabeled_dataset", help="file with unlabeled data")
args = parser.parse_args()

seed=None
if args.seed is not None:
    try:
        seed=open(args.seed,'r')
    except:
        print 'Warning: could not read from %s'%args.seed
        pass

try:
    unlabeled=open(args.unlabeled_dataset,'r')
except:
    print 'Error: could not read from %s'%args.unlabeled_dataset
    exit(1)

output=None
if args.output is not None:
    try:
        output=open(args.output,'w')
    except:
        print 'Warning: could not write to %s'%args.output
        pass


# Create a socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

queries=0
try:
    # Connect to server and perform handshake

    print 'connecting to %s:%d ...'%(args.host,args.port)
    sock.connect((args.host, args.port))
    print 'done'
    # Send seed dataset
    if seed:
        print 'seeding vw ...'
        for line in seed:
            sock.sendall(line)
            recvall(sock, 256)
        print 'done'
    # Send unlabeled dataset
    print 'sending unlabeled examples ...'
    for i,line in enumerate(unlabeled):
        sock.sendall(line)
        #print 'sending unlabeled '+repr(line[:20])
        response=recvall(sock, 256)
        #print 'unlabeled response '+repr(response)
        responselist=response.split(' ')
        if len(responselist)==2:
            #VW does not care about this label
            continue
        prediction,tag,importance=responselist
        try:
            imp=float(importance)
        except:
            continue
        queries+=1
        label=get_label(line if args.verbose else '\n', args.minus1, i, tag, float(prediction))
        if label is None:
            continue
        front,rest=line.split('|',1)
        if tag=='':
            tag="'empty"
        labeled_example=label+' '+"%g"%imp+' '+tag+' |'+rest
        #print 'sending labeled '+repr(labeled_example[:20])
        sock.sendall(labeled_example)
        if output:
            output.write(labeled_example)
            output.flush()
        recvall(sock, 256)
finally:
    sock.close()
    unlabeled.close()
    if output:
        output.close()
    if seed:
        seed.close()
