#include <game.h>
//贪吃蛇
// Operating system is a C program!
int w,h;
struct Snake snake;
int edge[4][2];
int food_1[2];
int food_2[2];
bool food_eaten=false;

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
bool collision(int x,int y,int fx,int fy){   //在这里，x,y是没有乘SIDE的, 而fx,fy是乘过SIDE的
  if(x==edge[0][0] || y==edge[0][1] || x*SIDE>=edge[3][0] || y*SIDE>=edge[3][1]) return true;
  if(fx==x*SIDE && fy==y*SIDE) return true;
  for(int i=0;i<snake.lenth;i++){
    if(x*SIDE-snake.node[i].x>=0 && x*SIDE-snake.node[i].x<SIDE && y*SIDE-snake.node[i].y>=0 && y*SIDE-snake.node[i].y<SIDE) return true;
  }
  return false;
}
void init_food(){
  srand(uptime());
  int x=rand()%(w/SIDE);
  int y=rand()%(h/SIDE);
  food_1[0]=x*SIDE;
  food_1[1]=y*SIDE;

  srand(uptime());
  x=rand()%(w/SIDE);
  y=rand()%(h/SIDE);
  food_2[0]=x*SIDE;
  food_2[1]=y*SIDE;
}


int main(const char *args) {
  _ioe_init();
  init();
  init_snake();
  init_edge();
  init_food();
  splash();
  draw_snake();
  draw_food();
  while (1) {
    if(read_key()==1)_halt(0);
    //print_key();
  }
  return 0;
}
