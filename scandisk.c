/*#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "bootsect.h"
#include "bpb.h"
#include "direntry.h"
#include "fat.h"
#include "dos.h"


void usage(char *progname) {
    fprintf(stderr, "usage: %s <imagename>\n", progname);
    exit(1);
}

int compare(uint16_t followclust, struct direntry *dirent, uint8_t *image_buf, struct bpb33* bpb){
	printf("lsdfknfdlknfslknflkfndlksnlggnslknglsknslkbgfslkj\n");
	uint32_t realsize = getulong(dirent->deFileSize);
	uint32_t clustersize = bpb->bpbSecPerClust*bpb->bpbBytesPerSec;
	int expected_num_clust = (realsize+511)/clustersize;
	
	//find actual
	uint16_t fatent = get_fat_entry(followclust, image_buf, bpb);
	uint16_t prev_fatent = fatent;
	int count = 1;

	while(!is_end_of_file(fatent)) {
		if (count==expected_num_clust) {
			set_fat_entry(prev_fatent,FAT12_MASK&CLUST_EOFS, image_buf, bpb);
			set_fat_entry(fatent, FAT12_MASK&CLUST_FREE, image_buf, bpb);
			fatent=get_fat_entry(fatent, image_buf, bpb);
			count++;
			}
		if (count> expected_num_clust) {
			set_fat_entry(fatent, FAT12_MASK&CLUST_FREE, image_buf, bpb);
			fatent=get_fat_entry(fatent, image_buf, bpb);
			count++;


			}
		
		else {
			prev_fatent=fatent;
			fatent=get_fat_entry(fatent,image_buf, bpb);
			count++;
			}
			
		}
	if (expected_num_clust>count){
		putulong(dirent->deFileSize, count);

	}
	return count;
}



uint16_t get_dirent( struct direntry *dirent, char *buffer)
{
    uint16_t followclust = 0;

    memset(buffer, 0, MAXFILENAME);

    int i;
    char name[9];
    char extension[4];
   
    name[8] = ' ';
    extension[3] = ' ';
	//printf("begin copy names\n");
    memcpy(name, &(dirent->deName[0]), 8);
    memcpy(extension, dirent->deExtension, 3);
	//printf("end copy names\n");

    if (name[0] == SLOT_EMPTY)
    {
	return followclust;
    }

   
    if (((uint8_t)name[0]) == SLOT_DELETED)
    {
	return followclust;
    }

    if (((uint8_t)name[0]) == 0x2E)
    {
	// dot entry ("." or "..")
	// skip it
        return followclust;
    }

   
    for (i = 8; i > 0; i--) 
    {
	if (name[i] == ' ') 
	    name[i] = '\0';
	else 
	    break;
    }

  
  for (i = 3; i > 0; i--) 
    {
	if (extension[i] == ' ') 
	    extension[i] = '\0';
	else 
	    break;
    }

    if ((dirent->deAttributes & ATTR_DIRECTORY) != 0) 
    {
	printf("in first else if");
        // don't deal with hidden directories; MacOS makes these
        // for trash directories and such; just ignore them.
	if ((dirent->deAttributes & ATTR_HIDDEN) != ATTR_HIDDEN)
        {
            strcpy(buffer, name);
            followclust = getushort(dirent->deStartCluster);
            
        }
    }
    else 
    {
	printf("In a normal file");
     
         * a "regular" file entry
         * print attributes, size, starting cluster, etc.
         */
 /*       strcpy(buffer, name);
        if (strlen(extension)>0)  
        {
            strcat(buffer, ".");
            strcat(buffer, extension);
        }
	followclust = getushort(dirent->deStartCluster);
	//followclust =0;
    }
    return followclust;
}



void follow_dir(uint8_t *image_buf, struct bpb33* bpb)
{
	
	uint16_t followclust;
	char buffer[128];
	
	printf("in folowdir loop\n");
        struct direntry *dirent = (struct direntry*)root_dir_addr( image_buf, bpb);
		
	int numDirEntries = bpb->bpbRootDirEnts;
        int i = 0;
	for ( ; i < numDirEntries; i++){		
		
		
		//followclust = get_dirent(dirent, buffer);
		followclust = getushort(dirent->deStartCluster);
		
		if(!is_valid_cluster(followclust,bpb)) {
			printf( "buffer: %s", buffer);
			printf( "folowclust was 0!!!\n");
			}

		else  {
			
			int result;
			result = compare(followclust, dirent, image_buf, bpb);

			printf("Result: %i \n",result);
			
			

			}
		dirent++;	    
		
	}

		
printf("lsknfsdlknfsklnf");


}


int main(int argc, char** argv) {
	printf("inside\n");
    uint8_t *image_buf;
    int fd;
    struct bpb33* bpb;
    if (argc < 2) {
	usage(argv[0]);
    }
	
    image_buf = mmap_file(argv[1], &fd);
    bpb = check_bootsector(image_buf);
*/
    // your code should start here...
	// what to look for
	//printf("Meta data%u\n",bpb->bpbFATs);
	// fat entries vs metadata(boot sector)

			//get metadata

			//look at start cluster for file and interate to count them
				//root_dir_adder
				
	//			follow_dir(image_buf, bpb);
				
				

		//print out list of files whose length is directory is != to length in data blocks
		// free clusters that are beyond the end of a file but FAT still points
		// adjust the size entry for a file if there is a free or bad cluster in FAT chain

	// system crashes halfway through writing data 
		


	// Directory entry is removed and system crashes
		// check all clusters and see if there are any clusters not referenced in directory print out
		// make new directory entry in the root directory... named found1.dat found2.dat

	//OS detects a bad cluster in the FAT
/*




    unmmap_file(image_buf, &fd);
    return 0;
}*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#include "bootsect.h"
#include "bpb.h"
#include "direntry.h"
#include "fat.h"
#include "dos.h"


void print_indent(int indent)
{
    int i;
    for (i = 0; i < indent*4; i++)
	printf(" ");
}

int compare(uint16_t followclust, struct direntry *dirent, uint8_t *image_buf, struct bpb33* bpb){

	uint32_t realsize = getulong(dirent->deFileSize);
	uint32_t clustersize = bpb->bpbSecPerClust*bpb->bpbBytesPerSec;
	int expected_num_clust = (realsize+511)/clustersize;
	printf("expected= %i\n",expected_num_clust);
	//find actual
	uint16_t fatent = get_fat_entry(followclust, image_buf, bpb);
	uint16_t prev_fatent = fatent;
	int count = 1;

	while(!is_end_of_file(fatent)) {
		uint16_t temp =get_fat_entry(fatent, image_buf, bpb);
		if (fatent == (FAT12_MASK & CLUST_BAD)){
			printf("Bad cluster");
			set_fat_entry(fatent, FAT12_MASK&CLUST_EOFS, image_buf, bpb);
			return count;
			}
		
		if(count>=expected_num_clust)	
			{	
			
			if (count==expected_num_clust) {
				printf("count is greater\n");
				set_fat_entry(prev_fatent,FAT12_MASK&CLUST_EOFS, image_buf, bpb);
				set_fat_entry(fatent, FAT12_MASK&CLUST_FREE, image_buf, bpb);
			
				}
		
			else {
				set_fat_entry(fatent, FAT12_MASK&CLUST_FREE, image_buf, bpb);
			
				}
			
			fatent=temp;
			count++;
			}
		
		else {
			prev_fatent=fatent;
			fatent=get_fat_entry(fatent,image_buf, bpb);
			count++;
			}
			
		
		}
	if (expected_num_clust>count){
		uint32_t clustersize = bpb->bpbSecPerClust*bpb->bpbBytesPerSec;
		putulong(dirent->deFileSize, (count*clustersize));
		printf("FUck this: %d\n", getulong(dirent->deFileSize));
	}
	return count;
}

uint16_t print_dirent(struct direntry *dirent, int indent)
{
    uint16_t followclust = 0;

    int i;
    char name[9];
    char extension[4];
    uint32_t size;
    uint16_t file_cluster;
    name[8] = ' ';
    extension[3] = ' ';
    memcpy(name, &(dirent->deName[0]), 8);
    memcpy(extension, dirent->deExtension, 3);
    if (name[0] == SLOT_EMPTY)
    {
	return followclust;
    }

    /* skip over deleted entries */
    if (((uint8_t)name[0]) == SLOT_DELETED)
    {
	return followclust;
    }

    if (((uint8_t)name[0]) == 0x2E)
    {
	// dot entry ("." or "..")
	// skip it
        return followclust;
    }

    /* names are space padded - remove the spaces */
    for (i = 8; i > 0; i--) 
    {
	if (name[i] == ' ') 
	    name[i] = '\0';
	else 
	    break;
    }

    /* remove the spaces from extensions */
    for (i = 3; i > 0; i--) 
    {
	if (extension[i] == ' ') 
	    extension[i] = '\0';
	else 
	    break;
    }

    if ((dirent->deAttributes & ATTR_WIN95LFN) == ATTR_WIN95LFN)
    {
	// ignore any long file name extension entries
	//
	// printf("Win95 long-filename entry seq 0x%0x\n", dirent->deName[0]);
    }
    else if ((dirent->deAttributes & ATTR_VOLUME) != 0) 
    {
	printf("Volume: %s\n", name);
    } 
    else if ((dirent->deAttributes & ATTR_DIRECTORY) != 0) 
    {
        // don't deal with hidden directories; MacOS makes these
        // for trash directories and such; just ignore them.
	if ((dirent->deAttributes & ATTR_HIDDEN) != ATTR_HIDDEN)
        {
	    
    	    //printf("%s/ (directory)\n", name);
            file_cluster = getushort(dirent->deStartCluster);
            followclust = file_cluster;
        }
    }
    else 
    {
        
         // a "regular" file entry
        //  print attributes, size, starting cluster, etc.
         printf("regular file entry\n");
	int ro = (dirent->deAttributes & ATTR_READONLY) == ATTR_READONLY;
	int hidden = (dirent->deAttributes & ATTR_HIDDEN) == ATTR_HIDDEN;
	int sys = (dirent->deAttributes & ATTR_SYSTEM) == ATTR_SYSTEM;
	int arch = (dirent->deAttributes & ATTR_ARCHIVE) == ATTR_ARCHIVE;

	size = getulong(dirent->deFileSize);
	
	file_cluster =getushort(dirent->deStartCluster);
	followclust = file_cluster;	
	printf("followclust: %i\n", followclust);
	print_indent(indent);
	printf("%s.%s (%u bytes) (starting cluster %d) %c%c%c%c\n", 
	       name, extension, size, getushort(dirent->deStartCluster),
	       ro?'r':' ', 
               hidden?'h':' ', 
               sys?'s':' ', 
               arch?'a':' ');
    }

    return followclust;
}


void follow_dir(uint16_t cluster, int indent,
		uint8_t *image_buf, struct bpb33* bpb)
{
    while (is_valid_cluster(cluster, bpb))
    {
        struct direntry *dirent = (struct direntry*)cluster_to_addr(cluster, image_buf, bpb);

        int numDirEntries = (bpb->bpbBytesPerSec * bpb->bpbSecPerClust) / sizeof(struct direntry);
     
	for ( int i=0; i < numDirEntries; i++)
	{
            
   		uint16_t followclust = print_dirent(dirent, indent);
		int count = -1;
		if (followclust!= 0) {
			count=compare(followclust,dirent,image_buf,bpb);
			}
		else {
			printf("followclust was 0\n");
			}		
		printf("count: %i\n", count);
            if (followclust)
                follow_dir(followclust, indent+1, image_buf, bpb);
            dirent++;
	}

	cluster = get_fat_entry(cluster, image_buf, bpb);
    }
}


void traverse_root(uint8_t *image_buf, struct bpb33* bpb)
{
	
    uint16_t cluster = 0;

    struct direntry *dirent = (struct direntry*)cluster_to_addr(cluster, image_buf, bpb);

    int i = 0;
    for ( ; i < bpb->bpbRootDirEnts; i++)
    {
        uint16_t followclust = print_dirent(dirent, 0);
        if (is_valid_cluster(followclust, bpb)) {
            follow_dir(followclust, 1, image_buf, bpb);
		printf("Cluster is valid\n");
		}
        dirent++;
    }
printf("finished going through stuff\n");
}


void usage(char *progname)
{
    fprintf(stderr, "usage: %s <imagename>\n", progname);
    exit(1);
}


int main(int argc, char** argv)
{
    uint8_t *image_buf;
    int fd;
    struct bpb33* bpb;
    if (argc != 2)
    {
	usage(argv[0]);
    }

    image_buf = mmap_file(argv[1], &fd);
    bpb = check_bootsector(image_buf);
    traverse_root(image_buf, bpb);

    unmmap_file(image_buf, &fd);

    return 0;
}





