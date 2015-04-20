#ifndef __BUF_H__
#define __BUF_H__

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

typedef struct sbuf {    /* stream buffer     */
	int off;         /* increase by write */
	int len;         /* increase by read  */
	int max;         /* off < len <= max  */
	char *buf;
} sbuf_t;

int sbuf_alloc(sbuf_t *sbuf, int max);
void sbuf_release(sbuf_t *sbuf);

/*
 * sbuf->off = 0;
 * sbuf->len = 0;
 * keep sbuf->max and sbuf->buf
 */
void sbuf_reset(sbuf_t *sbuf);
char * sbuf_detach(sbuf_t *sbuf);

/*
 * free sbuf->buf
 * sbuf->buf = buf;
 * sbuf->max = len;
 */
void sbuf_attach(sbuf_t *sbuf, char *buf, int len);

/*
 * make buf->max >= buf->len + len
 */
int sbuf_extend(sbuf_t *sbuf, int len);
int sbuf_append(sbuf_t *sbuf, void *buf, int len);

#endif /* __BUF_H__ */
