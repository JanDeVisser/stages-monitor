import io

s = """f4 01
4f 02
ab 02
07 03
63 03
bf 03
1b 04
76 04
d2 04
2e 05
8a 05
e6 05
42 06
9d 06
f9 06
55 07
b1 07
0d 08
69 08
c4 08
20 09
7c 09
d8 09
34 0a
90 0a
eb 0a
47 0b
a3 0b
ff 0b
5b 0c
b7 0c
12 0d
6e 0d
ca 0d
26 0e
82 0e
de 0e
39 0f
95 0f
f1 0f
4d 10
a9 10
05 11
60 11
bc 11
18 12
74 12
d0 12
2c 13
88 13"""

with io.StringIO(s) as sb:
    oldval = 0
    line = sb.readline()
    while line:
        x = line.strip().split(" ")
        val = 256*int(x[1], 16) + int(x[0], 16)
        print("| `{0} {1}` | {2} | {3}".format(x[0], x[1], val, val - oldval))
        oldval = val
        line = sb.readline()

