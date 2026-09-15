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
    string email;
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
int Length() {
    return stuCount;
}
PasswordItem getStudent(int index){
	return students[index];
}
int Locate(PasswordItem student){
	for (int i = 0;i < stuCount;i++) {
		if (students[i].name == student.name) {
			return i;
		}
	}
    return -1;
}
void Insert(int i,PasswordItem student){
    if (stuCount >= Max_Password || i<0 || i>stuCount) {
        cout << "Insert failed, invalid position!" << endl;
        return;
    }
    for (int j = stuCount;j > i;j--) {
        students[j] = students[j-1];
    }
    students[i] = student;
    stuCount++;
        
}
PasswordItem Delete(int index) {
    PasswordItem temp;
    if (index < 0 || index >= stuCount) {
        cout << "Delete failed, invalid position!" << endl;
        return temp;
    }
    temp = students[index];
    for (int j = index;j < stuCount - 1;j++) {
        students[j] = students[j + 1];
    }
    stuCount--;
    return temp;
}

int Empty(){
    return stuCount == 0 ? 1 : 0;
}
void PrintLine() {
    cout << "==================================================================" << endl;
}
void searchStudent() {
    system("cls");
    cout << "==================== INFO Search ====================" << endl;
    if (Empty()) {
        cout << "No password data!" << endl;
        system("pause");
        return;
    }
    string searchName;
    cout << "Input account name to search: ";
    cin >> searchName;
    bool findFlag = false;
    for (int i = 0;i < stuCount;i++) {
        if (students[i].name == searchName) {
            PrintLine();
            cout << "Account: " << students[i].name << endl;
            cout << "Password: " << students[i].key << endl;
            cout << "Address: " << students[i].address << endl;
            cout << "Email: " << students[i].email << endl;
            PrintLine();
            findFlag = true;
        }
    }
    if (!findFlag) {
        cout << "Not found!" << endl;
    }
    system("pause");
}
                
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
    /*
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
    }*/


	Insert(1, { "Jane Smith", "pass456", "456 Elm St", "janesmith@example.com" });

    cout<< Length() << endl;

    return 0;
}