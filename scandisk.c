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


void usage(char *progname) {
    fprintf(stderr, "usage: %s <imagename>\n", progname);
    exit(1);
}


uint16_t get_dirent(struct direntry *dirent, char *buffer)
{
    uint16_t followclust = 0;

    //memset(buffer, 0, MAXFILENAME);

    int i;
    char name[9];
    char extension[4];
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
	printf("Win95 long-filename entry seq 0x%0x\n", dirent->deName[0]);
    }
    else if ((dirent->deAttributes & ATTR_DIRECTORY) != 0) 
    {
        // don't deal with hidden directories; MacOS makes these
        // for trash directories and such; just ignore them.
	if ((dirent->deAttributes & ATTR_HIDDEN) != ATTR_HIDDEN)
        {
            //strcpy(buffer, name);
            file_cluster = getushort(dirent->deStartCluster);
            followclust = file_cluster;
        }
    }
    else 
    {
        /*
         * a "regular" file entry
         * print attributes, size, starting cluster, etc.
         */
        //strcpy(buffer, name);
        /*if (strlen(extension))  
        {
            strcat(buffer, ".");
            strcat(buffer, extension);
        }*/
	file_cluster = getushort(dirent->deStartCluster);
	printf("we normal file cluster!!!!!");
        followclust = file_cluster;
	
    }
    return followclust;
}



void follow_dir(uint8_t *image_buf, struct bpb33* bpb)
{
	uint16_t cluster = 0;
	uint16_t followclust;
	char *buffer=NULL;
	
	printf("in folowdir loop\n");
        struct direntry *dirent = (struct direntry*)cluster_to_addr(cluster, image_buf, bpb);
		
	int numDirEntries = (bpb->bpbBytesPerSec * bpb->bpbSecPerClust) / sizeof(struct direntry);
        int i = 0;
	for ( ; i < numDirEntries; i++){
		dirent = (struct direntry*)cluster_to_addr(i, image_buf, bpb);
		
		followclust = get_dirent(dirent, buffer);

		uint32_t realsize = getulong(dirent->deFileSize);
		uint32_t clustersize = bpb->bpbSecPerClust*bpb->bpbBytesPerSec;
		int expected_num_clust = realsize/clustersize;
		printf( "expected clusters: %i \n", expected_num_clust);
		//find actual number of clusters
		//printf("getfatentry: %hu\n" , get_fat_entry(followclust, image_buf, bpb));
		//printf("clust to add: %s\n" , cluster_to_addr(followclust, image_buf, bpb));
		uint16_t i = followclust;
		int actual = 0;
		while (!is_end_of_file(i)) {
			actual++;
			i++;
		}
		printf("Actual clusters: %i \n",actual);
		if (expected_num_clust>actual) {
			printf("Inconsistent cluster numbers\n");
			uint32_t diff = getulong(expected_num_clust - actual);
			//update rootdir info and metadata
			uint32_t currentsize = realsize-clustersize*diff;
			putulong(dirent->deFileSize, currentsize);
			bpb->bpbResSectors -= diff * bpb->bpbSecPerClust;
		}

		else if (expected_num_clust<actual) {
			printf("print name here\n");
			uint16_t j = i+1;
			while (j<=actual) {
				// free the cluster at index j
				set_fat_entry(j, CLUST_FREE, image_buf, bpb);
			}
		}
		else {
			printf("they the same");
			}
		/*for (i = 2; i < total_clusters; i++) 
		    {
			if (get_fat_entry(i, image_buf, bpb) == CLUST_FREE) 
			{
			    break;
			}
		    }*/		

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

    // your code should start here...
	// what to look for
	//printf("Meta data%u\n",bpb->bpbFATs);
	// fat entries vs metadata(boot sector)

			//get metadata

			//look at start cluster for file and interate to count them
				//root_dir_adder
				uint8_t *root_addr;
				root_addr = root_dir_addr(image_buf, bpb);
				follow_dir(image_buf, bpb);
				
				

		//print out list of files whose length is directory is != to length in data blocks
		// free clusters that are beyond the end of a file but FAT still points
		// adjust the size entry for a file if there is a free or bad cluster in FAT chain

	// system crashes halfway through writing data 
		


	// Directory entry is removed and system crashes
		// check all clusters and see if there are any clusters not referenced in directory print out
		// make new directory entry in the root directory... named found1.dat found2.dat

	//OS detects a bad cluster in the FAT





    unmmap_file(image_buf, &fd);
    return 0;
}
