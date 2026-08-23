#!/bin/sh
set -eu

if [ -z "${SOFTADASTRA_BOX_ROOT:-}" ] && [ "$(id -u)" -ne 0 ]; then
  echo "softadastra box installation requires root" >&2
  exit 1
fi

if [ "$#" -ne 1 ] || [ ! -x "$1" ]; then
  echo "usage: $0 /path/to/softadastra" >&2
  exit 2
fi

source_binary=$1
root=${SOFTADASTRA_BOX_ROOT:-}
prefix=$root/opt/softadastra
binary=$prefix/bin/softadastra
unit=$root/etc/systemd/system/softadastra.service
script_directory=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

if [ -e "$binary" ] || [ -e "$unit" ]; then
  echo "existing Softadastra installation found; refusing to overwrite it" >&2
  exit 1
fi

if [ -z "$root" ] && ! id softadastra >/dev/null 2>&1; then
  useradd --system --home-dir /var/lib/softadastra --shell /usr/sbin/nologin softadastra
fi

if [ -z "$root" ]; then
  install -d -o softadastra -g softadastra -m 0750 /var/lib/softadastra
else
  install -d -m 0750 "$root/var/lib/softadastra"
fi
install -d -m 0755 "$prefix/bin"
install -d -m 0755 "$(dirname "$unit")"
install -m 0755 "$source_binary" "$binary"
install -m 0644 "$script_directory/softadastra.service" "$unit"
if [ -z "$root" ]; then
  systemctl daemon-reload
  systemctl enable softadastra.service
  systemctl start softadastra.service
fi
echo "Softadastra Box Host started"
