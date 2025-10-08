
#include <arpa/inet.h>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <locale>
#include <netdb.h>
#include <sstream>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <cdinput.h>

#include "comm_func.h"
#include <hv_version.h>
#include "data.h"
#include "conn_mgr.h"

bool isNumric(const char *str_nums) {
    for (const char *p = str_nums; *p; p++) {
        if (*p < '0' || *p > '9') { return false; }
    }

    // 待完善，支持小数、负数。。。

    return true;
}

std::string getTimeStr() {
    char           buff[16];
    std::time_t    t   = std::time(NULL);
    struct std::tm cur = *std::localtime(&t);
    snprintf(buff, sizeof(buff), "%02d%c%02d", cur.tm_hour,':',cur.tm_min);

    // static int state = 1;
    // if(++state%2)
    //     snprintf(buff, sizeof(buff), "%02d%c%02d", cur.tm_hour,':',cur.tm_min);
    // else
    //     snprintf(buff, sizeof(buff), "%02d%c%02d", cur.tm_hour,' ',cur.tm_min);
    // LOGD("getTimeStr()  %s" , buff);
      
    // snprintf(buff, sizeof(buff), "%02d%c%02d", g_appData.RealClkHour,':',g_appData.RealClkMin);
    return std::string(buff);
}

std::string getWeekDay(){
    std::time_t    t   = std::time(NULL);
    struct std::tm tm_now = *std::localtime(&t);

    std::vector<std::string> weekdays = {
        "周日","周一", "周二", "周三", "周四", "周五", "周六"
    };
    return std::string(weekdays.at(tm_now.tm_wday));
}

std::string getMonthDayDateTime(){
    std::time_t    t   = std::time(NULL);
    struct std::tm tm_now = *std::localtime(&t);

    return std::to_string(tm_now.tm_mon)+"/"+std::to_string(tm_now.tm_mday);
}


int64_t getTodayTimeSec() {
    std::time_t    t   = std::time(NULL);
    struct std::tm cur = *std::localtime(&t);
    return cur.tm_sec;
}

int64_t getTimeSec() {
    struct timeval _val;
    gettimeofday(&_val, NULL);
    return _val.tv_sec;
}

int64_t getTimeMSec() {
    struct timeval _val;
    gettimeofday(&_val, NULL);
    return (int64_t)_val.tv_sec * 1000 + _val.tv_usec / 1000;
}

std::string getDateTime() {
    char           buffer[256];
    struct timeval _val;

    time_t     now;
    struct tm *tm_now;
    char       datetime[128];

    gettimeofday(&_val, NULL);
    now    = _val.tv_sec;
    tm_now = localtime(&now);
    strftime(datetime, sizeof(datetime), "%Y年%m月%d日 %H时%M分", tm_now);

    // snprintf(buffer, sizeof(buffer), "%s.%03ld.%03ld", datetime, _val.tv_usec / 1000, _val.tv_usec % 1000);

    return std::string(datetime);
}

std::string getDateTime2() {
    char           buffer[256];
    struct timeval _val;

    time_t     now;
    struct tm *tm_now;
    char       datetime[128];

    gettimeofday(&_val, NULL);
    now    = _val.tv_sec;
    tm_now = localtime(&now);
    strftime(datetime, sizeof(datetime), "%Y-%m-%d ", tm_now);
    
    std::vector<std::string> weekdays = {
        "星期天","星期一", "星期二", "星期三", "星期四", "星期五", "星期六"
    };

    // snprintf(buffer, sizeof(buffer), "%s.%03ld.%03ld", datetime, _val.tv_usec / 1000, _val.tv_usec % 1000);

    return std::string(datetime) + std::string(weekdays.at((*tm_now).tm_wday));
}

int64_t getZeroTime() {
    time_t     t  = time(NULL);
    struct tm *tm = localtime(&t);
    tm->tm_hour   = 0;
    tm->tm_min    = 0;
    tm->tm_sec    = 0;
    return mktime(tm);
}

bool isToday(int64_t sec) {
    int64_t zeroSec = getZeroTime();
    return zeroSec <= sec && sec < zeroSec + DAY_SECONDS;
}

std::string fillLength(int num, int len, std::string pre /* = "0"*/) {
    std::string retString;
    std::string numString = std::to_string(num);
    while (numString.length() + retString.length() < len) { retString.append(pre); }
    retString += numString;
    return retString;
}

std::string fillLength(std::string &str, int len, char end /* = ' '*/) {
    std::string ret = str;
    if (ret.size() < len) ret.append(len - ret.size(), end);
    return ret;
}

std::string floatDataDecimalLength(float num, int decLen){
    std::string formatterStr = std::to_string(num);
    formatterStr = formatterStr.substr(0,formatterStr.find(".")+decLen+1);
    return formatterStr;
}

void hexdump(const char *label, unsigned char *buf, int len, int width /* = 16*/) {
    int  i, clen;
    char line_buf[512];

    if (width <= 0 || width > 64) { width = 16; }

    LOG(DEBUG) << label << " hex dump:";
    clen = 0;
    for (i = 0; i < len; i++) {
        clen += snprintf(line_buf + clen, sizeof(line_buf) - clen, "%02X ", buf[i]);
        if ((i + 1) % width == 0) {
            LOG(DEBUG) << "  " << line_buf;
            clen = 0;
        }
    }
    if (clen > 0) { LOG(DEBUG) << "  " << line_buf; }
}

std::string hexstr(unsigned char *buf, int len) {
    std::ostringstream oss;
    for (int i = 0; i < len; i++) {
        oss << ' ' << std::hex << std::setfill('0') << std::setw(2) << (int)buf[i];
        if ((i + 1) % 16 == 0) {
            oss << '\n';
        }
    }
    return oss.str();
}

std::string &toUpper(std::string &letter) {
    int         i;
    std::string old = letter;
    letter          = "";
    for (i = 0; i < old.size(); i++) {
        if (old[i] >= 'a' && old[i] <= 'z') {
            letter.append(1, old[i] - 32);
        } else {
            letter.append(1, old[i]);
        }
    }
    return letter;
}

std::string &toLower(std::string &letter) {
    int         i;
    std::string old = letter;
    letter          = "";
    for (i = 0; i < old.size(); i++) {
        if (old[i] >= 'A' && old[i] <= 'Z') {
            letter.append(1, old[i] + 32);
        } else {
            letter.append(1, old[i]);
        }
    }
    return letter;
}

int stringSplit(const std::string &str, std::vector<std::string> &out, char ch /* = ',' */) {
    int    offset = 0;
    size_t pos    = str.find(ch, offset);
    while (pos != std::string::npos) {
        if (offset < pos) out.push_back(str.substr(offset, pos - offset));
        offset = pos + 1;
        pos    = str.find(ch, offset);
    }
    if (offset < str.size()) { out.push_back(str.substr(offset)); }

    return out.size();
}

void timeSet(int year, int month, int day, int hour, int min, int sec) {
    struct timeval tv;
    std::time_t    t;
    struct std::tm cur;

    if (year == 0 && month == 0 && day == 0 && hour == 0 && min == 0 && sec == 0) { return; }

    t   = std::time(NULL);
    cur = *std::localtime(&t);

    if (year > 0)  cur.tm_year = year - 1900;
    if (month > 0) cur.tm_mon  = month - 1;
    if (day > 0)   cur.tm_mday = day;
    cur.tm_hour = hour;
    cur.tm_min  = min;
    cur.tm_sec = sec;

    timeSet((int64_t)mktime(&cur));
}

void timeSet(const int64_t &time_sec) {
    struct timeval tv;
    tv.tv_sec  = time_sec;
    tv.tv_usec = 0;    
// #ifndef DEBUG
    LOG(VERBOSE) << "set time " << tv.tv_sec;
    settimeofday(&tv, NULL);
    sysCommand("hwclock --systohc");
// #endif
}

int wordLen(const char *buffer) {
    const char *pos  = buffer;
    std::wstring wstr;
    wstr.reserve(sizeof(buffer));

    while (*pos) {
        if (*pos & 0x80) {
            // 多字节字符
            wchar_t wch = 0;
            int numBytes = 0;

            if ((*pos & 0xE0) == 0xC0) {
                // 2字节字符
                wch = *pos & 0x1F;
                numBytes = 1;
            } else if ((*pos & 0xF0) == 0xE0) {
                // 3字节字符
                wch = *pos & 0x0F;
                numBytes = 2;
            } else if ((*pos & 0xF8) == 0xF0) {
                // 4字节字符
                wch = *pos & 0x07;
                numBytes = 3;
            }

            for (int i = 0; i < numBytes; ++i) {
                wch = (wch << 6) | (*(++pos) & 0x3F);
            }

            wstr.push_back(wch);
        } else {
            // 单字节字符
            wstr.push_back(static_cast<wchar_t>(*pos));
        }
        pos++;
    }
    std::ostringstream oss;
    for (int i = 0; i < wstr.size(); i++) {
        oss << ' ' << std::hex << std::setfill('0') << std::setw(2) << (int)wstr[i];
        if ((i + 1) % 16 == 0) {
            oss << '\n';
        }
    }

    LOGV("wstr = %s    size = %d",oss.str().c_str(),wstr.size());
    return wstr.size();
}

int getChineseWordLen(const char *buffer){
    const char *pos  = buffer;
    int chineseWord = 0;

    while (*pos) {
        if (*pos & 0x80) {
            // 多字节字符
            wchar_t wch = 0;
            int numBytes = 0;

            if ((*pos & 0xE0) == 0xC0) {
                // 2字节字符
                wch = *pos & 0x1F;
                numBytes = 1;
            } else if ((*pos & 0xF0) == 0xE0) {
                // 3字节字符
                wch = *pos & 0x0F;
                numBytes = 2;
            } else if ((*pos & 0xF8) == 0xF0) {
                // 4字节字符
                wch = *pos & 0x07;
                numBytes = 3;
            }

            for (int i = 0; i < numBytes; ++i) {
                wch = (wch << 6) | (*(++pos) & 0x3F);
            }
            chineseWord++;
        }
        pos++;
    }

    return chineseWord;
}

// 获取字符串长度，中文算两个字符
std::string getWordChineseDouble(const char *buffer, int count){
    const char *pos  = buffer;
    int chineseWord = 0,wordPos = 0;
    std::wstring wstr;
    wstr.reserve(sizeof(buffer));

    while (*pos) {
        
        if (*pos & 0x80) {
            // 多字节字符
            wchar_t wch = 0;
            int numBytes = 0;

            if ((*pos & 0xE0) == 0xC0) {
                // 2字节字符
                wch = *pos & 0x1F;
                numBytes = 1;
            } else if ((*pos & 0xF0) == 0xE0) {
                // 3字节字符
                wch = *pos & 0x0F;
                numBytes = 2;
            } else if ((*pos & 0xF8) == 0xF0) {
                // 4字节字符
                wch = *pos & 0x07;
                numBytes = 3;
            }

            for (int i = 0; i < numBytes; ++i) {
                wch = (wch << 6) | (*(++pos) & 0x3F);
            }

            
            if(wstr.size() + chineseWord + 2 > count)break;
            wstr.push_back(wch);
            chineseWord++;
            wordPos += numBytes+1;
        } else {
            // 单字节字符
            if(wstr.size() + chineseWord + 1 > count)break;
            wstr.push_back(static_cast<wchar_t>(*pos));
            wordPos++;
        }
        pos++;
    }
    std::ostringstream oss;
    for (int i = 0; i < wstr.size(); i++) {
        oss << ' ' << std::hex << std::setfill('0') << std::setw(2) << (int)wstr[i];
        if ((i + 1) % 16 == 0) {
            oss << '\n';
        }
    }

    std::string wordBuf = buffer;
    LOGV("wordBuf = %s/%s wordPos = %d",wordBuf.c_str(), wordBuf.substr(0,wordPos).c_str(),wordPos);
    return wordBuf.substr(0,wordPos);
}

std::string getWord(const char *buffer, int count) {
    std::string words;
    int         wordCount = 0;
    const char *pos       = buffer;
    while (*pos && wordCount < count) {
        if (*pos & 0x80) {
            // 多字节字符
            int numBytes = 0;

            if ((*pos & 0xE0) == 0xC0) {
                // 2字节字符
                numBytes = 1;
            } else if ((*pos & 0xF0) == 0xE0) {
                // 3字节字符
                numBytes = 2;
            } else if ((*pos & 0xF8) == 0xF0) {
                // 4字节字符
                numBytes = 3;
            }
            
            for (int i = 0; i < numBytes+1; ++i) {
                words.push_back(*(pos++));
            }
            
        }else{
            words.push_back(*(pos++));
        }
        wordCount++;
    }
    LOGV(" words = (%s) To (%s)",buffer,words.c_str());
    return words;
}

std::string decLastWord(const char *buffer){
    std::string words;
    const char *pos       = buffer;
    while (*pos) {
        if (*pos & 0x80) {
            // 多字节字符
            int numBytes = 0;

            if ((*pos & 0xE0) == 0xC0) {
                // 2字节字符
                numBytes = 1;
            } else if ((*pos & 0xF0) == 0xE0) {
                // 3字节字符
                numBytes = 2;
            } else if ((*pos & 0xF8) == 0xF0) {
                // 4字节字符
                numBytes = 3;
            }

            if(!*(pos + numBytes + 1 )) break;
            for (int i = 0; i < numBytes+1; ++i) {
                words.push_back(*(pos++));
            }
            
        }else{
            if(!*(pos + 1)) break;
            words.push_back(*(pos++)); 
        }
    }
    LOGV(" words = (%s) To (%s)",buffer,words.c_str());
    return words;
}

std::string sysCommand(const std::string &cmd) {
    std::string result;
    char        buffer[128]; // 存储输出数据的缓冲区

    // 执行系统命令，并读取输出数据
    FILE *fp = popen(cmd.c_str(), "r");
    if (fp == NULL) {
        perror("Failed to execute the command.\n");
        return "";
    }

    // 逐行读取输出数据并打印
    bzero(buffer, sizeof(buffer));
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        result.append(buffer);
        bzero(buffer, sizeof(buffer));
    }

    // 关闭文件指针
    pclose(fp);

    return result;
}

std::string getHostIp(const std::string &host) {
    std::string     ip_addr;
    const char *    hostname = host.c_str(); // 要获取IP地址的主机名
    struct addrinfo hints, *result;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;   // 支持IPv4和IPv6
    hints.ai_socktype = SOCK_STREAM; // 使用TCP协议

    // 解析主机名
    int status = getaddrinfo(hostname, NULL, &hints, &result);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return "";
    }

    // 遍历结果列表并打印IP地址
    struct addrinfo *p = result;
    while (p != NULL) {
        void *addr;
        if (p->ai_family == AF_INET) { // IPv4地址
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr                     = &(ipv4->sin_addr);
        } else { // IPv6地址
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr                      = &(ipv6->sin6_addr);
        }

        char ip[INET6_ADDRSTRLEN];
        inet_ntop(p->ai_family, addr, ip, sizeof(ip));
        ip_addr = ip;
        break;

        p = p->ai_next;
    }

    // 释放内存
    freeaddrinfo(result);

    return ip_addr;
}

char *stristr(const char *str1, const char *str2) {
    char *cp = (char *)str1;
    char *s1, *s2;

    if (!*str2) return ((char *)str1);

    while (*cp) {
        s1 = cp;
        s2 = (char *)str2;

        while (*s1 && *s2 && toupper(*s1) == toupper(*s2)) { s1++, s2++; }

        if (!*s2) return (cp);

        cp++;
    }

    return (NULL);
}

/* 编写函数：
 * string& replace_all (string& src, const string& old_value, const string& new_value); 
 * 参数：源字符串src    被替换的子串old_value    替换的子串new_value
 *
 * 功能：将 源串src 中 子串old_value 全部被替换为 new_value
 */ 
std::string& replace_all(std::string& src, const std::string& old_value, const std::string& new_value) {
    // 每次重新定位起始位置，防止上轮替换后的字符串形成新的old_value
    for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length()) {
        if ((pos = src.find(old_value, pos)) != std::string::npos) {
			src.replace(pos, old_value.length(), new_value);
		}
		else break;
	}
	return src;
}

// 强信号：Quality > 75% 且 Signal level > -50 dBm
// 中等信号：Quality > 50% 且 Signal level > -60 dBm
// 弱信号：Quality > 25% 且 Signal level > -70 dBm
// 非常弱信号：Quality < 25% 或 Signal level < -70 dBm

// 计算 wifi 信号的大小
int calculation_signal(float Quality,int SignalLevel){
    if(Quality > 0.45 && SignalLevel > -60)
        return 4;
    else if(Quality > 0.35 && SignalLevel > -75)
        return 3;
    else if(Quality > 0.25 && SignalLevel > -90)
        return 2;
    else if(Quality < 0.15 || SignalLevel < -90)
        return 1;
    else 
        return 1;
}

void playSound(int type){ 
    if(g_appData.isMute) return;
    static int64_t lastBeep_tick = 0;
    int64_t now_tick = SystemClock::uptimeMillis();

    if (now_tick - lastBeep_tick > 150) {
        g_objConnMgr->setBuzzer_SetBit(1,1,1);
        lastBeep_tick = now_tick;
        LOGE("playSound");
    } else {
        LOG(WARN) << "limit beep time. diff=" << (now_tick - lastBeep_tick);
    }
}

void scaleDrawBackground(View *fromView,View *toView,int scale){
    Cairo::RefPtr<Cairo::ImageSurface> parentBitmap = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,fromView->getWidth(), fromView->getHeight());
    Canvas *mcanvas = new Canvas(parentBitmap);
    fromView->draw(*mcanvas);

    Cairo::RefPtr<Cairo::ImageSurface> bitmap = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32,fromView->getWidth()/scale, fromView->getHeight()/scale);
    Cairo::RefPtr<Cairo::Context> canvas = Cairo::Context::create(bitmap);
    // canvas->set_operator(Cairo::Context::Operator::SOURCE);
    canvas->scale(1.0f/scale,1.0f/scale);
    canvas->set_source(mcanvas->get_target(),0,0);
    canvas->get_source_for_surface()->set_filter(Cairo::SurfacePattern::Filter::GOOD);
    canvas->paint();

    Drawable *drawable = new BitmapDrawable(bitmap);
    
    toView->setBackground(drawable);
    delete mcanvas;
}


// 将mcu的版本号转换成字符串(V1.1.1.1)的形式
std::string getMCUVersion(std::string version){
    std::string sVersion;
    size_t pos =  version.find("_V");
    if(pos != std::string::npos){
        sVersion = version.substr(pos+2,1);
        sVersion = version.substr(pos+2,1) + "." + version.substr(pos+3,1) + "." + version.substr(pos+4,1);
    }
    
    LOGD("sVersion = %s",sVersion.c_str());
    return sVersion;
}

void InjectKey(int type, int code, int value) {
    INPUTEVENT i = { 0 };
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    i.tv_sec = ts.tv_sec;
    i.tv_usec = ts.tv_nsec / 1000;
    i.type = type;
    i.code = code;
    i.value = value;
    i.device = INJECTDEV_KEY;
    InputInjectEvents(&i, 1, 1);
}

// 拆分版本号字符串为数字部分
std::vector<int> splitVersion(const std::string& version) {
    std::vector<int> result;
    std::istringstream iss(version);
    std::string part;
    while (std::getline(iss, part, '.')) {
        result.push_back(std::stoi(part));
    }
    return result;
}

// 检查网络版本号 是否大于 当前的版本号
int checkVersion(const std::string &version,const std::string &localVersion){
    std::vector<int> otaVersion     = splitVersion(version);
    std::vector<int> localOtaVersion   = splitVersion(localVersion);

    // 比较每个部分
    for (size_t i = 0; i < std::min(otaVersion.size(), localOtaVersion.size()); ++i) {
        if (otaVersion[i] < localOtaVersion[i]) {
            return -1;
        } else if (otaVersion[i] > localOtaVersion[i]) {
            return 1; 
        }
    }

    // 版本号的前缀部分相同，比较长度确定大小关系
    if (otaVersion.size() < localOtaVersion.size()) {
        return -1;
    } else if (otaVersion.size() > localOtaVersion.size()) {
        return 1;
    }

    return 0;
}

// 获取文件大小的函数
long getFileSize(const char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        return -1; // 文件打开失败
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);

    return size;
}

// log写入到文本中 
void logToFile(const char* message) {
   const char* filename = "/appconfigs/logfile.txt";
    long fileSize = getFileSize(filename);

    if (fileSize >= 0 && fileSize <= 200000) { // 文件大小不超过200k
        FILE* fp = fopen(filename, "a");
        
        if (fp == NULL) {
            printf("Error opening file!\n");
            return;
        }

        fprintf(fp, "[%s]:%s\n",getDateTime().c_str(), message);

        fclose(fp);
    } else {
        printf("File size exceeds 200k limit. Writing in overwrite mode...\n");
        
        FILE* fp = fopen(filename, "w"); // 以覆盖方式打开文件
        
        if (fp == NULL) {
            printf("Error opening file!\n");
            return;
        }

        fprintf(fp, "[%s]:%s\n",getDateTime().c_str(), message); // 写入新的log消息

        fclose(fp);
    }
}

bool isWhitespace(const std::string& str) {
    for (char c : str) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            return false; // 如果不是空白字符，则返回 false
        }
    }
    
    // 检查是否是纯空格字符串
    if (str.find_first_not_of(' ') == std::string::npos) {
        return true; // 如果是纯空格字符串，则返回 true
    }

    return true; // 其他情况下也返回 true
}

std::string extractJson(const std::string& text) {
    // size_t start = text.find('{');
    // size_t start2 = text.find('[');
    // if((start == std::string::npos) && (start2 == std::string::npos)){
    //     return "";
    // }
    // start = std::min(start,start2);

    // size_t end = text.find_last_of('}');
    // size_t end2 = text.find_last_of(']');
    // if((end == std::string::npos) && (end2 == std::string::npos)){
    //     return "";
    // }
    // end = std::min(end,end2);
    // if (end > start) {
    //     return text.substr(start, end - start + 1);
    // }
    // return "";


    // 查找 ```json 开始标记
    std::string start_marker = "```json";
    std::string end_marker = "```";
    
    size_t start_pos = text.find(start_marker);
    if (start_pos == std::string::npos) {
        // 如果没有找到 ```json，尝试找普通的 ```
        start_marker = "```";
        start_pos = text.find(start_marker);
        if (start_pos == std::string::npos) {
            return "";
        }
        start_pos += start_marker.length();
    } else {
        start_pos += start_marker.length();
    }
    
    // 查找结束标记 ```
    size_t end_pos = text.find(end_marker, start_pos);
    if (end_pos == std::string::npos) {
        // 如果没有结束标记，尝试提取到字符串末尾
        end_pos = text.length();
    }
    
    // 提取 JSON 内容
    std::string json_content = text.substr(start_pos, end_pos - start_pos);
    
    // 清理内容（去除前后空白和换行）
    size_t content_start = json_content.find_first_not_of(" \n\r\t");
    size_t content_end = json_content.find_last_not_of(" \n\r\t");
    
    if (content_start != std::string::npos && content_end != std::string::npos) {
        return json_content.substr(content_start, content_end - content_start + 1);
    }
    
    return json_content;

}
