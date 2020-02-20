#include <game.h>
//贪吃蛇
// Operating system is a C program!
int w,h;
struct Snake snake;
int edge[4][2];
int food_1[2];
int food_2[2];
int score=0;
bool food_eaten=false;
bool dead=false;
bool succ=false;


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
  while(collision(x,y,-1,-1)){
      x=rand()%(w/SIDE);
      y=rand()%(h/SIDE);
  }
  food_1[0]=x*SIDE;
  food_1[1]=y*SIDE;

  srand(uptime());
  x=rand()%(w/SIDE);
  y=rand()%(h/SIDE);
  while(collision(x,y,food_1[0],food_1[1])){  
    x=rand()%(w/SIDE);
    y=rand()%(h/SIDE);
  }
  food_2[0]=x*SIDE;
  food_2[1]=y*SIDE;
}
bool eat_food(int x,int y){
  if(food_1[0]-x>=0 && food_1[0]-x<SIDE && food_1[1]-y>=0 && food_1[1]-y<SIDE) return true;
  if(food_2[0]-x>=0 && food_2[0]-x<SIDE && food_2[1]-y>=0 && food_2[1]-y<SIDE) return true;
  return false;
}
void gen_food(){
  srand(uptime());
  int x=rand()%(w/SIDE);
  int y=rand()%(h/SIDE);
  if(snake.node[snake.lenth-1].x==food_1[0] && snake.node[snake.lenth-1].y==food_1[1]){
    while(collision(x,y,food_2[0],food_2[1])){
      x=rand()%(w/SIDE);
      y=rand()%(h/SIDE);
    }
    food_1[0]=x*SIDE;
    food_1[1]=y*SIDE;
  }
  else if(snake.node[snake.lenth-1].x==food_2[0] && snake.node[snake.lenth-1].y==food_2[1]){
    while(collision(x,y,food_1[0],food_1[1])){
      x=rand()%(w/SIDE);
      y=rand()%(h/SIDE);
    }
    food_2[0]=x*SIDE;
    food_2[1]=y*SIDE;
  }
}
bool is_dead(int x,int y){
  if(x==edge[0][0] || y==edge[0][1] || x==edge[3][0] || y==edge[3][1]) return true;
  for(int i=0;i<snake.lenth;i++){
    //TODO }
    if(snake.node[i].x==x && snake.node[i].y==y) return true;
  }
  return false;
}
void update_snake(int mov){
  //计算头的位置
  struct Node head={.x=0,.y=0};
  switch (mov){
    case NONE:
      if(snake.dire==UP || snake.dire==DOWN){
        head.y=snake.node[snake.lenth-1].y+SIDE*(snake.dire==UP?-1:1);
        head.x=snake.node[snake.lenth-1].x;
      }
      else{
        head.x=snake.node[snake.lenth-1].x+SIDE*(snake.dire==LEFT?-1:1);
        head.y=snake.node[snake.lenth-1].y;
      }
      break;
    case UP: head.y=snake.node[snake.lenth-1].y-SIDE;head.x=snake.node[snake.lenth-1].x;break;
    case DOWN:head.y=snake.node[snake.lenth-1].y+SIDE;head.x=snake.node[snake.lenth-1].x; break;
    case LEFT: head.x=snake.node[snake.lenth-1].x-SIDE;head.y=snake.node[snake.lenth-1].y;break;
    case RIGHT:head.x=snake.node[snake.lenth-1].x+SIDE;head.y=snake.node[snake.lenth-1].y;break;
    default:assert(0);break;
  }
  if(eat_food(head.x,head.y)){
    food_eaten=true;
  }
  if(is_dead(head.x,head.y)){
    dead=true;
    return;
  }
  //未碰到食物，向前移动 
  if(!food_eaten){
    for(int i=0;i<snake.lenth-1;i++){
      snake.node[i].x=snake.node[i+1].x;
      snake.node[i].y=snake.node[i+1].y;
    }  
    snake.node[snake.lenth-1].x=head.x;
    snake.node[snake.lenth-1].y=head.y;
  }
  //碰到了食物，只需要更新头
  else{
    snake.node[snake.lenth].x=head.x;
    snake.node[snake.lenth].y=head.y;
    snake.lenth++;
  }
  snake.dire=((mov==NONE)?snake.dire:mov); 
  score=(snake.lenth-3)*100;
  if(snake.lenth==MAX_LEN){
    succ=true;
  }  
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
  while(true){
    unsigned long last_time=uptime();
    unsigned long curr_time=uptime();
  
    while (true) {
      while((curr_time=uptime())<last_time+150){};
      last_time=curr_time;
      int key=read_key();
      if(key==1)_halt(0);
      int mov=0;
      switch (key){//w,s,a,d
        case 30:mov=UP;break;
        case 44:mov=DOWN;break;
        case 43:mov=LEFT;break;
        case 45:mov=RIGHT;break;
        default:mov=0;break;
      }
      update_snake(mov);
      if(dead||succ){
        break;
      }
      if(food_eaten){
        gen_food();
        food_eaten=false;
      }
      splash();
      draw_food();
      draw_snake();
      if(succ){
        break;
      }
    }
    //死了或赢了后重启
    while((curr_time=uptime())<last_time+500){};
    init_snake();
    init_food();
    food_eaten=0;
    score=0;
    dead=false;
    succ=false;
    splash();
    draw_snake();
    draw_food();
  }
  return 0;
}
