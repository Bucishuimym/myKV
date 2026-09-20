# myKV — CLI密码管理器

C++ 课程设计。单文件实现（`KVmain.cpp`），带用户体系、两级权限、本地文件持久化的
命令行密码/账号管理程序。所有数据存在内存的顺序表里，启动时从文件读回、退出时写回文件，
所以关掉再打开数据不会丢。

## 功能概览

- **用户体系**：注册、登录、注销；管理员与普通用户两级权限
- **密码数据管理**：插入、按 UID 查询、按位置删除
- **数据隔离**：普通用户只能看到、只能删自己的数据；管理员能看全部
- **密文存储**：密码异或 0x7F 后落盘，显示时同时给出密文（十六进制）和明文
- **本地持久化**：`users.txt` / `data.txt`，重启不丢数据

## 编译与运行

| 方式 | 做法 |
| --- | --- |
| CLion / CMake | 直接打开工程构建（`CMakeLists.txt` 已为 g++ 配好 `-fexec-charset=GBK`，中文不乱码） |
| Visual Studio | 打开 `myKV.vcxproj`（或 `myKV.slnx`）构建 |
| 命令行 g++ | `g++ -std=c++17 -fexec-charset=GBK -o myKV KVmain.cpp` |

程序在**当前工作目录**下读写 `users.txt` 和 `data.txt`（不存在就自动创建）。
首次运行的默认管理员：**admin / 123**。

> g++ 动态链接的运行需要 MinGW 的 `bin` 目录在 `PATH` 上，否则进程会静默启动失败。
> MSVC 构建无此问题。

## 目录结构

```
myKV/
├── KVmain.cpp          全部实现：常量、数据结构、算法、菜单、main
├── kvH.h               预留头文件（当前为空）
└── users.txt           运行时生成：用户表
    data.txt            运行时生成：密码数据表
```

## 功能模块图

```text
myKV（KVmain.cpp 单文件实现）
│
├── 1. 全局状态与常量
│   ├── 容量：Max_User = 100（用户表）、Max_Password = 1000（密码表）
│   ├── 权限令牌：RULE_ADMIN = 2、RULE_USER = 1
│   ├── 会话状态：ruletoken（-1 表示未登录）、userUid（000 表示未登录）
│   └── 文件名：USER_FILE = "users.txt"、DATA_FILE = "data.txt"
│
├── 2. 数据结构
│   ├── User         { name, key, uid, rule }      → users[100]  + userCount
│   └── PasswordItem { Username, URL, key, UID }   → Slist[1000] + pasCount
│
├── 3. 通用顺序表模板（对两张表通用，不依赖具体类型）
│   ├── Insert(list, count, i, item, maxSize)   在下标 i 处插入，位置非法抛异常
│   ├── Delete(list, count, index)              按下标删除；index > 9000 时把它当 UID 先定位
│   └── getUid(User) / getUid(PasswordItem)     重载，把 uid 和 UID 两个名字统一起来
│
├── 4. 用户管理模块
│   ├── initAdmin()                             首次运行建 admin/123，rule = RULE_ADMIN
│   ├── signUp()                                注册新用户，rule 固定 RULE_USER
│   ├── login(outRule)                          输入明文先加密再与表里密文比对，成功写回权限和 UID
│   ├── uidCount()                              生成 UID：(userCount + 234565) % 1000 + 9000 → 9000~9999
│   ├── LocateUserByUsers() / LocateUserByUid() 按用户名 / 按 UID 定位
│   └── PrintLineUser() / PrintUserData()       用户信息打印
│
├── 5. 密码数据模块
│   ├── encryptDecrypt()                        异或 0x7F，加密解密是同一个函数
│   ├── hexVal() / toHex() / fromHex()          密文字节 ↔ 十六进制字符串
│   ├── LocatePasswordBySeq(UID)                按 UID 在密码表定位
│   ├── LocateMyDataBySeq(seq)                  "我的第 seq 条" → 表里绝对下标
│   ├── ListData()                              admin 列全部；user 只列自己的，且不暴露 UID
│   └── Length() / Empty() / PrintLineData()
│
├── 6. 持久化模块
│   ├── outFile()                               把 users[] 和 Slist[] 覆盖写回两个文件
│   └── inFile()                                启动时读回；字段用 \t 分隔，key 存十六进制
│
└── 7. 交互界面
    ├── main()                                  主菜单：1 登录 / 2 注册 / 3 退出；启动与退出时负责读写文件
    ├── adminMenu()                             管理员菜单，9 项
    └── userMenu()                              普通用户菜单，6 项
```

## 系统流程图

```text
开始
 │
 ├─ inFile()          先把 users.txt / data.txt 读回内存
 │
 ├─ userCount == 0 ?
 │     ├─ 是 ─► initAdmin()      只有首次运行（两个文件都读不到）才建 admin
 │     └─ 否 ─► 沿用文件里的用户表，不会再插一个 admin
 │
 ├─► 主菜单： [1] 登录    [2] 注册    [3] 退出
 │     │
 │     ├─ [1] ─► login()
 │     │          ├─ 用户名不存在 / 密码错 ─► 回主菜单
 │     │          └─ 成功 ─► rule == RULE_ADMIN ?
 │     │                       ├─ 是 ─► adminMenu()   （1~8 项操作内存里的表，9 退出登录）
 │     │                       └─ 否 ─► userMenu()    （1~4 项操作自己的数据，5 登出，6 注销）
 │     │                                │
 │     │                                ▼ 菜单返回（主动登出，或输入流 EOF）
 │     │                            outFile() 落盘 ─► 回主菜单
 │     │
 │     ├─ [2] ─► signUp()（rule = RULE_USER）─► 回主菜单
 │     │
 │     └─ [3] ─► outFile() 落盘 ─► 结束
 │
 └─ 输入流 EOF（管道 / 重定向场景）：落盘后直接退出，不再空转
```

**数据流向**：内存里的 `users[]` / `Slist[]` 是唯一数据源，菜单里的增删改查只动内存；
`inFile()` 在启动时灌入，`outFile()` 在退出、登出、EOF 三个时机覆盖写回。

```text
  启动 ──► inFile() ──► users[] / Slist[]（内存）──► outFile() ──► 退出
                              ▲   │
                              │   └── 菜单增删改查（Insert / Delete / signUp / ListData）
                              └──────┘
```

## 菜单与权限

| 权限 | `ruletoken` | 能做什么 |
| --- | --- | --- |
| 管理员 | 2 | 看全部用户和全部密码数据；按 UID 查、按位置删；注册新用户；删用户时会级联删掉该用户的所有数据 |
| 普通用户 | 1 | 只看/只改自己的数据；删自己的数据时用「列表里的序号」，不是表里下标 |
| 未登录 | -1 | 只能注册、登录、退出 |

管理员菜单（`adminMenu`）：

| 序号 | 功能 | 序号 | 功能 |
| --- | --- | --- | --- |
| 1 | 按 UID 查询密码数据 | 6 | 按位置 / UID 删除用户数据（级联删其数据） |
| 2 | 按用户名查询用户 | 7 | 打印全部密码数据 |
| 3 | 插入密码数据 | 8 | 打印全部用户数据 |
| 4 | 插入用户数据（走 `signUp`） | 9 | 退出登录 |
| 5 | 按位置删除密码数据 | | |

普通用户菜单（`userMenu`）：

| 序号 | 功能 | 序号 | 功能 |
| --- | --- | --- | --- |
| 1 | 查看我的账号信息 | 4 | 按位置删除我的数据 |
| 2 | 查看我的密码数据 | 5 | 退出登录 |
| 3 | 插入我的密码数据 | 6 | 注销该账号（级联删数据） |

## 数据文件格式

以 `\t`（制表符）分隔字段，`key` 一律存**十六进制**（`toHex` / `fromHex`）：

```text
users.txt   一行一个用户：  用户名 \t 密文hex \t rule \t UID
            例：            admin<TAB>4E4D4C<TAB>2<TAB>9565

data.txt    一行一条数据：  用户名 \t URL \t 密文hex \t 归属UID
            例：            udata<TAB>http://a/b?c=d<TAB>0A0C1A0D<TAB>9565
```

两个设计上的原因：

1. **分隔符不能用 `/`**：URL 里必然有 `/`（`http://...`），用 `/` 拆分会让 URL 被切成好几段，
   读回来 URL 只剩 `http:`、UID 变成 `atoi("z") = 0`。
2. **`key` 必须 hex 化**：密文是异或后的**裸字节**，可能正好是分隔符，甚至可能是 `0x0A` / `0x0D`
   ——明文 `u` ^ 0x7F = 0x0A（换行）、明文 `r` ^ 0x7F = 0x0D（回车）。直接写裸字节会把这一行
   拦腰截断，读回来整张表就乱了。hex 之后只剩 `0-9A-F`，既不怕分隔符也不怕断行。

用户名和 URL 都是用 `cin >>` 读进来的，不可能含空白字符，所以 `\t` 在文件里不会产生歧义。

## 已知限制

- **不是真正的加密**：异或 0x7F 只是让密码不明文躺在文件里，起不到保密作用。
- **含空格的名字/密码读不了**：`cin >> string` 遇空白即截断。
- **不支持旧格式文件**：早期版本用 `/` 分隔 + 裸密文写出的 `users.txt` / `data.txt` 读不进来，
  需删掉重新生成。
- **定长数组**：用户上限 100、密码数据上限 1000。`signUp` 自己挡了用户表的上限，但密码表满了
  `Insert` 会抛异常，而调用处没有 `catch`，会直接终止进程。
- **预留但未实现的接口**：`numToStr`、`strToNum`、`PasswordData`、`searchPassword`、
  `LocatePasswordBySeq(const string&)`（按用户名查密码表）目前只有声明。
- 管理员账号是硬编码的 `admin / 123`，程序里没有改密码 / 改权限的入口（`signUp` 撞到同名用户会拒绝），
  想换只能改 `initAdmin()` 里的初值，或手工编辑 `users.txt`。
