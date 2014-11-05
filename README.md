xlog功能    
----------
实时收集指定日志文件内容，发送到sources类型为netcat的flume端(理论上支持所有以socket形式监听的日志收集服务端)       
实际测试单进程每秒收集发送2000行nginx日志没有问题,并且对系统cpu、io、带宽、内存占用极低，几乎可以忽略对本机其他业务的影响                   


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





