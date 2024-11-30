threshholds = [4, 12, 20, 28, 38, 45, 55, 64, 72, 80, 88, 96, 105, 115]
a = 0
b = 8.4734

def new(ticks):
    return round((ticks + a) / b)

def new2(ticks):
    num = (ticks * 100000) / 84734
    if num % 10 > 5:
        return int(num / 10) + 1
    return int(num / 10)

def old(ticks):
    for i in range(len(threshholds)):
        if ticks <= threshholds[i]:
            return i

for i in range(0,115):
    print(f"i={i}, old={old(i)}, new={new2(i)}", end="")
    if old(i) != new2(i):
        print(" x", end="")
    print()
