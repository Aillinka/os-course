#!/usr/bin/env python3

from datetime import datetime
from pathlib import Path
import time

log_file = Path.home() / "os-course/pr01/clock.log"

while True:
    now = datetime.now().isoformat(timespec="seconds")

    with log_file.open("a") as file:
        file.write(now + "\n")

    print("Recorded:", now, flush=True)
    time.sleep(10)
