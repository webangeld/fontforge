/* Copyright (C) 2005 by George Williams */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "plugins.h"
#include "dynamic.h"

#include <sys/types.h>
#include <dirent.h>

void LoadPlugin(char *dynamic_lib_name) {
    void *plugin;
    char *pt, *spt, *freeme = NULL;
    int (*init)(void);

    spt = strrchr(dynamic_lib_name,'/');
    if ( spt==NULL )
	spt = dynamic_lib_name;
    pt = strrchr(spt,'.');
    if ( pt==NULL ) {
	freeme = galloc(strlen(dynamic_lib_name)+strlen(SO_EXT)+4);
	strcpy(freeme,dynamic_lib_name);
	strcat(freeme,SO_EXT);
	dynamic_lib_name = freeme;
    }
    plugin = dlopen(dynamic_lib_name,RTLD_LAZY);
    if ( plugin==NULL ) {
	LogError("Failed to dlopen: %s", dynamic_lib_name);
	free(freeme);
return;
    }
    init = (int (*)(void)) dlsym(plugin,"FontForgeInit");
    if ( init==NULL ) {
	LogError("Failed to find init function in %s", dynamic_lib_name);
	dlclose(plugin);
	free(freeme);
return;
    }
    if ( (*init)()==0 )
	/* Init routine in charge of any error messages */
	dlclose(plugin);
    free(freeme);
}

static int isdyname(char *filename) {
    char *pt;

    pt = strrchr(filename,'.');
    if ( pt==NULL )
return( false );
    if ( strcmp(pt,SO_EXT)==0 )
return( true );

return( false );
}

void LoadPluginDir(char *dir) {
    DIR *diro;
    struct dirent *ent;
    char buffer[1025];

    if ( dir==NULL ) {
	/* First load system plug-ins */
#if defined( PLUGINDIR )
	LoadPluginDir( PLUGINDIR );
#elif defined( PREFIX )
	LoadPluginDir( PREFIX "/share/fontforge/plugins" );
#endif
	/* Then load user defined ones */
	if ( getPfaEditDir(buffer)!=NULL ) {
	    strcpy(buffer,getPfaEditDir(buffer));
	    strcat(buffer,"/plugins");
	    LoadPluginDir(buffer);
	}
return;
    }

    diro = opendir(dir);
    if ( diro==NULL )		/* It's ok not to have any plugins */
return;
    
    while ( (ent = readdir(diro))!=NULL ) {
	if ( isdyname(ent->d_name) ) {
	    sprintf( buffer, "%s/%s", dir, ent->d_name );
	    LoadPlugin(buffer);
	}
    }
    closedir(diro);
}