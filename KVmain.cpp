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
    string name;
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
template <typename DataType>
int Locate(DataType list[], int count, const string& name) {
    for (int i = 0;i < count;i++) {
        if (list[i].name == name) {
            return i;
        }
    }
    return -1;
}

//在下标 i 处插入。count 必须传引用，因为要就地加一
template <typename DataType>
void Insert(DataType list[], int& count, int i, DataType item, int maxSize) {
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

//删除下标 index 处的元素并把它返回
template <typename DataType>
DataType Delete(DataType list[], int& count, int index) {
    DataType temp;
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
PasswordItem getPasswordData(int index);
int Empty();
void PrintLine();
void searchPassword();

/* =========================================================
 * 用户管理模块
 * ========================================================= */
//初始化管理员用户
void initAdmin() {
    users[0].name = "admin";
    users[0].key = "123";
    users[0].rule = RULE_ADMIN;
    users[0].uid = uidCount();
    userCount = 1;
}

int uidCount() {
    int num = userCount++;
    int UID = (num + 234565) % 1000 + 9000;
    return UID;
}

//登录
int login(int& outRule) {
    userUid = 000;
    string inputName, inputKey;
    system("cls");
    cout << "============== User Login ==============" << endl;

    cout << "Enter user name: ";cin >> inputName;
    cout << "Enter user key: ";cin >> inputKey;

    int i = Locate(users, userCount, inputName);   //先用通用模板按用户名定位
    if (i != -1 && users[i].key == inputKey) {
        cout << "\nLogin Successful Hello," << users[i].name << "\n" << endl;
        outRule = users[i].rule;
        userUid = users[i].uid;
        system("pause");
        return 1;
    }
    cout << "\nUser Not Found" << endl;
    system("pause");
    return -1;
}

//注册
void signUp() {
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

    if (Locate(users, userCount, name) != -1) {
        cout << "该用户名已存在，注册失败！" << endl;
        system("pause");
        return;
    }

    cout << "Enter new user key: ";
    cin >> key;

    users[userCount].name = name;
    users[userCount].key = key;
    users[userCount].rule = RULE_USER;
    users[userCount].uid = uidCount();
    userCount++;

    cout << "Registration succeeded!" << endl;
    //cout<<users[1].uid<<endl;
    system("pause");
}

/* =========================================================
 * 密码数据操作模块（表长、查询、打印）
 * 增删查已经收进上面的通用模板 Locate / Insert / Delete，
 * 这里只留密码表自己用得到的那几个和界面相关的函数。
 * ========================================================= */
int Length(){
    return pasCount;
}

PasswordItem getPasswordData(int index) {
    return Slist[index];
}

int Empty() {
    return pasCount == 0 ? 1 : 0;
}

void PrintLine() {
    cout << "==================================================================" << endl;

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
void adminMenu() {}

void userMenu() {}

int main()
{

    initAdmin();


    int choice;
    while (true)
    {
        system("cls");
        //cout<<users[0].uid<<endl;
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
                    else
                        userMenu();
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
