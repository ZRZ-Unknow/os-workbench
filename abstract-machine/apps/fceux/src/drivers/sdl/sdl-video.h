#ifndef __FCEU_SDL_VIDEO_H
#define __FCEU_SDL_VIDEO_H

uint32 PtoV(uint16 x, uint16 y);
bool FCEUD_ShouldDrawInputAids();
bool FCEUI_AviDisableMovieMessages();
bool FCEUI_AviEnableHUDrecording();
void FCEUI_SetAviEnableHUDrecording(bool enable);
bool FCEUI_AviDisableMovieMessages();
void FCEUI_SetAviDisableMovieMessages(bool disable);
#endif
