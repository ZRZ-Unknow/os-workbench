#include <game.h>

extern int w,h;
extern int edge[4][2];
extern int food_1[2];
extern int food_2[2];
extern struct Snake snake;


void init() {
  _DEV_VIDEO_INFO_t info = {0};
  _io_read(_DEV_VIDEO, _DEVREG_VIDEO_INFO, &info, sizeof(info));
  w = info.width;
  h = info.height;
  //char ch[32]="";
  //sprintf(ch,"You Need To Get %d Score To Win The Game\n",(MAX_LEN-3)*100);
  printf("This Is A Gluttonous Snake Game\n");
  printf("Press W,S,A,D To Control The Direction,Press 1,2,3 To Switch The Snake's Speed\n");
  //printf(ch);
  //printf("Good Luck!\n");
}

static void draw_tile(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // careful! stack is limited!
  _DEV_VIDEO_FBCTRL_t event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  _io_write(_DEV_VIDEO, _DEVREG_VIDEO_FBCTRL, &event, sizeof(event));
}
void splash() {
  for (int x = 0; x <= w; x+=SIDE) {
    for (int y = 0; y<= h;y+=SIDE) {
      if(x==edge[0][0] || x>=edge[3][0] || y==edge[0][1] || y>=edge[3][1])
        draw_tile(x,y,SIDE,SIDE,CHOCOLATE);
      else draw_tile(x, y, SIDE, SIDE,BLACK);
    }
  }
}
void draw_snake(){
  for(int i=0;i<snake.lenth;i++){
    if(i==snake.lenth-1){
      draw_tile(snake.node[i].x,snake.node[i].y,SIDE,SIDE,RED);
      break;
    }
    draw_tile(snake.node[i].x,snake.node[i].y,SIDE,SIDE,PURPLE);
  }
}
void draw_food(){
  draw_tile(food_1[0],food_1[1],SIDE,SIDE,RED);
  draw_tile(food_2[0],food_2[1],SIDE,SIDE,RED);
}
