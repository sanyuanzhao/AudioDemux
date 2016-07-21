// Demux.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "demux.h"

int main()
{
	const char* args[4] = {"", "hao.mp3", "hao.pcm", "hao.pcm" };
	return demux(4, args);
}

