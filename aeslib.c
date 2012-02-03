/*
    This file is part of the aeslib.
    Copyright (C) 2012 Davy Landman (davy.landman@gmail.com) 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "aeslib.h"
#include <stdint.h>
#include "aes.h"
#include "blockcipher_descriptor.h"
#include "bcal_aes128.h"
#include "bcal-cbc.h"
#include <avr/pgmspace.h>

// encrypt multiple blocks of 128bit data, data_len but be mod 16
// key and iv are assumed to be both 128bit thus 16 uint8_t's
void aes128_cbc_enc(uint8_t* key, uint8_t* iv, void* data, uint16_t data_len){
	if (data_len % 16 != 0) {
		return;
	}
	bcal_cbc_ctx_t ctx;
	uint8_t r;
	r = bcal_cbc_init(&aes128_desc, key, 128, &ctx);
	if (r) {
		return;
	}
	bcal_cbc_encMsg(iv, data, data_len / 16, &ctx);
	bcal_cbc_free(&ctx);
}

// encrypt single 128bit block. data is assumed to be 16 uint8_t's
// key and iv are assumed to be both 128bit thus 16 uint8_t's
void aes128_enc_single(uint8_t* key, void* data){
	aes128_ctx_t ctx;
	aes128_init(key, &ctx);
	aes128_enc(data, &ctx);
}

// prepare an encrypted to use for encrypting multiple blocks lateron.
// key and iv are assumed to be both 128bit thus 16 uint8_t's
aes_context aes128_cbc_enc_start(uint8_t* key, void* iv){
	bcal_cbc_ctx_t* ctx = (bcal_cbc_ctx_t*)malloc(sizeof(bcal_cbc_ctx_t));
	uint8_t r = bcal_cbc_init(&aes128_desc, key, 128, ctx);
	if (r) {
		free(ctx);
		return NULL;
	}
	bcal_cbc_loadIV(iv, ctx);
	return (aes_context)ctx;
}

// encrypt one or more blocks of 128bit data
// data_len should be mod 16
void aes128_cbc_enc_continue(aes_context ctx, void* data, uint16_t data_len){
	if (data_len % 16 != 0) {
		return;
	}
	bcal_cbc_ctx_t* _ctx = (bcal_cbc_ctx_t*)ctx;
	uint16_t msg_blocks = data_len / 16;
	while(msg_blocks--){
		bcal_cbc_encNext(data, _ctx);
		data = (uint8_t*)data + _ctx->blocksize_B;
	}
}

// cleanup encryption context
void aes128_cbc_enc_finish(aes_context ctx){
	bcal_cbc_free((bcal_cbc_ctx_t*)ctx);
	free(ctx);
}