#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include "throbber.h"

#define MAX_THROBBERS 20
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"

int get_pos(int *y, int *x){
	char buf[30]={0};
	int ret, i, pow;
	char ch;
	*y = 0;
	*x = 0;
	struct termios term, restore;
	tcgetattr(0, &term);
	tcgetattr(0, &restore);
	term.c_lflag &= ~(ICANON|ECHO);
	tcsetattr(0, TCSANOW, &term);
	write(1, "\x1b[6n", 4);
	for( i = 0, ch = 0; ch != 'R'; ++i){
		ret = read(0, &ch, 1);
		if(!ret){
			tcsetattr(0, TCSANOW, &restore);
			return 1;
		}
		buf[i] = ch;
	}
	if(i < 2){
		tcsetattr(0, TCSANOW, &restore);
		return 1;
	}
	for(i -= 2, pow = 1; buf[i] != ';'; --i, pow *= 10)
		*x = *x + (buf[i] - '0') * pow;
	for(--i, pow = 1; buf[i] != '['; --i, pow *= 10)
		*y = *y + ( buf[i] - '0' ) * pow;
	tcsetattr(0, TCSANOW, &restore);
	return 0;
}

int main(){
	system("clear");
	throbber_t throbbers[MAX_THROBBERS] = {{0, NULL}};
	int x, y;
	fputs("Running: ", stdout);
	fflush(stdout);
	get_pos(&x, &y);
	puts("");
	throbbers[0] = start_throbber(x, y, RED);
	size_t i;
	for(i = 1; i < MAX_THROBBERS; ++i){
		fprintf(stdout, "Throbber %d: ", i);
		fflush(stdout);
		get_pos(&x, &y);
		puts("");
		throbbers[i] = start_throbber(x, y, GREEN);
		usleep(100000);
	}
	while(i--){
		stop_throbber(throbbers[i]);
		usleep(10000);
	}
	puts("Done!");
}
