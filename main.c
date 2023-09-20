#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TESTING 0

static inline void chknz(size_t);
static inline void chkz(size_t);
static uint8_t log2_ceil(size_t);
/* This is NOT a pure function. Not yet. It may halt the program after an invalid argument as I haven't implemented it completely yet. */

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
	(void)fprintf(stderr, "chknz: failed");
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

int32_t
main(void)
{
	FILE *a_txt;
    char *buf_ptr;
    size_t buf_size;
	size_t size;
	a_txt = fopen("./misc/a.txt", "r");
	chkz((size_t)a_txt);
	chknz((size_t)fseek(a_txt, 0, SEEK_END));

    {
        int64_t a = ftell(a_txt);
        chkz((size_t)(a + 1));
        size = (size_t)a;
    }

	rewind(a_txt);
    buf_size = 1 << log2_ceil(size);
    buf_ptr = malloc(buf_size);
    chkz((size_t)buf_ptr);

    {
        void *a = memset(buf_ptr, 0, buf_size);
        chknz((size_t)a - (size_t)buf_ptr);
    }
    {
        size_t a = fread(buf_ptr, 1, size, a_txt);
        chknz(a - size);
    }

    free(buf_ptr);
	(void)fclose(a_txt);
	(void)printf("a.txt: %lu\n", size);
	(void)printf("buf: %lu\n", buf_size);
	return 0;
}
