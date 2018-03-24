#include <stddef.h>
#include <stdint.h>

#ifdef VMWOS
#include "syscalls.h"
#include "vlibc.h"
#include "vmwos.h"
#else
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#endif
#define p putchar

		     static char w[]="4_6N4_L XSQ$FSQFIQ SFJ"
		     "YWGEXIH f} zq{9Dgsvripp l$jsv$l"
		     "ipt ... 3'` 5WGSVI>$$$ 5WLMIPHW"
		     ">$ 555KEQI$SZIV% 5LMX$VEXMS>$ 4"
		               "_ LIPT5AAAA"
			       "50$qsziw$pi"
			       "jx52$qsziw$"
			       "vmklx5wtegi"
			       "fev$wlssxw5"
			       "w$xskkpiw$w"
			       "syrh5t$teyw"
			       "iw5u$uymxw",





			       _,*O;static int i,s=
			       0,H=5,T,F=0,Z
			       =0,N=1,X=0,E=
			       0,x=7,V[8],M[
			         8],W[8],m
			         ,a;static int r(
			         int s){if
			          (s)a=s;
			          while(!

			          (a&1&&a
			          %5))a++
			          ;a*=997




                        ;a&=0xffffff;return 15&a>>
                        4;}void P(int x){puts(w+x)
                        ;}int G(){return read(0,&_
                        ,1);}void Y(){while(G()<0)
                        ;}  void I(int y,int x)  {
                        printf("%s%d",w+y,x);}void
                        Q(int x){I(114,x/100);p(67
                        );P(x%100);}void B(int c){
                        printf("\033[%d;%d;40m",c>
                        37,c&63);}static struct termios o
                      ,n;void b(int w,int c){B(c);p(
                    w);B ('%');}int main(){O=w;do if(*
                  O==32) *O = 0; else if (*O==52) *O=27;
                else if( *O==53)*O=10;else *O-=4; while(*++
              O);tcgetattr(0,&o);tcgetattr(0,&n); n.c_lflag&=
            ~ICANON;n.c_cc[VMIN]=1;n.c_lflag&=~ECHO;tcsetattr(
          0,TCSANOW,&n);  fcntl (0, F_SETFL,fcntl  (0, F_GETFL)|
        O_NONBLOCK) ; B( '%');for(i=0;i<8;i++)V[i ]= M [i]=W[i]=0;
      while (i){P(0);for (m=0;m<i;m++)P(7);if(m<7 )Q(308);if (m<6)Q(
    319);if(m<5)Q(730);  if(m<4)Q(233);if(m<3)P(7  );if(m<2)Q(346);i--
  ;usleep(300000);}Y();T =r(255);while(!E){B('%'); F++;if(!T){*M|=1<<r(0
                           );T=r(0)-s/15;if(T<1
                         )T=1;}else T--;if(G()){if
                       (_==113)E=1;if(_==44)x--;if(_
                         ==46)x++  ;if  (_==104){P
                        (0)  ;P(   117);   Y();}
                       if    (_       ==    115)
                        N     =!     N;    if  (_
                               ==



     112){Y();    _=0;}if(_==   32){V[7]|=1     <<(x+1);Z   ++;a+=F;}
    if(_         ==27          ){G       ();    if(    _==  91)
    {G()         ;if(          _==       68)    x--    ;if  ( _
     ==67)x++;   }}if          (x<       1)x    ++;if(x>    13)x--;_
           =0;}  P(0)          ;for      (i=    0;i <8;i    ++)
           {for  (m=1;         m<1<<    16;m    <<=   1){   if(
    (V[i]&m)&(    M[i]&m)){b(   42,'_');V[i     ]&=    ~m;  M[i]&=~m;


 if(!(++s&   31)   ) H   ++;}  else if(  V[i        ]&m)b       (33,'a');
else         if(   M[i    ]&   m)b       (84        ,'`' );    else
if(W         [i]   &m)    p(   46);      else       p(   32)   ;}p
 (73);P(7)   ;}m=7<<x;    if   (M[7]&m)  {if(       --    H<0)  E=1;if(!
        H)X  =5;   if(    N)   p(7       );M[       7]     &=~        m;B
       ('_'  );Q   (x*   100   +57       );B(       '%'   );}        else
{B('e');Q(   x *   100  +61);  B('%');}  I(66,s);I  (76,H);p    (10);if(




               !(F&7)){for(i  =7;i>0;i--)W[i  ]=W[i-1];*W=1<<
                r(0);}if(F&    1){for(i=0;i    <7;i++)V[i]=V
                 [i+1];V[7      ]=0;}if(!(      F&3)){for(i
                  =7;i>0;        i--)M[i]        =M[i-1];*
                   M=0;}          usleep          (33000)
                    ;if            (X){            if (!
                    --              X==             0)if
                     (               N               )p

                    (7);}}P(0);P(88);I(65,s);if(Z)I(101
                    ,s*    100/Z);p(37);p(10);p    (10)
                    ;tcsetattr(0,TCSANOW,&o);return 0;}
