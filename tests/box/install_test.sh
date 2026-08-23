#!/bin/sh
set -eu

test_root=$(mktemp -d)
trap 'rm -rf "$test_root"' EXIT
test_binary=$test_root/source-softadastra
printf '%s\n' '#!/bin/sh' 'exit 0' > "$test_binary"
chmod 0755 "$test_binary"

SOFTADASTRA_BOX_ROOT=$test_root/target sh box/install.sh "$test_binary"
test -x "$test_root/target/opt/softadastra/bin/softadastra"
test -f "$test_root/target/etc/systemd/system/softadastra.service"

if SOFTADASTRA_BOX_ROOT=$test_root/target sh box/install.sh "$test_binary"; then
  exit 1
fi
