windows:
	gcc .\main.c -l"ws2_32" -o GoFish.exe
linux:
	gcc ./main.c -o GoFish
	chmod +x GoFish