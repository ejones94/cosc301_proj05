
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
#define SIZE_MAX (4294967295U)




//makes an array to keep track of good fat clusters and bad fat clusters for freeing later
//-1 is bad cluster;; 0 is untouched cluster;; 1 is touched and ok cluster;; 2 is eof;; 3 is touched and free
// the hints told us we needed 
int desperation[4096] ={0}; 

void find_orphans(uint8_t *image_buf, struct bpb33* bpb) {//looks through the array we made and looks for clusters we didnt touch
	for(int i=0; i<4096, i++) {
		if (desperation[i]==0){
			if(get_fat_entry(i,image_buf,pbp)!=FAT12_MASK&CLUST_FREE) {
				//if finds one we didnt touch and it is not currently set free this will free it
				set_fat_entry(i, FAT12_MASK&CLUST_FREE, image_buf, bpb);
				}
			}
		}
	}


int compare(char *filename, uint16_t followclust, struct direntry *dirent, uint8_t *image_buf, struct bpb33* bpb){
	//will loop through fat until eof marking the good/bad/free clusters and freeing them
	uint32_t realsize = getulong(dirent->deFileSize);
	uint32_t clustersize = bpb->bpbSecPerClust*bpb->bpbBytesPerSec;
	int expected_num_clust = (realsize+511)/clustersize;  // finds num clusters according to the meta data
	
	//find actual num clusters in fat
	uint16_t fatent = get_fat_entry(followclust, image_buf, bpb);
	uint16_t prev_fatent = fatent;
	int count = 1;
	
	while(!is_end_of_file(fatent)) {  //loops through checking if eof
		uint16_t temp =get_fat_entry(fatent, image_buf, bpb);
		if (fatent == (FAT12_MASK & CLUST_BAD)){ //if the fatent is bad it will mark it in the array
			printf("Bad cluster");
			desperation[followclust+count]=-1;
			printf("File is inconsistant: name is: %s\n",filename);
			}
		
		

		if(count>=expected_num_clust)	//if we have more clusters than expected
			{	
			
			if (count==expected_num_clust) {  //will only happen once
				printf("count is greater\n");
				//will set the previous entry to be eof and fee the current cluster
				//will also set desperation =2 at the eof and 3 at the free cluster
				// this is becasue we want a record in desperation of free normal or not looked at clusters
				set_fat_entry(prev_fatent,FAT12_MASK&CLUST_EOFS, image_buf, bpb);
				desperation[followclust+count-1]=2;
				set_fat_entry(fatent, FAT12_MASK&CLUST_FREE, image_buf, bpb);
				desperation[followclust+count]=3;
				printf("file is inconsistant!!! name is: %s\n",filename);
			
				}
		
			else {
				// if we are over the size just free current cluster
				set_fat_entry(fatent, FAT12_MASK&CLUST_FREE, image_buf, bpb);
			
				}
			//when count is over expected we will put 3 in every cluster over size eof
			desperation[followclust+count]=3;
			fatent=temp;
			count++;
			}
		
		else {

			//if we are still running through the clusters then if we find a free set it as 3 or if it is just normal then set it as 1
			if (fatent == (FAT12_MASK&CLUST_FREE)) {
				printf("FREE CLUSTER BEFORE EOF WEEWOOWEEWOO");
				desperation[followclust+count]=3;
				
				}
			else {
				desperation[followclust+count]=1;
				}
			prev_fatent=fatent;
			fatent=get_fat_entry(fatent,image_buf, bpb);
			count++;
			}
			
		
		}
	desperation[followclust+count]=2;//set this block as an eof in desperation
	if (expected_num_clust>count){
		//if at the end cluster is greater than expected change the size in the meta data to be what the fat size is
		uint32_t clustersize = bpb->bpbSecPerClust*bpb->bpbBytesPerSec;
		putulong(dirent->deFileSize, (count*clustersize));
		printf("File is inconsistant!!! name is: %s \n", filename);
	}
	//loop through the current files clusters on desperation and until you hit the eof
	int i= followclust;
	int countgucci=0;
	while (desperation[i]!=2)
		{
		if (desperation[i]==1) {
			countgucci++;
			}
		}
	//put the newsize into the metadata
	putulong(dirent->deFileSize, (countgucci*clustersize));
	return count;
}

uint16_t print_dirent(char *filename, struct direntry *dirent, int indent)
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
         //printf("regular file entry\n");
	int ro = (dirent->deAttributes & ATTR_READONLY) == ATTR_READONLY;
	int hidden = (dirent->deAttributes & ATTR_HIDDEN) == ATTR_HIDDEN;
	int sys = (dirent->deAttributes & ATTR_SYSTEM) == ATTR_SYSTEM;
	int arch = (dirent->deAttributes & ATTR_ARCHIVE) == ATTR_ARCHIVE;

	size = getulong(dirent->deFileSize);
	if (arch==1 & ro==0 & hidden==0 & sys==0) {
	file_cluster =getushort(dirent->deStartCluster);
	followclust = file_cluster;	
	//printf("\nfollowclust: %i\n", followclust);
	}

	//print_indent(indent);
	/*printf("%s.%s (%u bytes) (starting cluster %d) %c%c%c%c\n", 
	       name, extension, size, getushort(dirent->deStartCluster),
	       ro?'r':' ', 
               hidden?'h':' ', 
               sys?'s':' ', 
               arch?'a':' ');*/
	filename=name;
    }

    return followclust;
}


void follow_dir(char* filename, uint16_t cluster, int indent,
		uint8_t *image_buf, struct bpb33* bpb)
{
    while (is_valid_cluster(cluster, bpb))
    {
        struct direntry *dirent = (struct direntry*)cluster_to_addr(cluster, image_buf, bpb);

        int numDirEntries = (bpb->bpbBytesPerSec * bpb->bpbSecPerClust) / sizeof(struct direntry);
     
	for ( int i=0; i < numDirEntries; i++)
	{
            
   		uint16_t followclust = print_dirent(filename, dirent, indent);
		int count = -1;
		if (followclust!= 0) {
			count=compare(filename, followclust,dirent,image_buf,bpb);
			}
		else {
			
			}		
		//printf("count: %i\n", count);
            if (followclust !=0)
                follow_dir(filename, followclust, indent+1, image_buf, bpb);
            dirent++;
	}

	cluster = get_fat_entry(cluster, image_buf, bpb);
    }
}


void traverse_root(char * filename, uint8_t *image_buf, struct bpb33* bpb)
{
	
    uint16_t cluster = 0;

    struct direntry *dirent = (struct direntry*)cluster_to_addr(cluster, image_buf, bpb);

    int i = 0;
    for ( ; i < bpb->bpbRootDirEnts; i++)
    {
        uint16_t followclust = print_dirent(filename,dirent, 0);
        if (is_valid_cluster(followclust, bpb)) {
            follow_dir(filename, followclust, 1, image_buf, bpb);
		printf("Cluster is valid\n");
		}
        dirent++;
    }
printf("finished going through stuff\n");
	//
	find_orphans(image_buf, bpb);
			
			
		
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
    char filename[128];
    image_buf = mmap_file(argv[1], &fd);
    bpb = check_bootsector(image_buf);
    traverse_root(filename, image_buf, bpb);

    unmmap_file(image_buf, &fd);

    return 0;
}

/*Hi professor Sommers!
 As you see by some of our variable names we have been struggling pretty hard on this project. We cant even test our code becasuse the next cluster we look at always returns 0 and we dont know why. We have spent at 20-25 hours trying to fix it and are totally unsure how to progress. We came to office hours on thrusday but the line was far to long and we had class at 1:20. I know it is our fault that we didn't get a chance to see you, but we have tried to implent as much code as we could with the problems we have been having. I feel like our code should work and we have made 1 stupid mistake, but we just cant seem to find it. WE HAVE REWRITTEN OUR ENTIRE CODE TWICE. WE ARE GOING MAD. that is all. Have a good break. 
*/


