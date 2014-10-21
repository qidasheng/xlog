#valgrind --tool=memcheck --leak-check=full --track-origins=yes  --show-reachable=yes -v ./xlog -c ./xlog_new.conf 
make clean
make uninstall
./configure 
make
make install

