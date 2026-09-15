#include <cstring>
#include <iostream>
#include <string>
#include <cstdlib>
#include <windows.h>
using namespace std;

const int Max_User = 100;
const int Max_Password = 1000;
const int RULE_ADMIN = 2;
const int RULE_USER = 1;

int ruletoken = -1;

//用户组
struct User
{
    string name;
    string key;
    int rule;
};
User users[Max_User];
int userCount = 0;  //用户数量

//数据结构体
struct PasswordItem {
    string name;
    string key;
    string address;
    string emalil;
};
PasswordItem students[Max_Password];    //顺序表
int stuCount = 0;  //表长度

//初始化管理员用户
void initAdmin(){
    users[0].name = "admin";
    users[0].key = "123";
    users[0].rule = RULE_ADMIN;
    userCount = 1;
}

//登录
int login(int &outRule) {
    string inputName,inputKey;
    system("cls");
    cout<<"============== User Login =============="<<endl;

    cout<<"Enter user name: ";cin>>inputName;
    cout<<"Enter user key: ";cin>>inputKey;

    for (int i=0;i<userCount;i++) {
        if (users[i].name == inputName && users[i].key == inputKey) {
            cout<<"\nLogin Successful Hello,"<<users[i].name<<"\n"<<endl;
            outRule = users[i].rule;
            system("pause");
            return 1;
        }
    }
    cout<<"\nUser Not Found"<<endl;
    system("pause");
    return -1;
}

void signUp() {}  //注册函数！普通用户权限为 RULE_USER
int Length() {}
PasswordItem getStudent(int index){}
int Locate(PasswordItem student){}
void Insert(int i,PasswordItem student){}
PasswordItem Delete(int index){}
int Empty(){}
void PrintLine(){}
void adminMenu() {}
void userMenu() {}

/*----------------------------------------按成员查找------------------------------------*/
void searchStudent() {
    system("cls");
    cout<<"==================== INFO ===================="<<endl;

    if (stuCount == 0)
        cout<<"No Student Found"<<endl;
    system("pause");
    return;

    system("cls");
    cout<<"==================== INFO ===================="<<endl;
    cout<<""<<endl;
    cout<<""<<endl;
    cout<<""<<endl;
    cout<<""<<endl;
    cout<<""<<endl;
    cout<<""<<endl;
}
/*-------------------------------------------END---------------------------------------*/

int main()
{
    initAdmin();

    int choice;
    while(true) {
        system("cls");
        cout<<"=================================================================="<<endl;
        cout<<"+----- Main Menu -----+"<<endl;
        cout<<"|  1. Login           |"<<endl;
        cout<<"|  2. Sign Up         |"<<endl;
        cout<<"|  3. Logout          |"<<endl;
        cout<<"+---------------------+"<<endl;
        cout<<"Enter your choice: ";
        cin>>choice;
        switch(choice) {
            case 1: {
                int loginToken;
                while(true) {
                    loginToken = login(ruletoken);
                    if (loginToken == 1) {
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
                cout<<"Thank you again!"<<endl;
                system("pause");
                return 0;
                break;
            default: {
                cout << "Invalid choice, please try again!" << endl;
                system("pause");
                break;
            }
        }
    }
    return 0;
}