Docker 使用教程

1. 创建一个名为test的docker容器(共享文件夹配置)
	> docker run -i -t --name=test ubuntu:xenial 
	> docker run -i -t --name=test -v [path of shared folder]  ubuntu:xenial
	共享文件夹位于/root/share。

2. 进入退出容器
	> docker start -i test
	> exit

3. 查看所有的docker容器
	> docker ps -a

4. 删除docker的容器：
	> docker rm test

5. 多个终端
	> docker exec -i -t test /bin/bash