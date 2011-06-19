#include <stdio.h>
#include <wendy/wendy.h>

static WendyAssetCallback assetCallback = NULL;

void WENDYAPI wendyConnect(WendyAssetCallback callback)
{
	printf("Iniiit\n");
	assetCallback = callback;
}

void WENDYAPI wendyDisconnect()
{
}

void WENDYAPI wendyPollEvents()
{
	WendyAsset asset;
	asset.id = 42ll;
	asset.path = "plop/yop/lol";
	assetCallback(&asset);
}

void WENDYAPI wendyWaitEvents()
{
}

