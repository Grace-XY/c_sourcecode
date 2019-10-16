/*
 * Copyright (c) 2014-2016 Alibaba Group. All rights reserved.
 * License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */


#ifndef _IOTX_COMMON_SHA1_H_
#define _IOTX_COMMON_SHA1_H_

#include <stdint.h>

/**
 * \brief          SHA-1 context structure
 */
typedef struct {
    uint32_t total[2];          /*!< number of bytes processed  */
    uint32_t state[5];          /*!< intermediate digest state  */
    unsigned char buffer[64];   /*!< data block being processed */
}sw_sha1_context;

/**
 * \brief          Initialize SHA-1 context
 *
 * \param ctx      SHA-1 context to be initialized
 */
void sw_sha1_init(sw_sha1_context *ctx);

/**
 * \brief          Clear SHA-1 context
 *
 * \param ctx      SHA-1 context to be cleared
 */
void sw_sha1_free(sw_sha1_context *ctx);

/**
 * \brief          Clone (the state of) a SHA-1 context
 *
 * \param dst      The destination context
 * \param src      The context to be cloned
 */
void sw_sha1_clone(sw_sha1_context *dst,const sw_sha1_context *src);

/**
 * \brief          SHA-1 context setup
 *
 * \param ctx      context to be initialized
 */
void sw_sha1_starts(sw_sha1_context *ctx);

/**
 * \brief          SHA-1 process buffer
 *
 * \param ctx      SHA-1 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void sw_sha1_update(sw_sha1_context *ctx, const unsigned char *input, size_t ilen);

/**
 * \brief          SHA-1 final digest
 *
 * \param ctx      SHA-1 context
 * \param output   SHA-1 checksum result
 */
void sw_sha1_finish(sw_sha1_context *ctx, unsigned char output[20]);

/* Internal use */
void sw_sha1_process(sw_sha1_context *ctx, const unsigned char data[64]);

/**
 * \brief          Output = SHA-1( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SHA-1 checksum result
 */
void utils_sha1(const unsigned char *input, size_t ilen, unsigned char output[20]);

#endif
