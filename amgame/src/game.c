#include <game.h>
//贪吃蛇
// Operating system is a C program!
int w,h;
struct Snake snake;
void init_screen() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
}
void itoa(int a){
  char ch[64]="";
  int i=0;
  while(a){
    int b=a%10;
    ch[i++]=b+'0';
    a/=10;
  }
  ch[i]='\0';
  for(int j=i-1;j>=0;j--){
    _putc(ch[j]);
  }
  _putc('\n');
}
void init_snake(){
  snake.dire=UP;
  snake.lenth=3;
  snake.node[0].x=(w/SIDE/2)*SIDE,snake.node[0].y=(h/SIDE/2)*SIDE;
  snake.node[1].x=(w/SIDE/2)*SIDE,snake.node[1].y=(h/SIDE/2)*SIDE-SIDE;
  snake.node[2].x=(w/SIDE/2)*SIDE,snake.node[2].y=(h/SIDE/2)*SIDE-2*SIDE;
}
void draw_snake(){
  for(int i=0;i<snake.lenth;i++){
    if(i==snake.lenth-1){
      _draw(snake.node[i].x,snake.node[i].y,SIDE,SIDE,RED);
      break;
    }
    _draw(snake.node[i].x,snake.node[i].y,SIDE,SIDE,BLACK);
  }
}




int main(const char *args) {
  _ioe_init();
  init_screen();
  init_snake();
  puts("mainargs = \"");
  puts(args); // make run mainargs=xxx
  puts("\"\n");

  splash();
  draw_snake();
  puts("Press any key to see its key code...\n");
  while (1) {
    if(read_key()==1)_halt(0);
    //print_key();
  }
  return 0;
}
