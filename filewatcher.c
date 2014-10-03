/*
 * Copyright (C) 2014  carlos neira
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/inotify.h>

void get_event (int fd, const char * target);
uint32_t  szmask_to_uint32mask (const char * szmask);
void handle_error (int error);

/* ----------------------------------------------------------------- */

int main (int argc, char *argv[])
{
  char target[FILENAME_MAX];
  int fd;
  int wd;  
  struct rlimit rl;
  rl.rlim_cur = 3072;
  rl.rlim_max = 8172;
  uint32_t mask=0;
  setrlimit (RLIMIT_NOFILE, &rl);

  if (argc < 2) {
    fprintf (stderr, "Watching the current directory\n");
    fprintf (stderr, "No mask specified using IN_ALL_EVENTS\n");
    strcpy (target, ".");
    mask = IN_ALL_EVENTS;
  }
  else {
    fprintf (stderr, "Watching %s\n", argv[1]);
    strcpy (target, argv[1]);
#ifdef _DEBUG
    fprintf(stderr, "argv[1] <%s> argv[2] <%s>  argc %d", argv[1],argv[2],argc);
#endif
    mask = szmask_to_uint32mask(argv[2]);
  }

  fd = inotify_init();
  if (fd < 0) {
    printf("inotify_init failed\n");
    handle_error (errno);
    return -1;
  }

  wd = inotify_add_watch (fd, target, mask);

  if (wd < 0) {
    printf("add_watch failed\n");
    handle_error (errno);
    return -1;
  }

for (;;) 
    get_event(fd, target);
  

  return 0;
}

/* ----------------------------------------------------------------- */
/* Allow for 1024 simultanious events */
#define BUFF_SIZE ((sizeof(struct inotify_event)+FILENAME_MAX)*1024)

void get_event (int fd, const char * target)
{
  ssize_t len, i = 0;
  char buff[BUFF_SIZE] = {0};

  len = read (fd, buff, BUFF_SIZE);

  while (i < len) {
    struct inotify_event *pevent = (struct inotify_event *)&buff[i];
    char action[81+FILENAME_MAX] = {0};

    if (pevent->len)
      strcpy (action, pevent->name);
    else
      strcpy (action, target);

    if (pevent->mask & IN_ACCESS)
      strcat(action, " was read");
    if (pevent->mask & IN_ATTRIB)
      strcat(action, " Metadata changed");
    if (pevent->mask & IN_CLOSE_WRITE)
      strcat(action, " opened for writing was closed");
    if (pevent->mask & IN_CLOSE_NOWRITE)
      strcat(action, " not opened for writing was closed");
    if (pevent->mask & IN_CREATE)
      strcat(action, " created in watched directory");
    if (pevent->mask & IN_DELETE)
      strcat(action, " deleted from watched directory");
    if (pevent->mask & IN_DELETE_SELF)
      strcat(action, " watched file/directory was itself deleted");
    if (pevent->mask & IN_MODIFY)
      strcat(action, " was modified");
    if (pevent->mask & IN_MOVE_SELF)
      strcat(action, " watched file/directory was itself moved");
    if (pevent->mask & IN_MOVED_FROM)
      strcat(action, " moved out of watched directory");
    if (pevent->mask & IN_MOVED_TO)
      strcat(action, " moved into watched directory");
    if (pevent->mask & IN_OPEN)
      strcat(action, " was opened");

#ifdef _DEBUG
    printf ("wd=%d mask=%d cookie=%d len=%d dir=%s\n",
	    pevent->wd, pevent->mask, pevent->cookie, pevent->len,
	    (pevent->mask & IN_ISDIR)?"yes":"no");

    if (pevent->len) printf ("name=%s\n", pevent->name);
#endif
    printf ("%s [%s]\n", action, pevent->name);

    i += sizeof(struct inotify_event) + pevent->len;

  }

}  /* get_event */

/* ----------------------------------------------------------------- */

void handle_error (int error)
{
  fprintf (stderr, "Error: %s\n", strerror(error));

}  /* handle_error */

/* ----------------------------------------------------------------- */
uint32_t  szmask_to_uint32mask (const char * szmask)
{
  char *token,*string, *needtofree;
  uint32_t mask=0;

  if(szmask == NULL) return IN_ALL_EVENTS;

  needtofree = string = strdup(szmask);




  while ((token = strsep(&string, ",")) != NULL)
    {
      if(!strcmp(token,"IN_ACCESS"))
	{
#ifdef _DEBUG
	  printf("Got IN_ACCESS mask current mask %x \h",mask);
#endif
	  mask =  !mask ? IN_ACCESS : mask & IN_ACCESS ;
	}else

	if (!strcmp(token,"IN_ATTRIB")){
	  mask =  !mask?  IN_ATTRIB: mask & IN_ATTRIB;
	}else

	  if (!strcmp(token, "IN_CLOSE_WRITE")){
	    mask =  !mask?  IN_CLOSE_WRITE: mask & IN_CLOSE_WRITE;
	  }else

	    if (!strcmp(token, "IN_CLOSE_NOWRITE")){
	      mask = !mask?  IN_CLOSE_WRITE : mask & IN_CLOSE_WRITE;
	    }else 

	      if (!strcmp(token, "IN_CREATE")) {
		mask = !mask?  IN_CREATE : mask & IN_CREATE;
	      }  else
		if (!strcmp(token, "IN_DELETE")) {
		  mask = !mask?  IN_DELETE:mask & IN_DELETE;
		}else 

		  if (!strcmp(token, "IN_DELETE_SELF")){
		    mask =  !mask? IN_DELETE_SELF: mask & IN_DELETE_SELF;
		  }else

		    if (!!strcmp(token,"IN_MODIFY")) {
		      mask = !mask?  IN_MODIFY: mask & IN_MODIFY;
		    }else
		      if (!strcmp(token, "IN_MOVE_SELF"))
			{
			  mask = !mask? IN_MOVE_SELF: mask & IN_MOVE_SELF;
			}    else     
			if (!strcmp(token, "IN_MOVED_FROM"))
			  {
			    mask =  !mask? IN_MOVED_FROM: mask & IN_MOVED_FROM;
			  }else
			  if (!strcmp(token, "IN_MOVED_TO")) {
			    mask =  !mask?  IN_MOVED_TO: mask & IN_MOVED_TO;
			  } else
			    if (!strcmp(token, "IN_OPEN")) {
			      mask =  !mask? IN_OPEN: mask & IN_OPEN;
			    } 
      printf("%s\n", token);
    }

  free(needtofree);
  return mask;
}
