#ifndef UIMPL_H
#define UIMPL_H

#include <stdint.h>

struct uuarray {
	uint64_t size;
	char buffer[]; // !! C99 flexible
};

struct mapcont {
	uint64_t mtype;
	struct uuarray *param;
	struct uuarray *ret;
	void *fun;
};

struct redcont {
	uint64_t rtype;
	struct uuarray *param;
	void *ret;
	void *fun;
};

enum map_call_type {
	MT1 = 0,
	MT2 = 1,
	MT3 = 2,
	MT4 = 3
};

void putint(int x);
void putfloat(float x);
void putaddr(struct uuarray*);

#endif //UIMPL_H
