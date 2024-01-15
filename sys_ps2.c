/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/*
	sys_ps2.c -- PlayStation 2 sys driver
	
	by Nicolas Plourde a.k.a nic067 <nicolasplourde@hotmail.com>
	
	See http://www.ps2dev.org for all your ps2 coding need.
*/

#include <stdio.h> 
#include <tamtypes.h>
#include <sifrpc.h>
#include <kernel.h>
#define NEWLIB_PORT_AWARE
#include <fileXio_rpc.h>
#include <loadfile.h>
#include <fileio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <debug.h>
#include <iopcontrol.h>
#include <sbv_patches.h>
#include <iopheap.h>

#include "ps2_gs.h"
#include "pad.h"
#include "quakedef.h"

#ifdef _SOUND
#include "SDL.h"
#endif
cvar_t  sys_linerefresh = {"sys_linerefresh","0"};// set for entity display

qboolean			isDedicated;

#define NEWLIB

// external IRX modules
#define EXTERN_IRX(_irx) \
  extern u8 _irx[]; \
  extern int size_##_irx

EXTERN_IRX(usbd_irx);
EXTERN_IRX(usbhdfsd_irx);
EXTERN_IRX(iomanx_irx);
EXTERN_IRX(fileXio_irx);
EXTERN_IRX(sio2man_irx);
EXTERN_IRX(padman_irx);

void IOP_reset();
void loadmodules();


//////////////////////////////////////////////////////////////////////////////////////
// SYSCALLS NECESSARY
//////////////////////////////////////////////////////////////////////////////////////
int H_EnableIntcHandler(int inter)
{
	__asm __volatile__  (
							"addiu  $3, $0, 0x0014  \n"      
							"syscall				\n"   
							"nop					\n"
						);
	return 0;
}
int H_DisableIntcHandler(int inter)
{
	__asm __volatile__  (
							"addiu  $3, $0, 0x0015  \n"      
							"syscall				\n"   
							"nop					\n"
						);
	return 0;
}

// LIST OF ID FOR INTERRUPTS

enum
{
INT_GS,
INT_SBUS,
INT_VBLANK_START,
INT_VBLANK_END,
INT_VIF0,
INT_VIF1,
INT_VU0,
INT_VU1,
INT_IPU,
INT_TIMER0,
INT_TIMER1
};

//////////////////////////////////////////////////////////////////////////////////////
// REGISTERS FOR TIMERS
//////////////////////////////////////////////////////////////////////////////////////

// timer T0
#define T0_COUNT      *((volatile unsigned long*)0x10000000)
#define T0_MODE         *((volatile unsigned long*)0x10000010)
#define T0_COMP         *((volatile unsigned long*)0x10000020)
#define T0_HOLD         *((volatile unsigned long*)0x10000030)

// timer T1
#define T1_COUNT      *((volatile unsigned long*)0x10000800)
#define T1_MODE         *((volatile unsigned long*)0x10000810)
#define T1_COMP         *((volatile unsigned long*)0x10000820)
#define T1_HOLD         *((volatile unsigned long*)0x10000830)

unsigned count_time=0;

//////////////////////////////////////////////////////////////////////////////////////
// interrupt handler
//////////////////////////////////////////////////////////////////////////////////////
int handlerItim(int ca)
{
	count_time+=1; // count in steps of 2 ms

	T0_MODE|=1024; // enable next interrupt
	return -1; // only this handler
}

#define TIME_MS 1.0
#define CLOCK_BUS 147456.0

int id_TIM; // id handler

void IOP_reset() // resets IOP and apply sbv patches.	
{	
	SifInitRpc(0);
	
	while(!SifIopReset("rom0:UDNL rom0:EELOADCNF",0));
	while(!SifIopSync());
	fioExit();
	fileXioExit();
	SifExitIopHeap();
	SifLoadFileExit();
	SifExitRpc();
	SifExitCmd();
	SifInitRpc(0);
  	FlushCache(0);
  	FlushCache(2);
  	//twice, some in-hdloader hack
  	while(!SifIopReset("rom0:UDNL rom0:EELOADCNF",0));
  	while(!SifIopSync());
  	fioExit();
	fileXioExit();
  	SifExitIopHeap();
  	SifLoadFileExit();
  	SifExitRpc();
  	SifExitCmd();

  	SifInitRpc(0);
  	FlushCache(0);
  	FlushCache(2);

  	SifLoadFileInit();
}	

void start_ps2_timer()
{
	T0_MODE=0; // disable timer
	id_TIM=AddIntcHandler(INT_TIMER0,handlerItim,0); // set handler
	H_EnableIntcHandler(INT_TIMER0); // enable handler

	count_time=0; // counter

	T0_COMP=(unsigned) (TIME_MS/(256.0/CLOCK_BUS)); //  adjust comparator to 2 ms
	T0_COUNT=0; // counter at zero
	T0_MODE=256+128+64+2; // set mode to clock=BUSCLK/256, reset to 0,count and interrupt if comparator equal...
}

void stop_ps2_timer()
{
	T0_MODE=0; // disable timer
	H_DisableIntcHandler(INT_TIMER0); // disable handler
	RemoveIntcHandler(INT_TIMER0,id_TIM); // kill handler
}
//////////////////////////////////////////////////////////////////////////

void loadmodules()
{
   int ret, id;
#if 0
    if ((id = SifExecModuleBuffer(&iomanx_irx, size_iomanx_irx, 0, NULL, &ret)) < 0) {
		printf("Error loading iomanx\n");
	}
   ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
   if(ret < 0){
          printf("Error loading rom0:XSIO2MAN\n");
              }
   /*ret = SifLoadModule("rom0:MCMAN", 0, NULL);
   if(ret < 0){
          printf("Error loading rom0:MCMAN\n");
              }
   ret = SifLoadModule("rom0:MCSERV", 0, NULL);
   if(ret < 0){
          printf("Error loading rom0:MCSERV\n");
          }*/
  //#ifdef _IOPRESET

 // #endif            
   ret = SifLoadModule("rom0:PADMAN", 0, NULL);
   if(ret < 0){
          printf("Error loading rom0:XPADMAN\n");
              }
  
  //ret = SifLoadModule("rom0:IOMAN", 0, NULL);
  // if(ret < 0){
     //     printf("Error loading rom0:IOMAN\n");
      //        }
      
    if ((id = SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, &ret)) < 0) {
		printf("Error loading usbd\n");
	}
    if ((id = SifExecModuleBuffer(&usbhdfsd_irx, size_usbhdfsd_irx, 0, NULL, &ret)) < 0) {
		printf("Error loading usbhdfsd\n");
	}
	 
    /*if ((id = SifExecModuleBuffer(&ps2kbd_irx, size_ps2kbd_irx, 0, NULL, &ret)) < 0) {
		scr_printf("Error loading ps2kbd\n");
	}
	 if ((id = SifExecModuleBuffer(&ps2mouse_irx, size_ps2mouse_irx, 0, NULL, &ret)) < 0) {
		scr_printf("Error loading ps2mouse\n");
	}
	if ((id = SifExecModuleBuffer(&ps2snd_irx, size_ps2snd_irx, 0, NULL, &ret)) < 0) {
		scr_printf("Error loading ps2snd\n");
	}*/
/*	ret = SifLoadModule("mass:irx/fileXio.irx", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: fileXio.irx");
		//SleepThread();
	}
	ret = SifLoadModule("mass:irx/ioptrap.irx", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: ioptrap.irx");
		//SleepThread();
	}*/
	
	ret = SifLoadModule("mass:irx/ps2kbd.irx", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: PS2KBD");
		//SleepThread();
	}	

	ret = SifLoadModule("mass:irx/ps2mouse.irx", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: PS2MOUSE");
	//	SleepThread();
	}
#else
    if ((id = SifExecModuleBuffer(&iomanx_irx, size_iomanx_irx, 0, NULL, &ret)) < 0) {
		printf("Error loading iomanx\n");
	}
	
	if ((id = SifExecModuleBuffer(&fileXio_irx, size_fileXio_irx, 0, NULL, &ret)) < 0) {
		printf("Error loading fileXio");		
	}
	fileXioInit(); //After both modules loaded correctly then initialize fileXio.

	if ((id = SifExecModuleBuffer(&sio2man_irx, size_sio2man_irx, 0, NULL, &ret)) < 0) {
		printf("Error loading sio2man");
	}

	if ((id = SifExecModuleBuffer(&padman_irx, size_padman_irx, 0, NULL, &ret)) < 0) {
		printf("Error loading padman");
	}

   /*ret = SifLoadModule("rom0:MCMAN", 0, NULL);
   if(ret < 0){
          printf("Error loading rom0:MCMAN\n");
              }
   ret = SifLoadModule("rom0:MCSERV", 0, NULL);
   if(ret < 0){
          printf("Error loading rom0:MCSERV\n");
          }*/
  //#ifdef _IOPRESET

 // #endif      
      

  //ret = SifLoadModule("rom0:IOMAN", 0, NULL);
  // if(ret < 0){
     //     printf("Error loading rom0:IOMAN\n");
      //        }
      
    if ((id = SifExecModuleBuffer(&usbd_irx, size_usbd_irx, 0, NULL, &ret)) < 0) {
		printf("Error loading usbd\n");
	}
    if ((id = SifExecModuleBuffer(&usbhdfsd_irx, size_usbhdfsd_irx, 0, NULL, &ret)) < 0) {
		printf("Error loading usbhdfsd\n");
	}
	 
    /*if ((id = SifExecModuleBuffer(&ps2kbd_irx, size_ps2kbd_irx, 0, NULL, &ret)) < 0) {
		scr_printf("Error loading ps2kbd\n");
	}
	 if ((id = SifExecModuleBuffer(&ps2mouse_irx, size_ps2mouse_irx, 0, NULL, &ret)) < 0) {
		scr_printf("Error loading ps2mouse\n");
	}
	if ((id = SifExecModuleBuffer(&ps2snd_irx, size_ps2snd_irx, 0, NULL, &ret)) < 0) {
		scr_printf("Error loading ps2snd\n");
	}*/
	
	/*
	ret = SifLoadModule("mass:irx/ioptrap.irx", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: ioptrap.irx");
		//SleepThread();
	}*/
#if 0 //no keyboard at the moment.
	ret = SifLoadModule("mass:irx/ps2kbd.irx", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: PS2KBD");
		//SleepThread();
	}	

	ret = SifLoadModule("mass:irx/ps2mouse.irx", 0, NULL);
	if (ret < 0) {
		printf("Failed to load module: PS2MOUSE");
	//	SleepThread();
	}
#endif
#endif
}

/*
===============================================================================

FILE IO

===============================================================================
*/

#define MAX_HANDLES 10
int sys_handles[MAX_HANDLES];

void inithandle (void)
{
	int i;
	
	for (i=1 ; i<MAX_HANDLES ; i++)
	{
		sys_handles[i] =-1;

	}
}

int findhandle (void)
{
	int i;
	
	for (i=1 ; i<MAX_HANDLES ; i++)
	{
		if(sys_handles[i] == -1)
		{
			return i;
		}
	}
	Sys_Error ("out of handles");
	return -1;
}

/*
================
filelength
================
*/
int filelength (int f)
{
	int pos;
	int end;

	end = lseek(f,0,SEEK_END);
	pos = lseek(f,0,SEEK_SET);

	return end;
}

int Sys_FileOpenRead (char *path, int *hndl)
{
	int f;
	int i;
	
	i = findhandle ();

	f = open(path,O_RDONLY, 0777);

	if (!f)
	{
		*hndl = -1;
		return -1;
	}
	sys_handles[i] = f;
	*hndl = i;
	
	if(filelength(f) < 0)
		return -1;
		
	return filelength(f);
}

int Sys_FileOpenWrite (char *path)
{
	int    f;
	int             i;
	
	i = findhandle ();
	f = open(path,O_WRONLY | O_CREAT, 0777);
	//FIXME
	//if(!f)
	//{
	//	Sys_Error ("Error opening %s: %s", path,strerror(errno));
	//}
	sys_handles[i] = f;
	
	return i;
}

void Sys_FileClose (int handle)
{

	close(sys_handles[handle]);
	sys_handles[handle] = -1;
}

void Sys_FileSeek (int handle, int position)
{
	lseek (sys_handles[handle], position, SEEK_SET);
}

int Sys_FileRead (int handle, void *dest, int count)
{
	return read(sys_handles[handle], dest, count);	
}

int Sys_FileWrite (int handle, void *data, int count)
{
	return write(sys_handles[handle], data, count);
}

int     Sys_FileTime (char *path)
{
	FILE    *f;
	
	f = fopen(path, "rb");
	if (f)
	{
		fclose(f);
		return 1;
	}
	
	return -1;
}

void Sys_mkdir (char *path)
{
#ifdef NEWLIB
	fioMkdir(path);
#else
	mkdir(path, 0777);
#endif
}


/*
===============================================================================

SYSTEM IO

===============================================================================
*/

void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
}


void Sys_Error (char *error, ...)
{
	va_list         argptr;

	printf ("Sys_Error: ");   
	va_start (argptr,error);
	vprintf (error,argptr);
	va_end (argptr);
	printf ("\n");

	exit (1);
}

void Sys_Printf (char *fmt, ...)
{
	va_list         argptr;
	
	va_start (argptr,fmt);
	vprintf (fmt,argptr);
	va_end (argptr);
}

void Sys_Quit (void)
{    
    IOP_reset();
    __asm__ __volatile__(
    "	li $3, 0x04;"
    "	syscall;"
    "	nop;" );

}

float Sys_FloatTime (void)
{
	static float t;
	t = count_time/100;
	return t;	
}

char *Sys_ConsoleInput (void)
{
	return NULL;
}

void Sys_Sleep (void)
{
	
}

void Sys_SendKeyEvents (void)
{
}

void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}

//=============================================================================

int main (int argc, char **argv)
{ 
    #ifdef _IOPRESET
    IOP_reset();
    #endif
	static quakeparms_t    parms;
	float  time, oldtime, newtime;
    
	//signal(SIGFPE, SIG_IGN);
	//SifInitRpc(0);
	loadmodules();
    #ifdef _SOUND 
	 if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
	{
    Sys_Error("VID: Couldn't load SDL: %s", SDL_GetError());
    }
    #endif
/*
	if(mcInit(MC_TYPE_MC) < 0) 
	{
		printf("Failed to initialise memcard\n");
		SleepThread();
	}
*/
	inithandle();
	
	parms.memsize = 24*1024*1024;
	parms.membase = malloc (parms.memsize);
	parms.basedir = ".";

	COM_InitArgv (argc, argv);

	parms.argc = com_argc;
	parms.argv = com_argv;

	printf ("Host_Init\n");
	Host_Init (&parms);
	
	start_ps2_timer();
	
	oldtime = Sys_FloatTime () - 0.1;
    while (1)
    {
// find time spent rendering last frame
        newtime = Sys_FloatTime ();
        time = newtime - oldtime;

		oldtime = newtime;

        Host_Frame (time);
    }
	stop_ps2_timer();
	
	return 0;
}


