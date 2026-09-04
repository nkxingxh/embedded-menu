# Embedded-Menu

面向对象的、易于使用的、适用于 Arduino 平台的轻量级菜单库。

该菜单库基于 U8g2 图形库实现，支持动态菜单构建、多级导航、对话框和提示框等功能。

> ⚠️ 该库使用 C++ 编写，必须使用 C++ 编译器编译，无法在纯 C 环境中使用。该库的许多公开函数使用了指针，如需正常使用该库，您需要了解 C++ 指针，错误地使用指针会使您的程序无法按预期运行。

**本项目根据 [AGPL-3.0](https://www.gnu.org/licenses/agpl-3.0.html) 许可证进行许可，使用本项目意味着您的软件也需要开源。**

~~Embedded-MenU 是不是可以简称 emu (? wonderhoy ?)~~

![emu](https://storage.moegirl.org.cn/moegirl/commons/b/ba/Wonderhoy.png!/fw/99?v=20211025164238)

## 接口

该库主要提供了两个类，分别为菜单导航类 `MenuNavigator` 与菜单项目类 `MenuItem`。

`MenuNavigator` 提供菜单的显示、导航和交互功能，`MenuItem` 用于构建菜单树结构。

在使用前, 请先引入 `embedded_menu.h` 头文件。

### 构建静态菜单

这是构建菜单的一种基本方法，适合用于构建根菜单与内容不变的菜单。

`MenuItem` 可以嵌套多层, 以构建多级菜单。

```cpp
// 构建静态菜单树
MenuItem rootMenu("主菜单", {
    MenuItem("关于", {
        MenuItem("版本: 1.0.0"),
        MenuItem("作者: NKXingXh"),
    }),
});
```

### 初始化导航器

菜单的绘制、操作都由导航器实现，所以除了构建菜单，我们还需要实例化导航器，并在需要时进行相应的配置。

导航器需要绑定根菜单与 u8g2 实例，所以在 `nav.begin()` 之前，请确保根菜单与 u8g2 都已实例化。

```cpp
// 需要先构建根菜单与实例化 u8g2, 参考上文

// 实例化导航器类
MenuNavigator nav;

void setup() {
    // 配置 u8g2...
    // u8g2.xxxx()

    // 绑定根菜单与 u8g2 实例
    nav.begin(&rootMenu, &u8g2);
    nav.draw();
}
```

### 进入菜单回调

当使用 `nav.enter()` 方法进入一个菜单项目时，如果该菜单项存在进入回调函数，则会先调用进入回调函数。如果回调函数返回 false, 则不会进入该菜单。

一些时候，我们希望动态显示菜单内容 (例如显示传感器数据)，这种情况下可以通过菜单回调函数来动态构建菜单。如果我们不想用户进入该菜单, 也可以返回 false。

需要特别说明的是, 根菜单也可以设置进入回调, 根菜单的进入回调会在 `nav.begin()` 时被调用，且回调函数的返回值会被忽略 (即便返回 false 也无法阻止进入根菜单), 同时你也需要确保根菜单存在项目。

```cpp
// 定义进入回调函数
bool settingsOnEnter(MenuItem* targetMenu, MenuItem* currentMenu) {
    // 动态构建子菜单
    targetMenu->items = {
        MenuItem("WiFi 设置", wifiOnEnter),
        MenuItem("显示设置", displayOnEnter),
    };
    return true;  // 允许进入
}

bool lockedOnEnter(MenuItem* targetMenu, MenuItem* currentMenu) {
    if (!isUnlocked()) {
        return false;  // 阻止进入
    }
    return true;
}

// 使用进入回调
MenuItem rootMenu("主菜单", {
    MenuItem("设置", settingsOnEnter),
    MenuItem("锁定功能", lockedOnEnter),
});
```

### 返回菜单回调

当退出一个菜单时 (返回上一级菜单), 如果该菜单项存在返回回调函数, 则会在返回上一级菜单前, 调用该函数。如果回调函数返回 false, 则会阻止返回上一级菜单。

```cpp
// 定义返回回调函数
bool saveOnBack(MenuItem* currentMenu) {
    saveSettings();
    return true;  // 允许返回
}

bool unsavedOnBack(MenuItem* currentMenu) {
    if (hasUnsavedChanges()) {
        return false;  // 阻止返回
    }
    return true;
}

// 使用返回回调
MenuItem settingsMenu("设置", settingsOnEnter, saveOnBack);
```

### 消息框与对话框

使用 `nav.showMsgBox()` 可以显示一个消息框，消息框会覆盖当前菜单显示。当存在对话框时, 返回、进入或移动菜单都会关闭对话框，且不会执行原先的操作。

```cpp
// 显示简单消息框
nav.showMsgBox(MenuItem("提示", {
    MenuItem("操作成功"),
    MenuItem("设置已保存"),
}));
```

使用 `nav.showPromptBox()` 可以显示一个带图标的提示框，适合显示状态提示或警告信息。

```cpp
// 显示带图标的提示框
nav.showPromptBox(
    u8g2_font_open_iconic_check_4x_t,  // 图标字体
    0x40,                               // 图标编码
    32,                                 // 图标宽度
    32,                                 // 图标高度
    "操作成功",                         // 第一行文本
    "设置已保存"                        // 第二行文本
);

// 显示三行文本的提示框
nav.showPromptBox(
    u8g2_font_open_iconic_embedded_4x_t,
    0x41,
    32,
    32,
    "警告",
    "有未保存的更改",
    "请先保存或取消"
);
```

### 常用导航器函数

**移动当前光标**：`nav.move(1, true)` 向下移动，`nav.move(1, false)` 向上移动。

**进入选中菜单**：`nav.enter()` 正常进入，`nav.enter(true)` 跳过回调与禁用检查直接进入。

**返回上级菜单**：`nav.back()` 返回一级，`nav.back(true, 3)` 返回三级。

**选中指定项目**：`nav.select(2)` 跳转到第 3 项（不进入）。

**获取当前选中项**：`nav.getSelect()` 获取当前菜单选中项索引，`nav.getSelect(1)` 获取父菜单选中项索引。

**获取父级菜单指针**：`nav.getParentMenu(0)` 获取父菜单指针，`nav.getParentMenu(0xff)` 获取根菜单指针（超出范围自动返回根菜单）。

**获取当前菜单指针**：`nav.getCurrentMenu()` 获取当前显示的菜单指针。

```cpp
// 移动光标
nav.move(1, true);   // 向下移动一项
nav.move(1, false);  // 向上移动一项

// 进入菜单
nav.enter();          // 正常进入
nav.enter(true);      // 直接进入

// 返回菜单
nav.back();           // 返回一级
nav.back(true, 3);    // 返回三级

// 选中指定项目
nav.select(2);        // 跳转到第 3 项

// 获取选中项
u16_t current = nav.getSelect();    // 当前菜单选中项
u16_t parent = nav.getSelect(1);    // 父菜单选中项

// 获取菜单指针
MenuItem* currentMenu = nav.getCurrentMenu();  // 当前菜单
MenuItem* parentMenu = nav.getParentMenu(0);   // 父菜单
MenuItem* parentMenu = nav.getParentMenu(1);   // 祖父菜单
```

## 许可

> Copyright (c) 2025-2026 NKXingXh

![agplv3-with-text](https://www.gnu.org/graphics/agplv3-with-text-162x68.png)

Embedded-Menu 根据 [AGPL-3.0](https://www.gnu.org/licenses/agpl-3.0.html) 许可证进行许可，有关详细信息，请参阅 [LICENSE](./LICENSE) 文件。

本软件按“原样”提供，不附带任何形式的明示或暗示担保。开发者不保证本软件将满足用户的要求或无故障运行。对于因使用或无法使用本软件所产生的任何直接或间接损害，开发者不承担任何责任。

## 鸣谢

感谢以下项目以及所有开源生态的贡献者:

 - **[U8g2](https://github.com/olikraus/u8g2)**：强大的图形库，提供了丰富的显示驱动与字体支持
 - **[Arduino](https://www.arduino.cc/)**：简单易用的开发框架与活跃的社区
 - **[乐鑫科技 (Espressif)](https://www.espressif.com/)**：提供 ESP32 / ESP8266 等一系列功能强大的硬件平台

开源精神因分享而传承，因参与而壮大。
