--- 模块功能：GPS模块管理
-- @module gps
-- @author 稀饭放姜，朱工
-- @license MIT
-- @copyright OpenLuat.com
-- @release 2017.10.23
require "pins"
module(..., package.seeall)
-- GPS模块和单片机通信的串口号，波特率，wake，MCU_TO_GPS,GPS_TO_MCU,ldo对应的IO
local uid, wake, m2g, g2m, ldo = 2
-- GPS模块的工作标志,“off"：关闭，"on"：打开，"update" ：更新星历
local GPS_STATE = "off"
-- NMEA模式
local DATA_MODE_NMEA = "AAF00E0095000000C20100580D0A"
-- BINARY模式
local DATA_MODE_BINARY = "$PGKC149,1,115200*"

--- 打开GPS模块
-- @return 无
-- @usage gps.open()
function open()
    pmd.ldoset(7, pmd.LDO_VCAM)
    if ldo then ldo(1) end
    rtos.sys32k_clk_out(1)
    uart.setup(uid, 115200, 8, uart.PAR_NONE, uart.STOP_1)
    GPS_STATE = "on"
end
--- 关闭GPS模块
-- @return boole,关闭成功返回true，如果正在更新星历，则返回nil
-- @usage gps.close()
function close()
    if GPS_STATE == "update" then return end
    pmd.ldoset(0, pmd.LDO_VCAM)
    if ldo then ldo(0) end
    uart.close(uid)
    rtos.sys32k_clk_out(0)
    GPS_STATE = "off"
    return true
end
--- 重启GPS模块
-- @number r,重启方式-0:外部电源重置; 1:热启动; 2:温启动; 3:冷启动
-- @return 无
-- @usage gps.restart()
function restart(r)
    if r == 0 then
        close()
        open()
    else
        writeCmd("$PGKC030," .. r .. "*")
    end
end
--- 设置GPS模块通信端口
-- @number id,串口号
-- @param w，唤醒GPS模块对应的PIO
-- @param m，mcu发给gps信号的PIO
-- @param g, GSP发给MCU信号的PIO
-- @param vp，GPS的电源供给控制PIO
-- @return 无
-- @usage gps.setup(2,pio.P0_23,pio.P0_22,pio.P0_21,pio.P0_8)
function setup(id, w, m, g, vp)
    uid = id or 2
    wake = pins.setup(w or pio.P0_23, 0)
    m2g = pins.setup(m or pio.P0_22, 0)
    g2m = pins.setup(g or pio.P0_21, 0)
    if vp then ldo = pins.setup(ldo, 0) end
end
--- 阻塞模式读取串口数据，需要线程支持
-- @return 返回以\r\n结尾的一行数据
-- @usage local str = gps.read()
function read()
    local cache_data = ""
    local co = coroutine.running()
    while true do
        local s = uart.read(uid, "*l")
        if s == "" then
            uart.on(uid, "receive", function()coroutine.resume(co) end)
            coroutine.yield()
            uart.on(uid, "receive")
        else
            cache_data = cache_data .. s
            if cache_data:find("\r\n") then return cache_data end
        end
    end
end
--- GPS串口写数据操作
-- @string str,HEX形式的字符串
-- @return 无
-- @usage gps.writeData(str)
function writeData(str)
    local str = str:fromhex()
    uart.write(uid, str)
end

--- GPS串口写命令操作
-- @string cmd,GPS指令(cmd格式："$PGKC149,1,115200*")
-- @return 无
-- @usage gps.writeCmd(cmd)
function writeCmd(cmd)
    local tmp = 0
    for i = 2, cmd:len() - 1 do
        tmp = bit.bxor(tmp, cmd:byte(i))
    end
    tmp = cmd .. string.upper(string.format("%02X", tmp)) .. "\r\n"
    uart.write(uid, tmp)
    log.info("gps.writecmd", tmp)
end

--- 更新星历到GPS模块
-- @string data,星历的十六进制字符表示字符串数据
-- @return boole, 成功返回true，失败返回nil
-- @usage gps.update(data)
function update(data)
    local tmp = ""
    if not data or GPS_STATE == "off" then return end
    GPS_STATE = "update"
    read()
    local function hexCheckSum(str)
        local sum = 0
        for i = 5, str:len(), 2 do
            sum = bit.bxor(sum, tonumber(str:sub(i, i + 1), 16))
        end
        return string.upper(string.format("%02X", sum))
    end
    -- 等待切换到BINARY模式
    writeCmd(DATA_MODE_BINARY)
    while tmp ~= "AAF00C0001009500039B0D0A" do tmp = read():tohex() end
    log.info("gps.update gpd_start:", tmp)
    -- while read():tohex() ~= "AAF00C0001009500039B0D0A" do end
    -- 写入星历数据
    local cnt = 0 -- 包序号
    for i = 1, #data, 1024 do
        tmp = data:sub(i, i + 1023)
        if tmp:len() < 1024 then tmp = tmp .. ("F"):rep(1024 - tmp:len()) end
        tmp = "AAF00B026602" .. string.format("%04X", cnt):upper() .. tmp
        tmp = tmp .. hexCheckSum(tmp) .. "0D0A"
        log.info("gps.update gpd_send:", tmp)
        writeData(tmp)
        for j = 1, 30 do
            local ack, len = read():tohex()
            log.info("gps.update send_ack:", ack, len)
            if len == 12 or ack:find("AAF00C0003") then break end
            if j == 30 then writeData("aaf00e0095000000c20100580d0a") return end
        end
        cnt = cnt + 1
    end
    -- 发送GPD传送结束语句
    writeData("aaf00b006602ffff6f0d0a")
    -- 切换为NMEA接收模式
    writeData("aaf00e0095000000c20100580d0a")
    GPS_STATE = "on"
    return true
end

--- 设置搜星模式
-- @number gps,GPS定位系统，1是打开，0是关闭
-- @number glonass,俄罗斯Glonass定位系统，1是打开，0是关闭
-- @number beidou,中国北斗定位系统，1是打开，0是关闭
-- @number galieo，Intel 欧盟伽利略定位系统，1是打开0是关闭
-- @return 无
-- @usage gps.setAeriaMode(0)
function setAerialMode(gps, glonass, beidou, galieo)
    local gps = gps or 0
    local glonass = glonass or 0
    local beidou = beidou or 0
    local galieo = galieo or 0
    if gps + glonass + beidou + galieo == 0 then gps = 1; beidou = 1 end
    writeCmd("$PGKC115," .. gps .. "," .. glonass .. "," .. beidou .. "," .. galieo .. "*")
end

--- 秒定位命令
-- @string lat,参考坐标纬度
-- @string lng,参考坐标经度
-- @return 无
-- @usage gps.fastFix(data)
function fastFix(lat, lng)
    local t = os.date("!*t")
    t = t.year .. "," .. t.month .. "," .. t.day .. "," .. t.hour .. "," .. t.min .. "," .. t.sec .. "*"
    writeCmd("$PGKC634," .. t)
    writeCmd("$PGKC635," .. lat .. lng .. 0 .. t)
end

---GPS模块的运行功耗模式
-- @number mod,“0”，正常运行模式
-- “1”，周期超低功耗跟踪模式
-- “2”，周期低功耗模式
-- “4”，直接进入超低功耗跟踪模式
-- “8”，自动低功耗模式，可以通过串口唤醒
-- “9”, 自动超低功耗跟踪模式，需要force on来唤醒
-- @number rt,模式1和2的运行时间(ms),模式0的上报间隔
-- @number st,模式1和2的睡眠时间(ms)
-- @return 无
-- @usage gps.runMode()
function runMode(mod, rt, st)
    local rt, st = rt or "", st or ""
    if mod == 0 and rt then
        if rt > 10000 then rt = 10000 end
        if rt < 200 then rt = 200 end
        writeCmd("$PGKC101," .. rt .. "*")
    end
    if mod == 1 or mod == 2 then
        writeCmd("$PGKC105," .. mod .. "," .. rt .. "," .. st .. "*")
    else
        writeCmd("$PGKC105," .. mod .. "*")
    end
end

function wakeup()
    wake(1)
end
