/*
 * SSHTerm - SSH2 shell client
 *
 * Copyright (C) 2019-2020 Fredrik Wikstrom <fredrik@a500.org>
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the SSHTerm
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define LIBRARIES_Z_H
#include <zlib.h>
#include <interfaces/z.h>

extern struct ZIFace *IZ;

const char *zlibVersion (void) {
	return IZ->ZlibVersion();
}

int deflateInit_ (z_streamp strm, int level, const char *version, int stream_size) {
	const char *my_version = IZ->ZlibVersion();
	if (version == Z_NULL || version[0] != my_version[0] || stream_size != sizeof(z_stream)) {
		return Z_VERSION_ERROR;
	}
	return IZ->DeflateInit(strm, level);
}

int deflateInit2_ (z_streamp strm, int level, int method, int windowBits, int memLevel,
	int strategy, const char *version, int stream_size)
{
	const char *my_version = IZ->ZlibVersion();
	if (version == Z_NULL || version[0] != my_version[0] || stream_size != sizeof(z_stream)) {
		return Z_VERSION_ERROR;
	}
	return IZ->DeflateInit2(strm, level, method, windowBits, memLevel, strategy);
}

int deflateSetDictionary (z_streamp strm, const Bytef *dictionary, uInt dictLength) {
	return IZ->DeflateSetDictionary(strm, dictionary, dictLength);
}

int deflate (z_streamp strm, int flush) {
	return IZ->Deflate(strm, flush);
}

int deflateCopy (z_streamp dest, z_streamp source) {
	return IZ->DeflateCopy(dest, source);
}

int deflateReset (z_streamp strm) {
	return IZ->DeflateReset(strm);
}

int deflateParams (z_streamp strm, int level, int strategy) {
	return IZ->DeflateParams(strm, level, strategy);
}

int deflateTune (z_streamp strm, int good_length, int max_lazy, int nice_length,
	int max_chain)
{
	return IZ->DeflateTune(strm, good_length, max_lazy, nice_length, max_chain);
}

uLong deflateBound (z_streamp strm, uLong sourceLen) {
	return IZ->DeflateBound(strm, sourceLen);
}

int deflatePrime (z_streamp strm, int bits, int value) {
	return IZ->DeflatePrime(strm, bits, value);
}

int deflateSetHeader (z_streamp strm, gz_headerp head) {
	return IZ->DeflateSetHeader(strm, head);
}

int deflateEnd (z_streamp strm) {
	return IZ->DeflateEnd(strm);
}

int inflateInit_ (z_streamp strm, const char *version, int stream_size) {
	const char *my_version = IZ->ZlibVersion();
	if (version == Z_NULL || version[0] != my_version[0] || stream_size != sizeof(z_stream)) {
		return Z_VERSION_ERROR;
	}
	return IZ->InflateInit(strm);
}

int inflateInit2_ (z_streamp strm, int windowBits, const char *version, int stream_size) {
	const char *my_version = IZ->ZlibVersion();
	if (version == Z_NULL || version[0] != my_version[0] || stream_size != sizeof(z_stream)) {
		return Z_VERSION_ERROR;
	}
	return IZ->InflateInit2(strm, windowBits);
}

int inflateGetDictionary (z_streamp strm, Bytef *dictionary, uInt *dictLength) {
	return IZ->InflateGetDictionary(strm, dictionary, (uint32 *)dictLength);
}

int inflateSetDictionary (z_streamp strm, const Bytef *dictionary, uInt dictLength) {
	return IZ->InflateSetDictionary(strm, dictionary, dictLength);
}

int inflate (z_streamp strm, int flush) {
	return IZ->Inflate(strm, flush);
}

int inflateSync (z_streamp strm) {
	return IZ->InflateSync(strm);
}

int inflateCopy (z_streamp dest, z_streamp source) {
	return IZ->InflateCopy(dest, source);
}

int inflateReset (z_streamp strm) {
	return IZ->InflateReset(strm);
}

int inflatePrime (z_streamp strm, int bits, int value) {
	return IZ->InflatePrime(strm, bits, value);
}

int inflateGetHeader (z_streamp strm, gz_headerp head) {
	return IZ->InflateGetHeader(strm, head);
}

int inflateEnd (z_streamp strm) {
	return IZ->InflateEnd(strm);
}

int inflateBackInit_ (z_streamp strm, int windowBits, unsigned char *window,
	const char *version, int stream_size)
{
	const char *my_version = IZ->ZlibVersion();
	if (version == Z_NULL || version[0] != my_version[0] || stream_size != sizeof(z_stream)) {
		return Z_VERSION_ERROR;
	}
	return IZ->InflateBackInit(strm, windowBits, window);
}

int inflateBack (z_streamp strm, in_func in, void *in_desc, out_func out, void *out_desc) {
	return IZ->InflateBack(strm, in, in_desc, out, out_desc);
}

int inflateBackEnd (z_streamp strm) {
	return IZ->InflateBackEnd(strm);
}

int compress (Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen) {
	return IZ->Compress(dest, destLen, source, sourceLen);
}

int compress2 (Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen, int level) {
	return IZ->Compress2(dest, destLen, source, sourceLen, level);
}

uLong compressBound (uLong sourceLen) {
	return IZ->CompressBound(sourceLen);
}

int uncompress (Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen) {
	return IZ->Uncompress(dest, destLen, source, sourceLen);
}

uLong adler32 (uLong adler, const Bytef *buf, uInt len) {
	return IZ->Adler32(adler, buf, len);
}

uLong adler32_combine (uLong adler1, uLong adler2, z_off_t len2) {
	return IZ->Adler32Combine(adler1, adler2, len2);
}

uLong crc32 (uLong crc, const Bytef *buf, uInt len) {
	return IZ->CRC32(crc, buf, len);
}

uLong crc32_combine (uLong crc1, uLong crc2, z_off_t len2) {
	return IZ->CRC32Combine(crc1, crc2, len2);
}

const char *zError (int err) {
	return IZ->ZError(err);
}

