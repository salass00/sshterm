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

#include <proto/amissl.h>

const EVP_CIPHER *EVP_aes_128_cbc(void)
{
	return IAmiSSL->EVP_aes_128_cbc();
}

const EVP_CIPHER *EVP_aes_192_cbc(void)
{
	return IAmiSSL->EVP_aes_192_cbc();
}

const EVP_CIPHER *EVP_aes_256_cbc(void)
{
	return IAmiSSL->EVP_aes_256_cbc();
}

const EVP_CIPHER *EVP_aes_128_ctr(void)
{
	return IAmiSSL->EVP_aes_128_ctr();
}

const EVP_CIPHER *EVP_aes_192_ctr(void)
{
	return IAmiSSL->EVP_aes_192_ctr();
}

const EVP_CIPHER *EVP_aes_256_ctr(void)
{
	return IAmiSSL->EVP_aes_256_ctr();
}

const EVP_CIPHER *EVP_bf_cbc(void)
{
	return IAmiSSL->EVP_bf_cbc();
}

const EVP_CIPHER *EVP_cast5_cbc(void)
{
	return IAmiSSL->EVP_cast5_cbc();
}

const EVP_CIPHER *EVP_des_ede3_cbc(void)
{
	return IAmiSSL->EVP_des_ede3_cbc();
}

const EVP_CIPHER *EVP_rc4(void)
{
	return IAmiSSL->EVP_rc4();
}

DSA *PEM_read_bio_DSAPrivateKey(BIO *bp, DSA **x, pem_password_cb *cb, void *u)
{
	return IAmiSSL->PEM_read_bio_DSAPrivateKey(bp, x, cb, u);
}

RSA *PEM_read_bio_RSAPrivateKey(BIO *bp, RSA **x, pem_password_cb *cb, void *u)
{
	return IAmiSSL->PEM_read_bio_RSAPrivateKey(bp, x, cb, u);
}

EC_KEY *PEM_read_bio_ECPrivateKey(BIO *bp, EC_KEY **x, pem_password_cb *cb, void *u)
{
	return IAmiSSL->PEM_read_bio_ECPrivateKey(bp, x, cb, u);
}
