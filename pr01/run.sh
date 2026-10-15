#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
USER_NAME="$(id -un)"
PYTHON_BIN="$(command -v python3)"

if [[ -z "$PYTHON_BIN" ]]; then
    echo "Python 3 is required"
    exit 1
fi

UNIT_FILE="$(mktemp)"
trap 'rm -f "$UNIT_FILE"' EXIT

cat > "$UNIT_FILE" <<EOF
[Unit]
Description=OS Course Clock Logger
After=local-fs.target

[Service]
Type=simple
User=$USER_NAME
WorkingDirectory=$PROJECT_DIR
Environment=HOME=$HOME
ExecStart=$PYTHON_BIN $PROJECT_DIR/clock.py
Restart=on-failure
RestartSec=3

[Install]
WantedBy=multi-user.target
EOF

cp "$UNIT_FILE" machine-clock.service
sudo install -m 644 "$UNIT_FILE" /etc/systemd/system/machine-clock.service
sudo systemctl daemon-reload
sudo systemctl enable --now machine-clock.service
sudo systemctl restart machine-clock.service

echo "Status: $(systemctl is-active machine-clock.service)"
echo "Autostart: $(systemctl is-enabled machine-clock.service)"
