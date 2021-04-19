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
#include <pthread.h>
#include <stdbool.h>

struct _throb_args{
	unsigned x,y;
	const char* color;
	pthread_mutex_t lock;
	bool stop;
};

typedef struct{
	pthread_t id;
	struct _throb_args* args;
}throbber_t;

void* _throb(void*);

throbber_t start_throbber(const unsigned x, const unsigned y, const char* color){
	struct _throb_args* args = malloc(sizeof(struct _throb_args));
	args->x = x;
	args->y = y;
	args->color = color;
	args->stop = false;
	if(pthread_mutex_init(&args->lock, NULL)){
		puts("Throbber initialization failed");
		exit(1);
	}
	throbber_t t;
	t.args = args;
	pthread_create(&t.id, NULL, _throb, args);
	return t;
}

void* _throb(void* args){
	struct _throb_args* targs = args;
	unsigned x = targs->x;
	unsigned y = targs->y;
	const char* color = targs->color;
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
	if((x <= 0)||(y <= 0))return NULL;
	unsigned step = 0;
	fflush(stdout);
	pthread_mutex_lock(&targs->lock);
	while(!targs->stop){
		pthread_mutex_unlock(&targs->lock);
		fprintf(stdout, "\x1b[s\x1b[%d;%dH%s", x, y, color?color:""); //push and move cursor position
		fputs(frames[++step % 8], stdout);
		fputs(safe_exit, stdout);
		fflush(stdout); //prevents visual errors
		usleep(100000L);
		pthread_mutex_lock(&targs->lock);
	}
	free(targs);
	return NULL;
}

void stop_throbber(throbber_t t){
	const char *safe_exit = "\x1b[u\x1b[39;49m";
	pthread_mutex_lock(&t.args->lock);
	t.args->stop = true;
	pthread_mutex_unlock(&t.args->lock);
	fprintf(stdout, "\x1b[s\x1b[%d;%dH", t.args->x, t.args->y);
	fputs("  ", stdout);
	fputs(safe_exit, stdout);
	fflush(stdout);
	pthread_join(t.id, NULL);
	usleep(1000L);
	return;
}

#endif //THROBBER_H_
