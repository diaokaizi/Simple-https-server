#include "general.h"

char *get_filename(char h_s[]) {
  char *ptr = NULL;
  char *s = strtok_r(h_s, " ", &ptr);
  s = strtok_r(NULL, " ", &ptr);
  return s;
}

char *get_host(char host_s[]) {
  char *ptr = NULL;
  strtok_r(host_s, " ", &ptr);
  return ptr;
}

int *get_range(char range_s[]) {
  int *r ;
  r  = (int *)malloc(2*sizeof(int));
  r[0] = -1;
  r[1] = -1;
  if(range_s == NULL)
    return r;
  if(strstr(range_s, "Range") == NULL)
    return r; 
  char *str = NULL;
  strtok_r(range_s, " ", &str);
  strtok_r(NULL, "=", &str);
	int len = strlen(str);
	if(str[0] == '-' && str[len - 1] == '-'){
		return r;
	}
	if(str[0] == '-'){
		str++;
		r[1] = atoi(str);
	}
	else if(str[len - 1] == '-'){
		r[0] = atoi(str);
	}
	else{
		char* r2 = strstr(str, "-");
		r2++;
		r[0] = atoi(str);
		r[1] = atoi(r2);
	}
  return r;
}
