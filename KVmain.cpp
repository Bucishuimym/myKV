/* =========================================================
 * 头文件与全局常量定义
 * ========================================================= */
#include <cstring>
#include <iostream>
#include <string>
#include <cstdlib>
#include <windows.h>
#include <fstream>
#include <sstream>
using namespace std;

const int Max_User = 100;
const int Max_Password = 1000;
const int RULE_ADMIN = 2;
const int RULE_USER = 1;

int ruletoken = -1;  //当前登录用户的权限，-1 表示未登录
int userUid = 000; //记录当前登录用户的UID，登出重置000

/* =========================================================
 * 数据结构定义（用户表 + 密码表）
 * ========================================================= */
//用户组
struct User
{
    string name;
    string key;
    int uid;
    int rule;
    
};

User users[Max_User];
int userCount = 0;  //用户数量

//数据结构体
struct PasswordItem {
    string Username;
    string key;
    string URL; 
    int UID;

};

PasswordItem Slist[Max_Password];    //顺序表
int pasCount = 0;  //表长度

/* =========================================================
 * 通用顺序表模板
 * ========================================================= */
//按 name 查找，返回下标，找不到返回 -1
/*
template <typename DataType>
int Locate(DataType list[], int count, const string& name) {
    if (count == 0)
        throw "Not Found";

    for (int i = 0;i < count;i++) {
        if (list[i].name == name) {
            return i;
        }
    }
    return -1;
}*/

//在下标 i 处插入。count 必须传引用，因为要就地加一
template <typename DataType>
void Insert(DataType list[], int& count, int i, DataType item, int maxSize) {  //顺序表插入，表长 +1
    if (count >= maxSize || i<0 || i>count) {
        throw "Insert failed, invalid position!";
        return;
    }
    for (int j = count;j > i;j--) {
        list[j] = list[j - 1];
    }
    list[i] = item;
    count++;
}

//取出一条记录的 UID：用户表里叫 uid，密码表里叫 UID，用重载把两个名字统一起来
int getUid(const User& u) { return u.uid; }                                    //取 User 的 uid（重载）
int getUid(const PasswordItem& p) { return p.UID; }                            //取 PasswordItem 的 UID（重载）

//删除一个元素并把它返回
//index > 9000 时把 index 当成 UID，先按 UID 定位再删；否则 index 就是下标，直接按位置删
template <typename DataType>
DataType Delete(DataType list[], int& count, int index) {                      //顺序表删除，返回被删元素
    DataType temp{};    //值初始化：删除失败时返回的是一条空记录（uid == 0），调用方靠它判断成败
    if (index > 9000) {
        int pos = -1;
        for (int i = 0;i < count;i++) {
            if (getUid(list[i]) == index) {
                pos = i;
                break;
            }
        }
        if (pos == -1) {
            cout << "Delete failed, UID " << index << " not found!" << endl;
            return temp;
        }
        index = pos;
    }

    if (index < 0 || index >= count) {
        cout << "Delete failed, invalid position!" << endl;
        return temp;
    }
    temp = list[index];

    for (int j = index;j < count - 1;j++) {
        list[j] = list[j + 1];
    }
    count--;
    return temp;
}

/* =========================================================
 * 函数声明（前向声明）
 * 把所有函数的名字先列在这里，防止互相调用时报错
 * （模板定义好就能直接用不用在这里声明）
 * ========================================================= */
void initAdmin();
int login(int& outRule);
void signUp();
void adminMenu();
void userMenu();
int uidCount();
string numToStr(int num);
int strToNum(string str);
void encryptDecrypt(char* str);

int Length();
static int LocatePasswordBySeq(const string& username);
static int LocateUserByUsers(const string& username);
static int LocateUserByUid(int uid);
static int LocatePasswordBySeq(const int& ID);
static int LocateMyDataBySeq(int seq);
int PasswordData(int index);
int Empty();
void PrintLineData(const PasswordItem& item, bool showUid = true);
void PrintLineUser(const User& u);
void ListData();
void searchPassword();

/* =========================================================
 * 用户管理模块
 * ========================================================= */
void initAdmin() {                                                             //初始化管理员账号
    users[0].name = "admin";
    users[0].key = "123";
    users[0].rule = RULE_ADMIN;
    users[0].uid = uidCount();
    if (!users[0].key.empty())
    {
        encryptDecrypt(&users[0].key[0]);
    }
    userCount = 1;
}

int uidCount() {                                                               //生成下一个用户 UID
    int usC = userCount;
    int num = usC++;
    int UID = (num + 234565) % 1000 + 9000;
    return UID;
}

int LocateUserByUsers(const string& username) {                                //按用户名在用户组查找
    for (int i = 0;i < userCount;i++) {
        if (users[i].name == username) {
            return i;
        }
    }
    return -1;
}

int LocateUserByUid(int uid) {                                                 //按 UID 在用户组查找，找不到返回 -1
    for (int i = 0;i < userCount;i++) {
        if (users[i].uid == uid) {
            return i;
        }
    }
    return -1;
}

int LocatePasswordBySeq(const int& ID) {                                       //按 UID 在密码表查找
        for (int i = 0;i < pasCount;i++) {
            if (Slist[i].UID == ID) {
                return i;
            }
        }
    return -1;
}

//把"我的第 seq 条"还原成 Slist 里的绝对下标，找不到返回 -1
//过滤条件和 ListData() 给用户编号时用的是同一套，保证"看到几号就能删几号"
int LocateMyDataBySeq(int seq) {                                               //我的第 seq 条 → 表里绝对下标
    int shown = 0;
    for (int i = 0;i < pasCount;i++) {
        if (Slist[i].UID != userUid)
            continue;               //不是我的数据，不占号
        if (++shown == seq)
            return i;
    }
    return -1;
}

int login(int& outRule) {                                                      //登录，成功写回权限令牌（待办：加密、报错提示？）
    userUid = 000;
    string inputName, inputKey;
    system("cls");
    cout << "============== User Login ==============" << endl;

    cout << "Enter user name: ";cin >> inputName;
    cout << "Enter user key: ";cin >> inputKey;

    string key = inputKey;      //输入的是明文，先加密再跟表里的密文比
    if (!key.empty())
        encryptDecrypt(&key[0]);

    int i = LocateUserByUsers(inputName);   //先用函数按用户名定位
    if (i != -1 && users[i].key == key) {
        cout << "\nLogin Successful Hello," << users[i].name << "\n" << endl;
        outRule = users[i].rule;
        userUid = users[i].uid; 
        PrintLineUser(users[i]);    //登录成功，把这行用户信息打出来
        system("pause");
        return 1;
    }
    cout << "\nUser Not Found" << endl;
    system("pause");
    return -1;
}

void signUp() {                                                                //注册新用户
    system("cls");
    if (userCount >= Max_User) {
        cout << "Sorry! user max" << endl;
        system("pause");
        return;
    }

    string name, key;
    cout << "============== Sign up ==============" << endl;
    cout << "Enter new user name: ";
    cin >> name;

    if (LocateUserByUsers(name) != -1) {
        cout << "该用户名已存在，注册失败！" << endl;
        system("pause");
        return;
    }

    cout << "Enter new user key: ";
    cin >> key;
    if (!key.empty())
    {
        encryptDecrypt(&key[0]);
    }
    users[userCount].name = name;
    users[userCount].key = key;
    users[userCount].rule = RULE_USER;
    users[userCount].uid = uidCount();
   
    userCount++;
    
    cout << "Registration succeeded!" << endl; 
    system("pause");
}

/* =========================================================
 * 密码数据操作模块（表长、查询、打印）
 * 增删查已经收进上面的通用模板 Locate / Insert / Delete，
 * 这里只留密码表自己用得到的那几个和界面相关的函数。
 * ========================================================= */
void encryptDecrypt(char* str) {                                               //对称加解密（异或 0x7F）
    const char key = 0x7F;
    for (int i = 0;str[i] != '\0';i++)
    {
        str[i] = str[i] ^ key;
    }

}
int Length(){                                                                  //返回密码表长度
    return pasCount;
}

PasswordItem getPasswordData(int index) {                                      //取密码表下标 index 的元素
    return Slist[index];
}

int Empty() {                                                                  //密码表是否为空
    return pasCount == 0 ? 1 : 0;
}

//打印一条密码数据：key 同时给出密文（表里存的）和明文（解密后的）
//showUid 默认 true；普通用户看自己的数据时传 false，不把这条数据属于谁暴露出去
void PrintLineData(const PasswordItem& item, bool showUid) {                   //打印一行密码数据
    string showkey = item.key;      //拷贝一份来解密，不动表里存的密文
    encryptDecrypt(&showkey[0]);
    cout << "|————" << item.Username
         << "  |——" << item.URL
         << "  |——key:" << item.key << "(" << showkey << ")";
    if (showUid)
        cout << "  |_UID:" << item.UID;
    cout << endl;
}

//打印一条用户数据：key 同样给出密文和明文
void PrintLineUser(const User& u) {                                            //打印一行用户数据
    string showkey = u.key;         //拷贝一份来解密，不动表里存的密文
    encryptDecrypt(&showkey[0]);
    cout << "|————" << u.name
         << "  |——key:" << u.key << "(" << showkey << ")"
         << "  |_UID:" << u.uid << endl;
}

/* =========================================================
 * 持久化与加密模块（文件读写）
 * =========================================================
 * saveToFile() / loadFromFile()
 * 目前数据只存在内存里，程序退出后用户表和密码表都会丢失。
 * 实现时只要在「函数声明」那段里补上这两个函数的声明即可。
 * ========================================================= */


/* =========================================================
 * 菜单与主函数
 * ========================================================= */
//列出数据，分两层：先看当前令牌，admin 直接列出全部用户的数据，
//user 则按全局变量 userUid 匹配，只列出跟自己相关的那几条
//user 看到的每行前面带一个从 1 开始的序号，删除时输的就是它（见 LocateMyDataBySeq）
void ListData() {                                                              //列出数据：admin 列全部，user 只列自己的
    bool seeAll = (ruletoken == RULE_ADMIN);
    int shown = 0;
    for (int i = 0;i < pasCount;i++) {
        if (!seeAll && Slist[i].UID != userUid)
            continue;                       //不是自己的数据直接跳过，连 UID 都不露出来
        if (!seeAll)
            cout << "[" << shown + 1 << "] ";   //这个号只在"我自己的数据"里连续，不是表里的绝对下标
        PrintLineData(Slist[i], seeAll);    //admin 带 UID，user 不带
        shown++;
    }
    if (shown == 0)
        cout << (seeAll ? "数据表是空的。" : "你还没有存储任何数据。") << endl;
    else
        cout << "共 " << shown << " 条。" << endl;
}

void adminMenu() {                                                             //管理员菜单
    int choice;
    while (true) {
        system("cls");
        cout << "+================= 管理员菜单 =================+" << endl;
        cout << "| [1].按UID查询密码数据                        |" << endl;
        cout << "| [2].按用户名查询用户                         |" << endl;
        cout << "| [3].插入密码数据                             |" << endl;
        cout << "| [4].插入用户数据                             |" << endl;
        cout << "| [5].按位置删除密码数据                       |" << endl;
        cout << "| [6].按位置/UID删除用户数据                   |" << endl;
        cout << "| [7].打印全部密码数据                         |" << endl;
        cout << "| [8].打印全部用户数据                         |" << endl;
        cout << "| [9].退出登录                                 |" << endl;
        cout << "+----------------------------------------------+" << endl;
        cout << "Enter your choice: ";
        cin >> choice;      //输入要放在循环里，不然 choice 一直是第一次的值，菜单会死循环
        if (!cin) {
            bool atEof = cin.eof();     //要在 clear() 之前判断，clear() 会把 eofbit 一起清掉
            cin.clear();
            cin.ignore(1000, '\n');
            if (atEof)
                return;                 //输入流到头了（管道/重定向），直接退出
            cout << "Invalid input, please enter a number!" << endl;
            system("pause");
            continue;
        }

        switch (choice) {
            case 1: {  //[1].LocateData
                int id;
                cin>>id;
                int pos = LocatePasswordBySeq(id);
                if (pos != -1) {
                    string showkey = Slist[pos].key;
                    encryptDecrypt(&showkey[0]);
                    cout << "|————" << Slist[pos].Username << " "
                         << "  |——" <<Slist[pos].URL<<" "
                         << "  |——" <<showkey<<" "
                         << "  |_" <<Slist[pos].UID<<endl;
                }
                break;
            }
            case 2: { //[2].LocateUserdata
                string name;
                cin>>name;
                int pos = LocateUserByUsers(name);
                if (pos != -1) {
                    string showkey = users[pos].key;
                    encryptDecrypt(&showkey[0]);
                    cout << "|————" << users[pos].name << " "
                         << "  |——" <<showkey<<" "
                         << "  |_"<<users[pos].uid<<endl;
                }
                break;
            }
            case 3: {  //[3].InsertData
                int n;
                cout<<"Input number of pos: ";
                cin>>n;
                PasswordItem item;
                cout << "Enter your data(name;URL;key): ";
                cin>>item.Username>>item.URL>>item.key;
                item.UID = userUid;
                encryptDecrypt(&item.key[0]);
                Insert(Slist, pasCount,n,item,Max_Password);
                break;
            }
            case 4: { //[4].InsertUserdata
                signUp();
                //Insert(users,userCount,n,user,Max_User);
                break;
            }
            case 5: {  //[5].DeleteData
                int index;
                cout<<"Input index: ";
                cin>>index;
                PasswordItem x = Delete(Slist, pasCount,index);
                string showkey = x.key;
                encryptDecrypt(&showkey[0]);
                cout<<"Username:"<<x.Username<<"/"
                    <<"URL:"<<x.URL<<"/"
                    <<"key:"<<x.key<<"("<<showkey<<")"<<"/"
                    <<"UID:"<<x.UID<<"Delete succeeded!\n";
                system("pause");
                break;
            }
            case 6: {  //[6].DeleteUserdata
                int index = 0;
                cout << "Input index : ";
                if (!(cin >> index)) {      //输入了非数字，别让 index 带着 0 往下走
                    cin.clear();
                    cin.ignore(1000, '\n');
                    cout << "Invalid input, please enter a number!" << endl;
                    system("pause");
                    break;
                }
                //先定位一下：管理员在 users[0]，不允许删除
                int pos = (index > 9000) ? LocateUserByUid(index) : index;
                if (pos == 0) {
                    cout << "管理员账号不允许删除！" << endl;
                    system("pause");
                    break;
                }

                //index > 9000 时 Delete 内部按 UID 删除，否则按位置删
                User deleted = Delete(users, userCount, index);
                if (deleted.uid == 0) {     //删除失败时返回的是值初始化的空记录
                    system("pause");
                    break;
                }

                cout << "\n用户删除成功，被删除的用户数据：" << endl;
                PrintLineUser(deleted); //

                //级联删除该 UID 在数据组里缓存的全部数据
                int removed = 0;
                for (int i = 0;i < pasCount; ) {
                    if (Slist[i].UID == deleted.uid) {
                        PasswordItem d = Delete(Slist, pasCount, i);
                        PrintLineData(d);
                        removed++;
                        //这里不能再写 i++：删掉后 i 位置已经换成了后面的元素，i 自增会跳过一个
                    }
                    else {
                        i++;
                    }
                }

                if (removed == 0)
                    cout << "该用户在数据组中没有缓存数据。" << endl;
                else
                    cout << "共删除 " << removed << " 条数据。" << endl;

                //删掉的正好是当前登录用户时，登录态一起重置
                if (userUid == deleted.uid) {
                    userUid = 000;
                    ruletoken = -1;
                }

                cout << "userCount = " << userCount << "  pasCount = " << pasCount << endl;
                system("pause");
                break;
            }
            case 7: {  //[7].打印全部密码数据
                ListData();     //admin 走 seeAll 那一层，列出所有用户的数据
                system("pause");
                break;
            }
            case 8: {  //[8].PrintUserdata 还没写
                cout << "Not implemented yet!" << endl;
                system("pause");
                break;
            }
            case 9: {  //[9].Logout
                userUid = 000;      //登出重置 000
                ruletoken = -1;
                return;
            }
            default: {
                cout << "Invalid choice, please try again!" << endl;
                system("pause");
                break;
            }
        }
    }
}

void userMenu() {                                                              //普通用户菜单
    if (ruletoken != RULE_USER) {   //兜底：这个菜单只给普通用户开
        cout << "无权限访问用户菜单！" << endl;
        system("pause");
        return;
    }

    int choice;
    while (true) {
        system("cls");
        cout << "+================== 用户菜单 ==================+" << endl;
        cout << "| [1].查看我的账号信息                         |" << endl;
        cout << "| [2].查看我的密码数据                         |" << endl;
        cout << "| [3].插入我的密码数据                         |" << endl;
        cout << "| [4].按位置删除我的数据                       |" << endl;
        cout << "| [5].退出登录                                 |" << endl;
        cout << "+----------------------------------------------+" << endl;
        cout << "Enter your choice: ";
        cin >> choice;
        if (!cin) {
            bool atEof = cin.eof();
            cin.clear();
            cin.ignore(1000, '\n');
            if (atEof)
                return;
            cout << "Invalid input, please enter a number!" << endl;
            system("pause");
            continue;
        }

        switch (choice) {
            case 1: {  //[1].查看我的账号信息
                int pos = LocateUserByUid(userUid);     //只认全局变量里的 UID，不让输别人
                if (pos == -1)
                    cout << "找不到当前登录用户！" << endl;
                else
                    PrintLineUser(users[pos]);
                system("pause");
                break;
            }
            case 2: {  //[2].查看我的密码数据
                ListData();     //user 走 UID 匹配那一层，只看得到自己的
                system("pause");
                break;
            }
            case 3: {  //[3].插入我的密码数据
                int n = 0;
                cout << "Input number of pos: ";
                if (!(cin >> n)) {
                    cin.clear();
                    cin.ignore(1000, '\n');
                    cout << "Invalid input, please enter a number!" << endl;
                    system("pause");
                    break;
                }
                if (n < 0 || n > pasCount) {    //先自己挡一道，别让 Insert 抛异常把程序带崩
                    cout << "插入位置不合法！" << endl;
                    system("pause");
                    break;
                }
                PasswordItem item;
                cout << "Enter your data(name;URL;key): ";
                cin >> item.Username >> item.URL >> item.key;
                item.UID = userUid;             //UID 按全局变量自动匹配，不让用户填
                encryptDecrypt(&item.key[0]);   //写入表之前先加密
                Insert(Slist, pasCount, n, item, Max_Password);
                cout << "插入成功！" << endl;
                system("pause");
                break;
            }
            case 4: {  //[4].按位置删除我的数据
                int seq = 0;
                cout << "Input index (列表里的序号): ";
                if (!(cin >> seq)) {
                    cin.clear();
                    cin.ignore(1000, '\n');
                    cout << "Invalid input, please enter a number!" << endl;
                    system("pause");
                    break;
                }
                //用户输的是"我的第几条"，不是表里的绝对下标，先还原成下标
                int index = LocateMyDataBySeq(seq);
                if (index == -1) {
                    cout << "没有这条数据！" << endl;
                    system("pause");
                    break;
                }
                if (Slist[index].UID != userUid) {  //兜底：还原出来的必须还是自己的
                    cout << "只能删除自己的数据！" << endl;
                    system("pause");
                    break;
                }
                PasswordItem x = Delete(Slist, pasCount, index);
                cout << "删除成功：" << endl;
                PrintLineData(x, false);        //不显示 UID
                system("pause");
                break;
            }
            case 5: {  //[5].退出登录
                userUid = 000;      //登出重置 000
                ruletoken = -1;
                return;
            }
            default: {
                cout << "Invalid choice, please try again!" << endl;
                system("pause");
                break;
            }
        }
    }
}

int main()                                                                     //主函数：主菜单循环
{
    initAdmin();

    int choice;
    while (true)
    {
        system("cls");
        cout << "==================================================================" << endl;
        cout << "+----- Main Menu -----+" << endl;
        cout << "|  1. Login           |" << endl;
        cout << "|  2. Sign Up         |" << endl;
        cout << "|  3. Logout          |" << endl;
        cout << "+---------------------+" << endl;
        cout << "Enter your choice: ";
        cin >> choice;
        switch (choice) {
        case 1:
        {
            int loginToken;
            while (true)
            {
                loginToken = login(ruletoken);
                if (loginToken == 1)
                {
                    if (ruletoken == RULE_ADMIN)
                        adminMenu();
                    else if (ruletoken == RULE_USER)
                        userMenu();
                    else
                        throw "[INFO] - error: Unknown rule token";
                    break;
                }
                else if (loginToken == -1)
                    break;
            }
            break;
        }
        case 2:
            signUp();
            break;
        case 3:
            cout << "Thank you again!" << endl;
            system("pause");
            return 0;
            break;
        default:
        {
            cout << "Invalid choice, please try again!" << endl;
            system("pause");
            break;
        }
        }
    }


    return 0;
}