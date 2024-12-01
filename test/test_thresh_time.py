threshholds = [4, 12, 20, 28, 38, 45, 55, 64, 72, 80, 88, 96, 105, 115]
a = 0
b = 8.474

def new(ticks):
    return round((ticks + a) / b)

def new2(ticks):
    num = (ticks * 10000) / 8474
    if num % 10 > 5:
        return int(num / 10) + 1
    return int(num / 10)

def old(ticks):
    for i in range(len(threshholds)):
        if ticks <= threshholds[i]:
            return i

for i in range(0,115):
    print(f"i={i}, old={old(i)}, new={new(i)}", end="")
    if old(i) != new(i):
        print(" x", end="")
    print()
