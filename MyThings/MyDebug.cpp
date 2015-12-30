/* dbg.cpp */
/* Copyright (C) 2012 mbed.org, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "MyDebug.h"

#include "mbed.h"
#include "rtos.h"

#include "us_ticker_api.h"

#include <cstdio>
#include <cstdarg>

#define REDUCED

using namespace std;

static Serial debug_pc(USBTX, USBRX);

static char debug_newline[3];

static void debug_lock(bool set)
{
  static Mutex* mtx = new Mutex(); //Singleton runtime initialisation to avoid static initialisation chaos problems
  static bool init = false;
  if(set)
  {
    mtx->lock();
    if(!init)
    {
      strncpy( debug_newline, "\n", 2 );
      printf("[START]\n");
      fflush(stdout);
      init = true;
    }
  }
  else
  {
    mtx->unlock();
  }
}

void debug_init()
{
  debug_lock(true); //Force init
  debug_lock(false);
}

void debug_set_newline(const char* newline)
{
  debug_lock(true);
  strncpy( debug_newline, newline, 2 );
  debug_newline[2] = '\0';
  debug_lock(false);
}

void debug_set_speed(int speed)
{
  debug_pc.baud(speed);
}

void debug(int level, const char* module, int line, const char* fmt, ...)
{
  debug_lock(true);
#if not defined(REDUCED)
  switch(level)
  {
  default:
  case 1:
    printf("[ERROR]");
    break;
  case 2:
    printf("[WARN]");
    break;
  case 3:
    printf("[INFO]");
    break;
  case 4:
    printf("[DBG]");
    break;
  }
#endif

#if not defined(RECUDED)
  printf("%10d ", us_ticker_read());
#else
  printf("[%10d] Module %s - Line %d: ", us_ticker_read(), module, line);
#endif
  va_list argp;

  va_start(argp, fmt);
  vprintf(fmt, argp);
  va_end(argp);

  printf(debug_newline);

  fflush(stdout);

  debug_lock(false);

}

void debug_error(const char* module, int line, int ret)
{
  debug_lock(true);
  printf("[RC] Module %s - Line %d : Error %d\n", module, line, ret);
  fflush(stdout);
  debug_lock(false);
}

void debug_exact(const char* fmt, ...)
{
  debug_lock(true);
  va_list argp;

  va_start(argp, fmt);
  vprintf(fmt, argp);
  va_end(argp);
  debug_lock(false);
}

#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS 16
#endif

void debug_memdump(int level, const char* module, int line, const char* msg, const char* mem, int len)
{
    int i,j;
    debug(level,module,line,msg);
    debug_lock(true);
         
    for(i = 0; i < len + ((len % HEXDUMP_COLS) ? (HEXDUMP_COLS - len % HEXDUMP_COLS) : 0); i++) {
        /* print offset */
        if(i % HEXDUMP_COLS == 0) {
            printf("0x%04x: ", ((int)mem)+i);
        }
        
        /* print hex data */
        if(i < len) {
            printf("%02x ", 0xFF & ((char*)mem)[i]);
        } else {
            printf("   ");
        }
        
        /* print ASCII dump */
        if(i % HEXDUMP_COLS == (HEXDUMP_COLS - 1)) {
            for(j = i - (HEXDUMP_COLS - 1); j <= i; j++) {
                if(j >= len) {
                    printf(" ");
                } else if((*(mem+j)>31) && (*(mem+j)<128)){
                    printf("%c",*(mem+j));    
                } else {
                    printf(".");
                }
            }
            printf(debug_newline);
        }
    }
    
    printf(debug_newline);
    fflush(stdout);
    debug_lock(false);
}
