// Windows API 和标准库头文件
#include <windows.h>   // Windows API
#include <string>      // 字符串操作
#include <vector>      // 动态数组
#include <deque>       // 双端队列
#include <chrono>      // 时间库
#include <random>      // 随机数生成
#include <algorithm>   // 算法函数

// 二维坐标结构体
struct Vec2 { int x; int y; };

// 颜色属性结构体
struct Color { WORD attr; };

// 渲染器类 - 负责控制台绘图
struct Renderer {
    HANDLE hOut;                     // 控制台输出句柄
    int w;                           // 屏幕宽度
    int h;                           // 屏幕高度
    std::vector<CHAR_INFO> back;     // 后台缓冲区
    SMALL_RECT rect;                 // 控制台区域
    
    // 构造函数 - 初始化控制台
    Renderer(int width, int height) : hOut(GetStdHandle(STD_OUTPUT_HANDLE)), w(width), h(height), back(width*height) {
        rect = {0,0,(SHORT)(w-1),(SHORT)(h-1)};  // 设置控制台区域
        
        // 隐藏光标
        CONSOLE_CURSOR_INFO ci{1,FALSE};
        SetConsoleCursorInfo(hOut,&ci);
        
        // 设置控制台缓冲区大小
        COORD size = {(SHORT)w,(SHORT)h};
        SetConsoleScreenBufferSize(hOut,size);
        
        // 设置控制台窗口大小
        SetConsoleWindowInfo(hOut,TRUE,&rect);
    }
    
    // 清屏函数
    void clear(wchar_t ch, WORD attr){
        for(int i=0;i<w*h;i++){ 
            back[i].Char.UnicodeChar = ch; 
            back[i].Attributes = attr; 
        }
    }
    
    // 绘制文本
    void drawText(int x,int y,const std::wstring& s, WORD attr){
        int idx = y*w + x;
        for(size_t i=0;i<s.size();++i){ 
            if(x+(int)i>=0 && x+(int)i<w && y>=0 && y<h){ 
                back[idx+i].Char.UnicodeChar = s[i]; 
                back[idx+i].Attributes = attr; 
            } 
        }
        // 清空该行剩余部分
        if(y>=0 && y<h){ 
            for(int i=x+(int)s.size(); i<w; ++i){ 
                int p=y*w+i; 
                back[p].Char.UnicodeChar = L' '; 
                back[p].Attributes = attr; 
            } 
        }
    }
    
    // 绘制矩形框
    void drawBox(int x,int y,int bw,int bh, WORD attr){
        for(int yy=0; yy<bh; ++yy){ 
            for(int xx=0; xx<bw; ++xx){ 
                int p=(y+yy)*w+(x+xx); 
                if(p>=0 && p<w*h){ 
                    back[p].Char.UnicodeChar=L' '; 
                    back[p].Attributes=attr; 
                } 
            } 
        }
    }
    
    // 绘制进度条
    void drawBar(int x,int y,int bw,double ratio, WORD fillAttr, WORD emptyAttr){
        int fill = (int)std::clamp((int)(ratio*bw),0,bw);  // 计算填充长度
        for(int i=0;i<bw;i++){ 
            int p=y*w+(x+i); 
            back[p].Char.UnicodeChar=L' '; 
            back[p].Attributes = i<fill?fillAttr:emptyAttr;  // 根据位置选择颜色
        }
    }
    
    // 将后台缓冲区内容输出到控制台
    void present(){ 
        WriteConsoleOutputW(hOut, back.data(), {(SHORT)w,(SHORT)h}, {0,0}, &rect); 
    }
};

// 食材枚举
enum class Ingredient { Meat, Cucumber, Sauce, Fries, Ketchup };
// 小吃枚举
enum class Snack { Fries, Cola };
// 升级项目枚举
enum class Upgrade { AutoMeat, GoldPlate, ExpandStore };

// 库存结构体
struct Inventory {
    int bread=5;        // 面饼
    int meat=5;         // 肉
    int sauce=10;       // 沙司
    int cucumber=10;    // 黄瓜
    int ketchup=10;     // 番茄酱
    int potato=10;      // 土豆
    int fries=10;       // 薯条
    int cola=10;        // 可乐
    int wrapPaper=10;   // 包装纸
    int fryBox=10;      // 薯条盒
    int colaCup=10;     // 可乐杯
    int breadMax=5;     // 面饼最大容量
    int itemMax=20;     // 物品最大容量
};

// 沙威玛状态枚举
enum class ShawarmaState { Empty, Open, Wrapped, Grilling, Done };

// 沙威玛结构体
struct Shawarma {
    ShawarmaState state=ShawarmaState::Empty;  // 当前状态
    bool hasMeat=false;      // 是否有肉
    bool hasCucumber=false;  // 是否有黄瓜
    bool hasFries=false;     // 是否有薯条
    bool hasKetchup=false;   // 是否有番茄酱
    bool hasSauce=true;      // 是否有沙司
    int grillTime=0;         // 已烤时间
    int grillNeed=0;         // 需要烤的时间
};

// 订单项结构体
struct OrderItem { 
    bool shawarma=false;  // 是否要沙威玛
    bool fries=false;     // 是否要薯条
    bool cola=false;      // 是否要可乐
    bool noSauce=false;   // 是否不要沙司
};

// 顾客结构体
struct Customer {
    OrderItem want;       // 顾客需求
    int patienceMax=100;  // 最大耐心值
    int patience=100;     // 当前耐心值
    bool served=false;    // 是否已服务
};

// 游戏状态结构体
struct GameState {
    int day=0;           // 天数
    int coins=0;         // 金币
    int capacity=3;      // 顾客容量
    bool upAutoMeat=false;   // 自动切肉升级
    bool upGoldPlate=false;  // 金盘子升级
    bool upExpand=false;     // 扩展店面升级
};

// 随机数生成器类
struct RNG { 
    std::mt19937 rng; 
    RNG():rng((unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count()){} 
    int next(int a,int b){ 
        std::uniform_int_distribution<int> d(a,b); 
        return d(rng); 
    } 
    bool chance(int p){ 
        return next(1,100)<=p; 
    } 
};

// 输入处理类
struct Input {
    HANDLE hIn;
    Input(){ 
        hIn=GetStdHandle(STD_INPUT_HANDLE); 
        DWORD mode=0; 
        GetConsoleMode(hIn,&mode); 
        mode &= ~(ENABLE_ECHO_INPUT|ENABLE_LINE_INPUT);  // 禁用回显和行输入
        SetConsoleMode(hIn,mode); 
    }
    
    // 轮询按键输入
    bool pollKey(wchar_t& ch){ 
        INPUT_RECORD rec; 
        DWORD n=0; 
        if(PeekConsoleInputW(hIn,&rec,1,&n) && n){ 
            ReadConsoleInputW(hIn,&rec,1,&n); 
            if(rec.EventType==KEY_EVENT && rec.Event.KeyEvent.bKeyDown){ 
                ch = rec.Event.KeyEvent.uChar.UnicodeChar; 
                return true; 
            } 
        } 
        return false; 
    }
};

// 入口场景类
struct SceneEntrance {
    GameState& gs;  // 游戏状态引用
    Renderer& r;    // 渲染器引用
    Input& in;      // 输入引用
    
    bool wantStart=false;    // 是否想开始新游戏
    bool wantUpgrade=false;  // 是否想升级
    
    SceneEntrance(GameState& g, Renderer& rr, Input& ii):gs(g),r(rr),in(ii){}
    
    // 绘制入口界面
    void draw(){ 
        r.clear(L' ', FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,2,L"入口界面", FOREGROUND_GREEN|FOREGROUND_INTENSITY); 
        r.drawText(2,4,L"已玩局数: "+std::to_wstring(gs.day), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,5,L"累计金币: "+std::to_wstring(gs.coins), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,7,L"N 开启新的一天", FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,8,L"U 店铺升级", FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,10,L"Q 退出", FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.present(); 
    }
    
    // 入口界面主循环
    void loop(){ 
        wchar_t ch; 
        while(true){ 
            draw(); 
            if(in.pollKey(ch)){ 
                if(ch==L'N'||ch==L'n'){ 
                    wantStart=true; 
                    break; 
                } 
                if(ch==L'U'||ch==L'u'){ 
                    wantUpgrade=true; 
                    upgradeMenu(); 
                } 
                if(ch==L'Q'||ch==L'q'){ 
                    ExitProcess(0);  // 退出程序
                } 
            } 
            Sleep(1000/24);  // 约24帧/秒
        }
    }
    
    // 升级菜单
    void upgradeMenu(){ 
        while(true){ 
            r.clear(L' ', FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
            r.drawText(2,2,L"店铺升级", FOREGROUND_GREEN|FOREGROUND_INTENSITY); 
            r.drawText(2,4,L"A 自动切肉机 价格: 50 "+std::wstring(gs.upAutoMeat?L"[已购]":L""), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
            r.drawText(2,5,L"G 金盘子(饼价值+20%) 价格: 50 "+std::wstring(gs.upGoldPlate?L"[已购]":L""), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
            r.drawText(2,6,L"E 扩充店面(容量+3) 价格: 50 "+std::wstring(gs.upExpand?L"[已购]":L""), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
            r.drawText(2,8,L"B 返回", FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
            r.drawText(2,10,L"当前金币: "+std::to_wstring(gs.coins), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
            r.present(); 
            
            wchar_t ch; 
            if(Input().pollKey(ch)){ 
                if(ch==L'B'||ch==L'b') break;  // 返回
                
                // 购买自动切肉机
                if(ch==L'A'||ch==L'a'){ 
                    if(!gs.upAutoMeat && gs.coins>=50){ 
                        gs.coins-=50; 
                        gs.upAutoMeat=true; 
                    } 
                } 
                // 购买金盘子
                if(ch==L'G'||ch==L'g'){ 
                    if(!gs.upGoldPlate && gs.coins>=50){ 
                        gs.coins-=50; 
                        gs.upGoldPlate=true; 
                    } 
                } 
                // 购买店面扩展
                if(ch==L'E'||ch==L'e'){ 
                    if(!gs.upExpand && gs.coins>=50){ 
                        gs.coins-=50; 
                        gs.upExpand=true; 
                        gs.capacity+=3; 
                    } 
                } 
            } 
            Sleep(1000/24); 
        }
    }
};

// 主游戏场景类
struct SceneMain {
    GameState& gs;   // 游戏状态引用
    Renderer& r;     // 渲染器引用
    Input& in;       // 输入引用
    Inventory inv;   // 库存
    RNG rng;         // 随机数生成器
    
    Shawarma open;                       // 正在制作的面饼
    std::deque<Shawarma> packaged;       // 包装好的沙威玛队列
    std::deque<Shawarma> grilling;       // 正在烤制的沙威玛队列
    std::deque<Customer> customers;      // 顾客队列
    
    int dayTimeMax=120;   // 每天最大时间
    int dayTime=120;      // 当前剩余时间
    int secCounter=0;     // 秒计数器
    std::wstring msg;     // 消息文本
    
    // 准备食物状态结构体
    struct Prep { 
        bool taken=false;  // 是否已拿容器
        bool ready=false;  // 是否已准备好
    };
    Prep friesPrep;  // 薯条准备状态
    Prep colaPrep;   // 可乐准备状态
    
    SceneMain(GameState& g, Renderer& rr, Input& ii):gs(g),r(rr),in(ii){ 
        packaged.resize(3);  // 初始化包装槽
        grilling.resize(3);  // 初始化烤盘槽
    }
    
    // 计算沙威玛价格
    int priceShawarma(const Shawarma& s){ 
        int base=20;  // 基础价格
        if(s.hasCucumber) base+=3; 
        if(s.hasKetchup) base+=2; 
        if(s.hasFries) base+=8; 
        if(s.hasMeat) base+=10; 
        if(gs.upGoldPlate) base = base + base*20/100;  // 金盘子加成
        return base; 
    }
    
    // 薯条价格
    int priceFries(){ return 8; }
    // 可乐价格
    int priceCola(){ return 6; }
    
    // 生成顾客
    void spawnCustomer(){ 
        if((int)customers.size()>=gs.capacity) return; 
        
        Customer c; 
        int t=rng.next(0,3); 
        if(t==0){ 
            c.want.shawarma=true; 
            c.want.noSauce=rng.chance(30);  // 30%概率不要沙司
        } else if(t==1){ 
            c.want.shawarma=true; 
            c.want.fries=true; 
        } else { 
            c.want.shawarma=true; 
            c.want.cola=true; 
        } 
        c.patienceMax=rng.next(80,140); 
        c.patience=c.patienceMax; 
        customers.push_back(c); 
    }
    
    // 绘制顶部信息
    void drawTop(){ 
        r.drawText(2,1,L"主界面", FOREGROUND_GREEN|FOREGROUND_INTENSITY); 
        r.drawText(20,1,L"时间: "+std::to_wstring(dayTime), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(35,1,L"金币: "+std::to_wstring(gs.coins), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(55,1,L"容量: "+std::to_wstring(gs.capacity), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
    }
    
    // 绘制库存信息
    void drawInventory(){ 
        r.drawText(2,3,L"库存", FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,4,L"面饼: "+std::to_wstring(inv.bread)+L"/"+std::to_wstring(inv.breadMax), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,5,L"肉: "+std::to_wstring(inv.meat), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,6,L"黄瓜: "+std::to_wstring(inv.cucumber), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,7,L"沙司: "+std::to_wstring(inv.sauce), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,8,L"番茄酱: "+std::to_wstring(inv.ketchup), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,9,L"土豆: "+std::to_wstring(inv.potato), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,10,L"薯条: "+std::to_wstring(inv.fries), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,11,L"可乐: "+std::to_wstring(inv.cola), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,12,L"包装纸: "+std::to_wstring(inv.wrapPaper), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,13,L"薯条盒: "+std::to_wstring(inv.fryBox), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,14,L"可乐杯: "+std::to_wstring(inv.colaCup), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        
        // 显示准备状态
        std::wstring fs = friesPrep.ready?L"已完成":(friesPrep.taken?L"已拿":L"空"); 
        std::wstring cs = colaPrep.ready?L"已完成":(colaPrep.taken?L"已拿":L"空"); 
        r.drawText(2,15,L"薯条准备: "+fs, FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(2,16,L"可乐准备: "+cs, FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
    }
    
    // 获取沙威玛描述
    std::wstring shawarmaDesc(const Shawarma& s){ 
        std::wstring t=L""; 
        if(s.hasMeat) t+=L"肉 "; 
        if(s.hasCucumber) t+=L"黄瓜 "; 
        if(s.hasFries) t+=L"薯条 "; 
        if(s.hasKetchup) t+=L"番茄酱 "; 
        if(!s.hasSauce) t+=L"无沙司 "; 
        return t; 
    }
    
    // 绘制工作站状态
    void drawStations(){ 
        r.drawText(25,3,L"操作台", FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        r.drawText(25,4,L"摊开的面饼: "+(open.state==ShawarmaState::Open?shawarmaDesc(open):L"无"), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED);
        
        r.drawText(25,6,L"包装卷饼(3):", FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED);
        for(int i=0;i<3;i++){ 
            std::wstring line=L""; 
            if(packaged[i].state==ShawarmaState::Wrapped||packaged[i].state==ShawarmaState::Done){ 
                line=shawarmaDesc(packaged[i]); 
            } else { 
                line=L"空"; 
            } 
            r.drawText(25,7+i,L"槽"+std::to_wstring(i+1)+L": "+line, FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        }
        
        r.drawText(25,11,L"烤盘(3):", FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED);
        for(int i=0;i<3;i++){ 
            std::wstring line=L""; 
            if(grilling[i].state==ShawarmaState::Grilling){ 
                line=L"烤制中"; 
            } else if(grilling[i].state==ShawarmaState::Done){ 
                line=L"完成"; 
            } else { 
                line=L"空"; 
            } 
            r.drawText(25,12+i,L"位"+std::to_wstring(i+1)+L": "+line, FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        }
    }
    
    // 绘制顾客队列
    void drawCustomers(){ 
        r.drawText(2,18,L"顾客队列", FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
        for(int i=0;i<(int)customers.size();i++){ 
            auto& c=customers[i]; 
            std::wstring want=L""; 
            if(c.want.shawarma){ 
                want+=L"饼"; 
                if(c.want.noSauce) want+=L"(无沙司)"; 
            } 
            if(c.want.fries){ want+=L"+薯条"; } 
            if(c.want.cola){ want+=L"+可乐"; } 
            r.drawText(2,19+i,L"顾客"+std::to_wstring(i+1)+L": "+want, FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
            
            // 绘制耐心条
            r.drawBar(35,19+i,20,(double)c.patience/c.patienceMax, FOREGROUND_GREEN|FOREGROUND_INTENSITY, FOREGROUND_RED); 
        }
        
        r.drawText(2,19+(int)customers.size(), L"容量: "+std::to_wstring(gs.capacity)+L" 已在店内: "+std::to_wstring((int)customers.size()), FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED);
    }
    
    // 绘制操作帮助
    void drawHelp(){ 
        r.drawText(2,r.h-1,L"操作: B放饼 I添加食材 R卷饼 G上烤盘 T取烤 S上菜 F拿薯条 C拿可乐杯 P补货 M切肉 D切土豆 J炸薯条 Q结束", FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
    }
    
    // 绘制消息
    void drawMsg(){ 
        r.drawText(2,22,L"消息: "+msg, FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
    }
    
    // 添加食材到面饼
    void addIngredient(Ingredient ing){ 
        if(open.state!=ShawarmaState::Open){ 
            msg=L"请先放置面饼"; 
            return; 
        } 
        
        if(ing==Ingredient::Meat){ 
            if(inv.meat<=0 && gs.upAutoMeat){ 
                inv.meat = inv.itemMax;  // 自动切肉
            } 
            if(inv.meat>0){ 
                open.hasMeat=true; 
                inv.meat--; 
                if(inv.meat==0 && gs.upAutoMeat){ 
                    inv.meat = inv.itemMax;  // 自动补充
                } 
            } else { 
                msg=L"肉不足"; 
            } 
        } else if(ing==Ingredient::Sauce){ 
            if(inv.sauce>0){ 
                open.hasSauce=true; 
                inv.sauce--; 
            } else { 
                msg=L"沙司不足"; 
            } 
        } else if(ing==Ingredient::Cucumber){ 
            if(inv.cucumber>0){ 
                open.hasCucumber=true; 
                inv.cucumber--; 
            } else { 
                msg=L"黄瓜不足"; 
            } 
        } else if(ing==Ingredient::Fries){ 
            if(inv.fries>0){ 
                open.hasFries=true; 
                inv.fries--; 
            } else { 
                msg=L"薯条库存不足"; 
            } 
        } else if(ing==Ingredient::Ketchup){ 
            if(inv.ketchup>0){ 
                open.hasKetchup=true; 
                inv.ketchup--; 
            } else { 
                msg=L"番茄酱不足"; 
            } 
        }
    }
    
    // 补充小吃库存
    void restockSnack(Snack s){ 
        if(s==Snack::Fries){ 
            msg=L"薯条需通过切土豆与炸制"; 
        } else { 
            inv.cola = std::min(inv.itemMax, inv.cola+5); 
            msg=L"补货可乐完成"; 
        } 
    }
    
    int supplyCycle=0;  // 补货循环索引
    
    // 循环补货不同物品
    void restockCycle(){ 
        const wchar_t* names[] = {L"面饼",L"黄瓜",L"沙司",L"番茄酱",L"可乐",L"包装纸",L"薯条盒",L"可乐杯"}; 
        int idx = supplyCycle; 
        supplyCycle = (supplyCycle+1)%8; 
        
        if(idx==0){ 
            inv.bread = inv.breadMax; 
            msg=L"补货面饼完成"; 
        } else if(idx==1){ 
            inv.cucumber = std::min(inv.itemMax, inv.cucumber+5); 
            msg=L"补货黄瓜完成"; 
        } else if(idx==2){ 
            inv.sauce = std::min(inv.itemMax, inv.sauce+5); 
            msg=L"补货沙司完成"; 
        } else if(idx==3){ 
            inv.ketchup = std::min(inv.itemMax, inv.ketchup+5); 
            msg=L"补货番茄酱完成"; 
        } else if(idx==4){ 
            inv.cola = std::min(inv.itemMax, inv.cola+5); 
            msg=L"补货可乐完成"; 
        } else if(idx==5){ 
            inv.wrapPaper = std::min(inv.itemMax, inv.wrapPaper+5); 
            msg=L"补货包装纸完成"; 
        } else if(idx==6){ 
            inv.fryBox = std::min(inv.itemMax, inv.fryBox+5); 
            msg=L"补货薯条盒完成"; 
        } else if(idx==7){ 
            inv.colaCup = std::min(inv.itemMax, inv.colaCup+5); 
            msg=L"补货可乐杯完成"; 
        } 
    }
    
    // 切土豆
    void cutPotato(){ 
        inv.potato = std::min(inv.itemMax, inv.potato+5); 
        msg=L"已切土豆"; 
    }
    
    // 炸薯条
    void fryFriesFromPotato(){ 
        if(inv.potato>0){ 
            inv.potato--; 
            inv.fries = std::min(inv.itemMax, inv.fries+1); 
            msg=L"已炸薯条"; 
        } else { 
            msg=L"土豆不足"; 
        } 
    }
    
    // 拿薯条盒
    void takeFries(){ 
        if(!friesPrep.taken && !friesPrep.ready){ 
            if(inv.fryBox>0){ 
                inv.fryBox--; 
                friesPrep.taken=true; 
                msg=L"已拿薯条盒"; 
            } else { 
                msg=L"薯条盒不足"; 
            } 
        } else { 
            msg=L"薯条准备中或已完成"; 
        } 
    }
    
    // 向薯条盒添加薯条
    void addFriesIngredient(){ 
        if(!friesPrep.taken || friesPrep.ready){ 
            msg=L"请先拿薯条"; 
            return; 
        } 
        if(inv.fries>0){ 
            inv.fries--; 
            friesPrep.ready=true; 
            msg=L"已添加薯条"; 
        } else { 
            msg=L"薯条库存不足"; 
        } 
    }
    
    // 拿可乐杯
    void takeColaCup(){ 
        if(!colaPrep.taken && !colaPrep.ready){ 
            if(inv.colaCup>0){ 
                inv.colaCup--; 
                colaPrep.taken=true; 
                msg=L"已拿可乐杯"; 
            } else { 
                msg=L"可乐杯不足"; 
            } 
        } else { 
            msg=L"可乐准备中或已完成"; 
        } 
    }
    
    // 向可乐杯添加可乐
    void addColaIngredient(){ 
        if(!colaPrep.taken || colaPrep.ready){ 
            msg=L"请先拿可乐杯"; 
            return; 
        } 
        if(inv.cola>0){ 
            inv.cola--; 
            colaPrep.ready=true; 
            msg=L"已添加可乐"; 
        } else { 
            msg=L"可乐库存不足"; 
        } 
    }
    
    // 放置面饼
    void placeBread(){ 
        if(open.state==ShawarmaState::Open){ 
            msg=L"已有面饼"; 
            return; 
        } 
        if(inv.bread<=0){ 
            msg=L"面饼不足"; 
            return; 
        } 
        inv.bread--; 
        open=Shawarma(); 
        open.state=ShawarmaState::Open; 
        open.hasSauce=false; 
        msg=L"已放置面饼"; 
    }
    
    // 卷起沙威玛
    void roll(){ 
        if(open.state!=ShawarmaState::Open){ 
            msg=L"无面饼"; 
            return; 
        } 
        bool ok = open.hasMeat;  // 必须有肉
        if(!ok){ 
            msg=L"至少需要肉"; 
            return; 
        } 
        if(inv.wrapPaper<=0){ 
            msg=L"包装纸不足"; 
            return; 
        } 
        inv.wrapPaper--; 
        for(int i=0;i<3;i++){ 
            if(packaged[i].state==ShawarmaState::Empty){ 
                packaged[i]=open; 
                packaged[i].state=ShawarmaState::Wrapped; 
                open=Shawarma(); 
                msg=L"已卷饼"; 
                return; 
            } 
        } 
        msg=L"包装槽已满"; 
    }
    
    // 将包装好的沙威玛放到烤盘
    void toGrill(){ 
        for(int i=0;i<3;i++){ 
            if(packaged[i].state==ShawarmaState::Wrapped){ 
                for(int j=0;j<3;j++){ 
                    if(grilling[j].state==ShawarmaState::Empty){ 
                        grilling[j]=packaged[i]; 
                        grilling[j].state=ShawarmaState::Grilling; 
                        grilling[j].grillNeed=10;  // 需要烤10秒
                        grilling[j].grillTime=0; 
                        packaged[i]=Shawarma(); 
                        msg=L"已上烤盘"; 
                        return; 
                    } 
                } 
            } 
        } 
        msg=L"无可烤或烤盘满"; 
    }
    
    // 从烤盘取下沙威玛
    void takeFromGrill(){ 
        for(int j=0;j<3;j++){ 
            if(grilling[j].state==ShawarmaState::Done){ 
                for(int i=0;i<3;i++){ 
                    if(packaged[i].state==ShawarmaState::Empty){ 
                        packaged[i]=grilling[j]; 
                        packaged[i].state=ShawarmaState::Done; 
                        grilling[j]=Shawarma(); 
                        msg=L"取下完成卷饼"; 
                        return; 
                    } 
                } 
            } 
        } 
        msg=L"暂无已烤好卷饼"; 
    }
    
    // 检查沙威玛是否符合顾客订单
    bool matchOrder(const Shawarma& s, const Customer& c){ 
        if(!c.want.shawarma) return false; 
        if(c.want.noSauce && s.hasSauce) return false; 
        return (s.state==ShawarmaState::Wrapped || s.state==ShawarmaState::Done); 
    }
    
    // 服务顾客
    void serve(){ 
        for(size_t ci=0; ci<customers.size(); ++ci){ 
            auto& c=customers[ci]; 
            if(c.served) continue; 
            
            int shawIdx=-1; 
            for(int i=0;i<3;i++){ 
                if(packaged[i].state!=ShawarmaState::Empty && matchOrder(packaged[i], c)){ 
                    shawIdx=i; 
                    break; 
                } 
            }
            
            if(shawIdx==-1) continue; 
            
            // 检查小吃是否准备好
            bool friesOk = !c.want.fries || friesPrep.ready; 
            bool colaOk = !c.want.cola || colaPrep.ready; 
            
            if(!friesOk){ 
                msg=L"薯条未完成"; 
                return; 
            } 
            if(!colaOk){ 
                msg=L"可乐未完成"; 
                return; 
            } 
            
            // 计算总价
            int gain = priceShawarma(packaged[shawIdx]); 
            if(c.want.fries){ 
                gain += priceFries(); 
                friesPrep = Prep();  // 重置薯条状态
            } 
            if(c.want.cola){ 
                gain += priceCola(); 
                colaPrep = Prep();   // 重置可乐状态
            } 
            
            // 完成交易
            gs.coins += gain; 
            packaged[shawIdx]=Shawarma();  // 清空包装槽
            c.served=true; 
            msg=L"交易成功 +"+std::to_wstring(gain); 
            return; 
        } 
        msg=L"暂无匹配顾客"; 
    }
    
    // 切肉
    void cutMeat(){ 
        if(gs.upAutoMeat){ 
            msg=L"自动切肉生效"; 
            return; 
        } 
        inv.meat = std::min(inv.itemMax, inv.meat+5); 
        msg=L"已切肉"; 
    }
    
    // 每秒更新
    void tickSecond(){ 
        // 更新烤制进度
        for(int j=0;j<3;j++){ 
            if(grilling[j].state==ShawarmaState::Grilling){ 
                grilling[j].grillTime++; 
                if(grilling[j].grillTime>=grilling[j].grillNeed){ 
                    grilling[j].state=ShawarmaState::Done; 
                } 
            } 
        }
        
        // 生成新顾客
        if((int)customers.size() < gs.capacity+3 && rng.chance(10)) spawnCustomer(); 
        
        // 更新顾客耐心
        for(auto& c: customers){ 
            if(!c.served){ 
                c.patience--; 
            } 
        }
        
        // 移除已服务或没耐心的顾客
        while(!customers.empty() && (customers.front().patience<=0 || customers.front().served)){ 
            if(!customers.front().served){ 
                // 顾客离开但没有购买
            } 
            customers.pop_front(); 
        }
        
        dayTime--;  // 减少剩余时间
    }
    
    // 主场景循环
    void loop(){ 
        wchar_t ch; 
        int ingCycle=0;  // 食材循环索引
        
        while(dayTime>0){ 
            r.clear(L' ', FOREGROUND_BLUE|FOREGROUND_GREEN|FOREGROUND_RED); 
            
            // 更新时间
            secCounter++; 
            if(secCounter>=24){ 
                tickSecond(); 
                secCounter=0; 
            } 
            
            // 绘制所有界面元素
            drawTop(); 
            drawInventory(); 
            drawStations(); 
            drawCustomers(); 
            drawHelp(); 
            drawMsg(); 
            r.present(); 
            
            // 处理按键输入
            if(in.pollKey(ch)){
                if(ch==L'B'||ch==L'b') placeBread();
                else if(ch==L'I'||ch==L'i'){ 
                    // 智能添加：如果正在准备薯条或可乐，则添加对应食材
                    if(friesPrep.taken && !friesPrep.ready){ 
                        addFriesIngredient(); 
                    } else if(colaPrep.taken && !colaPrep.ready){ 
                        addColaIngredient(); 
                    } else { 
                        // 否则循环添加食材到面饼
                        Ingredient arr[5]={Ingredient::Meat,Ingredient::Cucumber,Ingredient::Fries,Ingredient::Ketchup,Ingredient::Sauce}; 
                        addIngredient(arr[ingCycle]); 
                        ingCycle=(ingCycle+1)%5; 
                    } 
                }
                else if(ch==L'R'||ch==L'r') roll();
                else if(ch==L'G'||ch==L'g') toGrill();
                else if(ch==L'T'||ch==L't') takeFromGrill();
                else if(ch==L'S'||ch==L's') serve();
                else if(ch==L'F'||ch==L'f') takeFries();
                else if(ch==L'C'||ch==L'c') takeColaCup();
                else if(ch==L'P'||ch==L'p'){ restockCycle(); }
                else if(ch==L'M'||ch==L'm') cutMeat();
                else if(ch==L'D'||ch==L'd') cutPotato();
                else if(ch==L'J'||ch==L'j') fryFriesFromPotato();
                else if(ch==L'Q'||ch==L'q') break;  // 提前结束当天
            }
            Sleep(1000/24);  // 控制帧率
        }
    }
};

// 主函数
int wmain(){ 
    // 初始化渲染器、输入和游戏状态
    Renderer renderer(100,28);  // 100列28行
    Input input; 
    GameState gs; 
    
    // 游戏主循环
    while(true){ 
        // 进入入口场景
        SceneEntrance entr(gs,renderer,input); 
        entr.loop(); 
        
        // 如果选择开始新的一天
        if(entr.wantStart){ 
            gs.day++;  // 天数增加
            SceneMain mainScene(gs,renderer,input); 
            mainScene.loop();  // 运行主场景
        } 
    }
    return 0; 
}