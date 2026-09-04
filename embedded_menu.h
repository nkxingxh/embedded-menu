/**
 * Embedded-Menu Copyright (c) 2025-2026 NKXingXh
 * License AGPLv3.0: GNU AGPL Version 3 <https://www.gnu.org/licenses/agpl-3.0.html>
 * This is free software: you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 * 
 * Github: https://github.com/nkxingxh/embedded-menu
 */

#ifndef _EMBEDDED_MENU_H
#define _EMBEDDED_MENU_H

// #define _MENU_DEBUG_SERIAL Serial

#include <deque>
#include <vector>
#include <Arduino.h>
#include <U8g2lib.h>


class MenuItem;  // 前向声明 MenuItem

// bool MenuEnterCall (要进入的菜单指针, 当前菜单指针)
typedef bool (*MenuEnterCall)(MenuItem*, MenuItem*);
// bool MenuBackCall (当前菜单指针)
typedef bool (*MenuBackCall)(MenuItem*);
// bool DrawOverlayCallback (绘制覆盖回调)
typedef void (*DrawOverlayCallback)(U8G2* u8g2);

struct MenuItem {
  String title = "";                 // 菜单标题
  std::vector<MenuItem> items = {};  // 菜单项目
  MenuEnterCall callback = nullptr;  // 进入菜单回调函数, 返回false阻止进入子菜单。如果是根菜单, 会在 begin() 时调用该函数, 但传入的 currentMenu 为 nullptr, 且会忽略返回值, 即无论如何都进入, 必须确保根菜单有项目
  MenuBackCall backCall = nullptr;   // 返回时的回调函数, 返回false阻止返回父菜单。
  bool disable = false;              // 是否禁用
  bool clearItemsOnBack = false;     // 返回时自动清理子项, 适用于动态构建的菜单释放空间
  u16_t defaultSelected = 0;         // 默认选择子项

  MenuItem();
  MenuItem(String title, std::vector<MenuItem> items = {}, MenuEnterCall callback = nullptr, bool disable = false, u16_t defaultSelected = 0, bool clearItemsOnBack = false);
  MenuItem(String title, std::vector<MenuItem> items, u16_t defaultSelected, bool disable = false, bool clearItemsOnBack = false);
  MenuItem(String title, MenuEnterCall callback, bool disable = false, bool clearItemsOnBack = false);
  MenuItem(String title, MenuEnterCall callback, MenuBackCall backCall, bool disable = false, bool clearItemsOnBack = false);
  MenuItem(String title, bool disable);
};

class MenuNavigator {
private:
  // std::stack<MenuItem*> menuStack;
  MenuItem* rootMenu;
  std::deque<u16_t> selectedStack;
  std::deque<u16_t> selectedOffsetStack;

  bool isMsgBox = false;   // 是否对话框
  bool msgBoxAllowSelect;  // 是否允许选择项目
  bool msgBoxAllowBack;    // 是否允许返回

  U8G2* u8g2 = nullptr;
  u16_t displayWidth = 0;
  u16_t displayHeight = 0;

  const uint8_t* font = nullptr;
  u8_t fontHeight = 0;

  // u8_t lineSpacing = 2;
  u8_t titleBottemMargin = 2;     // 标题区域 (标题行和分割线) 下边距
  u8_t lineTopMargin = 1;         // 行顶部边距
  u8_t lineBottemMargin = 1;      // 行底部边距
  u8_t displayableMenuItems = 0;  // 可显示菜单项数量 (不包括标题区域)

  void pushSelectStack(u16_t defaultSelected);
  bool popSelectStack(bool force = false);

public:
  int8_t arrowOffsetY = 0;
  char* addBackToSubMenu = nullptr;

  DrawOverlayCallback drawCallback = nullptr;  // 自定义叠加绘制

  MenuItem msgBox;  // 对话框菜单

  MenuNavigator();
  MenuNavigator(MenuItem* rootMenu, U8G2* u8g2);

  void begin(MenuItem* rootMenu, U8G2* u8g2 = nullptr);
  void setDisplay(U8G2* u8g2, const u16_t displayWidth = 0, const u16_t displayHeight = 0);
  void setStyle(const u8_t fontHeight, const u8_t lineTopMargin, const u8_t lineBottemMargin);
  void setFont(const uint8_t* font, const u8_t fontHeight = 0);
  void setLineMargin(const u8_t lineTopMargin, const u8_t lineBottemMargin);

  void draw();
  void move(u16_t offset = 1, bool down = true, bool needDraw = true);
  bool enter(bool direct = false, bool needDraw = true);
  void enterPath(const u16_t* path, u8_t depth, bool direct = false, bool needDraw = true);
  bool back(bool needDraw = true, u8_t level = 1);
  bool select(u16_t index);

  u16_t getSelect(u8_t parent = 0);
  MenuItem* getParentMenu(u8_t parent = 0);
  MenuItem* getCurrentMenu(bool ignoreMsgBox = false);

  void showMsgBox(MenuItem msgBox, bool allowBack = true, bool allowSelect = false);
  void showPromptBox(const uint8_t* iconFont, uint16_t iconEncoding, uint8_t iconW, uint8_t iconH, const char* line1 = nullptr, const char* line2 = nullptr, const char* line3 = nullptr);
};

#endif
