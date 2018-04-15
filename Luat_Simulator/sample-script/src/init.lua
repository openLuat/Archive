--[[**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    init.lua
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          The entry of 'Luat_simulator'.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************]]

print("[AMWatch Debug-Engine] Init...")

local function GetShellPath()
  local t=io.popen("cd")
  local path=t:read("*all"):sub(1,-2)
  t:close()
  
  return path
end

local function GetProcessorArch()
  local t=io.popen("set PROCESSOR_ARCHITECTURE")
  local s=t:read("*all"):sub(1,-2)
  t:close()
  
  local arch = string.match(s,"=(.+)")
  
  return arch
end

local shellRoot = GetShellPath()
local arch = GetProcessorArch()
--Win32_lib,x64_lib
local os_type = "dummy_lib"
if arch == "AMD64" then
  os_type="x64_lib"
elseif arch == "x86" then
  os_type="Win32_lib"
else
  os_type="dummy_lib"
end

if os_type=="dummy_lib" then
  print("[AMWatch Debug-Engine] Init FAIL! ",arch," is invalid processor-architecture.")
  return
end

local mAMWatchDllPath = string.format("%s/%s/AMWatchDll.dll",GetShellPath(),os_type)
print("mAMWatchDllPath=",mAMWatchDllPath)
local mAMWatchDll = package.loadlib(mAMWatchDllPath, "luaopen_AMWatch")

if mAMWatchDll then
  local mAMWatchTable = mAMWatchDll() --Throw exception.

  _G.print = function(...) return LuaShell.print(...) end

  _G.io.makedir = function(...) return LuaShell.makedir(...) end
  _G.io.findfirstdir = function(...) return LuaShell.findfirstdir(...) end
  _G.io.findfirst = function(...) return LuaShell.findfirst(...) end
  _G.io.findnext = function(...) return LuaShell.findnext(...) end
  _G.io.findclose = function(...) return LuaShell.findclose(...) end  
  
  print("mAMWatchDll ",mAMWatchTable)
  local luaRoot = shellRoot.."\\src\\lua\\?.lua;"
  
  package.path = string.format("%s/src/?.lua;%s/src/lua/?.lua",shellRoot,shellRoot)
  package.cpath = string.format("%s/src/lib/?.so;%s/src/lib/?.lib",shellRoot,shellRoot)
  
  print("Lua Version: ",_G._VERSION)
  print("LuaT Version ",LuaShell._VERSION)
  LuaShell.SetRoot(shellRoot)
else
  print("Can't load AMWatchDll!")
  return
end

function win32luamain()
  local luamain
  print("[AMWatch Debug-Engine] Loading LUA Shell...")
  luamain = require "main"
  print("[AMWatch Debug-Engine] Loading LUA Shell ... OK!")
  return luamain
end

LuaShell.MainLoop()

print("[AMWatch Debug-Engine] Exit...")