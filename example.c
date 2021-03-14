#include <stdio.h>
#include <unistd.h>
#include <termios.h>

#include "throbber.h"

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
	int x, y;
	system("clear");
	fputs("Waiting ", stdout);
	get_pos(&x, &y);
	puts("");
	throbber_t throbber1 = start_throbber(x, y + 8, RED);
	sleep(1);
	fputs("Loading ", stdout);
	get_pos(&x, &y);
	puts("");
	throbber_t throbber2 = start_throbber(x, y + 8, GREEN);
	sleep(5);
	stop_throbber(throbber2);
	sleep(1);
	stop_throbber(throbber1);
	puts("Done!");
}