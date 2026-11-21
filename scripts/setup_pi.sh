#!/usr/bin/env bash
# Run once on a fresh Raspberry Pi OS (64-bit) to make it a HIL test agent.
#   bash scripts/setup_pi.sh
set -euo pipefail

sudo apt-get update
sudo apt-get install -y git build-essential cmake ninja-build \
    gcc-arm-none-eabi libnewlib-arm-none-eabi \
    python3-pytest python3-serial python3-lgpio stlink-tools

# Serial port and GPIO access without sudo (default user usually has these)
sudo usermod -aG dialout,gpio "$USER"

echo
echo "Checking the Nucleo (plug it into a Pi USB port first):"
ls /dev/ttyACM* || echo "  no /dev/ttyACM* found: is the Nucleo plugged in?"
st-info --probe || true
echo
echo "Done. Log out and back in once, so the group changes apply."