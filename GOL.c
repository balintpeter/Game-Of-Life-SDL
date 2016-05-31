#include <SDL.h>
#include <SDL_config_win32.h>
#include <SDL_image.h>
#include <SDL_gfxPrimitives.h>
#include <math.h>

#define MAX 80
#define KERETSZIN 0xE1CFD3FF
#define NEIGHBOUR cells[y-1][x].alive+cells[y-1][x+1].alive+cells[y][x+1].alive+cells[y+1][x+1].alive+cells[y+1][x].alive+cells[y+1][x-1].alive+cells[y][x-1].alive+cells[y-1][x-1].alive
#define DIEIF cells[y][x].alive==1 && (NEIGHBOUR<=1 || NEIGHBOUR >=4)
#define BORNIF cells[y][x].alive==0 && NEIGHBOUR==3 && cells[y][x].mod==0
#define SURVIVEIF cells[y][x].alive==1 && NEIGHBOUR==2 || NEIGHBOUR==3

typedef struct 
{
	int alive;
	int mod;
	int willdie;
	int willlive;
}cell;

enum{MERET=10, KERET=15,HEZAG=50};

int palyapos(int koord) {
	return MERET*koord+KERET;
}

void drawcell(cell cells[MAX][MAX],SDL_Surface *cel,SDL_Surface *kep)
{
	int x,y;	

	for(y=0;y<MAX;y++)
		for(x=0;x<MAX;x++)
		{
			if(cells[y][x].alive==1)
			{
				SDL_Rect src={0,0,MERET,MERET};
				SDL_Rect dest={x*MERET+KERET,y*MERET+KERET,0,0};
				SDL_BlitSurface(kep,NULL, cel, &dest);
			}
			else if (x%2 != y%2)
				boxColor(cel,palyapos(x),palyapos(y),palyapos(x+1)-1,palyapos(y+1)-1,0xFFDFD2FF);
			else 
				boxColor(cel,palyapos(x),palyapos(y),palyapos(x+1)-1,palyapos(y+1)-1,0xE5C8BDFF);


		}
}

void round(cell cells[MAX][MAX])
{
	int x,y;

	for(y=1;y<MAX-1;y++)
	{
		for(x=1;x<MAX-1;x++)
			if(cells[y][x].mod==0)
				if(DIEIF)
				{
					cells[y][x].mod=1;
					cells[y][x].willdie=1;
				}
	}

	for(y=1;y<MAX-1;y++)
	{
		for(x=1;x<MAX-1;x++)
		{
			if(cells[y][x].mod==0)
				if(BORNIF)
				{
					cells[y][x].mod=1;
					cells[y][x].willlive=1;
				}
		}
	}	

	for(y=1;y<MAX-1;y++)
	{
		for(x=1;x<MAX-1;x++)
		{
			if(cells[y][x].willdie==1)
			{
				cells[y][x].alive=0;
				cells[y][x].willdie=0;
			}
			else if(cells[y][x].willlive==1)
			{

				cells[y][x].alive=1;
				cells[y][x].willlive=0;
			}
			cells[y][x].mod=0;		
		}
	}
}


int holazeger(int egerx)
{
	int x;
	for(x=0;x<MAX;x++)
	{
		if(x*MERET+KERET<=egerx && (x+1)*MERET+KERET>=egerx)
			return x;
	}

}

void mentes(cell cells[MAX][MAX])
{
	int x,y;
	FILE *fp;
	fp=fopen("save.txt","w");

	for(y=0;y<MAX;y++)
		for(x=0;x<MAX;x++)
			fprintf(fp,"%d ",cells[y][x].alive);

	fclose (fp);
}

int incircle(double egerx, double egery, double pontx, double ponty, int r)
{
	if((sqrt(pow(egerx-pontx,2)+pow(egery-ponty,2)))<=r)
		return 1;
	else return 0;
}


int main(int argc, char *argv[]) {
	SDL_Event ev,az;
	SDL_Surface *screen;
	SDL_Surface *kep;
	SDL_TimerID id;
	FILE *fp;
	int x, y,click=0,clicktwo=0,aut=0,quit=0,gomb=0,egerx,egery,nothinghappened=1;
	cell cells[MAX][MAX]={0};

	kep=IMG_Load("sejt.png");
	if(!kep)
		fprintf(stderr, "Nem sikerult betolteni a kepfajlt!\n");

	/* SDL inicializálása és ablak megnyitása */
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	screen=SDL_SetVideoMode(MAX*MERET+KERET*2+100, MAX*MERET+KERET*2-100, 0, SDL_FULLSCREEN);
	if (!screen) {
		fprintf(stderr, "Nem sikerult megnyitni az ablakot!\n");
		exit(1);
	} 
	SDL_WM_SetCaption("Game Of Life", "Game Of Life");
	
	SDL_FillRect(screen, NULL, 0x433e3f);
	boxColor(screen,KERET/3,KERET/3,MAX*MERET+KERET*2-KERET/3,MAX*MERET+KERET*2-KERET/3,KERETSZIN);

	drawcell(cells,screen,kep);



	while(!quit)
	{
		boxColor(screen, MAX*MERET+KERET*2+10,KERET+5,MAX*MERET+KERET*2+90,KERET+40,0xFFDFD2FF);
		boxColor(screen, MAX*MERET+KERET*2+14,KERET+9,MAX*MERET+KERET*2+86,KERET+36,0xE5C8BDFF);
		stringRGBA(screen, MAX*MERET+KERET*2+20, KERET+19, "Leptetes", 255, 255, 255, 255);

		boxColor(screen, MAX*MERET+KERET*2+10,KERET+5+HEZAG,MAX*MERET+KERET*2+90,KERET+40+HEZAG,0xFFDFD2FF);
		boxColor(screen, MAX*MERET+KERET*2+14,KERET+9+HEZAG,MAX*MERET+KERET*2+86,KERET+36+HEZAG,0xE5C8BDFF);
		if(aut==0)
			stringRGBA(screen, MAX*MERET+KERET*2+15, KERET+69, "Szimul.be", 255, 255, 255, 255);
		else
			stringRGBA(screen, MAX*MERET+KERET*2+15, KERET+69, "Szimul.ki", 255, 255, 255, 255);

		boxColor(screen, MAX*MERET+KERET*2+10,KERET+5+HEZAG*2,MAX*MERET+KERET*2+90,KERET+40+HEZAG*2,0xFFDFD2FF);
		boxColor(screen, MAX*MERET+KERET*2+14,KERET+9+HEZAG*2,MAX*MERET+KERET*2+86,KERET+36+HEZAG*2,0xE5C8BDFF);
		stringRGBA(screen, MAX*MERET+KERET*2+26, KERET+19+HEZAG*2, "Torles", 255, 255, 255, 255);

		boxColor(screen, MAX*MERET+KERET*2+10,KERET+5+HEZAG*3,MAX*MERET+KERET*2+90,KERET+40+HEZAG*3,0xFFDFD2FF);
		boxColor(screen, MAX*MERET+KERET*2+14,KERET+9+HEZAG*3,MAX*MERET+KERET*2+86,KERET+36+HEZAG*3,0xE5C8BDFF);
		stringRGBA(screen, MAX*MERET+KERET*2+27, KERET+19+HEZAG*3, "Kilovo", 255, 255, 255, 255);

		filledCircleColor(screen,MAX*MERET+2*KERET+80,9,8,0xFFDFD2FF);
		filledCircleColor(screen,MAX*MERET+2*KERET+80,9,6,0xE5C8BDFF);
		stringRGBA(screen,MAX*MERET+KERET*2+77,6,"X",255,255,255,255);
		
		SDL_Flip(screen);

		
		while(SDL_PollEvent(&ev)){
			switch(ev.type)
			{
				/*case SDL_KEYDOWN:
				switch(ev.key.keysym.sym)
				{
				case SDLK_s:
				mentes(cells);		   
				break;
				case SDLK_l:
				fp=fopen("save.txt","r");

				for(y=0;y<MAX;y++)
				for(x=0;x<MAX;x++)
				fscanf(fp,"%d ",&cells[y][x].alive);

				fclose (fp);

				drawcell(cells,screen,kep);
				SDL_Flip(screen);
				break;
				}
				break;*/

			case SDL_MOUSEBUTTONDOWN:
				if(ev.button.button==SDL_BUTTON_LEFT)
				{
					if(ev.button.x<=MAX*MERET+KERET){
						egerx=holazeger(ev.button.x);
						egery=holazeger(ev.button.y);

						if(cells[egery][egerx].alive==1)
							cells[egery][egerx].alive=0;
						else
							cells[egery][egerx].alive=1;
					}

					else if(incircle(ev.button.x,ev.button.y,MAX*MERET+2*KERET+80,9,8))
						quit=1;

					else if((ev.button.x<=MAX*MERET+KERET*2+90 && ev.button.x>=MAX*MERET+KERET*2+10) && (ev.button.y<=KERET+40 && ev.button.y>=KERET+5))//egyes lépés
					{
						round(cells);

					}
					else if((ev.button.x<=MAX*MERET+KERET*2+90 && ev.button.x>=MAX*MERET+KERET*2+10) && (ev.button.y<=KERET+90 && ev.button.y>=KERET+55))//szimulálás
					{
						if(aut==0)
							aut=1;
						else aut=0;	
					}
					else if((ev.button.x<=MAX*MERET+KERET*2+90 && ev.button.x>=MAX*MERET+KERET*2+10) && (ev.button.y<=KERET+40+HEZAG*2 && ev.button.y>=KERET+5+HEZAG*2))//egyes lépés
					{
						for(y=0;y<MAX;y++)
							for(x=0;x<MAX;x++)
								cells[y][x].alive=0;
					}
					else if((ev.button.x<=MAX*MERET+KERET*2+90 && ev.button.x>=MAX*MERET+KERET*2+10) && (ev.button.y<=KERET+40+HEZAG*3 && ev.button.y>=KERET+5+HEZAG*3))//egyes lépés
					{
						fp=fopen("save.txt","r");

						for(y=0;y<MAX;y++)
							for(x=0;x<MAX;x++)
								fscanf(fp,"%d ",&cells[y][x].alive);

						fclose (fp);

						drawcell(cells,screen,kep);
						SDL_Flip(screen);
					}
					drawcell(cells,screen,kep);
					SDL_Flip(screen);
				}
				break;

			case SDL_MOUSEBUTTONUP:
				if(ev.button.button==SDL_BUTTON_LEFT)
				{
					click=0;
					clicktwo=0;
				}
				break;

			case SDL_QUIT:
				quit=1;
				break;
			}
		}

		if(aut)
		{
			SDL_Delay(100);
			round(cells);
			drawcell(cells,screen,kep);
			SDL_Flip(screen);
		}


	}


	SDL_FreeSurface(kep);
	SDL_Quit();
	exit(0);
	return 0;
}
