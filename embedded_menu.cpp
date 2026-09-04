/**
 * Embedded-Menu Copyright (c) 2025-2026 NKXingXh
 * License AGPLv3.0: GNU AGPL Version 3 <https://www.gnu.org/licenses/agpl-3.0.html>
 * This is free software: you are free to change and redistribute it.
 * There is NO WARRANTY, to the extent permitted by law.
 * 
 * Github: https://github.com/nkxingxh/embedded-menu
 */

#include <embedded_menu.h>

MenuItem::MenuItem() {
}

MenuItem::MenuItem(String title, std::vector<MenuItem> items, MenuEnterCall callback, bool disable, u16_t defaultSelected, bool clearItemsOnBack) {
  this->title = title;
  this->items = items;
  this->callback = callback;
  this->disable = disable;
  this->defaultSelected = defaultSelected;
  this->clearItemsOnBack = clearItemsOnBack;
}

MenuItem::MenuItem(String title, std::vector<MenuItem> items, u16_t defaultSelected, bool disable, bool clearItemsOnBack) {
  this->title = title;
  this->items = items;
  this->defaultSelected = defaultSelected;
  this->disable = disable;
  this->clearItemsOnBack = clearItemsOnBack;
}

MenuItem::MenuItem(String title, MenuEnterCall callback, bool disable, bool clearItemsOnBack) {
  this->title = title;
  this->callback = callback;
  this->disable = disable;
  this->clearItemsOnBack = clearItemsOnBack;
}

MenuItem::MenuItem(String title, MenuEnterCall callback, MenuBackCall backCall, bool disable, bool clearItemsOnBack) {
  this->title = title;
  this->callback = callback;
  this->backCall = backCall;
  this->disable = disable;
  this->clearItemsOnBack = clearItemsOnBack;
}

MenuItem::MenuItem(String title, bool disable) {
  this->title = title;
  this->disable = disable;
}

/* MenuItem::MenuItem(String title = nullptr, std::initializer_list items, MenuEnterCall callback) {
  this->title = title;
  this->items = items;
  this->callback = callback;
} */


MenuItem* MenuNavigator::getParentMenu(u8_t parent) {
  MenuItem* current = rootMenu;
  u16_t n = selectedStack.size() - 1;  // 舍去选择栈中的最后一项 (当前菜单选择)
  // 超出范围直接返回根菜单
  if (parent > n) {
    parent = n;
  }
  n -= parent;  // 减去 parent 层
  for (u16_t i = 0; i < n; i++) {
    current = &(current->items[selectedStack[i]]);
  }
  return current;
}

MenuItem* MenuNavigator::getCurrentMenu(bool ignoreMsgBox) {
  // 检查是否存在对话框
  if (!ignoreMsgBox && isMsgBox) {
    return &msgBox;
  }
  // MenuItem* current = rootMenu;
  // u16_t n = selectedStack.size() - 1;  // 舍去选择栈中的最后一项 (当前菜单选择)
  // for (u16_t i = 0; i < n; i++) {
  //   current = &(current->items[selectedStack[i]]);
  // }
  // return current;
  return getParentMenu(0);
}

void MenuNavigator::pushSelectStack(u16_t defaultSelected) {
  // 先压入选择栈
  selectedStack.emplace_back(defaultSelected);

  // 然后取出当前菜单
  MenuItem* menu = getCurrentMenu();

  // 计算默认选中项目显示位置
  if (menu->items.size() > displayableMenuItems) {
    u16_t lastPageStart = menu->items.size() - displayableMenuItems;  // 最后一页的起始位置
    u8_t offset = defaultSelected > lastPageStart ? (defaultSelected - lastPageStart) : 0;
    selectedOffsetStack.emplace_back(offset);
  } else {
    selectedOffsetStack.emplace_back(defaultSelected);
  }

#ifdef _MENU_DEBUG_SERIAL
  _MENU_DEBUG_SERIAL.print(F("[menu::pushSelectStack] 压入 selectedStack: "));
  _MENU_DEBUG_SERIAL.print(defaultSelected);
  _MENU_DEBUG_SERIAL.print(F(", 压入 selectedOffsetStack: "));
  _MENU_DEBUG_SERIAL.println(selectedOffsetStack.back());
#endif
}

bool MenuNavigator::popSelectStack(bool force) {
  if (force) {
    if (selectedStack.size() <= 0 || selectedOffsetStack.size() <= 0) {
#ifdef _MENU_DEBUG_SERIAL
      _MENU_DEBUG_SERIAL.println(F("[menu::popSelectStack] 栈中已无元素"));
#endif
      return false;
    }
  } else {
    if (selectedStack.size() <= 1 || selectedOffsetStack.size() <= 1) {
#ifdef _MENU_DEBUG_SERIAL
      _MENU_DEBUG_SERIAL.println(F("[menu::popSelectStack] 已经位于根菜单"));
#endif
      return false;
    }
  }
  selectedStack.pop_back();
  selectedOffsetStack.pop_back();
  return true;
}

MenuNavigator::MenuNavigator() {
}

MenuNavigator::MenuNavigator(MenuItem* rootMenu, U8G2* u8g2) {
  // this->rootMenu = rootMenu;
  // selectedStack.emplace_back(rootMenu->defaultSelected);
  // setDisplay(u8g2);
  begin(rootMenu, u8g2);
}

void MenuNavigator::begin(MenuItem* rootMenu, U8G2* u8g2) {
#ifdef _MENU_DEBUG_SERIAL
  _MENU_DEBUG_SERIAL.print(F("[menu::begin] 将使用新的根菜单: "));
  _MENU_DEBUG_SERIAL.println(rootMenu->title);
#endif

  // 替换根菜单
  this->rootMenu = rootMenu;

  // 重置选择栈
  selectedStack.clear();
  selectedOffsetStack.clear();

  // 执行回调函数
  if (rootMenu->callback != nullptr) {
#ifdef _MENU_DEBUG_SERIAL
    _MENU_DEBUG_SERIAL.println(F("[menu::begin] 调用根菜单进入回调函数"));
#endif
    rootMenu->callback(rootMenu, nullptr);
  }

  // 压入选择栈
  pushSelectStack(rootMenu->defaultSelected);

  // 重置部分配置 (算了, 还是不重置配置比较好)
  // addBackToSubMenu = nullptr;
  // drawCallback = nullptr;

  // 重置状态
  isMsgBox = false;

  if (u8g2 != nullptr) {
    setDisplay(u8g2);
  }
}

void MenuNavigator::setDisplay(U8G2* u8g2, const u16_t displayWidth, const u16_t displayHeight) {
  this->u8g2 = u8g2;
  this->displayWidth = displayWidth > 0 ? displayWidth : u8g2->getDisplayWidth();
  this->displayHeight = displayHeight > 0 ? displayHeight : u8g2->getDisplayHeight();
  setStyle(fontHeight == 0 ? u8g2->getMaxCharHeight() : fontHeight, lineTopMargin, lineBottemMargin);
}

void MenuNavigator::setStyle(const u8_t fontHeight, const u8_t lineTopMargin, const u8_t lineBottemMargin) {
  this->fontHeight = fontHeight;
  // this->lineSpacing = lineSpacing;
  this->lineTopMargin = lineTopMargin;
  this->lineBottemMargin = lineBottemMargin;
  // 计算屏幕可显示多少菜单项目
  u8_t 行高 = lineTopMargin + fontHeight + lineBottemMargin;
  displayableMenuItems = (displayHeight - (行高 + 1 + titleBottemMargin)) / 行高;
#ifdef _MENU_DEBUG_SERIAL
  _MENU_DEBUG_SERIAL.print(F("[menu::setStyle] 屏幕高度可显示菜单项: "));
  _MENU_DEBUG_SERIAL.println(displayableMenuItems);
#endif
}

void MenuNavigator::setFont(const uint8_t* font, const u8_t fontHeight) {
  this->font = font;
  setStyle(fontHeight == 0 ? u8g2->getMaxCharHeight() : fontHeight, this->lineTopMargin, this->lineTopMargin);
}

void MenuNavigator::setLineMargin(const u8_t lineTopMargin, const u8_t lineBottemMargin) {
  setStyle(this->fontHeight, lineTopMargin, lineBottemMargin);
}

void MenuNavigator::draw() {
  MenuItem* menu = getCurrentMenu();
  u16_t selected = selectedStack.back();
  u16_t selectOffset = selectedOffsetStack.back();

  u8g2->clearBuffer();
  u8g2->setFontMode(1);
  u8g2->setDrawColor(1);  // 黑底白字
  if (font != nullptr) {
    u8g2->setFont(font);
  }

  // 是否对话框样式
  bool useMsgBoxStyle = isMsgBox && !msgBoxAllowSelect;

  // 临时变量
  u16_t h, w, x, y;

  // 绘制标题
  y = lineTopMargin + fontHeight - 1;  // 文字底部 (位置是从 0 开始, 所以需要 -1)
  const char* title = menu->title.c_str();
  if (menu->title != nullptr) {
    // 如果是不可选择的对话框, 则采用白底黑字标题
    if (useMsgBoxStyle) {
      u8g2->drawBox(0, 0, displayWidth, lineTopMargin + fontHeight + lineBottemMargin);
      u8g2->setDrawColor(0);
    }

    w = u8g2->getUTF8Width(title);  // 计算宽度
    x = (displayWidth - w) / 2;     // 计算起始位置
    u8g2->drawUTF8(x, y, title);    // 绘制标题文字
  }

  u8g2->setDrawColor(1);  // 设置黑底白字

  // 分割线
  y += lineBottemMargin + 1;  // +1 包含线条高度
  if (!useMsgBoxStyle) u8g2->drawLine(0, y, displayWidth, y);
  y += titleBottemMargin;  // 标题区域下边距

  // 计算要显示的菜单项目
  u16_t itemsCount = menu->items.size();
  // u16_t i = max(0, selected - end + 1);  // 起始项目位置
  // end = min(itemsCount
  //             + ((addBackToSubMenu && selectedStack.size() > 1
  //                 && (isMsgBox
  //                       ? (msgBoxAllowSelect && msgBoxAllowBack)
  //                       : true))
  //                  ? 1
  //                  : 0),
  //           i + end);  // 结束项目位置

  u16_t i = selected > selectOffset ? (selected - selectOffset) : 0;
  u16_t end = i + displayableMenuItems;

#ifdef _MENU_DEBUG_SERIAL
  _MENU_DEBUG_SERIAL.print(F("[menu::draw] 可显示菜单项目数: "));
  _MENU_DEBUG_SERIAL.println(displayableMenuItems);
#endif

  if (end > itemsCount) {
    end = itemsCount;
    // 满足条件则添加返回按钮
    if (addBackToSubMenu && selectedStack.size() > 1 && (isMsgBox ? (msgBoxAllowSelect && msgBoxAllowBack) : true)) {
      end++;
    }
  }

#ifdef _MENU_DEBUG_SERIAL
  _MENU_DEBUG_SERIAL.print(F("[menu::draw] 绘制菜单项目, 起始: "));
  _MENU_DEBUG_SERIAL.print(i);
  _MENU_DEBUG_SERIAL.print(F(", 结束 (不包含):"));
  _MENU_DEBUG_SERIAL.println(end);
#endif

  // 遍历菜单项目
  for (; i < end; i++) {
    // 跳过 没设置标题的项目
    /* if (i < itemsCount && menu->items[i].title == nullptr) {
      // 如果还有空间 则向后补偿一个
      if (end < itemsCount) {
        n++;
      }
      continue;
    } */

    // 当前项目信息
    bool currentItemIsBack = i >= itemsCount;
    bool currentItemDisabled = currentItemIsBack ? false : menu->items[i].disable;
    bool currentItemCanBeEnter = currentItemIsBack ? false : (menu->items[i].items.size() > 0 || menu->items[i].callback != nullptr);

    u8g2->setDrawColor(1);  // 黑底白字

    // 绘制选中项方框
    if (i == selected && !useMsgBoxStyle) {
      // 判断当前项目是否禁用
      if (currentItemDisabled) {
        u8g2->drawFrame(0, y + 1, displayWidth, lineTopMargin + fontHeight + lineBottemMargin);
      } else {
        u8g2->drawBox(0, y + 1, displayWidth, lineTopMargin + fontHeight + lineBottemMargin);
        u8g2->setDrawColor(0);  // 白底黑字
      }
    }

    y += lineTopMargin + fontHeight;
    /* if (y > displayHeight) {  // 判断是否超高
      break;
    } */

    // 绘制选中项左右箭头
    if (i == selected && !useMsgBoxStyle) {
      w = u8g2->getMaxCharWidth();
      if (currentItemIsBack) {
        x = (w - u8g2->getStrWidth("<")) / 2;
        u8g2->drawStr(x, y + arrowOffsetY, "<");
      } else if (currentItemCanBeEnter) {
        x = displayWidth - w + (w - u8g2->getStrWidth(">")) / 2;
        u8g2->drawStr(x, y + arrowOffsetY, ">");
      }
      // #ifdef _MENU_DEBUG_SERIAL
      //       _MENU_DEBUG_SERIAL.print(F("[menu::draw] 箭头位置: "));
      //       _MENU_DEBUG_SERIAL.print(x);
      //       _MENU_DEBUG_SERIAL.print(", ");
      //       _MENU_DEBUG_SERIAL.print(y + arrowOffsetY);
      //       _MENU_DEBUG_SERIAL.println();
      // #endif
    }

    const char* row = currentItemIsBack ? addBackToSubMenu : menu->items[i].title.c_str();
    w = u8g2->getUTF8Width(row);  // 计算宽度
    x = (displayWidth - w) / 2;   // 计算起始位置
    u8g2->drawUTF8(x, y, row);

    y += lineBottemMargin;
  }

  if (drawCallback != nullptr) {
    // 恢复颜色配置
    u8g2->setDrawColor(1);  // 黑底白字
#ifdef _MENU_DEBUG_SERIAL
    _MENU_DEBUG_SERIAL.println(F("[menu::draw] 调用自定义覆盖绘制函数"));
#endif
    drawCallback(u8g2);
  }
  u8g2->sendBuffer();

#ifdef _MENU_DEBUG_SERIAL
  _MENU_DEBUG_SERIAL.print(F("[menu::draw] 绘制完毕, 占用高度 (含上下边距): "));
  _MENU_DEBUG_SERIAL.println(y);
#endif
}

void MenuNavigator::move(u16_t offset, bool down, bool needDraw) {
  // 如果是不可选择的对话框, 则逻辑修改为返回
  if (isMsgBox && !msgBoxAllowSelect) {
    back();
    return;
  }
  MenuItem* menu = getCurrentMenu();
  u16_t& selected = selectedStack.back();
  u16_t& displayOffset = selectedOffsetStack.back();
  u16_t n = menu->items.size();
  if (addBackToSubMenu && selectedStack.size() > 1) {
    n++;
  }
  offset %= n;
  if (down) {
    // 向下移动
    selected = (selected + offset) % n;
    displayOffset = min(displayOffset + offset, displayableMenuItems - 1);
  } else {
    // 向上移动
    // 防止负数溢出, 所以单独处理操作
    if (offset > selected) {
      // 如果向前移动量会超出队头
      u16_t rOffset = offset - selected;
      selected = n - rOffset;
      displayOffset = displayableMenuItems - min((u16_t)rOffset, (u16_t)displayableMenuItems);
    } else {
      // 如果向前移动但不超出队头
      selected -= offset;
      displayOffset -= min(offset, displayOffset);
    }
  }
  if (needDraw) {
    draw();
  }
}

bool MenuNavigator::enter(bool direct, bool needDraw) {
  // 如果是对话框, 则逻辑修改为返回
  if (isMsgBox) {
#ifdef _MENU_DEBUG_SERIAL
    _MENU_DEBUG_SERIAL.println(F("[menu] 当前为对话框, 执行返回逻辑"));
#endif
    return back();
  }
  MenuItem* menu = getCurrentMenu();
  u16_t selected = selectedStack.back();
  // 判断是否超出范围
  if (selected >= menu->items.size()) {
    if (addBackToSubMenu && selectedStack.size() > 1) {
      return back();
    }
    return false;
  }
  MenuItem* target = &(menu->items[selected]);  // 进入的子项
  if (!direct) {
    // 判断是否禁用
    if (target->disable) {
#ifdef _MENU_DEBUG_SERIAL
      _MENU_DEBUG_SERIAL.println(F("[menu] 项目被禁用"));
#endif
      return false;
    }
    // 执行回调函数
    if (target->callback != nullptr) {
#ifdef _MENU_DEBUG_SERIAL
      _MENU_DEBUG_SERIAL.println(F("[menu] 调用进入回调函数"));
#endif
      if (!target->callback(target, menu)) {
#ifdef _MENU_DEBUG_SERIAL
        _MENU_DEBUG_SERIAL.println(F("[menu] 回调函数返回 false, 不尝试进入子项"));
#endif
        return false;
      }
    }
    // 检查是否存在子项目
    if (target->items.size() <= 0) {
#ifdef _MENU_DEBUG_SERIAL
      _MENU_DEBUG_SERIAL.println(F("[menu] 不存在子项"));
#endif
      return false;
    }
  }
  // 先判断 defaultSelected 是否在有效范围内
  if (target->defaultSelected >= target->items.size()) {
    target->defaultSelected = 0;
  }
  // 压入选择栈
  pushSelectStack(target->defaultSelected);
  if (needDraw) {
    draw();
  }
  return true;
}

void MenuNavigator::enterPath(const u16_t* path, u8_t depth, bool direct, bool needDraw) {
  for (u8_t i = 0; i < depth; i++) {
#ifdef _MENU_DEBUG_SERIAL
    _MENU_DEBUG_SERIAL.print(F("[menu] 选择并进入子菜单 "));
    _MENU_DEBUG_SERIAL.println(path[i]);
#endif
    select(path[i]);
    enter(direct, i < (depth - 1) ? false : needDraw);
  }
}

bool MenuNavigator::back(bool needDraw, u8_t level) {
  // 对话框分支处理
  if (isMsgBox) {
    // 判断对话框是否允许返回
    if (!msgBoxAllowBack) {
#ifdef _MENU_DEBUG_SERIAL
      _MENU_DEBUG_SERIAL.println(F("[menu] 对话框不允许返回"));
#endif
      return false;
    }
// 清理对话框
#ifdef _MENU_DEBUG_SERIAL
    _MENU_DEBUG_SERIAL.println(F("[menu] 清理对话框"));
#endif
    isMsgBox = false;
    msgBox.items = {};
  } else {
    // 普通菜单退出逻辑
    MenuItem* menu = getCurrentMenu();
    if (menu->backCall != nullptr) {
#ifdef _MENU_DEBUG_SERIAL
      _MENU_DEBUG_SERIAL.println(F("[menu] 调用返回回调函数"));
#endif
      if (!menu->backCall(menu)) {
#ifdef _MENU_DEBUG_SERIAL
        _MENU_DEBUG_SERIAL.println(F("[menu] 回调函数返回 false, 不返回上一级菜单"));
#endif
        return false;
      }
    }
    if (menu->clearItemsOnBack) {
#ifdef _MENU_DEBUG_SERIAL
      _MENU_DEBUG_SERIAL.println(F("[menu] 释放菜单项目"));
#endif
      menu->items = {};
    }
  }
  // 弹出选择栈
  if (!popSelectStack()) {
    return false;
  }
  // 递归返回逻辑
  if (level > 1) {
    // 递归时不重复绘制
    return back(false, level - 1);
  }
  // 绘制菜单
  if (needDraw) {
    draw();
  }
  return true;
}

bool MenuNavigator::select(u16_t index) {
  MenuItem* menu = getCurrentMenu();
  // 判断 index 是否存在
  if (index >= menu->items.size()) {
    return false;
  }
  popSelectStack(true);    // 强制弹出当前选择栈
  pushSelectStack(index);  // 压入选择栈
  return true;
}

u16_t MenuNavigator::getSelect(u8_t parent) {
  u8_t n = selectedStack.size();
  // 防止边界溢出
  if (parent >= n) {
    parent = n - 1;
  }
  return selectedStack[n - 1 - parent];
}

void MenuNavigator::showMsgBox(MenuItem msgBox, bool allowBack, bool allowSelect) {
  // 判断当前是否已经显示了一个对话框
  if (this->isMsgBox) {
    // 弹出选择栈
    popSelectStack(true);
  }

  this->isMsgBox = true;                    // 标记对话框
  pushSelectStack(msgBox.defaultSelected);  // 压入选择栈

  this->msgBox = msgBox;
  msgBoxAllowSelect = allowSelect;
  msgBoxAllowBack = allowBack;
  draw();  // 绘制
}

void MenuNavigator::showPromptBox(const uint8_t* iconFont, uint16_t iconEncoding, uint8_t iconW, uint8_t iconH, const char* line1, const char* line2, const char* line3) {
  u8g2->clearBuffer();    // 清除缓冲区
  u8g2->setDrawColor(1);  // 黑底白字
  u8g2->setFontMode(1);   // 透明模式

  // 计算图标底部位置
  uint8_t y = (u8g2->getDisplayHeight() + iconH) / 2;

  u8g2->setFont(iconFont);
  u8g2->drawGlyph(0, y, iconEncoding);
  u8g2->setFontMode(0);  // 还原设置
  u8g2->setFont(font);
  // u8g2->setFont(u8g2_font_wqy13_t_gb2312);

  // 计算文本中心 x
  uint8_t textCenterX = (u8g2->getDisplayWidth() + iconW) / 2;

  if (line3 == nullptr) {
    if (line1 != nullptr) u8g2->drawUTF8(textCenterX - u8g2->getUTF8Width(line1) / 2, 24, line1);
    if (line2 != nullptr) u8g2->drawUTF8(textCenterX - u8g2->getUTF8Width(line2) / 2, 50, line2);
  } else {
    if (line1 != nullptr) u8g2->drawUTF8(textCenterX - u8g2->getUTF8Width(line1) / 2, 16, line1);
    if (line2 != nullptr) u8g2->drawUTF8(textCenterX - u8g2->getUTF8Width(line2) / 2, 40, line2);
    u8g2->drawUTF8(textCenterX - u8g2->getUTF8Width(line3) / 2, 56, line3);
  }
  u8g2->sendBuffer();
}
