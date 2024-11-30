threshholds = [4, 12, 20, 28, 38, 45, 55, 64, 72, 80, 88, 96, 105, 115]

def new(ticks, a, b):
    return round((ticks + a) / b)

def old(ticks):
    for i in range(len(threshholds)):
        if ticks <= threshholds[i]:
            return i

# Initialize variables
best_b = None
min_mismatches = float('inf')  # Start with a large number
b_candidates = []  # List to store b values with minimum mismatches

# Brute force search
for l in range(80000, 90000):  # Iterate over b values
    a = 0.0
    b = l / 10000
    mismatches = []
    for i in range(0, 115):  # Check all ticks
        if old(i) != new(i, a, b):
            mismatches.append(i)  # Collect mismatched ticks

    mismatch_count = len(mismatches)

    if mismatch_count < min_mismatches:  # Found a new minimum
        min_mismatches = mismatch_count
        b_candidates = [b]  # Start a new list
    elif mismatch_count == min_mismatches:  # Found another candidate
        b_candidates.append(b)

# Find the middle b value (median of b_candidates)
b_candidates.sort()
middle_index = len(b_candidates) // 2
middle_b = b_candidates[middle_index]

print(f"\nMiddle b: {middle_b} with {min_mismatches} mismatches")
