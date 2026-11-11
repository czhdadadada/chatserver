# chatserver
基于 muduo 库实现的集群聊天服务器，搭配控制台客户端；借助 Nginx TCP 四层负载均衡做集群分发，使用 Redis 发布订阅完成跨节点消息转发，MySQL 持久化用户、群组、离线消息。支持用户注册登录、私聊、群聊、离线消息。


## 功能特性
- 用户模块：注册、登录、退出，密码加密存储
- 私聊模块：在线实时私聊，用户下线自动存储离线消息
- 群聊模块：创建群组、加入群组、群内消息广播
- 离线消息：用户上线自动拉取并推送未读离线消息
- 集群方案：Nginx TCP 负载均衡 + Redis Pub/Sub 跨服务节点消息同步
- 客户端：控制台交互式客户端，轻量易用

## 技术栈
- C++11
- Muduo（Reactor 高性能网络库）
- MySQL（用户、好友、群组、离线消息持久化）
- Redis（发布订阅，集群节点间消息同步）
- Nginx（Stream 四层 TCP 负载均衡）
- CMake

## 项目目录结构
```

├── bin/            # 编译输出可执行文件
├── include/        # 项目头文件
├── src/            # 服务端业务源码
├── test/           # 控制台客户端源码
├── thirdparty/     # 第三方依赖库
├── CMakeLists.txt  # 项目构建脚本
├── .gitignore      # Git 忽略配置
├── LICENSE         # 开源协议
└── README.md       # 项目说明文档

```

## 编译方式
```bash
cd build
rm -rf *
cmake ..
make
```

## 部署 & 运行步骤

1. 准备数据库：启动 MySQL，导入项目 SQL，创建 `user`、`friend`、`group`、`groupuser`、`offlinemessage` 数据表
2. 启动 Redis 服务，默认端口 6379，无需额外配置
3. 启动多台 ChatServer 服务节点，配置相同 Redis 地址，监听不同端口
4. 修改 Nginx 配置，开启 stream 模块，配置 TCP 负载均衡指向所有服务节点
5. 启动控制台客户端，连接 Nginx 负载均衡地址，注册账号后即可私聊、群聊

## 集群通信原理

Nginx 作为四层 TCP 负载均衡入口，客户端连接 Nginx，由 Nginx 将连接分发到任意一台 ChatServer 节点。
如果发送方连接在服务节点 A，接收方在线在服务节点 B：节点 A 将消息 Publish 到 Redis 指定频道；所有服务节点订阅该频道，节点 B 监听到消息后，推送给本地在线用户，以此实现跨机器消息互通。
用户下线期间收到的消息存入 MySQL 离线消息表，下次登录自动拉取推送。

## 环境依赖

- muduo 网络库
- mysqlclient 开发库
- hiredis、redis-plus-plus
- cmake >= 3.10
- gcc/g++ 支持 C++11

## 数据库建表 SQL

```
-- 用户表
CREATE TABLE `user` (
  `id` INT PRIMARY KEY AUTO_INCREMENT,
  `name` VARCHAR(50) NOT NULL UNIQUE,
  `password` VARCHAR(100) NOT NULL,
  `state` ENUM('online','offline') DEFAULT 'offline'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 好友表
CREATE TABLE `friend` (
  `userid` INT NOT NULL,
  `friendid` INT NOT NULL,
  PRIMARY KEY (`userid`,`friendid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 群组表
CREATE TABLE `group` (
  `id` INT PRIMARY KEY AUTO_INCREMENT,
  `groupname` VARCHAR(50) NOT NULL,
  `groupdesc` VARCHAR(200) DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 群成员表
CREATE TABLE `groupuser` (
  `groupid` INT NOT NULL,
  `userid` INT NOT NULL,
  `role` ENUM('creator','normal') DEFAULT 'normal',
  PRIMARY KEY (`groupid`,`userid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 离线消息表
CREATE TABLE `offlinemessage` (
  `id` INT PRIMARY KEY AUTO_INCREMENT,
  `userid` INT NOT NULL,
  `message` TEXT NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
```

> 注意：nginx 需要开启 stream 模块（--with-stream）
```nginx
stream {
    upstream chatserver {
        server 127.0.0.1:6000;
        server 127.0.0.1:6001;
    }
    server {
        listen 8000;
        proxy_pass chatserver;
    }
}
