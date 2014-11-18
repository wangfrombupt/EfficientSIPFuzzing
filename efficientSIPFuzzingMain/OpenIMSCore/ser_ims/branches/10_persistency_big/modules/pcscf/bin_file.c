/*
 * $Id: bin_file.c 161 2007-03-01 14:06:01Z vingarzan $
 *
 * Copyright (C) 2004-2007 FhG Fokus
 *
 * This file is part of Open IMS Core - an open source IMS CSCFs & HSS
 * implementation
 *
 * Open IMS Core is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * For a license to use the Open IMS Core software under conditions
 * other than those described here, or to purchase support for this
 * software, please contact Fraunhofer FOKUS by e-mail at the following
 * addresses:
 *     info@open-ims.org
 *
 * Open IMS Core is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * It has to be noted that this Open Source IMS Core System is not
 * intended to become or act as a product in a commercial context! Its
 * sole purpose is to provide an IMS core reference implementation for
 * IMS technology testing and IMS application prototyping for research
 * purposes, typically performed in IMS test-beds.
 *
 * Users of the Open Source IMS Core System have to be aware that IMS
 * technology may be subject of patents and licence terms, as being
 * specified within the various IMS-related IETF, ITU-T, ETSI, and 3GPP
 * standards. Thus all Open IMS Core users have to take notice of this
 * fact and have to agree to check out carefully before installing,
 * using and extending the Open Source IMS Core System, if related
 * patents and licences may become applicable to the intended usage
 * context.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/**
 * \file
 *
 * Binary codec operations - file dump/load
 *
 *  \author Dragos Vingarzan vingarzan -at- fokus dot fraunhofer dot de
 *
 */


#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>

#include "bin_file.h"




/* file dumping routines */


int bin_files_keep_count=3;				/**< how many old snapshots to keep				*/



/**
 * Creates the file to write to.
 * The file names contain the time of the dump. If a file was dumped 
 * just partialy it will contain a ".part" in the name. 
 * A link to the last complete file is created each time.
 * Old files are deleted and only scscf_persistency_keep_count time-stamped-files are kept.
 * @param location - where to place the file
 * @param prepend_fname - what to prepend to the file_name
 * @returns the file handler on success or 0 on failure
 */
FILE* bin_dump_to_file_create(char *location,char *prepend_fname,time_t unique)
{
	char c_part[256];
	FILE *f;
	
	sprintf(c_part,"%s/%s_%.10u.bin.part",location,prepend_fname,(unsigned int)unique);

	/* first dump to a partial file */	
	f = fopen(c_part,"w");
	if (!f){
		LOG(L_ERR,"ERR:"M_NAME":bin_dump_to_file_create: error when opening file <%s> for writting [%s]\n",c_part,strerror(errno));
		return 0;
	}
	return f;
}

/**
 * Writes some data to the file.
 * @param file - the file descriptor
 * @param x - the binary data to write
 * @returns the number of written bytes
 */
int bind_dump_to_file_append(FILE* file, bin_data *x)
{
	int k=0,l;
	do {
		l = fwrite(x->s+k,1,x->len-k,file);
		k += l;
	} while(l!=0 && k<x->len);
	LOG(L_INFO,"INFO:"M_NAME":bin_dump_to_file: Dumped %d bytes.\n",k);
	fflush(file);
	return k;			
}

/**
 * Closes the file and moves it properly.
 * The file names contain the time of the dump. If a file was dumped 
 * just partialy it will contain a ".part" in the name. 
 * A link to the last complete file is created each time.
 * Old files are deleted and only scscf_persistency_keep_count time-stamped-files are kept.
 * @param file - the file descriptor
 * @param x - the binary data to write
 * @returns the number of written bytes
 */
int bind_dump_to_file_close(FILE* file,char *location,char *prepend_fname,time_t unique)
{	
	char c_part[256],c_time[256],c_last[256];

	sprintf(c_part,"%s/%s_%.10u.bin.part",location,prepend_fname,(unsigned int)unique);
	sprintf(c_time,"%s/%s_%.10u.bin",location,prepend_fname,(unsigned int)unique);
	sprintf(c_last,"%s/_%s.bin",location,prepend_fname);

	/* then rename it as a complete file with timestamp */
	if (rename(c_part,c_time)<0){
		LOG(L_ERR,"ERR:"M_NAME":bin_dump_to_file: error when renaming  <%s> -> <%s> [%s]\n",c_part,c_time,strerror(errno));
		return 0;
	}
	
	/* then link the last snapshot to it */
	if (remove(c_last)<0 && errno!=ENOENT){
		LOG(L_ERR,"ERR:"M_NAME":bin_dump_to_file: error when removing symlink <%s> [%s]\n",c_last,strerror(errno));
		return 0;
	}
	if (symlink(c_time,c_last)<0){
		LOG(L_ERR,"ERR:"M_NAME":bin_dump_to_file: error when symlinking <%s> -> <%s> [%s]\n",c_time,c_last,strerror(errno));
		return 0;
	}
	/* then remove old snapshots */
	{
		struct dirent **namelist;
		int i,n,k=bin_files_keep_count;
		int len=strlen(prepend_fname);					
		n = scandir(location,&namelist,0,alphasort);
		if (n>0){
			for(i=n-1;i>=0;i--){
				if (strlen(namelist[i]->d_name)>len &&
					memcmp(namelist[i]->d_name,prepend_fname,len)==0) {
					if (k) k--;
					else {							
						sprintf(c_part,"%s/%s",location,namelist[i]->d_name);
						remove(c_part);
					}
				}
				free(namelist[i]);
			}
			free(namelist);
		}
	}
	return 1;	
}

/**
 * Opens the file and returns a file handler to it
 * @param location - where the file is located
 * @param prepend_fname - with what to prepend the filename
 * @returns the file handler
 */
FILE* bin_load_from_file_open(char *location,char *prepend_fname)
{
	char c[256];
	FILE *f;
	
	sprintf(c,"%s/_%s.bin",location,prepend_fname);
	f = fopen(c,"r");
	if (!f) {
		LOG(L_ERR,"ERR:"M_NAME":bin_load_from_file: error opening %s : %s\n",c,strerror(errno));
		return 0;
	}
	return f;
}

/**
 * Read the next len bytes from the file
 * @param f - the file descriptor
 * @param x - where to load. it will load from x.max until x.len
 * @returns how many bytes have been read
 */
int bin_load_from_file_read(FILE* f,bin_data *x)
{
	int left=x->max - x->len;
	int cnt=0,k;
	while(!feof(f)&&left>0){
		k = fread(x->s+x->len,1,left,f);
		x->len += k;
		left = left - k;
		cnt+=k;
	}
	LOG(L_INFO,"INFO:"M_NAME":bin_load_from_file: Read %d bytes from persistency file.\n",cnt);
	return cnt;
}


void bin_load_from_file_close(FILE* f)
{
	fclose(f);	
}

/* end of bin library functions */
