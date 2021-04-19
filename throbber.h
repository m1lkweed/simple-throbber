// Simple command-line progress bar in C
// (c)m1lkweed 2021 GPLv3+
// Works on all ANSI-compliant, UNIX-like terminals with compatible fonts
//
// start_throbber() returns the pid of the child that's displaying the throbber.
// this should be passed to stop_throbber() or it will run until the parent exits.
// start_throbber() returns -1 on error, no child will be created.

#ifndef THROBBER_H_
#define THROBBER_H_

#include <stdio.h>
#include <stdlib.h>
#include <threads.h>
#include <stdbool.h>

struct _throb_args{
	unsigned x,y;
	const char *color;
	mtx_t lock;
	bool stop;
};

typedef struct{
	thrd_t id;
	struct _throb_args *args;
}throbber_t;

int _throb(void*);

throbber_t start_throbber(const unsigned x, const unsigned y, const char *color){
	struct _throb_args *args = malloc(sizeof(struct _throb_args));
	args->x = x;
	args->y = y;
	args->color = color;
	args->stop = false;
	if(mtx_init(&args->lock, mtx_plain) == thrd_error){
		fputs("Throbber initialization failed", stderr);
		free(args);
		return (throbber_t){-1, NULL};
	}
	throbber_t t;
	t.args = args;
	thrd_create(&t.id, _throb, args);
	return t;
}

int _throb(void *args){
	struct _throb_args *targs = args;
	unsigned x = targs->x;
	unsigned y = targs->y;
	const char *color = targs->color;
	const char *frames[] = {
		"⠯⠃",
		"⠏⠇",
		"⠫⠇",
		"⠭⠇",
		"⠮⠇",
		"⠧⠇",
		"⠯⠆",
		"⠯⠅"
	};
	const char *safe_exit = "\x1b[u\x1b[39;49m"; //pop cursor position and reset colors
	if((x <= 0)||(y <= 0))return 0;
	unsigned step = 0;
	fflush(stdout);
	mtx_lock(&targs->lock);
	do{
		mtx_unlock(&targs->lock);
		fprintf(stdout, "\x1b[s\x1b[%d;%dH%s%s%s", x, y, color?color:"", frames[++step % 8], safe_exit); //push and move cursor position, print throbber, then pop and reset colors
		fflush(stdout);
		usleep(100000L);
		mtx_lock(&targs->lock);
	}while(!targs->stop);
	free(targs);
	return 0;
}

void stop_throbber(throbber_t t){
	if(t.id == -1)
		return;
	mtx_lock(&t.args->lock);
	t.args->stop = true;
	fprintf(stdout, "\x1b[s\x1b[%d;%dH  \x1b[u\x1b[39;49m", t.args->x, t.args->y); // like the one in _throb() but using spaces to clear the throbber
	fflush(stdout);
	mtx_unlock(&t.args->lock);
	thrd_join(t.id, NULL);
	return;
}

#endif //THROBBER_H_
