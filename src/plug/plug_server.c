#include <errno.h>
#include <dlfcn.h>
#include "../../include/plug/plug.h"

#ifdef RELOAD
#	include <stdlib.h>
#	include <stdio.h>
#	include "../../vendor/raylib/include/raylib.h"
#else
#	include "../../include/handle_game.h"
#endif

#define return_error(...) \
	do {\
		fprintf(stderr, "%s:%d: ", __FILE__, __LINE__);\
		fprintf(stderr, __VA_ARGS__); \
		fputc('\n', stderr);\
		if(errno != 0) perror("ERROR");\
		exit(1); \
	} while(0)

#define DLSYM(plug, func, sym) \
	do {\
		func = dlsym((plug)->handle, (sym));\
		if((func) == NULL) \
			return_error("dlsym: %s\n", dlerror());\
	} while(0)


void reloadPlugServer(PlugServer* plug){
#ifdef RELOAD
	if(plug->handle != NULL)
		dlclose(plug->handle);

	plug->handle = dlopen(plug->libpath, RTLD_NOW);
	if(plug->handle == NULL)
		return_error("dlopen: %s\n", dlerror());


	DLSYM(plug, plug->handleGameState, "handleGameState");
	//plug->handleGameState = dlsym(plug->handle, "handleGameState");
#else
	plug->handleGameState = handleGameState;
#endif
}
