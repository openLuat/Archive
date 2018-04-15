---模块功能：通用库函数、编码格式转换、时区时间转换
-- @module common
-- @author zhutianhua
-- @license MIT
-- @copyright OpenLuat.com
-- @release 2017.02.20
--定义模块,导入依赖库
module(..., package.seeall)

--加载常用的全局函数至本地
local tinsert, ssub, sbyte, schar, sformat, slen = table.insert, string.sub, string.byte, string.char, string.format, string.len

--- ascii字符串的unicode编码的16进制字符串 转化为 ascii字符串
-- @string,inum：待转换字符串
-- @return string,转换后的字符串
-- @usage local str = common.ucs2toascii("0031003200330034")
-- @usage str is "1234"
function ucs2toascii(inum)
    local tonum = {}
    for i = 1, slen(inum), 4 do
        tinsert(tonum, tonumber(ssub(inum, i, i + 3), 16) % 256)
    end
    return schar(unpack(tonum))
end
--- ascii字符串 转化为 ascii字符串的unicode编码的16进制字符串(仅支持数字和+)
-- @string,inum：待转换字符串
-- @return string,转换后的字符串
-- @usage local str = common.nstrToUcs2Hex("+1234")
-- @usage str is "002B0031003200330034"
function nstrToUcs2Hex(inum)
    local hexs = ""
    local elem = ""
    for i = 1, slen(inum) do
        elem = ssub(inum, i, i)
        if elem == "+" then
            hexs = hexs .. "002B"
        else
            hexs = hexs .. "003" .. elem
        end
    end
    return hexs
end
--- ASCII字符串 转化为 BCD编码格式字符串(仅支持数字和+)
-- @string,num：待转换字符串
-- @return string,转换后的字符串
-- @usage local str = common.numtobcdnum("+8618126324567")
-- @usage str is "91688121364265f7" （表示第1个字节是0x91，第2个字节为0x68，......）
function numtobcdnum(num)
    local len, numfix, convnum = slen(num), "81", ""
    
    if ssub(num, 1, 1) == "+" then
        numfix = "91"
        len = len - 1
        num = ssub(num, 2, -1)
    end
    
    if len % 2 ~= 0 then --奇数位
        for i = 1, len / 2 do
            convnum = convnum .. ssub(num, i * 2, i * 2) .. ssub(num, i * 2 - 1, i * 2 - 1)
        end
        convnum = convnum .. "F" .. ssub(num, len, len)
    else --偶数位
        for i = 1, len / 2 do
            convnum = convnum .. ssub(num, i * 2, i * 2) .. ssub(num, i * 2 - 1, i * 2 - 1)
        end
    end
    
    return numfix .. convnum
end
--- BCD编码格式字符串 转化为 号码ASCII字符串(仅支持数字和+)
-- @string,num：待转换字符串
-- @return string,转换后的字符串
-- @usage local str = common.bcdnumtonum("91688121364265f7") --表示第1个字节是0x91，第2个字节为0x68，......
-- @usage str is "+8618126324567"
function bcdnumtonum(num)
    local len, numfix, convnum = slen(num), "", ""
    
    if len % 2 ~= 0 then
        print("your bcdnum is err " .. num)
        return
    end
    
    if ssub(num, 1, 2) == "91" then
        numfix = "+"
    end
    
    len, num = len - 2, ssub(num, 3, -1)
    
    for i = 1, len / 2 do
        convnum = convnum .. ssub(num, i * 2, i * 2) .. ssub(num, i * 2 - 1, i * 2 - 1)
    end
    
    if ssub(convnum, len, len) == "f" or ssub(convnum, len, len) == "F" then
        convnum = ssub(convnum, 1, -2)
    end
    
    return numfix .. convnum
end
--- unicode小端编码 转化为 gb2312编码
-- @param ucs2s,unicode小端编码数据
-- @return param,gb2312编码数据
-- @usage local gb = common.ucs2togb2312(ucs2s)
function ucs2togb2312(ucs2s)
    local cd = iconv.open("gb2312", "ucs2")
    return cd:iconv(ucs2s)
end

--- gb2312编码 转化为 unicode小端编码
-- @param gb2312s,gb2312编码数据
-- @return param,unicode小端编码数据
-- @usage local ucs = common.gb2312toucs2(gb2312s)
function gb2312toucs2(gb2312s)
    local cd = iconv.open("ucs2", "gb2312")
    return cd:iconv(gb2312s)
end

--- unicode大端编码 转化为 gb2312编码
-- @param ucs2s,unicode大端编码数据
-- @return param ,gb2312编码数据
-- @usage gb = common.ucs2betogb2312(ucs2s)
function ucs2betogb2312(ucs2s)
    local cd = iconv.open("gb2312", "ucs2be")
    return cd:iconv(ucs2s)
end

--- gb2312编码 转化为 unicode大端编码
-- @param gb2312s,gb2312编码数据
-- @return param,unicode大端编码数据
-- @usage local ucs = common.gb2312toucs2be(gb2312s)
function gb2312toucs2be(gb2312s)
    local cd = iconv.open("ucs2be", "gb2312")
    return cd:iconv(gb2312s)
end

--- unicode小端编码 转化为 utf8编码
-- @param ucs2s,unicode小端编码数据
-- @return param ,utf8编码数据
-- @usage u8 = common.ucs2toutf8(ucs2s)
function ucs2toutf8(ucs2s)
    local cd = iconv.open("utf8", "ucs2")
    return cd:iconv(ucs2s)
end

--- utf8编码 转化为 unicode小端编码
-- @param utf8s,utf8编码数据
-- @return param,unicode小端编码数据
-- @usage local ucs = common.utf8toucs2(utf8s)
function utf8toucs2(utf8s)
    local cd = iconv.open("ucs2", "utf8")
    return cd:iconv(utf8s)
end

--- unicode大端编码 转化为 utf8编码
-- @param ucs2s,unicode大端编码数据
-- @return param ,utf8编码数据
-- @usage u8 = common.ucs2betoutf8(ucs2s)
function ucs2betoutf8(ucs2s)
    local cd = iconv.open("utf8", "ucs2be")
    return cd:iconv(ucs2s)
end

--- utf8编码 转化为 unicode大端编码
-- @param utf8s,utf8编码数据
-- @return param,unicode大端编码数据
-- @usage local ucs = common.utf8toucs2be(utf8s)
function utf8toucs2be(utf8s)
    local cd = iconv.open("ucs2be", "utf8")
    return cd:iconv(utf8s)
end

--- utf8编码 转化为 gb2312编码
-- @param utf8s,utf8编码数据
-- @return param,gb2312编码数据
-- @usage local gb = common.utf8togb2312(utf8s)
function utf8togb2312(utf8s)
    local cd = iconv.open("ucs2", "utf8")
    local ucs2s = cd:iconv(utf8s)
    cd = iconv.open("gb2312", "ucs2")
    return cd:iconv(ucs2s)
end

--- gb2312编码 转化为 utf8编码
-- @param gb2312s,gb2312编码数据
-- @return param ,utf8编码数据
-- @usage local u8 = common.gb2312toutf8(gb2312s)
function gb2312toutf8(gb2312s)
    local cd = iconv.open("ucs2", "gb2312")
    local ucs2s = cd:iconv(gb2312s)
    cd = iconv.open("utf8", "ucs2")
    return cd:iconv(ucs2s)
end

local function timeAddzone(y, m, d, hh, mm, ss, zone)
    if not y or not m or not d or not hh or not mm or not ss then
        return
    end
    hh = hh + zone
    if hh >= 24 then
        hh = hh - 24
        d = d + 1
        if m == 4 or m == 6 or m == 9 or m == 11 then
            if d > 30 then
                d = 1
                m = m + 1
            end
        elseif m == 1 or m == 3 or m == 5 or m == 7 or m == 8 or m == 10 then
            if d > 31 then
                d = 1
                m = m + 1
            end
        elseif m == 12 then
            if d > 31 then
                d = 1
                m = 1
                y = y + 1
            end
        elseif m == 2 then
            if (((y + 2000) % 400) == 0) or (((y + 2000) % 4 == 0) and ((y + 2000) % 100 ~= 0)) then
                if d > 29 then
                    d = 1
                    m = 3
                end
            else
                if d > 28 then
                    d = 1
                    m = 3
                end
            end
        end
    end
    local t = {}
    t.year, t.month, t.day, t.hour, t.min, t.sec = y, m, d, hh, mm, ss
    return t
end
local function timeRmozone(y, m, d, hh, mm, ss, zone)
    if not y or not m or not d or not hh or not mm or not ss then
        return
    end
    hh = hh + zone
    if hh < 0 then
        hh = hh + 24
        d = d - 1
        if m == 2 or m == 4 or m == 6 or m == 8 or m == 9 or m == 11 then
            if d < 1 then
                d = 31
                m = m - 1
            end
        elseif m == 5 or m == 7 or m == 10 or m == 12 then
            if d < 1 then
                d = 30
                m = m - 1
            end
        elseif m == 1 then
            if d < 1 then
                d = 31
                m = 12
                y = y - 1
            end
        elseif m == 3 then
            if (((y + 2000) % 400) == 0) or (((y + 2000) % 4 == 0) and ((y + 2000) % 100 ~= 0)) then
                if d < 1 then
                    d = 29
                    m = 2
                end
            else
                if d < 1 then
                    d = 28
                    m = 2
                end
            end
        end
    end
    local t = {}
    t.year, t.month, t.day, t.hour, t.min, t.sec = y, m, d, hh, mm, ss
    return t
end

--- 当前时区的时间转换为新时区的时间
-- @param y,当前时区年份
-- @param m,当前时区月份
-- @param d,当前时区天
-- @param hh,当前时区小时
-- @param mm,当前时区分
-- @param ss,当前时区秒
-- @param pretimezone,当前时区
-- @param nowtimezone,新时区
-- @return table ,返回新时区对应的时间，table格式{year,month.day,hour,min,sec}
function transftimezone(y, m, d, hh, mm, ss, pretimezone, nowtimezone)
    local t = {}
    local zone = nil
    zone = nowtimezone - pretimezone
    
    if zone >= 0 and zone < 23 then
        t = timeAddzone(y, m, d, hh, mm, ss, zone)
    elseif zone < 0 and zone >= -24 then
        t = timeRmozone(y, m, d, hh, mm, ss, zone)
    end
    return t
end
