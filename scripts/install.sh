#!/usr/bin/env sh

#
# This script will install the latest pre-built nimbuswm release from GitHub.
# Depends on curl, shasum, tar, cp, cut.
#
# ARG1:   Directory in which to store the nimbuswm binary; must be an absolutepath.
#         Fallback: /usr/local/bin
#
# ARG2:   Directory in which to store the nimbuswm man-page; must be an absolutepath.
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

AUTHOR="john-json"
NAME="nimbuswm"
VERSION="7.1.14"
EXPECTED_HASH="a886bc9124c8fe864a78546b0289a2ef11a71c30703ad17625a429cf81229425"
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
    echo "If you want nimbuswm to be managed by launchd (start automatically upon login):"
    echo "  nimbuswm --start-service"
    echo ""
    echo "When running as a launchd service logs will be found in:"
    echo "  /tmp/nimbuswm_<user>.[out|err].log"
    echo ""
    echo "If you are using the scripting-addition; remember to update your sudoers file:"
    echo "  sudo visudo -f /private/etc/sudoers.d/nimbuswm"
    echo ""
    echo "Sudoers file configuration row:"
    echo "  $(whoami) ALL=(root) NOPASSWD: sha256:$(shasum -a 256 ${BIN_DIR}/nimbuswm | cut -d " " -f 1) ${BIN_DIR}/nimbuswm --load-sa"
    echo ""
    echo "README: https://github.com/john-json/nimbuswm/wiki/Installing-nimbuswm-(latest-release)#configure-scripting-addition"
else
    echo "Hash does not match the expected value.. abort."
    echo "Expected hash: $EXPECTED_HASH"
    echo "  Actual hash: $FILE_HASH"
fi

popd
rm -rf $TMP_DIR
