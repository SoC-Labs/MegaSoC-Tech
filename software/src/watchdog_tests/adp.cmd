A
a 0x00800000
a
w byte-0 write test 0xb0
a
w byte-1 write test 0xb1
a
w half-1 write test 0xb3b2
a
w word write test 0xb7b6b5b4
a
a 0x00800000
a
r read verify as bytes x 8 0x8
a
a 0x00800000
a
r verify as half-words x 4 0x004
a
a 0x00800000
a
r verify as words x 2 0x000002
a
a 0x00800000
a
r
r
a
u 4
abcd
a
u 0
a
u

! bad command


a 0x00800000

r 000003
a 0x00800000
W
a 0x00800003
r 0000003
a 000800003
r 003
a 0x00800003
r 3


S STDIN push 0x41
S STDIN blocked 0x42
S 0x43
A 0
R 00000004
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
S STDIN push 0x58
X
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
