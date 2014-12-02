xlog功能    
----------
实时收集指定日志文件内容，发送到sources类型为netcat的flume端(理论上支持所有以socket形式监听的日志收集服务端)         
可以满足单个日志文件的实时收集，可以配置nginx每个项目一个日志文件，每个日志文件一个单独的进程进行收集；并且对系统cpu、io、带宽、内存占用极低.                       


安装xlog    
----------    

下载
-----------
https://github.com/qidasheng/xlog    


脚本一键安装   
-----------
```Bash
/bin/sh run.sh
```


一步一步安装      
-----------
```Bash
./configure   
make    
make install
```

清理    
```Bash
make clean
```

卸载    
```Bash
make uninstall
```

配置    
```Vim
根目录xlog.conf有注解
```


运行
```Bash
xlog -c xlog.conf 
```

后台运行
```Vim
配置
daemonize = yes    
或者     
xlog -c xlog.conf -d
```





