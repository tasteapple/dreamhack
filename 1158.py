import sys

N, K = map(int, sys.stdin.readline().split())

list1 = list(range(1, N + 1))
list2 = []

tmp = 0
while len(list1) != 0:
    tmp += K - 1
    if tmp >= len(list1):
        tmp %= len(list1)
    list2.append(list1[tmp])
    list1.pop(tmp)

print("<", end="")
for i in range(len(list2) - 1):
    print(list2[i], end=", ")
print(list2[-1], end=">")
