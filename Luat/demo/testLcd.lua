--- MenuTitle Demo
-- @module testLcd
-- @author hotdll
-- @license MIT
-- @copyright openLuat
-- @release 2017.10.17
require "ui"
local newList = ui.newList
module(..., package.seeall)
local menuBar = {"menu", "set", "message", "alarm", "device", "help", "mange", "test", "user", }
local menuItems = {"menu菜单2级1", "menu菜单2级2", "menu菜单2级3", "menu菜单2级4", "menu菜单2级5", "menu菜单2级6"}
local setItems = {"set菜单2级1", "set菜单2级2", "set菜单2级3", "set菜单2级4", "set菜单2级5", "set菜单2级6"}
local msgItems = {"msg菜单2级1", "msg菜单2级2", "msg菜单2级3", "msg菜单2级4", "msg菜单2级5", "msg菜单2级6"}
local alarmItems = {"alarm菜单2级1", "alarm菜单2级2", "alarm菜单2级3", "alarm菜单2级4", "alarm菜单2级5", "alarm菜单2级6"}
local deviceItems = {"device菜单2级1", "device菜单2级2", "device菜单2级3", "device菜单2级4", "device菜单2级5", "device菜单2级6"}
local helpItems = {"help菜单2级1", "help菜单2级2", "help菜单2级3", "help菜单2级4", "help菜单2级5", "help菜单2级6"}
local mangeItems = {"mange菜单2级1", "mange菜单2级2", "mange菜单2级3", "mange菜单2级4", "mange菜单2级5", "mange菜单2级6"}
local testItems = {"test菜单2级1", "test菜单2级2", "test菜单2级3", "test菜单2级4", "test菜单2级5", "test菜单2级6"}
local userItems = {"user菜单2级1", "user菜单2级2", "user菜单2级3", "user菜单2级4", "user菜单2级5", "user菜单2级6"}
local userItems2 = {"user菜单3级1", "user菜单3级2", "user菜单3级3", "user菜单3级4", "user菜单3级5", "user菜单3级6"}

local rootMenu = newList(menuBar)
local menuItem = newList(menuItems, true)
local setItem = newList(setItems, true)
local msgItem = newList(msgItems, true)
local alarmItem = newList(alarmItems, true)
local deviceItem = newList(deviceItems, true)
local helpItem = newList(helpItems, true)
local mangeItem = newList(mangeItems, true)
local testItem = newList(testItems, true)
local userItem = newList(userItems, true)
local userItem2 = newList(userItems2, true)

userItem.append(userItems[1], userItem2)

rootMenu.append(menuBar[1], menuItem)
rootMenu.append(menuBar[2], setItem)
rootMenu.append(menuBar[3], msgItem)
rootMenu.append(menuBar[4], alarmItem)
rootMenu.append(menuBar[5], deviceItem)
rootMenu.append(menuBar[6], helpItem)
rootMenu.append(menuBar[7], mangeItem)
rootMenu.append(menuBar[8], testItem)
rootMenu.append(menuBar[9], userItem)

rootMenu.display()
