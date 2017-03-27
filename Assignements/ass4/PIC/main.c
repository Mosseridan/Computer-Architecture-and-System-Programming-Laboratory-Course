#include <stdio.h>

#define MAX_FUNC_LEN 1000
#define RET_CMD      0xC3

int x = 0;

char junk_func[MAX_FUNC_LEN];

extern Start_pic;

void my_func();
void my_pic_func();
void my_strict_pic_func();

main(int ac, char **av) {
	char *p, *q;
	int i;
	void (*f)();

	/* Copy code to new location */
	p = (char *)(&Start_pic);
	q = junk_func;
	for(i = 0; i < MAX_FUNC_LEN; i++) {
		q[i] = p[i];
	}


	printf("\n\n\nCalling strict position-independent code\n");
	b1:
	my_strict_pic_func();

	printf("\nNow call in new location\n");
	f =  q+((unsigned int)(&my_strict_pic_func) -
			(unsigned int)(&Start_pic));
	b2:
	f();

	printf("\n\nCalling semi-position independent code\n");
	b3:
	my_pic_func();

	printf("\nNow call in new location\n");

	f =  q+((unsigned int)(&my_pic_func) -
			(unsigned int)(&Start_pic));
	b4:
	f();

	printf("\n\n\nNow calling non-IPC code, my_func()\n");
	b5:
	my_func();

	printf("\nNow call in new location\n");
	f =  q+((unsigned int)(&my_func) -

			(unsigned int)(&Start_pic));

        *((char *)f) = RET_CMD;   /* Self-modifying code ! */
	b6:
	f();

        printf("Last function would have resulted in seg fault\n");
}
