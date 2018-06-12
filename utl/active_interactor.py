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

def _find_getch():
    try:
        import Carbon    # try OS/X
        Carbon.Evt       # unix doesn't have this
        def _getch():
            if Carbon.Evt.EventAvail(0x0008)[0]==0: # 0x0008 is the keyDownMask
                return ''
            else:
                (what,msg,when,where,mod)=Carbon.Evt.GetNextEvent(0x0008)[1]
                ch = chr(msg & 0x000000FF)
                if ord(ch) == 3:
                    raise Exception("control-c")
                return ch
        return _getch()
    except:
        pass

    try:
        import msvcrt   # try windows
        def _getch():
            ch = msvcrt.getch
            if ord(ch) == 3:
                raise Exception("control-c")
            return ch
    except:
        pass
    
    # POSIX system. Create and return a getch that manipulates the tty.
    try:
        import sys, tty, termios
        def _getch():
            fd = sys.stdin.fileno()
            old_settings = termios.tcgetattr(fd)
            try:
                tty.setraw(fd)
                ch = sys.stdin.read(1)
            finally:
                termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
                if ord(ch) == 3:
                    raise Exception("control-c")
                return ch

        return _getch
    except:
        pass

    return lambda: raw_input('')
        
def get_label(example,minus1,i,tag,pred, input_fn=raw_input):
    print '\nrequest for example %d: tag="%s", prediction=%g: %s'%(i,tag,pred,example)
    while True:
        label=input_fn('Provide? [0/1/skip]: ')
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
parser.add_argument("-u","--human", help="human readable examples in this file (same # of lines as unlabeled); use with -v")
parser.add_argument("-v","--verbose", action="store_true", help="show example (in addition to tag)")
parser.add_argument("-m","--minus1", action="store_true", help="interpret 0 as -1")
parser.add_argument("-k","--keypress", action="store_true", help="don't require 'Enter' after keypresses")
parser.add_argument("host", help="the machine VW is running on")
parser.add_argument("port", type=int, help="the port VW is listening on")
parser.add_argument("unlabeled_dataset", help="file with unlabeled data")
args = parser.parse_args()

input_fn = raw_input
if args.keypress:
    getch = _find_getch()
    def raw_input_keypress(str):
        print str,
        return getch()
    input_fn = raw_input_keypress

seed=None
if args.seed is not None:
    try:
        seed=open(args.seed,'r')
    except:
        print 'Warning: could not read from %s'%args.seed

try:
    unlabeled=open(args.unlabeled_dataset,'r')
except:
    print 'Error: could not read from %s'%args.unlabeled_dataset
    exit(1)

human=None
if args.human is not None:
    try:
        human = open(args.human, 'r')
    except:
        print 'Warning: could not read human examples from %s' % args.human
    
output=None
if args.output is not None:
    try:
        output=open(args.output,'w')
    except:
        print 'Warning: could not write to %s'%args.output


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
        humanString = line
        if human is not None:
            try:
                humanString = human.next()
            except:
                print 'Warning: out of lines in human data, reverting to vw strings'
                humanString = None
        sock.sendall(line)
        print 'sending unlabeled '+repr(line[:20])
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
        label=get_label(humanString if args.verbose else '\n', args.minus1, i, tag, float(prediction), input_fn)
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
    if human:
        human.close()
