#include "Server.h"


std::unordered_map<std::string, int> Server::name_sock_map;
std::unordered_map<int, std::set<int>> Server::group_sock_map;
std::unordered_map<int, std::string> Server::sock_cookie_map;
std::mutex Server::name_sock_lock;
std::mutex Server::group_sock_lock;
std::mutex Server::sock_cookie_lock;

MYSQL *Server::mysql = mysql_init(mysql);

Server::Server(const char* ip, int port)
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);
	
	if (mysql_real_connect(mysql, "127.0.0.1", "root", "0814", "chatroom", 3306, NULL, 0))
		log(LogLevel::info, "成功连接到mysql数据库");
	else
		log(LogLevel::warning, "连接到数据库失败");

	if (RedisConnect("127.0.0.1", "AUTH 123654") == "OK")
		log(LogLevel::info, "成功连接到Redis服务器");
	else
		log(LogLevel::warning, "连接到Redis服务器失败");
}

Server::~Server()
{
	for (auto& c : name_sock_map) {
		closesocket(c.second);	// c.second是客户端的sockfd
	}
	closesocket(sockfd);
}

void Server::run()
{
	sockaddr_in client_addr;
	int ret;
	ret = bind(sockfd, (sockaddr*)&server_addr, sizeof(sockaddr));
	listen(sockfd, SOMAXCONN);
	int len = sizeof(SOCKADDR);

	while (1) {
		client_sock = accept(sockfd, (SOCKADDR*)&client_addr, &len);
		if (client_sock < 0)
			log(LogLevel::warning, "错误代码:{}", WSAGetLastError());
		else
			log(LogLevel::info, "与客户端[{}]连接, 准备接受数据", client_sock);


		std::thread recvThread(RecvMsg, client_sock);
		recvThread.detach();
	}
}

void Server::RecvMsg(int conn)
{
	char recvbuf[1000];
	connect_info info;
	/* 
	 * bool logined;			客户端是否已经登录
	 * std::string login_name;	客户端登录用户名
	 * std::string target_name;	目标客户端用户名
	 * int target_conn；			目标客户端套接字
	 * int group_num			客户端所在群
	 */
	std::get<0>(info) = false;	// logined = false;
	std::get<3>(info) = -1;		// target_conn = -1;

	while (1) {
		memset(recvbuf, 0, sizeof(recvbuf));
		int ret = recv(conn, recvbuf, sizeof(recvbuf), 0);
		if (ret > 0) {
			// log("收到消息: " + recvbuf, LogLevel::info);
			HandleRequest(conn, recvbuf, info);
		}
		else {
			log(LogLevel::warning, "错误代码: {}, 关闭与客户端[{}]的连接", ret, conn);
			closesocket(conn);
			name_sock_map.erase(std::get<1>(info));
			break;
		}
	}
}

// 客户端的套接字, 接收到的信息, 基本的连接信息
void Server::HandleRequest(int conn, std::string str, connect_info& info) {

	// 基本的连接信息
	bool logined				= std::get<0>(info);
	std::string login_name		= std::get<1>(info);
	std::string target_name		= std::get<2>(info);
	int target_conn				= std::get<3>(info);
	int group_num				= std::get<4>(info);

	// connect to mysql server

	std::string sendstr;


	// Register, str="[reg]name:pass"
	if (str.find("[reg]") == 0) {
		std::string name, pass;
		int seperator = str.find(":"), length = str.length();
		pass = str.substr(seperator + 1);
		name = str.substr(5, length - pass.length() - 6/* sizeof("[reg]") + sizeof(":") */);
		log(LogLevel::info, "用户名:{}, 密码:{}", name, pass);
		// INSERT INTO users VALUES( "name", "pass" );
		str = "INSERT INTO users VALUES( \"" + name + "\", \"" + pass + "\");";
		log(LogLevel::info, "SQL语句:{}", str);
		int ret = mysql_query(mysql, str.c_str());
		if (ret) {
			log(LogLevel::warning, "INSERT语句失败, 错误代码: {}", ret);
			sendstr = "[ret]error";	// register failed
		}
		else {
			log(LogLevel::info, "INSERT语句成功");
			sendstr = "[ret]ok";
		}
		send(conn, sendstr.c_str(), sendstr.length(), 0);	// 向客户端返回结果
	}
	// Login, str="[login]name:pass"
	else if (str.find("[login]") == 0) {
		std::string name, pass, sqlstatement;
		int seperator = str.find(":"), length = str.length();
		pass = str.substr(seperator + 1);
		name = str.substr(7, length - pass.length() - 8/* sizeof("[reg]") + sizeof(":") */);
		log(LogLevel::info, "用户名:{}, 密码:{}", name, pass);
		// SELECT name, pass FROM users WHERE name="name";
		sqlstatement = "SELECT name, pass FROM users WHERE name=\"" + name + "\";";
		int ret = mysql_query(mysql, sqlstatement.c_str());
		MYSQL_RES* result = mysql_store_result(mysql);
		MYSQL_ROW sql_row = mysql_fetch_row(result);
		if (!ret && sql_row) {	// 找到结果
			log(LogLevel::info, "找到MYSQL中的数据: name={}, pass={}", sql_row[0], sql_row[1]);
			if (pass == sql_row[1]) {
				// 密码正确
				log(LogLevel::info, "客户端[{}]登陆时密码正确", conn);
				sendstr = "[ret]ok";

				// 更新name_sock_map表
				name_sock_lock.lock();
				name_sock_map[name] = conn;	// 加入到name_sock_map表中, 代表用户已登录
				name_sock_lock.unlock();

				logined = true;
				login_name = name;

				// 生成cookie
				srand(time(NULL));	// 用时间作为随机算法的种子
				std::string cookie;
				for (int i = 0; i < 10; i++) {	// 十位数cookie
					int type = rand() % 3;	// 0:数字, 1:小写字母, 2:大写字母
					int random;
					switch (type) {
					case 0:
						random = rand() % 10;
						cookie += '0' + random;
						break;
					case 1:
						random = rand() % 26;
						cookie += 'a' + random;
						break;
					case 2:
						random = rand() % 26;
						cookie += 'A' + random;
						break;
					}
				}
				log(LogLevel::info, "生成cookie: {}", cookie);
				std::string rCmd = "HSET session " + cookie + " " + login_name;	// "HGET session cookie login_name"
				std::string result = RedisCommand(rCmd.c_str());	// 没有返回
				log(LogLevel::info, "Redis语句: {}", rCmd);
				if (result != "(null)")
					log(LogLevel::error, "Redis: {}", result);
				// 设置Redis Key过期时间: 30s
				//rCmd = "EXPIRE " + cookie + " 30";
				//result = RedisCommand(rCmd.c_str());
				//log(LogLevel::info, "Redis语句: {}", rCmd);
				//if (result != "(null)")
				//	log(LogLevel::error, "Redis: {}", result);

				sendstr += cookie;	// 将生成的cookie发送到客户端

			}
			else { // 密码错误
				sendstr = "[ret]incorrect_password";
				log(LogLevel::info, "客户端[{}]登陆时密码错误", conn);
			} 
		}
		else {	// 没找到该用户
			log(LogLevel::warning, "不存在名为[{}]的用户", name);
			sendstr = "[ret]not_found";
		}
		send(conn, sendstr.c_str(), sendstr.length(), 0);	// 向客户端返回结果
	}
	else if (str.find("[target]") == 0) {	// 设置私聊对象 
		std::string target = str.substr(8);
		if (name_sock_map.find(target) == name_sock_map.end()) {
			// 目标未找到
			log(LogLevel::warning, "未找到名为[{}]的目标", target);
			sendstr = "[ret]not_found";
		}
		else {
			// 找到目标, 建立连接
			target_name = target;
			target_conn = name_sock_map[target];
			log(LogLevel::info, "准备建立从{}到{}的连接", login_name, target_name);
			sendstr = "[ret]ok";
		}
		send(conn, sendstr.c_str(), sendstr.length(), 0);	// 向客户端返回结果
	}
	else if (str.find("[message]") == 0) {
		// 发送私聊消息
		sendstr = str.substr(9);	// 去掉前缀"[message]"
		if (sendstr == "exit") {
			// 客户端下线, 更新name_sock_map表
			log(LogLevel::info, "用户[{}]下线", login_name);
			closesocket(conn);
			name_sock_map.erase(login_name);
		} else {
			log(LogLevel::info, "从{}转发消息:\"{}\"到{}", login_name, sendstr, target_name);
			sendstr = "[" + login_name + "]" + sendstr;	// 格式化为"[login_name]message..."
			send(target_conn, sendstr.c_str(), sendstr.length(), 0);	// 将消息转发给目标客户端
		}
	}
	else if (str.find("[group_num]") == 0) {	// 绑定群聊号
		group_num = std::stoi(str.substr(11));	// 去掉前缀[group], 转换成数字

		log(LogLevel::info, "用户[{}]加入群组[{}]", login_name, group_num);
		group_sock_lock.lock();
		group_sock_map[group_num].insert(conn);	// 将当前客户端的sockfd加入群聊中
		group_sock_lock.unlock();
	}
	else if (str.find("[gr_message]") == 0)	{	// 群聊消息
		sendstr = str.substr(12);	// 去掉前缀[gr_message]
		log(LogLevel::info, "用户[{}]向群[{}]发送消息: {}", login_name, group_num, sendstr);
		sendstr = "[" + login_name + "]" + sendstr;
		for (auto c : group_sock_map[group_num]) {
			if (c != conn)
				send(c, sendstr.c_str(), sendstr.length(), 0);
		}
	}
	else if (str.find("[cookie]") == 0) {	// cookie登录
		std::string cookie = str.substr(8);
		log(LogLevel::info, "收到客户端[{}]的cookie: {}", conn, cookie);
		std::string rCmd = "HGET session " + cookie;
		std::string result = RedisCommand(rCmd.c_str());	// 应该返回cookie对应的用户名, 否则返回"(null)"
		if (result != "(null)" && result.substr(0, 3) != "ERR") {	// 找到对应用户, 直接登录
			logined = true;
			login_name = result;
			log(LogLevel::info, "找到用户{}", login_name);
			result = "[ret]ok" + result;	// "[ret]oklogin_name" 即 [ret]ok+用户名
		}
		else
			log(LogLevel::info, "没有找到cookie对应的用户");
		// 没有找到cookie对应的用户或者cookie过期, 直接返回"(null)", 客户端将不做处理
		send(conn, result.c_str(), result.length(), 0);
	}


	// 更新基本信息info
	std::get<0>(info) = logined;
	std::get<1>(info) = login_name;
	std::get<2>(info) = target_name;
	std::get<3>(info) = target_conn;
	std::get<4>(info) = group_num;
}
