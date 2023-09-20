#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILENAME_MAX 4096

static inline void assert(size_t);
static inline void chknz(size_t);
static inline void chkz(size_t);
static uint8_t log2_ceil(size_t);
/* This is NOT a pure function. Not yet. It may halt the program after an invalid argument as I haven't implemented it completely yet. */
static size_t sizebyte_from(uint8_t);
static uint8_t sizebyte_to(size_t);
static size_t strnlen(const char*, size_t);
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
	else {
		(void)fprintf(stderr, "log2_ceil: TODO");
		exit(-1);
	}
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
	return strlen(s);
}

int32_t
main(void)
{
	const char *archive_path = "./archive1.bar";
	const char *file_correspondences[] = { "a.txt", NULL };
	const char *filelist[] = { "./misc/a.txt", NULL };
	size_t file_count;
	size_t filelist_bufsize;
	for (file_count = 0; filelist[file_count]; ++file_count){}
	/* I should probably check if the archive path already exists. */
	{ /* Scoping for archive_file. */
	FILE *archive_file = fopen(archive_path, "w");
	chkz((size_t)archive_file);

	{
		size_t a = fwrite("bar", 1, 3, archive_file);
		assert(a == 3);
		a = (size_t)fputc(0, archive_file);
		assert(a == 0);
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

	(void)fclose(archive_file);
	}

	return 0;
}
