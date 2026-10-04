# myfind
## In Docker/Ubuntu starten

docker run -it --rm -v "$PWD":/work -w /work ubuntu bash

make
make clean
./myfind -R -i ./testdata test.txt      rekusriv suche egal buchstaben gröse 
./myfind ./testdata test.txt        nicht rekursiv
./myfind -R -i ./testdata TEST.TXT HELLO.TXT notes.md       /mehrere dateien
exit Docker verlassen