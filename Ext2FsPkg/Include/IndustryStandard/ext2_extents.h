/*-
 * Copyright (c) 2012, 2010 Zheng Liu <lz@freebsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */
#ifndef _FS_EXT2FS_EXT2_EXTENTS_H_
#define	_FS_EXT2FS_EXT2_EXTENTS_H_

#define	EXT4_EXT_MAGIC  0xf30a

/*
 * Ext4 file system extent on disk.
 */
struct ext4_extent {
	uint32_t e_blk;	/* first logical block */
	uint16_t e_len;	/* number of blocks */
	uint16_t e_start_hi;	/* high 16 bits of physical block */
	uint32_t e_start_lo;	/* low 32 bits of physical block */
};

/*
 * Extent index on disk.
 */
struct ext4_extent_index {
	uint32_t ei_blk;	/* indexes logical blocks */
	uint32_t ei_leaf_lo;	/* points to physical block of the
				 * next level */
	uint16_t ei_leaf_hi;	/* high 16 bits of physical block */
	uint16_t ei_unused;
};

/*
 * Extent tree header.
 */
struct ext4_extent_header {
	uint16_t eh_magic;	/* magic number: 0xf30a */
	uint16_t eh_ecount;	/* number of valid entries */
	uint16_t eh_max;	/* capacity of store in entries */
	uint16_t eh_depth;	/* the depth of extent tree */
	uint32_t eh_gen;	/* generation of extent tree */
};

#endif /* !_FS_EXT2FS_EXT2_EXTENTS_H_ */

