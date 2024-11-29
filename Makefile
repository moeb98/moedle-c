all: moedle

moedle: moedle.c words.c words.h Makefile
	gcc -std=c17 -Os -Wall -pedantic moedle.c words.c -o moedle

exe: Makefile words.c moedle.c
	cl /nologo /D_CRT_SECURE_NO_WARNINGS /std:c99 /Os /Wall /wd4668 /wd4820 /wd4061 /wd5045 moedle.c words.c
