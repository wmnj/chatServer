œ∑´®†¥¨ˆøπ“‘«åß∂ƒ©˙∆˚¬…æ≈√∫˜µ≤≥÷≠–ºª•¶§∞¢£™¡`
### 使用技术
c++ mysql redis nginx muduo
### 功能
1. 注册、登录  
2. 加好友  
3. 创建群聊、加入群聊  
4. 一对一聊天、群聊  
5. 使用nginx实现分布式服务器，使用redis作为服务器通信的中间件


### 使用
* 编译：
1. 直接运行autobuild.sh脚本
2. 依次使用命令 cmake make
* 运行：  
编译后可执行文件在当前项目的bin目录下，有服务器可客户端两个可执行文件。  
启动服务器：./chatServer 127.0.0.1 6000，如有多台服务器可同时开启。   
启动客户端：./ChatClient 127.0.0.1 8000, 注意启动的时候访问的端口号是在nginx中配置的端口号。
### nginx配置
1. 下载安装在/usr/local/nginx目录下，sbin是可执行文件，直接./nginx启动。  
2. conf文件夹下有配置文件nginx.conf, vim该文件配置服务器信息，在event和http中间加上：
```xml
stream {
    upstream MyServer {
         server 127.0.0.1:10002 weight=1 max_fails=3 fail_timeout=30s;
         server 127.0.0.1:10003 weight=1 max_fails=3 fail_timeout=30s;
    }
    server {
         proxy_connect_timeout ls;
         # proxy_timeout 3s
         listen 8000;
         proxy_pass MyServer;
         tcp_nodelay on;
    }
}  
  
```

重新加载配置文件 ：nginx -s reload  
终止服务（不要用kill）：nginx -s stop  


### mysql数据库表设计
 
1. OfflineMessage，存储用户的离线消息    
```sql
CREATE TABLE OfflineMessage (
    userid INT PRIMARY KEY,
    Message VARCHAR(500) NOT NULL);
    CREATE TABLE OfflineMessage (
    userid INT not null,
    Message VARCHAR(500) NOT NULL); 
```
userid不能当作主键 否则同一个用户不能发两次离线消息给另外一个人；  

2. 用户和所属群组的表
```sql
CREATE TABLE GroupUser (
    groupid INT PRIMARY KEY,
    userid VARCHAR(50) NOT NULL,
    grouprole ENUM('creator', 'normal') DEFAULT 'normal'
);
```
   
3. 群组表
   ```sql
   CREATE TABLE AllGroup (
    id INT AUTO_INCREMENT PRIMARY KEY,
    groupname VARCHAR(50) NOT NULL,
    groupdesc VARCHAR(50) DEFAULT '');

   ```
   
4. 用户表
   ```sql
   CREATE TABLE User (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(50) NOT NULL,
    state ENUM('online', 'offline') DEFAULT 'offline');
   ```

5. 朋友关系表
   ```sql
   CREATE TABLE Friend (
    userid INT NOT NULL,
    friendid INT NOT NULL,
    CONSTRAINT PK_Friend PRIMARY KEY (Userid, friendid));
   ```

### 小细节
* 编写客户端程序的时候，在主菜单栏的循环显示中，如果用户的输入无效，需要清空缓存区。