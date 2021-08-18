all: user pd as fs

user: user.c 
	gcc user.c -o user

pd: pd.c 
	gcc pd.c -o pd

as: as.c 
	gcc as.c -o as

fs: fs.c
	gcc fs.c -o fs

clean:
	rm -f user as pd fs