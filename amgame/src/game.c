#include <game.h>
//贪吃蛇
// Operating system is a C program!
int w,h;
struct Snake snake;
int edge[4][2];
int food_1[2];
int food_2[2];
bool food_eaten=false;



void init_screen() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
}
void init_edge(){
  edge[0][0]=0;edge[0][1]=0;            //左上
  edge[1][0]=0;edge[1][1]=(h/SIDE-1)*SIDE;    //左下
  edge[2][0]=(w/SIDE-1)*SIDE;edge[2][1]=0;      //右上
  edge[3][0]=(w/SIDE-1)*SIDE;edge[3][1]=(h/SIDE-1)*SIDE; //右下
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
