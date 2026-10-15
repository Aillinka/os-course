import mmap
import os
import ctypes

PAGE_SIZE = os.sysconf("SC_PAGE_SIZE")


def inspect_maps():
    print("\n--- /proc/self/maps ---")

    with open("/proc/self/maps", "r", encoding="utf-8") as file:
        for line in file.readlines()[:12]:
            print(line.rstrip())


def inspect_pagemap():
    print("\n--- /proc/self/pagemap ---")

    value = ctypes.c_int(123)
    virtual_address = ctypes.addressof(value)

    page_number = virtual_address // PAGE_SIZE
    offset = page_number * 8

    try:
        with open("/proc/self/pagemap", "rb") as file:
            file.seek(offset)
            entry = int.from_bytes(file.read(8), "little")

        present = (entry >> 63) & 1
        swapped = (entry >> 62) & 1
        pfn = entry & ((1 << 55) - 1)

        print(f"Virtual address: {hex(virtual_address)}")
        print(f"Page size: {PAGE_SIZE} bytes")
        print(f"Page present: {present}")
        print(f"Page swapped: {swapped}")

        if present and pfn != 0:
            print(f"Physical frame number: {pfn}")
        elif present:
            print("PFN is hidden by the Linux kernel or unavailable.")
        else:
            print("Page is not currently present in RAM.")

    except PermissionError:
        print("Permission denied when reading pagemap.")


def shared_mapping():
    print("\n--- Shared memory mapping ---")

    filename = "shared_memory.bin"

    with open(filename, "wb") as file:
        file.write(b"\0" * PAGE_SIZE)

    with open(filename, "r+b") as file:
        with mmap.mmap(
            file.fileno(),
            PAGE_SIZE,
            access=mmap.ACCESS_WRITE
        ) as mapped:
            mapped[0:18] = b"Shared memory test"
            print("Written:", mapped[0:18].decode())
            print("Mapping size:", len(mapped))

    os.remove(filename)


def main():
    print("PRACTICAL WORK 10 - VARIANT C")
    print("=" * 45)

    inspect_maps()
    inspect_pagemap()
    shared_mapping()


if __name__ == "__main__":
    main()
