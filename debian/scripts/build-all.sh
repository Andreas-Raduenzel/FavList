#!/usr/bin/env bash
set -euo pipefail

PKG="favlist"

# Welche Distros bauen?
# Format: DISTRO  SUFFIX              MESSAGE
build_one() {
  local dist="$1"
  local suffix="$2"
  local msg="$3"

  echo
  echo "============================================================"
  echo "=== Target: $dist ($suffix) ==="
  echo "============================================================"

  # Changelog sichern
  local chl_backup
  chl_backup="$(mktemp)"
  cp -a debian/changelog "$chl_backup"

  # Basis-Version (z.B. 0.1.1-1) aus aktuellem changelog
  local basever
  basever="$(dpkg-parsechangelog -SVersion)"

  # Zielversion (z.B. 0.1.1-1~deb12u1)
  local newver="${basever}${suffix}"

  # Achtung: ~ macht Version "kleiner" als basever -> daher -b erzwingen
  dch -b --distribution "$dist" --newversion "$newver" "$msg"

  echo "==> Changelog-Version jetzt: $(dpkg-parsechangelog -SVersion)"

  # Source-Paket bauen (ohne Signatur!)
  dpkg-buildpackage -S -us -uc

  # sbuild aus .dsc bauen
  local parent
  parent="$(cd .. && pwd)"
  local dsc="${parent}/${PKG}_${newver}.dsc"
  [[ -f "$dsc" ]] || { echo "DSC fehlt: $dsc"; exit 1; }

  sbuild -d "$dist" "$dsc"

  # Artefakte einsammeln
  mkdir -p "dist/${dist}"
  shopt -s nullglob

  for f in \
    "${parent}/${PKG}_${newver}"*.deb \
    "${parent}/${PKG}_${newver}"*.buildinfo \
    "${parent}/${PKG}_${newver}"*.changes \
    "${parent}/${PKG}_${newver}.dsc" \
    "${parent}/${PKG}_${newver}.debian.tar."* \
    "${parent}/${PKG}_"*.orig.tar.* \
  ; do
    [[ -e "$f" ]] && mv -f "$f" "dist/${dist}/"
  done

  shopt -u nullglob

  # Changelog zurücksetzen
  mv -f "$chl_backup" debian/changelog

  echo "==> Fertig: dist/${dist}/"
}

# ------------------------------------------------------------
# 0) Sicherstellen, dass orig tarball existiert
#    (damit dpkg-source bei -S nicht meckert)
# ------------------------------------------------------------
UPSTREAM_VER="$(dpkg-parsechangelog -SVersion | sed 's/-.*//')"   # 0.1.1 aus 0.1.1-1
PARENT_DIR="$(cd .. && pwd)"
ORIG="${PARENT_DIR}/${PKG}_${UPSTREAM_VER}.orig.tar.xz"

if [[ ! -f "$ORIG" ]]; then
  echo "==> orig tarball fehlt, erstelle: $(basename "$ORIG")"
  # packe aktuellen Tree als "favlist-<version>/" - ohne debian/ und ohne build/obj-*
  tar -cJf "$ORIG" \
    --exclude='./debian' \
    --exclude='./build' \
    --exclude='./obj-*' \
    --exclude='./.git' \
    --exclude='./.cache' \
    --transform="s,^,${PKG}-${UPSTREAM_VER}/," \
    .
else
  echo "==> orig tarball vorhanden: $(basename "$ORIG")"
fi

# ------------------------------------------------------------
# 1) Builds
# ------------------------------------------------------------
build_one "bookworm" "~deb12u1"        "Build for Debian 12 (bookworm)."
build_one "trixie"   "~deb13u1"        "Build for Debian 13 (trixie)."
build_one "jammy"    "~ubuntu22.04.1"  "Build for Ubuntu 22.04 (jammy)."
build_one "noble"    "~ubuntu24.04.1"  "Build for Ubuntu 24.04 (noble)."

echo
echo "=== ALL BUILDS FINISHED ==="
echo "Pakete liegen unter:"
echo "  dist/bookworm/"
echo "  dist/trixie/"
echo "  dist/jammy/"
echo "  dist/noble/"
