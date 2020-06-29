.PHONY: clean

mkfs: mkfs.c
	gcc -O1 -Wall -Werror -o $@ -I../framework $<

clean:
	rm -rf mkfs
