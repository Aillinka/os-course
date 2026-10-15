PAGE_SIZE = 1024
NUM_PAGES = 16

page_table = {
    0: 5,
    1: 2,
    2: 7,
    3: 0,
    4: None,
    5: 4,
    6: 1,
    7: 6,
    8: None,
    9: 3,
    10: None,
    11: None,
    12: None,
    13: None,
    14: None,
    15: None
}


def translate(logical_address):
    page = logical_address // PAGE_SIZE
    offset = logical_address % PAGE_SIZE

    if page >= NUM_PAGES:
        return None

    frame = page_table.get(page)

    if frame is None:
        return None

    physical_address = frame * PAGE_SIZE + offset
    return page, offset, frame, physical_address


def main():
    print("PRACTICAL WORK 10 - VARIANT A")
    print("=" * 45)

    try:
        with open("refs.txt", "r", encoding="utf-8") as file:
            addresses = [int(line.strip()) for line in file if line.strip()]
    except FileNotFoundError:
        print("Error: refs.txt not found")
        return

    for address in addresses:
        result = translate(address)

        if result is None:
            print(f"Logical address {address}: PAGE NOT PRESENT")
        else:
            page, offset, frame, physical = result
            print(
                f"Logical={address:5} | Page={page:2} | "
                f"Offset={offset:4} | Frame={frame:2} | "
                f"Physical={physical}"
            )


if __name__ == "__main__":
    main()
