

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <math.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "log.h"
#include "types.h"
#include "alarm.h"
#include "bus.h"
#include "emu6502.h"
#include "vdrive.h"
#include "convert.h"
#include "mem.h"
#include "devices.h"

#define	MAXLINE		200

#define	T_VC1541	1541
#define	VCLINE		40
#define	VCBUF		256
#define	VCNBUF		5

/**************************************************************************/


typedef struct {
		size_t		pos;		/* rw position in buf */
		size_t 		len;		/* length of valid data */
		size_t 		max;		/* length of buffer */
		char		*buf;
} vcBuf;

vcBuf *vcBuf_get(size_t size) {
	char *m1=malloc(size+1);
	vcBuf *m2 = NULL;
	if(m1) {
	  m2=malloc(sizeof(vcBuf));
	  if(m2) {
	    m2->buf=m1;
	    m2->max=size;
	    m2->len=0;
	    m2->pos=0;
	  } else {
	    free(m1);
	  }
	}
	return(m2);
}

void vcBuf_fre(vcBuf *buf) {
	free(buf->buf);
	free(buf);
}

/*
 * write a byte to a buffer
 * returns true if buffer is full
 */
size_t vcBuf_write(vcBuf *buf,char byte) {
	if(buf->pos<buf->max) {
	  buf->buf[buf->pos++]=byte;
	}
	buf->len=buf->pos;
	return(buf->pos>=buf->max);
}

/*
 * return true if a buffer read pointer is at the end
 */
static inline int vcBuf_end(vcBuf *b) {
	 return (b->pos>=b->len);
}

/*
 * read a byte from a buffer
 */
static inline uchar vcBuf_read(vcBuf *b) {
	uchar rv = (b->buf[b->pos]);
	b->pos++;
	return rv;
}

/*
 * convert each byte of the buffer contents with a single byte conversion function
 */
void vcBuf_convert(vcBuf* b,int (*func)(int)) {
	size_t i;
	for(i=0;i<b->len;i++) {
		b->buf[i]=func(b->buf[i]);
	}
}

/**************************************************************************/

typedef struct {
		union {
			int		fd;		/* file descriptor */
			DIR		*dir;
		} f;
		int		mode;
		int		eof;
		uchar		bufd;			// buffered byte for ack
		uchar		bval;			// buffered byte is valid?
		uchar 		bstat;			// buffered status
		char 		mask[VCLINE];
		char		path[MAXLINE];
		vcBuf		*buf;
} vcFile;


typedef struct {
		device		dev;

		int		atn;
		int		channel;
		int		talk;
		int		talkadr;
		int 		listen;
		int 		listenadr;
		int 		secadr;
		int		cmd;
	
		const char 	*drive[2];	/* unix pathnames */

		vcFile		bufp[16];
		vcBuf		*cmdbufp;
		//vcBuf		*errbufp;	
} VC1541;

void out_1541(uchar byte, int isatn, VC1541* vc);
uchar get_1541(VC1541 *vc, uchar *status, uchar ackflag);
void close_1541(void);
		

void err_1541(int ernum);
VC1541	*vc;

#define	VCE_OK		0
#define	VCE_NOBUF	1
#define	VCE_NOOPEN	2
#define	VCE_FOPEN	3
#define	VCE_NOTOPEN	4
#define	VCE_NOTWRITE	5
#define	VCE_CMDSYNTAX	6
#define	VCE_NOTIMPL	7
#define	VCE_SYNTAX	8
#define	VCE_BUFLEN	9
#define	VCE_NOTLOAD	10
#define	VCE_PASTEND	11
#define	VCE_ILLMODE	12
#define	VCE_NOTSAVE	13
#define	VCE_OPENERR	14
#define	VCE_NOTFOUND	15
#define	VCE_DOSVER	16
#define	VCE_EXISTS	17


int vdrive_init(void) {
	return 0;
}


int init_1541(VC1541 *v, int dev) {
	int i;

	v->dev.out =(void(*)(uchar, int, device*))out_1541;
	v->dev.get =(uchar(*)(device*,uchar*,uchar))get_1541;
	v->drive[0]=v->drive[1]=NULL;

	for(i=0;i<16;i++) {
		v->bufp[i].buf=NULL;
	}
	v->cmdbufp=vcBuf_get(VCLINE);
	// status buffer
	v->bufp[15].buf=vcBuf_get(VCLINE);

	v->talk=v->listen=v->cmd=0;
	v->talkadr = dev|0x40;
	v->listenadr = dev|0x20;

	vc=v;
	err_1541(VCE_DOSVER);	
	return(0);
}

int vdrive_setdrive(int dev, int drive, const char *pathname) {
	VC1541 *v;

	logout(1,"set drive(unit=%d, drive=%d) to %s",dev,drive,pathname);

	
	device *d = device_get(dev);

	if(dev<0 || dev>15 || drive<0 || drive>1)
		return(-1);
	if(d && (d->type!=T_VC1541))
		return(-2);
	if(pathname==NULL) {
		/* TODO: remove dev */
	}

	if(d == NULL) {
		v=malloc(sizeof(VC1541));
		if(!v)
			return(-3);
		init_1541(v,dev);
		device_set(dev, (device*)v);
	} else {
		v=(VC1541*)d;
	}
	v->drive[drive]=pathname;
	return(0);
}

typedef struct vc1541_name {
		char 		name[MAXLINE];
		char 		mode;
		char 		type;
		int		drive;

} vc1541_name;
 
static char *erstr_1541[]= {
	"no buffer", "file not open", "file open", "file not open",
	"file not write",
	"syntax", "command not implemented", "syntax", "buffer length", 
	"file not load", "write past end", "illegal mode", "file not save",
	"file open", "file not found", "xcbm version 0.1.0", "file exists" 
};

void err_1541(int ernum) {
	vcBuf *vb = vc->bufp[15].buf;
	if(!ernum) {
		strcpy(vb->buf,"00, ok, 00, 00\015");
	} else {
		sprintf(vb->buf,"%02d, %s error, 00, 00\015",ernum,
			erstr_1541[ernum-1]);
		vc->talk=vc->listen=0;
		logout(3,"err=%s",vb->buf);
	}
	vb->len=strlen(vb->buf);
	vb->pos=0;
/*logout(3,"err=%s",vc->errbufp->buf);*/
}

int parse_1541(char *cmd, vc1541_name *ns, int needcolon, char **cmdp) {
	int i=0;
	ns->drive=-1;
	ns->type=ns->mode='\0';

	*cmdp=cmd;
	if(!*cmd) {
	  return(0);
	}
	if(strchr(cmd,':') || needcolon) {
	  /* read drive */
	  if(isdigit(*cmd)) {
	    ns->drive=atoi(cmd);
	    cmd++;
	  } else {
	    if(*cmd=='\0') {
	      *cmdp=cmd;
	      return(0);
	    } else
	    if(*cmd!=':') { 
              err_1541(VCE_SYNTAX);
	      return(-1);
	    }
	  }
	  if(*cmd=='\0') {
	    *cmdp=cmd;
	    return(0);
	  }
	  if(*cmd==':') {
	    cmd++;
	  } else {
	    err_1541(VCE_SYNTAX);
	    return(-1);
	  }
	}
	while(*cmd && !strchr("=,",*cmd)) {
	  ns->name[i++]=*(cmd++);
	}
	ns->name[i]='\0';

	if(!*cmd || *cmd=='=') {
	  *cmdp=cmd;
	  return(0);
	}
	if(*cmd==',') {
	  /* dateityp */
	  cmd++;
	  if(strchr("PUSLDpusld",*cmd)) {
	    ns->type=toupper(*(cmd++));
	  } else {
	    err_1541(VCE_SYNTAX);
	    return(-1);
	  }
	  if(!*cmd || *cmd=='=') {
	    *cmdp=cmd;
	    return(0);
	  }
	  if(*cmd==',') {
	    /* i/o-mode */
	    cmd++;
	    if(strchr("RWAMrwam",*cmd)) {
	      ns->mode=toupper(*(cmd++));
	    } else {
	      err_1541(VCE_SYNTAX);
	      return(-1);
	    }
	  }
	  if(*cmd && *cmd!='=') {
	    err_1541(VCE_SYNTAX);
	    return(-1);
	  }
	}
	*cmdp=cmd;
	return(0);
}

/**************************************************************************/
/* This part handles the directory search and manages file type 
   distinction. 
   File types are saved in the Un*x file name, as an extension separated
   by a comma, i.e. add a ",P", ",S" etc to a file name to force file
   type to PRG, SEQ, etc. Without a file type extension, "PRG" is assumed,
   except for directories, which get a new type "DIR".
*/
    
int open_1541dir(vcFile *vf, vc1541_name *n1) {
	char *p;
	int i;
	if(n1->drive<0) n1->drive=0;
	if(vf->buf=vcBuf_get(VCBUF)) {
	  if(n1->name[0]=='/') {
	    strcpy(vf->path,"/");
	    for(i=0;n1->name[i]=n1->name[i+1];i++);
	  } else {
	    strcpy(vf->path,vc->drive[n1->drive]);
	    if(vf->path[strlen(vf->path)-1]!='/') strcat(vf->path,"/");
	  }
	  if((p=strrchr(n1->name,'/'))){
	    strcpy(vf->mask,p+1);
	    *p='\0';
	    strcat(vf->path,n1->name);
	  } else {
	    strcpy(vf->mask,n1->name);
	  }
	  if(!(vf->f.dir=opendir(vf->path))) { ; /*vc->drive[n1->drive]);*/
	    logout(1,"error opening dir %s, er=%d, %s",vf->path,errno,strerror(errno));
	    err_1541(VCE_NOTFOUND);
	    vcBuf_fre(vf->buf);
	    vf->buf=NULL;
	    return(VCE_NOTFOUND);
	  } else {
	    return(VCE_OK);
	  }
	}
	err_1541(VCE_NOBUF);
	return(VCE_NOBUF);
}

void get_next1541(vcFile *vf, struct dirent **de) {
	int i;
	char *m;
	while(*de=readdir(vf->f.dir)) {
/*logout(0,"dirmask=%s, filename=%s",vf->mask,(*de)->d_name);*/
	  if(!vf->mask[0]) break;
	  m=vf->mask;
	  i=0; 
	  while(m[i]) {
	    if(m[i]=='?') i++;
	    else if(m[i]=='*') { i=strlen(m); break; }
	    else if(m[i]!=(*de)->d_name[i]) break;
	    else i++;
/* TODO: *=[P|U|S|R] file type selection */
	  }
	  if(!m[i]) break; 
	}
}

void close_1541dir(vcFile *vf) {
	closedir(vf->f.dir);	
}

/*****************************************************************************/

int getdir_1541(vcFile *vf) {
	struct dirent *de;
	char *p,*b=vf->buf->buf;
	unsigned int i;
	struct stat statbuf;
	char writeprotect=' ';
	char *type="PRG";
	int offset = 0;
/*
	while(de=readdir(vf->f.dir)) {
	  if(!vf->mask[0]) break;
	  m=vf->mask;
	  i=0; 
	  while(m[i]) {
	    if(m[i]=='?') i++;
	    else if(m[i]=='*') { i=strlen(m); break; }
	    else if(m[i]!=de->d_name[i]) break;
	    else i++;
	  }
	  if(!m[i]) break; 
	}
*/
	get_next1541(vf,&de);
	if(de) {
	  i=strlen(vf->path);
	  strcat(vf->path,"/");
	  strcat(vf->path,de->d_name);
	  stat(vf->path,&statbuf);
	  if(!statbuf.st_mode&S_IWUSR) writeprotect='<';
	  if(statbuf.st_mode&S_IFDIR) type="DIR";
	  vf->path[i]='\0';
	  i=(statbuf.st_size/254)+1;	/* size of CBM blocks */
	  i=i>65534?65535:i;		/* if too big */
	  offset = 9-((int)log10(i));
	  if((p=strrchr(de->d_name,',')) && p[1]!=0 && p[2]==0) {
	    switch(p[1]) {
	    case 'P': type="PRG"; *p='\0'; break;
	    case 'R': type="REL"; *p='\0'; break;
	    case 'S': type="SEQ"; *p='\0'; break;
	    case 'U': type="USR"; *p='\0'; break;
	    default:  break;
	    }
	  }
	  sprintf(b,"%c%c                             ",1,1);
	  sprintf(&b[offset],"\"%s\"                                   ",
					de->d_name);
	  vf->buf->len=34;
	  vf->buf->pos=0;
	  vcBuf_convert(vf->buf,a2pet);
	  b[2]=i&0xff; b[3]=(i>>8)&0xff;
	  sprintf(&b[19+offset],"%s%c        ",type,writeprotect);
	  b[33]='\0';
logout(0,"dir entry -> len=33");
	  return(0);
	} else {
	  if(vf->eof>0) {
logout(0,"dir entry -> past end");
		return(1);
	  }

	  vf->buf->buf[0]=vf->buf->buf[1]=0;
	  vf->buf->len=2;
	  vf->buf->pos=0;
	  vf->eof=1;
logout(0,"dir entry -> len=2");
	  return(0);
	}
}

int read_1541(vcFile *vf) {
	int er;
	er=read(vf->f.fd, vf->buf->buf, vf->buf->max);
	vf->buf->len=er;
	vf->buf->pos=0;
	return(er<=0);
}

void cmd_1541(void) {
	struct dirent *de;
	static char *cmds="i"; /*"vidmbup&crsn";*/
	static vc1541_name n1;
	char cmd='\0';
	char *bp=vc->cmdbufp->buf;
	int channel=vc->channel;
	int p=0,i;		/* p is pointer in cmdbuf */
	vcFile *vf;
	char filename[MAXLINE];

	n1.name[0]='\0';
	err_1541(VCE_OK);
	vf=&vc->bufp[channel];
	vcBuf_convert(vc->cmdbufp,pet2a);
	bp[vc->cmdbufp->pos]='\0';
	while(isspace(*bp)) bp++;

	if(channel==15) {
	  if(!vc->cmdbufp->pos) {
	    return;
	  }
	  i=0;
	  while(cmds[i]) {
/*logout(2,"cmd: i=%d, *bp=%c, ->pos=%d",i,*bp,vc->cmdbufp->pos);*/
	    if(cmds[i]==tolower(*bp)) {
	      cmd=cmds[i];
	      while(isalpha(*bp)) bp++;
	      break;
	    }
	    i++;
	  }
	  if(cmd=='\0') {
	    err_1541(VCE_CMDSYNTAX);
	    return;
	  }
	} else {
	  cmd='o';	/* open */
	}

logout(2,"cmd=%c, cmdbuf=%s",cmd,bp+p);
	switch(cmd) {
	case 'o':
	  vf->bval = 0;
 	  if(bp[p]=='$') {
	    /* open directory */
	    p++;
	    if(!parse_1541(bp+p,&n1,1,&bp)) {
	      if(!open_1541dir(vf,&n1)) {
	        vf->mode=MODE_DIR; 
		vf->eof=0;
		vf->buf->buf[0]=1;
		vf->buf->buf[1]=4;
		vf->buf->len=2;
	      } else {
		err_1541(VCE_NOTFOUND);
	      }
	    } 
	  } else {
	    /* open file */
	    if(!parse_1541(bp+p,&n1,0,&bp)) {
	      // channel 0 is LOAD
	      if(channel==0) {
	        if(!n1.mode) {
		  n1.mode='R';
	     	} else { 
	          if(n1.mode!='R') {
	            err_1541(VCE_NOTLOAD);
	            return;
	          }
		}
	      }
	      // channel 1 is SAVE
	      if(channel==1) {
		if(!n1.mode) {
		  n1.mode='W';
		} else { 
		  if(n1.mode!='W') {
	            err_1541(VCE_NOTSAVE);
	            return;
		  }
	        }
	      }
	      // if write, no wildcards allowed
	      if(n1.mode=='W') {
		if(strchr(n1.name,'*') || strchr(n1.name,'?')) {
		  err_1541(VCE_SYNTAX);
		  return;
		}
	      }
	      // open directory to scan for the file
	      if(!open_1541dir(vf,&n1)) {
	        strcpy(filename,vf->path);
	        if(filename[strlen(filename)-1]!='/') {
		  strcat(filename,"/");
		}
		// get directory entry
		get_next1541(vf,&de);
		if(n1.mode!='W') {
	 	  if(de) {
		    // found
		    strcat(filename,de->d_name);
		  } else {
		    // not found, so FILE NOT FOUND
		    vcBuf_fre(vf->buf);
		    vf->buf=NULL;
		    err_1541(VCE_NOTFOUND);
		    return;
		  }
		} else {
		  if(de) {
		    // found, so FILE EXISTS
		    vcBuf_fre(vf->buf);
		    vf->buf=NULL;
		    err_1541(VCE_EXISTS);	
		    return;
		  } else {
		    // not found, so we can create one to write
		    strcat(filename,n1.name);
		  }
		}
	      } else {
		// dir open failed
		return;
	      }

logout(2,"open filename=%s, channel=%d, mode=%c",filename,channel,n1.mode);

	      switch(n1.mode) {
	      case 'W':
/*	 	if(vf->buf=vcBuf_get(VCBUF)) {*/
		  vf->f.fd=open(filename,O_WRONLY|O_CREAT|O_EXCL,0644);
logout(0,"open file %s gives fd=%d",filename,vf->f.fd);
		  if(vf->f.fd==-1) {
		    logout(3,"open %s for write failed, errno=%d",filename,errno);
		    err_1541(VCE_OPENERR);
	            vcBuf_fre(vf->buf);
		    vf->buf=NULL;
		  } else {
		    vf->mode=MODE_WRITE;
		  }
/*
		} else {
		  err_1541(VCE_NOBUF);
		}
*/		
		break;
	      case 'R':
/*		if(vf->buf=vcBuf_get(VCBUF)) {*/
		  vf->f.fd=open(filename,O_RDONLY);
		  if(vf->f.fd==-1) {
		    logout(3,"open %s for read failed, errno=%d",filename,errno);
		    if(errno=2) {
		      err_1541(VCE_NOTFOUND);
		    } else {
		      err_1541(VCE_OPENERR);
		    }
	            vcBuf_fre(vf->buf);
		    vf->buf=NULL;
		  } else {
		    uchar status = 0;
		    vf->mode=MODE_READ;
		    read_1541(vf);
		    vf->eof=vf->buf->len?0:-1;
		    vf->bval = 0;
		    get_1541(vc, &status, 0);
		  }
/*
		} else {
		  err_1541(VCE_NOBUF);
		}
*/
		break;
	      default:
		vcBuf_fre(vf->buf);
		vf->buf=NULL;
		err_1541(VCE_ILLMODE);
	        break;
	      }
	    }
	  }
	  break;
	default:
	  err_1541(VCE_NOTIMPL);
	  break;  
	}	
}


void write_1541(vcFile *vf) {
logout(2,"write_1541(vcFile *vf=%p)",vf);
	int r = write(vf->f.fd, vf->buf->buf, vf->buf->pos);
	if (r<0) {
		logout(0, "Write error: %s", strerror(errno));
	}
	vf->buf->len=vf->buf->pos=0;
}

void close_1541(void) {
	vcFile *vf=&vc->bufp[vc->channel];
logout(2,"close_1541: vf=%p",vf);
	if (vc->channel == 15) {
	  // command channel
          err_1541(VCE_OK);
	  return;
	}
	if(vf->buf) {
	  if(vf->mode==MODE_WRITE) {
	    write_1541(&(vc->bufp[vc->channel]));
	  }
	  vcBuf_fre(vf->buf);
	  vf->buf=NULL;
	  if(vf->mode!=MODE_DIR) {
	    close(vf->f.fd); 
	  } else {
	    close_1541dir(vf);
	  }
	} else {
          err_1541(VCE_NOTOPEN);
	}	
}

void out_1541(uchar byte, int isatn, VC1541* vcp) {
	vcFile *vf;
	vc = vcp;
/*printf("out_1541, atn=%d\n",isatn);*/
	if(isatn) {
	  if(!vc->atn) {
	    vc->listen=vc->talk=0;
	  }
	  vc->atn=1;
	  if(byte==0x3f) { 		/* unlisten */
	    vc->listen = 0; 
	  } else
	  if(byte==0x5f) {
	    vc->talk = 0;
	    vc->cmd=0;
	  } else
	  if(byte==vc->talkadr) {
	    vc->talk=1;
	    vc->listen=0;
	  } else 
	  if(byte==vc->listenadr) {
	    vc->listen=1;
	    vc->talk=0;
	  } else 
	  if((byte & 0x60)==0x60) {
	    vc->secadr=byte;
	    vc->channel=byte & 0x0f;
	    if(((byte&0xf0)==0xf0) || vc->channel==15) {
	      vc->cmd=1;
logout(0,"detect cmd/err channel or open");
	      vc->cmdbufp->pos=vc->cmdbufp->len=0;
	    } else   
	    if((byte & 0xf0)==0xe0) {
	      close_1541();
	    }
	  }
	  if(!(vc->talk+vc->listen)) {
	    if(vc->cmd) {
	      cmd_1541();
	      vc->cmd=0;
	    }
	  }
	} else { 	/* atn is not lo */

	  if(!vc->listen) return;	
	  if(vc->atn) { 	/* first byte after command */
/*
	    if(((vc->secadr&0xf0)==0xf0) || vc->channel==15) {
	      vc->cmd=1;
	      vc->cmdbufp->pos=vc->cmdbufp->len=0;
	    }
*/
	    vc->atn=0; 
	  }

	  if(vc->cmd) {
	    vcBuf_write(vc->cmdbufp,byte);
	  } else { 
	    if(vf=&vc->bufp[vc->channel]) {
	      if(vf->mode==MODE_WRITE) {
	        if(vcBuf_write(vf->buf,byte)) {
	     	  write_1541(vf);
		} 
	      } else {
	        err_1541(VCE_NOTWRITE);
	      }
	    } else {
	      err_1541(VCE_NOTOPEN);
	    }
	  }
	}
}

#define	seteof()	*status|=PAR_STATUS_EOI
#define	settimeout()	*status|=PAR_STATUS_TIMEOUT
#define	hastimeout()	(*status & PAR_STATUS_TIMEOUT)

/*
 * get the next byte for a channel, set status too
 */
uchar get_1541_int(vcFile *vf, uchar *status) {
	uchar byte = 0xff;

	*status = 0;

	if(vf->buf) {
  		if(vf->eof>=0) { 
			// not yet EOF
			// read byte from buffer
    			byte=vcBuf_read(vf->buf);
    			if(vcBuf_end(vf->buf)) {
				// buffer is empty, try to re-fill it
      				if(vf->mode==MODE_DIR) {
					// directory
       					if(getdir_1541(vf)) {
						logout(0,"getdir->seteof");
  						seteof();
					}
      				} else {
					// file
					if(vc->channel!=15) {
  						if(read_1541(vf)) {
    							/*logout(0,"read_1541->seteof");*/
    							seteof();
  						}
					} else {
						// command channel
  						seteof();
  						err_1541(VCE_OK);
					}
      				}	
    			}
  		} else {
			// read past EOF
 			settimeout();
    			err_1541(VCE_PASTEND);
  		}
 	} else {
		// file not open
 		settimeout();
  		err_1541(VCE_NOTOPEN);
 	}

	return byte;
}


uchar get_1541(VC1541 *vcp, uchar *status, uchar ackflg) {
/* see d39b */
	vcFile *vf;
	uchar byte = 0;

	*status = 0;

	vc=vcp;
	vf=&vc->bufp[vc->channel];

	vcp->dev.timeout=0;

	if (vc->talk == 0) {
		settimeout();
	} else {
		// we have a byte and get a repeated call
		if (vf->bval) {
			// consume buffer
			*status = vf->bstat;
			byte = vf->bufd;
			if (ackflg) {
				// fill up buffer again
				vf->bufd = get_1541_int(vf, &vf->bstat);
			}
		} else {
			// fill buffer in the first place
			vf->bufd = get_1541_int(vf, &vf->bstat);
			*status = vf->bstat;
			byte = vf->bufd;
			vf->bval = !ackflg;
		}
	}
	if(hastimeout()) {
	  	vcp->dev.timeout = 1;
		logout(1,"get_1541: set timeout");
	}

	return(byte);
}

