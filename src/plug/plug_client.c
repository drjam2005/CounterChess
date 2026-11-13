#include <errno.h>
#include <dlfcn.h>
#include "../../include/plug/plug.h"

#ifdef RELOAD
#	include <stdlib.h>
#	include <stdio.h>
#	include "../../vendor/raylib/include/raylib.h"
#else
#	include "../../include/render_assets.h"
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

void reloadPlugClient(PlugClient* plug){
#ifdef RELOAD
	bool windowReady = IsWindowReady();
	if (windowReady && plug->unloadPieceTextures != NULL)
		plug->unloadPieceTextures();

	if(plug->handle != NULL)
		dlclose(plug->handle);

	plug->handle = dlopen(plug->libpath, RTLD_NOW);
	if(plug->handle == NULL)
		return_error("dlopen: %s\n", dlerror());

	plug->renderBoard = dlsym(plug->handle, "renderBoard");
	if(plug->renderBoard == NULL)
		return_error("dlsym: %s\n", dlerror());

	plug->loadPieceTextures = dlsym(plug->handle, "loadPieceTextures");
	if(plug->loadPieceTextures == NULL)
		return_error("dlsym: %s\n", dlerror());

	plug->unloadPieceTextures = dlsym(plug->handle, "unloadPieceTextures");
	if(plug->unloadPieceTextures == NULL)
		return_error("dlsym: %s\n", dlerror());

	plug->toBoardPosition = dlsym(plug->handle, "toBoardPosition");
	if(plug->toBoardPosition == NULL)
		return_error("dlsym: %s\n", dlerror());

	DLSYM(plug, plug->renderMessages, "renderMessages");

	if (windowReady)
		plug->loadPieceTextures();
#else
	plug->renderBoard = renderBoard;
	plug->loadPieceTextures = loadPieceTextures;
	plug->unloadPieceTextures = unloadPieceTextures;
	plug->toBoardPosition = toBoardPosition;
	plug->renderMessages = renderMessages;
#endif
}
