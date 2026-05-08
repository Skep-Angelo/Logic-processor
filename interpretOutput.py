with open('4bit processor.txt', 'r') as f:
    lines = f.readlines()

filtered_lines = []
filtered_lines.append(lines[0])  # Keep the header line
for line in lines[1:]:
    parts = line.strip().split()
    if float(parts[6]) == 1:
        for all in parts[1:]:
            parts[parts.index(all)] = int(float(all))
        del parts[6]
        filtered_lines.append(' '.join(str(x) for x in parts) + '\n')

with open('output.txt', 'w') as f:
    f.writelines(filtered_lines)

print("Filtered and converted lines have been written back to output.txt")