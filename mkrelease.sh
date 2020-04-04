#!/bin/sh
#
# Script for creating a release archive
#

DESTDIR='tmp'
FULLVERS=`version SSHTerm`
NUMVERS=`echo "${FULLVERS}" | cut -d' ' -f2`

LIBSSH2DIR='libssh2-1.9.0'

ARCHIVE="sshterm-${NUMVERS}.7z"

rm -rf ${DESTDIR}
mkdir -p ${DESTDIR}/SSHTerm

cp -p SSHTerm ${DESTDIR}/SSHTerm
cp -p README ${DESTDIR}/SSHTerm
cp -p releasenotes ${DESTDIR}/SSHTerm
cp -p COPYING ${DESTDIR}/SSHTerm
cp -p libtsm/COPYING ${DESTDIR}/SSHTerm/COPYING-libtsm
cp -p libtsm/LICENSE_htable ${DESTDIR}/SSHTerm/
cp -p ${LIBSSH2DIR}/COPYING ${DESTDIR}/SSHTerm/COPYING-libssh2

rm -f ${ARCHIVE}
7za u ${ARCHIVE} ./${DESTDIR}/*

rm -rf ${DESTDIR}

echo "${ARCHIVE} created"

