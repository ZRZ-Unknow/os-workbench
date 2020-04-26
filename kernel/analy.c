#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

int main(){
  FILE *fp=fopen("./a.txt","r");
  char txt[4096*4];
  fread(&txt[0],sizeof(txt),1,fp);
  fclose(fp);
  int count=0;
  int first=-1;
  for(int i=0;i<sizeof(txt);i++){
    if(txt[i]=='('){
      count++;
      if(first==-1) first=i;
    }
    else if(txt[i]==')') count--;
    printf("count is %d,first is %d\n",count,first);
    if(count<0) {printf("%d,dddddddddddddddddddddddddddddddddddddd\n",i);assert(0);}
    if(count>5) {printf("%d,ccccccccccccccccccccccccccccccccccccc\n",i);assert(0);}
  }
  return 0;
}