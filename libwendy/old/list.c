/**
 * Simple test - just list assets
 */

#include <wendy.h>
#include <stdio.h>
#include <time.h>

void cb(const WendyAsset *asset)
{
	printf("Asset %llu was received!\n", asset->id);
	printf("Path: %s\n", asset->path);
}

int main()
{
	wendyConnect(cb);
	
	wendyPollEvents();
	
	wendyDisconnect();
	
	return 0;
}

