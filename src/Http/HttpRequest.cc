#include "HttpRequest.h"

const std::unordered_set<std::string> WebServer::HttpRequest::DEFAULT_HTML{
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture", };

const std::unordered_map<std::string, int> WebServer::HttpRequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };


void WebServer::HttpRequest::init()
{
    method=path=version=body="";
    state = REQUEST_LINE;
    headers.clear();
    post.clear();
}



bool WebServer::HttpRequest::IsKeepAlive() const
{
    if(headers.count("Connetion") == 1)
    {
        return headers.find("Connection")->second == "keep-alive" && version == "1.1";
    }
    return false;
}

bool WebServer::HttpRequest::pares(Buffer& buff)
{
    const char CRLF[]="\r\n";
    if(buff.readableBytes() <= 0)
        return false;
    while (buff.readableBytes() && state != FINISH)  //能读并且没读完
    {
      const char* lineEnd = std::search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);
      std::string line(buff.Peek(),lineEnd);
      switch (state)
      {
        case REQUEST_LINE:
            if(!ParseRequestLine(line))
                return false;
            ParsePath();
            break;
        case HEADERS:
            ParseHeader(line);
            if(buff.readableBytes()<=2)
            {
                state=FINISH;
            }
            break;
        case BODY:
            ParseBody(line);
            break;
        default:
            break;
      }
        if(lineEnd == buff.BeginWrite()) { break; }
        buff.RetrieveUntil(lineEnd + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", method.c_str(), path.c_str(), version.c_str());
   return true; 
}


bool WebServer::HttpRequest::ParseRequestLine(const std::string& line)
{
    //GET / HTTP/1.1
    std::regex patten("([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch subMatch;
    if(std::regex_match(line,subMatch,patten))
    {
        method = subMatch[1];
        path = subMatch[2];
        version = subMatch[3];
        state = HEADERS;
        return true;
    }
    return false;
}
void WebServer::HttpRequest::ParsePath()
{
    if(path == "/") {
        path = "/index.html"; 
    }
    else {
        for(auto &item: DEFAULT_HTML) {
            if(item == path) {
                path += ".html";
                break;
            }
        }
    }
}
void WebServer::HttpRequest::ParseHeader(const std::string& line)
{
     //Host: localhost:9600
    std::regex patten("([^ ]*): ?(.*)$");
    std::smatch subMatch;
    if(std::regex_match(line,subMatch,patten))
    {
       headers[subMatch[1]]=subMatch[2];
    }
    else
    {
        state = BODY;
    }
}
void WebServer::HttpRequest::ParseBody(const std::string& line)
{
    body = line;
    ParsePost();
    state = FINISH;
}

void WebServer::HttpRequest::ParsePost() {
    if(method == "POST" && headers["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFromUrlencoded();
        if(DEFAULT_HTML_TAG.count(path)) {
            int tag = DEFAULT_HTML_TAG.find(path)->second;
            LOG_DEBUG("Tag:%d", tag);
            if(tag == 0 || tag == 1) {
                bool isLogin = (tag == 1);
                if(UserVerify(post["username"], post["password"], isLogin)) {
                    path = "/welcome.html";
                } 
                else {
                    path = "/error.html";
                }
            }
        }
    }   
}

void WebServer::HttpRequest::ParseFromUrlencoded() {
    if(body.size() == 0) { return; }

    std::string key, value;
    int num = 0;
    int n = body.size();
    int i = 0, j = 0;

    for(; i < n; i++) {
        char ch = body[i];
        switch (ch) {
        case '=':
            key = body.substr(j, i - j);
            j = i + 1;
            break;
        case '+':
            body[i] = ' ';
            break;
        case '%':
            num = ConverHex(body[i + 1]) * 16 + ConverHex(body[i + 2]);
            body[i + 2] = num % 10 + '0';
            body[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':
            value = body.substr(j, i - j);
            j = i + 1;
            post[key] = value;
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    assert(j <= i);
    if(post.count(key) == 0 && j < i) {
        value = body.substr(j, i - j);
        post[key] = value;
    }
}

int WebServer::HttpRequest::ConverHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}

bool WebServer::HttpRequest::UserVerify(const std::string &name, const std::string &pwd, bool isLogin) {
    if(name == "" || pwd == "") { return false; }
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());
    MYSQL* sql;
    SqlConnRAII(&sql,  SqlConnPool::Instance());
    assert(sql);
    
    bool flag = false;
    unsigned int j = 0;
    char order[256] = { 0 };
    MYSQL_FIELD *fields = nullptr;
    MYSQL_RES *res = nullptr;
    
    if(!isLogin) { flag = true; }
    /* 查询用户及密码 */
    snprintf(order, 256, "SELECT username, password FROM user WHERE username='%s' LIMIT 1", name.c_str());
    LOG_DEBUG("%s", order);

    if(mysql_query(sql, order)) { 
        mysql_free_result(res);
        return false; 
    }
    res = mysql_store_result(sql);
    j = mysql_num_fields(res);
    fields = mysql_fetch_fields(res);

    while(MYSQL_ROW row = mysql_fetch_row(res)) {
        LOG_DEBUG("MYSQL ROW: %s %s", row[0], row[1]);
        std::string password(row[1]);
        /* 注册行为 且 用户名未被使用*/
        if(isLogin) {
            if(pwd == password) { flag = true; }
            else {
                flag = false;
                LOG_DEBUG("pwd error!");
            }
        } 
        else { 
            flag = false; 
            LOG_DEBUG("user used!");
        }
    }
    mysql_free_result(res);

    /* 注册行为 且 用户名未被使用*/
    if(!isLogin && flag == true) {
        LOG_DEBUG("regirster!");
        bzero(order, 256);
        snprintf(order, 256,"INSERT INTO user(username, password) VALUES('%s','%s')", name.c_str(), pwd.c_str());
        LOG_DEBUG( "%s", order);
        if(mysql_query(sql, order)) { 
            LOG_DEBUG( "Insert error!");
            flag = false; 
        }
        flag = true;
    }
    SqlConnPool::Instance()->FreeConn(sql);
    LOG_DEBUG( "UserVerify success!!");
    return flag;
}