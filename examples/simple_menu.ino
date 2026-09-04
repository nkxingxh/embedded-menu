/**
 * Embedded-Menu 简单菜单示例
 * 通过该例程，你将了解到如何使用本菜单库。包括创建基本菜单、动态创建菜单等。
 * 
 * Embedded-Menu Copyright (c) 2025-2026 NKXingXh
 * License AGPLv3.0: GNU AGPL Version 3 <https://www.gnu.org/licenses/agpl-3.0.html>
 * This is free software: you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 * 
 * Github: https://github.com/nkxingxh/embedded-menu
 */


#include <embedded_menu.h>

#include <U8g2lib.h>
#include "Wire.h"


// 屏幕配置与实例化 u8g2
#define DISPLAY_OLED_SDA 5  // 屏幕数据引脚
#define DISPLAY_OLED_SCL 4  // 屏幕时钟引脚
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;   // 此处使用硬件 I2C 的定义


// 前向声明回调函数
bool enterCallback(MenuItem* target, MenuItem* current);
bool msgBoxCallback(MenuItem* target, MenuItem* current);
bool loadingCallback(MenuItem* target, MenuItem* current);
bool dynListCallback(MenuItem* target, MenuItem* current);
bool listMemberCallback(MenuItem* target, MenuItem* current);
bool backCallback(MenuItem* current);

// 实例化根菜单
// 可直接嵌套多级菜单
// MenuItem 有多种函数签名, 可根据不同场景和需求使用
MenuItem rootMenu("菜单示例", {
    // 静态菜单树
    MenuItem("静态子菜单", {
        MenuItem("正常项目"),
        MenuItem("禁用项目", true),
    }),
    // 通过菜单回调函数动态生成菜单
    // 可以把 clearItemsOnBack 设置为 true, 这样会在返回上一级时自动清理菜单 vector 以释放内存空间
    MenuItem("动态子菜单", enterCallback, false, true),
    MenuItem("模拟加载中", loadingCallback),
    // 当需要加载很长的列表 be like: 这个菜单的子项目都共用一个回调函数
    MenuItem("动态列表示例", dynListCallback, false, true)
});

// 实例化菜单导航器
MenuNavigator nav;

void setup() {
    // 初始化串口
    Serial.begin(115200);

    // 初始化 u8g2
    // 这里使用硬件 IIC
    Wire.begin(DISPLAY_OLED_SDA, DISPLAY_OLED_SCL);
    u8g2.begin();   // u8g2 会直接使用硬件IIC
    u8g2.enableUTF8Print(); // 启用 UTF8 支持以显示中文
    
    // 给“静态子菜单”绑定一个返回回调函数
    // 以实现每返回一次，其“禁用菜单”就变为“启用菜单” (或“启用菜单”变为“禁用菜单”)
    // 由于“静态子菜单”是挂在 rootMenu 下的, 并没有 MenuItem 实例或指针指向它, 我们使用 items[x] 进行访问
    // rootMenu.items[0] 即为 “静态子菜单” 的 MenuItem 实例 (而非指针)
    rootMenu.items[0].backCall = backCallback;

    // 绑定导航器
    // 注意根菜单和 u8g2 实例需要取地址
    nav.begin(&rootMenu, &u8g2);

    // 设置字体和文字高度
    nav.setFont(u8g2_font_wqy13_t_gb2312, 13);

    // 设置返回菜单
    // addBackToSubMenu 默认为 nullptr, 设置后, 会在根菜单以外的菜单末尾自动添加返回上一级
    nav.addBackToSubMenu = "返回";

    // 绘制叠加层 (通过自定义回调函数)
    // nav.drawCallback = menuDrawOverlay;

    // 其他业务代码
    // ...

    // 当初始化完成或准备好后, 绘制第一帧菜单
    nav.draw();
}

// 主循环
void loop() {
    // 你的业务代码
    // ......

    // 对菜单的操作和绘制，实际上都是由导航器完成的。所以要控制菜单光标、进入或返回上一级，都可以通过导航器接口完成。
    // 实际使用中通常使用按钮来控制菜单, 例如不同的按钮分配到不同的菜单操作上 (上、下、进入、返回)
    // 为了方便演示, 例程用串口控制菜单。直接发送相应字符即可控制菜单:
    // 1: 上, 2: 下, 3: 进入, 4: 返回
    if (Serial.available() > 0) {
        char received = Serial.read();
        switch (received)
        {
        case '1':
            nav.move(1, false);
            break;

        case '2':
            nav.move(1, true);
            break;

        case '3':
            nav.enter();
            break;

        case '4':
            nav.back();
            break;
        
        default:
            break;
        }
    }
}

// 进入菜单回调函数
bool enterCallback(MenuItem* target, MenuItem* current) {
    // 由于传入的 target 和 current 是 MenuItem 指针, 所以访问时需要使用 -> 操作符解引用, 而非使用 . 操作符
    // 正确示例: target->items[x]
    // 错误示例: target.items[x]

    // 直接替换 items
    target->items = {
        // 动态显示当前 SoC 温度
        MenuItem("温度: " + String(temperatureRead()) + " °C"),
        // 进入该菜单, 弹出消息框, 每次值 +1
        MenuItem("消息框示例", msgBoxCallback)
    };

    // 必须要返回 bool 类型值, 否则会异常
    return true;
}

bool msgBoxCallback(MenuItem* target, MenuItem* current) {
    static unsigned int cnt = 0;
    
    // 计数器 +1
    cnt++;

    // 调用 showMsgBox 函数
    // MsgBox 其实也是一个 MenuItem, 但只支持两级菜单, 更多的菜单层级会被忽略
    // 开发者注: 其实把 allowSelect 参数设置为 true, 那么这个 MsgBox 就是普通菜单的样式, 理论上也可以选择项目。但我自己实际从没有这样用过, 你如果想的话, 或许可以尝试(?)
    nav.showMsgBox(MenuItem("提示", {
        MenuItem("这是一个MsgBox"),
        MenuItem(String(cnt)),
        MenuItem("进入MsgBox次数")
    }));
    
    // 其实这里返回什么值都无所谓了, 因为“消息框示例”菜单的 items 没有设置, 所以默认就是空的
    // 那么在 items 为空的情况下, 即使返回 true, 也没有菜单能进入了, 但你还是必须返回一个值 否则会炸
    // 但如果 items 存在值, 返回 true 则会把新的菜单压入菜单栈, 但由于先前已经弹出了一个 msgBox, 需要先处理掉 msgBox 才会显示菜单
    return false;
}

// 模拟加载中
bool loadingCallback(MenuItem* target, MenuItem* current) {
    // 在实际的业务中, 一些操作可能需要一段时间来完成
    // 此时可以弹出一个 msgBox 告诉用户正在加载

    // 直接弹就可以
    // 虽然 showMsgBox 有 allowBack 参数, 其默认值为 true, 即允许用户关闭该 msgBox
    // 但我们的业务通常是阻塞的, 在阻塞期间, 用户也没办法对菜单进行操作。
    // 所以, 即使不设置 allowBack 参数为 false, 在阻塞期间实际上用户就是没有办法关掉这个 msgBox 的
    nav.showMsgBox(MenuItem("提示", {
        MenuItem("正在加载数据"),
        MenuItem("请稍候")
    }));

    // 模拟业务加载耗时
    delay(3000);

    // 清空原先存在的项目
    // 开发者注: 清空原先的项目并不是必须的, 但如果不清空, 且没有设置返回时自动清理, 那么会导致先前的菜单项目被保留。需要根据实际情况选择
    target->items = {};

    // 给目标菜单添加一些项目 (实际业务中按需添加)
    for(uint8_t i = 0; i < 8; i++) {
        target->items.emplace_back(String("第") + String(i) + String("项"));
    }

    // 依旧是记得返回
    return true;
}

bool dynListCallback(MenuItem* target, MenuItem* current) {
    // 这里构建一个 100 项的菜单, 共用同一个回调函数
    for(uint8_t i = 1; i <= 100; i++) {
        target->items.emplace_back(MenuItem(String(i), listMemberCallback));
    }
    return true;
}

bool listMemberCallback(MenuItem* target, MenuItem* current) {
    // 为区分从哪一项进来的, 可以使用导航器提供的 getSelect
    // 当调用进入菜单回调函数时, 实际上目标菜单还未被压入菜单栈, 那么 getSelect 函数取得的仍然是“当前”菜单中选中的项目索引
    // 索引从 0 开始
    uint16_t selected = nav.getSelect();

    // 替换为你的业务逻辑
    // 这里示例所以弹个 msgBox 出来
    nav.showMsgBox(MenuItem("提示", {
        MenuItem("你选中了第"),
        MenuItem(String(selected)),
        MenuItem("项")
    }));

    return false;
}

// 菜单返回回调函数
bool backCallback(MenuItem* current) {
    // 这里接收的是当前的菜单指针
    // 此时还没返回上一级菜单

    // 可以在回调函数进行一些清理工作
    // 比如一些数据存在全局变量, 或其他全局空间, 可以在返回回调函数中进行清理
    // 这里就通过串口输出一条文本内容
    Serial.println("菜单返回回调被调用了");

    // 实现每返回一次，其“禁用菜单”就变为“启用菜单” (或“启用菜单”变为“禁用菜单”)
    static unsigned int cnt = 0;
    if (cnt++ % 2 == 0) {
        current->items[1].title = "启用项目";
        current->items[1].disable = false;
    } else {
        current->items[1].title = "禁用项目";
        current->items[1].disable = true;
    }

    // 关于返回值
    // 返回 false 可以阻止返回上一级菜单
    // 一样的 必须返回一个 bool 类型值
    return true;
}