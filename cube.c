#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <assert.h>
#include <semaphore.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "cube.h"
#include "wizard.h"
#include "wizard.c"

#define DEFAULT_CUBE_SIZE 4
#define DEFAULT_TEAM_SIZE 5
#define DEFAULT_SEED 1
#define HOWBUSY 100000000
#define TRUE 1
#define FALSE 0

int size = 0;
sem_t sem;
sem_t ui;
int threads = 0;
pthread_t thr[1000];

void command_line_usage(){
  fprintf(stderr, "-size <size of cube> -teamA <size of team> -teamB <size of team> -seed <seed value>\n");
}

void kill_wizards(struct wizard *w){
  //Fill in
	pthread_exit(NULL);
  return;
}

int check_winner(struct cube* cube){
  //Fill in
	int counterA = 0;
	int counterB = 0;
	int i, j;
	for(i = 0; i < cube->teamA_size; i++){
		if(cube->teamA_wizards[i]->status == 1){
			counterA++;
//			printf("ID: %d Status: %d\n", cube->teamA_wizards[i]->id, cube->teamA_wizards[i]->status);
		}
	}

	for(j = 0; j < cube->teamB_size; j++){
		if(cube->teamB_wizards[j]->status == 1){
			counterB++;
//			printf("ID: %d Status: %d\n", cube->teamA_wizards[j]->id, cube->teamA_wizards[j]->status);
		}
	}

	if(counterA >= cube->teamA_size){
		printf("Team A lost.\n");
		print_cube(cube);
		return 1;
	}
	if(counterB >= cube->teamB_size){
		printf("Team B lost.\n");
		print_cube(cube);
		return 1;
	}

  return 0;
}

void print_cube(struct cube *cube){
  int i;
  int j;
  int k;

  assert(cube);

  for (i = 0; i < cube->size; i++){
      printf("+");
      for (j = 0; j < cube->size; j++){
	  printf("--+");
	}
      printf("\n|");

      for (j = 0; j < cube->size; j++){
	  /* Print the status of wizards in this room here */
	  for (k = 0; k < 2; k++){
	      if (cube->rooms[j][i]->wizards[k] != NULL){
		  if (cube->rooms[j][i]->wizards[k]->status){
		      printf("%c", tolower(cube->rooms[j][i]->wizards[k]->team));
		    }else{
		      printf("%c", toupper(cube->rooms[j][i]->wizards[k]->team));
		    }
		}else{
		  printf(" ");
		}
	    }
	  printf("|");
	}
      printf("\n");

    }
  printf("+");
  for (j = 0; j < cube->size; j++){
      printf("--+");
    }
  printf("\n");
  return;
}

struct wizard *init_wizard(struct cube* cube, char team, int id){
  int x, newx;
  int y, newy;
  int initflag;
  struct wizard *w;

  w = (struct wizard *)malloc(sizeof(struct wizard));
  assert(w);

  initflag = FALSE;

  w->team = team;
  w->id = id;
  w->status = 0;
  w->cube = cube;

  x = rand() % cube->size;
  y = rand() % cube->size;

  if (cube->rooms[x][y]->wizards[0] == NULL){
      cube->rooms[x][y]->wizards[0] = w;
      w->x = x;
      w->y = y;
    }
  else if (cube->rooms[x][y]->wizards[1] == NULL){
      cube->rooms[x][y]->wizards[1] = w;
      w->x = x;
      w->y = y;
    }else{
      newx = (x + 1) % cube->size;
      if (newx == 0) newy = (y + 1) % cube->size;
      else newy = y;

      while((newx != x || newy != y) && !initflag){

	  if (cube->rooms[newx][newy]->wizards[0] == NULL){
	      cube->rooms[newx][newy]->wizards[0] = w;
	      w->x = newx;
	      w->y = newy;
	      initflag = TRUE;
	    }
	  else if (cube->rooms[newx][newy]->wizards[1] == NULL){
	      cube->rooms[newx][newy]->wizards[1] = w;
	      w->x = newx;
	      w->y = newy;
	      initflag = TRUE;
	    }else{
	      newx = (newx + 1) % cube->size;

	      if (newx == 0){
		  newy = (newy + 1) % cube->size;
		}
	    }

	}
      if (!initflag){
	free(w);
	return NULL;
      }
    }
  //Fill in
//	pthread_t thr; //need thread array
	int r;
	if(threads < size){

		r = pthread_create(&thr[threads], NULL, (void*)(*wizard_func), (void *)w);
		threads++;
		if(r){
			printf("ERROR; Return code from pthread_create");
			exit(-1);
		}
	}


/*	r = pthread_create(&thr, NULL, wizard_func(w), NULL);
	if(r){
		printf("ERROR, Return code from pthread_create");
		exit(-1);
	}
*/
  return w;
}

int interface(void *cube_ref){
  struct cube* cube;
  char *line;
  char *command;
  int i;

  cube = (struct cube *)cube_ref;
  assert(cube);

  using_history();
  while (1){

      line = readline("cube> ");
      if (line == NULL) continue;
      if (strlen(line) == 0) continue;

      add_history(line);

      i = 0;
      while (isspace(line[i])) i++;

      command = &line[i];
      if (!strcmp(command, "exit")){
	  		return 0;
			}else if (!strcmp(command, "show")){
	  		print_cube(cube);
			}
      else if (!strcmp(command, "start")){
	  if (cube->game_status == 1){
	      fprintf(stderr, "Game is over. Cannot be started again\n");
	    }
		  else if (cube->game_status == 0){
		      fprintf(stderr, "Game is in progress. Cannot be started again\n");
		  }else{
		      cube->game_status = 0;
		      //Start the game
		    }
		}else if (!strcmp(command, "stop")){
		  //Stop the game
		  return 1;
		}else if(!strcmp(command, "s") && cube->game_status == 0){
			sem_post(&sem);
			sem_wait(&ui);
		}else if(!strcmp(command, "c") && cube->game_status == 0){
			while(check_winner(cube) != 0){
				sem_post(&sem);
				dostuff();
				sem_wait(&ui);
			}
		}else{
		  fprintf(stderr, "unknown command %s\n", command);
		}
      free(line);
    }

  return 0;
}

int main(int argc, char** argv){
  int cube_size = DEFAULT_CUBE_SIZE;
  int teamA_size = DEFAULT_TEAM_SIZE;
  int teamB_size = DEFAULT_TEAM_SIZE;
  unsigned seed = DEFAULT_SEED;
  struct cube *cube;
  struct room *room;
  struct room **room_col;
  int res;
  struct wizard *wizard_descr;
  int i, j;


  /* Parse command line and fill:
     teamA_size, timeBsize, cube_size, and seed */
  i = 1;
  while(i < argc) {
      if (!strcmp(argv[i], "-size")) {
	  i++;
	  if (argv[i] == NULL) {
              fprintf(stderr, "Missing cube size\n");
              command_line_usage();
              exit(-1);
            }
          cube_size = atoi(argv[i]);
	  if (cube_size == 0){
              fprintf(stderr, "Illegal cube size\n");
              exit(-1);
	    }
        }
      else if (!strcmp(argv[i], "-teamA")) {
          i++;
          if (argv[i] == NULL) {
              fprintf(stderr, "Missing team size\n");
              command_line_usage();
              exit(-1);
            }
          teamA_size = atoi(argv[i]);
	  if (teamA_size == 0){
              fprintf(stderr, "Illegal team size\n");
              exit(-1);
	    }
        }
      else if (!strcmp(argv[i], "-teamB")) {
          i++;
          if (argv[i] == NULL) {
              fprintf(stderr, "Missing team size\n");
              command_line_usage();
              exit(-1);
            }
          teamB_size = atoi(argv[i]);
	  if (teamB_size == 0){
              fprintf(stderr, "Illegal team size\n");
              exit(-1);
	    }
        }
      else if (!strcmp(argv[i], "-seed")) {
          i++;
          if (argv[i] == NULL) {
              fprintf(stderr, "Missing seed value\n");
              command_line_usage();
              exit(-1);
            }
          seed = atoi(argv[i]);
	  if (seed == 0){
              fprintf(stderr, "Illegal seed value\n");
              exit(-1);
	    }
        }else{
          fprintf(stderr, "Unknown command line parameter %s\n", argv[i]);
          command_line_usage();
          exit(-1);
        }
      i++;
    }

  /* Sets the random seed */
  srand(seed);

  /* Checks that the number of wizards does not violate
     the "max occupancy" constraint */
  if ((teamA_size + teamB_size) > ((cube_size * cube_size) * 2)){
      fprintf(stderr, "Sorry but there are too many wizards!\n");
      exit(1);
    }

	size = teamA_size + teamB_size;

  /* Creates the cube */
  cube = (struct cube *)malloc(sizeof(struct cube));
  assert(cube);
  cube->size = cube_size;
  cube->game_status = -1;

  /* Creates the rooms */
  cube->rooms = malloc(sizeof(struct room **) * cube_size);
  assert(cube->rooms);

  for (i = 0; i < cube_size; i++){
      /* Creates a room column */
      room_col = malloc(sizeof(struct room *) * cube_size);
      assert(room_col);

      for (j = 0; j < cube_size; j++){
	  /* Creates a room */
	  room = (struct room *)malloc(sizeof(struct room));
	  assert(room);
	  room->x = i;
	  room->y = j;
	  room->wizards[0] = NULL;
	  room->wizards[1] = NULL;
	  room_col[j] = room;

	  //Fill in
		room->lock = 0;
		}
      cube->rooms[i] = room_col;
    }

  /* Creates the wizards and positions them in the cube */
  cube->teamA_size = teamA_size;
  cube->teamA_wizards = (struct wizard **)malloc(sizeof(struct wizard *) *
						 teamA_size);
  assert(cube->teamA_wizards);

  cube->teamB_size = teamB_size;
  cube->teamB_wizards = (struct wizard **)malloc(sizeof(struct wizard *) *
						 teamB_size);

  assert(cube->teamB_wizards);
//	printf("Initializing wizards");

	sem_init(&sem, 0, 0);
	sem_init(&ui, 0, 0);
 /* Team A */
  for (i = 0; i < teamA_size; i++){

      if ((wizard_descr = init_wizard(cube, 'A', i)) == NULL){
			  fprintf(stderr, "Wizard initialization failed (Team A number %d)\n", i);
			  exit(1);
	}
      cube->teamA_wizards[i] = wizard_descr;
    }

  /* Team B */

  for (i = 0; i < teamB_size; i++){
    if ((wizard_descr = init_wizard(cube, 'B', i)) == NULL){
	  fprintf(stderr, "Wizard initialization failed (Team B number %d)\n", i);
	  exit(1);
		}
      cube->teamB_wizards[i] = wizard_descr;
  }

  //Fill in 
	//Double for loop to set the initial value of lock room to 1 if 2 wizards spawn in the room
	for(i = 0; i < cube_size; i++){
		for(j = 0; j < cube_size; j++){
			if(room->wizards[0] != NULL && room->wizards[1] !=0)
				room->lock = 1;
		}

	}

  /* Goes in the interface loop */
  res = interface(cube);

  exit(res);
}

void dostuff(){
  int i;
  int wait;

  wait = rand() % HOWBUSY;

  for (i = 0; i < wait; i++) {}

  return;
}

struct room *choose_room(struct wizard* w){
  int newx = 0;
  int newy = 0;

  /* The two values cannot be both 0 - no move - or 1 - diagonal move */
  while (newx == newy){
      newx = rand() % 2;
      newy = rand() % 2;
    }
  if ((rand() % 2) == 1) {
    newx = 0 - newx;
    newy = 0 - newy;
  }
  return w->cube->rooms[(w->x + w->cube->size + newx) % w->cube->size][(w->y + w->cube->size + newy) % w->cube->size];
}

int try_room(struct wizard *w, struct room *oldroom, struct room* newroom){
  //Fill in
  /*Checks to see if wizard is not frozen*/
  	if(w->status == 0){
		if(newroom->lock == 1){		//Room locked
			printf("Request denied. Room locked\n");
			return 1;
		}

		else{
			return 0;		//Room unlocked and wizard isn't frozen
		}
	}

	else{
  	return 1;		//Wizard frozen, continues trying to get room
	}
}

struct wizard *find_opponent(struct wizard* self, struct room *room){
  struct wizard *other = NULL;

  //Updates room wizards and determines opponent
  if (room->wizards[0] == self){
      other = room->wizards[1];
    }
  else if (room->wizards[1] == self){
      other = room->wizards[0];
    }

  return other;
}

void switch_rooms(struct wizard *w, struct room *oldroom, struct room* newroom){
  struct wizard *other;

  /* Removes self from old room */
  if (oldroom->wizards[0] == w){
      oldroom->wizards[0] = NULL;
    }
  else if (oldroom->wizards[1] == w){
      oldroom->wizards[1] = NULL;
    }
  else /* This should never happen */
    {
      printf("Wizard %c%d in room (%d,%d) can't find self!\n",
	     w->team, w->id, oldroom->x, oldroom->y);
      print_cube(w->cube);
      exit(1);
    }

  //Fill in
	if(oldroom->lock == 1){
		oldroom->lock = 0;
	}
  //Updates room wizards and determines opponent
  if (newroom->wizards[0] == NULL){
      newroom->wizards[0] = w;
      other = newroom->wizards[1];
    }else if (newroom->wizards[1] == NULL){
      newroom->wizards[1] = w;
      other = newroom->wizards[0];
    }else{
      printf("Wizard %c%d in room (%d,%d) gets in a room already filled with people!\n",
	     w->team, w->id, newroom->x, newroom->y);
      print_cube(w->cube);
      exit(1);
    }

	//Lock if two wizards are in room
	if(newroom->wizards[0] != NULL && newroom->wizards[1] != NULL){
		newroom->lock = 1;
	}

  //Sets self's location to current room
  w->x = newroom->x;
  w->y = newroom->y;
}

int fight_wizard(struct wizard *self, struct wizard *other, struct room *room){
  int res;

  /* Computes the result of the fight */
  res = rand() % 2;

  /* The opponent becomes frozen */
  if (res == 0){
      printf("Wizard %c%d in room (%d,%d) freezes enemy %c%d\n",
	     self->team, self->id, room->x, room->y,
	     other->team, other->id);
      //Fill in
	other->status = 1; //Other wizard gets frozen
    }

  /* Self freezes and release the lock */
  else{
      printf("Wizard %c%d in room (%d,%d) gets frozen by enemy %c%d\n",
	     self->team, self->id, room->x, room->y,
	     other->team, other->id);
      //Fill in
	  self->status = 1; //Self wizard gets frozen
      return 1;
    }
  return 0;
}

int free_wizard(struct wizard *self, struct wizard *other, struct room* room){
  int res;
  //Computes the results of the unfreeze spell
  res = rand() % 2;
  //The friend is unfrozen
  if (res == 0)
    {
      printf("Wizard %c%d in room (%d,%d) unfreezes friend %c%d\n",
	     self->team, self->id, room->x, room->y,
	     other->team, other->id);
      //Fill in
	  other->status = 0; //Other wizard gets unfrozen
    }
  /* The spell failed */
  else{
	 printf("Wizard %c%d in room (%d,%d) fails to unfreeze friend %c%d\n",
	 self->team, self->id, room->x, room->y,
	 other->team, other->id);
	}
  return 0;
}
