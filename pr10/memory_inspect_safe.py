import os

print("Process ID:", os.getpid())

print("\nVirtual memory regions from /proc/self/maps:")
try:
    with open("/proc/self/maps", "r") as file:
        for i, line in enumerate(file):
            if i == 15:
                break
            print(line.strip())
except OSError as error:
    print("Cannot read maps:", error)

print("\nMemory inspection completed.")
