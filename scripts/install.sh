#!/usr/bin/env sh

#
# This script will install the latest pre-built yabai release from GitHub.
# Depends on curl, shasum, tar, cp, cut.
#
# ARG1:   Directory in which to store the yabai binary; must be an absolutepath.
#         Fallback: /usr/local/bin
#
# ARG2:   Directory in which to store the yabai man-page; must be an absolutepath.
#         Fallback: /usr/local/man/man1
#
# Author: Åsmund Vikane
#   Date: 2024-02-13
#

BIN_DIR="$1"
MAN_DIR="$2"

if [ -z "$BIN_DIR" ]; then
    BIN_DIR="/usr/local/bin"
fi

if [ -z "$MAN_DIR" ]; then
    MAN_DIR="/usr/local/share/man/man1"
fi

if [ "${BIN_DIR%%/*}" ]; then
    echo "Error: Binary target directory '${BIN_DIR}' is not an absolutepath."
    exit 1
fi

if [ ! -d "$BIN_DIR" ]; then
    echo "Error: Binary target directory '${BIN_DIR}' does not exist."
    exit 1
fi

if [ ! -w "$BIN_DIR" ]; then
    echo "Error: User does not have write permission for binary target directory '${BIN_DIR}'."
    exit 1
fi

if [ "${MAN_DIR%%/*}" ]; then
    echo "Error: Man-page target directory '${MAN_DIR}' is not an absolutepath."
    exit 1
fi

if [ ! -d "$MAN_DIR" ]; then
    echo "Error: Man-page target directory '${MAN_DIR}' does not exist."
    exit 1
fi

if [ ! -w "$MAN_DIR" ]; then
    echo "Error: User does not have write permission for man-page target directory '${MAN_DIR}'."
    exit 1
fi

AUTHOR="koekeishiya"
NAME="yabai"
VERSION="6.0.12"
EXPECTED_HASH="d1824ded7469b1e5d99d55c372cbfff30f297a7d2c139daa4264bec459e2e95d"
TMP_DIR="./${AUTHOR}-${NAME}-v${VERSION}-installer"

mkdir $TMP_DIR
pushd $TMP_DIR

curl --location --remote-name https://github.com/${AUTHOR}/${NAME}/releases/download/v${VERSION}/${NAME}-v${VERSION}.tar.gz
FILE_HASH=$(shasum -a 256 ./${NAME}-v${VERSION}.tar.gz | cut -d " " -f 1)

if [ "$FILE_HASH" = "$EXPECTED_HASH" ]; then
    echo "Hash verified. Preparing files.."
    tar -xzvf ${NAME}-v${VERSION}.tar.gz
    rm ${BIN_DIR}/${NAME}
    rm ${MAN_DIR}/${NAME}.1
    cp -v ./archive/bin/${NAME} ${BIN_DIR}/${NAME}
    cp -v ./archive/doc/${NAME}.1 ${MAN_DIR}/${NAME}.1
    echo "Finished copying files.."
    echo ""
    echo "If you want yabai to be managed by launchd (start automatically upon login):"
    echo "  yabai --start-service"
    echo ""
    echo "When running as a launchd service logs will be found in:"
    echo "  /tmp/yabai_<user>.[out|err].log"
    echo ""
    echo "If you are using the scripting-addition; remember to update your sudoers file:"
    echo "  sudo visudo -f /private/etc/sudoers.d/yabai"
    echo ""
    echo "Sudoers file configuration row:"
    echo "  $(whoami) ALL=(root) NOPASSWD: sha256:$(shasum -a 256 ${BIN_DIR}/yabai | cut -d " " -f 1) ${BIN_DIR}/yabai --load-sa"
    echo ""
    echo "README: https://github.com/koekeishiya/yabai/wiki/Installing-yabai-(latest-release)#configure-scripting-addition"
else
    echo "Hash does not match the expected value.. abort."
    echo "Expected hash: $EXPECTED_HASH"
    echo "  Actual hash: $FILE_HASH"
fi

popd
rm -rf $TMP_DIR
