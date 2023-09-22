#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION_CURRENT 0
#define FILENAME_MAX 4096

static void        archive_create(const char*, const char**, const char**, size_t);
/* Out of order in the definitions because it's practically a main function. */
static inline void assert(size_t);
static inline void chknz(size_t);
static inline void chkz(size_t);
static uint8_t     log2_ceil(size_t);
/* This is NOT a pure function. Not yet. It may halt the program after an invalid argument as I haven't implemented it completely yet. */
static size_t      sizebyte_from(uint8_t);
static uint8_t     sizebyte_to(size_t);
static size_t      strnlen(const char*, size_t);
/* Not properly implemented yet. */

static inline void
assert(size_t x)
{
	if (x == 0) {
		(void)fprintf(stderr, "assert: failed");
		exit(-1);
	}
}

static inline void
chknz(size_t x)
{
	if (x != 0) {
		(void)fprintf(stderr, "chknz: failed");
		exit(-1);
	}
}

static inline void
chkz(size_t x)
{
	if (x == 0) {
		(void)fprintf(stderr, "chkz: failed");
		exit(-1);
	}
}

static uint8_t
log2_ceil(size_t num)
{
	if (num < 2)
		return 0;
	else if (num == 2)
		return 1;
	else if (num <= 4)
		return 2;
	else if (num <= 8)
		return 3;
	else if (num <= 16)
		return 4;
	else if (num <= 32)
		return 5;
	else {
		(void)fprintf(stderr, "log2_ceil: TODO\n");
		exit(-1);
	}
}

static size_t
sizebyte_from(uint8_t byte)
{
	if (byte == 0) {
		return 0;
	}
	byte -= 1;
	return 1 << byte;
}

static uint8_t
sizebyte_to(size_t sz)
{
	if (sz == 0) {
		return 0;
	}
	return log2_ceil(sz) + 1;
}

static size_t
strnlen(const char *s, size_t maxlen)
{
	const char *p = memchr(s, 0, maxlen);
	return p ? (size_t)(p - s) : maxlen;
}

static void
archive_create(const char *archive_path, const char **file_list, const char **file_correspondences, size_t file_count)
{
	FILE *archive_file;
	size_t filelist_bufsize;
	/* I should probably check if the archive path already exists. */
	{ /* Scoping for archive_file. */
	archive_file = fopen(archive_path, "w");
	chkz((size_t)archive_file);

	{
		size_t a = fwrite("bar", 1, 3, archive_file);
		assert(a == 3);
		a = (size_t)fputc(VERSION_CURRENT, archive_file);
		assert(a == VERSION_CURRENT);
	}

	filelist_bufsize = 0;
	for (size_t i = 0; i < file_count; ++i) {
		filelist_bufsize += strnlen(file_correspondences[i], FILENAME_MAX) + 1;
	}
	filelist_bufsize = 1 << log2_ceil(filelist_bufsize);

	{
		uint8_t byte = sizebyte_to(filelist_bufsize);
		int a = fputc(byte, archive_file);
		assert(a == byte);
	}

	{ /* Scoping for filelist_buf. */
	char *filelist_buf = calloc(filelist_bufsize, 1);
	size_t writerhead = 0;
	chkz((size_t)filelist_buf);

	for (size_t i = 0; i < file_count; ++i) {
		size_t filename_size = strnlen(file_correspondences[i], FILENAME_MAX);
		void *a = memcpy(filelist_buf + writerhead, file_correspondences[i], filename_size);
		assert(a == filelist_buf + writerhead);
		writerhead += filename_size + 1;
	}

	{
		size_t a = fwrite(filelist_buf, 1, filelist_bufsize, archive_file);
		assert(a == filelist_bufsize);
	}

	free(filelist_buf);
	}

	for (size_t i = 0; i < file_count; ++i) {
		uint8_t *entry_buf;
		size_t   entrybuf_size;
		uint8_t  entrybuf_sizebyte;
		FILE    *file;
		uint8_t *file_buf;
		size_t   file_size;
		size_t   filebuf_size;
		uint8_t  filebuf_sizebyte;
		uint8_t *header_buf;
		size_t   headerbuf_size;
		uint8_t  headerbuf_sizebyte;
		size_t   writerhead;
		file = fopen(file_list[i], "r");
		chkz((size_t)file);

		chknz((size_t)fseek(file, 0, SEEK_END));
		{
			long a = ftell(file);
			assert(a != -1);
			file_size = (size_t)a;
		}
		rewind(file);

		/* Make the header buffer. */
		headerbuf_sizebyte = 0;
		headerbuf_size = sizebyte_from(headerbuf_sizebyte) + 1;
		header_buf = calloc(headerbuf_size, 1);
		chkz((size_t)header_buf);
		header_buf[0] = headerbuf_sizebyte;
		/*
		 * Since I still haven't figured out what to include in the file
		 * headers, the only space I give for the header is the solitary
		 * size byte saying that the header buffer is 0 in size.
		 * TODO: Make an actual header.
		 */
		/* Make the file buffer. */
		writerhead = 0;
		filebuf_sizebyte = sizebyte_to(file_size);
		filebuf_size = sizebyte_from(filebuf_sizebyte) + 1;
		file_buf = calloc(filebuf_size, 1);
		chkz((size_t)file_buf);
		file_buf[0] = filebuf_sizebyte;
		writerhead += 1;
		{
			size_t a = fread(file_buf + writerhead, 1, file_size, file);
			assert(a == file_size);
		}
		/* Merge them in an entry buffer. */
		writerhead = 0;
		entrybuf_sizebyte = sizebyte_to(headerbuf_size + filebuf_size);
		entrybuf_size = sizebyte_from(entrybuf_sizebyte) + 1;
		entry_buf = calloc(entrybuf_size, 1);
		chkz((size_t)entry_buf);
		entry_buf[0] = entrybuf_sizebyte;
		writerhead += 1;
		{
		void *a;
		a = memcpy(entry_buf + writerhead, header_buf, headerbuf_size);
		assert(a == entry_buf + writerhead);
		writerhead += headerbuf_size;
		a = memcpy(entry_buf + writerhead, file_buf, filebuf_size);
		assert(a == entry_buf + writerhead);
		}
		free(header_buf);
		free(file_buf);
		/* Write the entry buffer to the archive file. */
		{
		size_t a = fwrite(entry_buf, 1, entrybuf_size, archive_file);
		assert(a == entrybuf_size);
		}
		free(entry_buf);

		(void)fclose(file);
	}

	(void)fclose(archive_file);
	}
}

int32_t
main(void)
{
	const char *archive_path = "./archive1.bar";
	const char *file_correspondences[] = { "a.txt", "b.txt", NULL };
	size_t      file_count;
	const char *file_list[] = { "./misc/a.txt", "./misc/b.txt", NULL };
	for (file_count = 0; file_list[file_count]; ++file_count){}
	archive_create(archive_path, file_list, file_correspondences, file_count);
}
