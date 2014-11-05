安装xlog
----------

\#下载
https://github.com/qidasheng/xlog


\#安装
./configure
make
make install

\#清理
make clean

\#卸载
make uninstall


\#配置
根目录xlog.conf有注解


\#运行
xlog -c xlog.conf

\#后台运行
配置daemonize = yes
或者
xlog -c xlog.conf -d



