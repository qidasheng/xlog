make clean
make uninstall
rm -f depcomp 
rm -f missing 
aclocal
autoconf 
autoheader 
automake --add-missing
./configure 
make
make install

