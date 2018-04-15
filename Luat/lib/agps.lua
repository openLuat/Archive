--- 模块功能：星历更新服务
-- @module agps
-- @author 稀饭放姜
-- @license MIT
-- @copyright OpenLuat.com
-- @release 2017.10.23
require "http"
require "net"
module(..., package.seeall)
-- 星历数据本地文件名
local GPD_FILE = "/GPD.txt"
local LBS_FILE = "/LBS.txt"
local GPD_URL = "http://download.openluat.com/9501-xingli/brdcGPD.dat_rda"
local LBS_URL = "http://api.openluat.com/iot/cell_location"
local LBS_KEY = "zLakaGugFktk2sgZ"
local LBS_SECRET = "BKW3tSMv0YBqomUlqen3DjyIbg0hvuj6P3EHFeRKCBmVkVjDYy7NQcIcWw1rTd9C"
--- 设置基站定位需要登陆的服务器，验证账号和密码
-- @string host,LBS基站定位服务器地址
-- @string key, HTTP Basic Authorization 认证用户名
-- @string secret,HTTP Basic Authorization 认证密码
-- @return 无
-- @usage agps.LBS_Setup("api.openluat.com","user","123345")
function LBS_Setup(host, key, secret)
    LBS_HOST = host or LBS_HOST
    LBS_KEY = key or LBS_KEY
    LBS_SECRET = secret or LBS_SECRET
end
--- 下载星历数据
-- @number timeout,下载星历超时等待时间
-- @return string,星历数据的HEX字符串
-- @usage agps.refresh(30000)
function refresh(timeout)
    while not socket.isReady() do sys.wait(1000) end
    local code, head, data = http.request("GET", GPD_URL, timeout)
    if code == "200" then
        local data, len = data:tohex()
        log.info("agps.gpd length,file:", len, io.writefile(GPD_FILE, data))
        return data
    end
end
--- 获取星历数据
-- @return string,星历数据的HEX字符串
-- @usage agps.getGPD()
function getGPD()
    return io.readfile(GPD_FILE) or refresh(30000)
end
--- 下载基站坐标
-- @number timeout，下载基站信息超时等待时间
-- @return string,基站坐标字符串,基站没准备好返回nil
-- @usage agps.cellTrack()
function cellTrack(timeout)
    local ct, info = {['cell'] = {}}
    while not socket.isReady() do sys.wait(1000) end
    info = net.getCellInfoExt()
    if info == "" then return end
    for mcc, mnc, lac, ci, rssi in info:gmatch("(%d+)%.(%d+)%.(%d+)%.(%d+)%.(%d+);") do
        local tmp = {}
        tmp.mcc = tonumber(mcc)
        tmp.mnc = tonumber(mnc)
        tmp.lac = tonumber(lac)
        tmp.ci = tonumber(ci)
        tmp.hex = "10"
        tmp.csq = (tonumber(rssi) > 31) and 31 or tonumber(rssi)
        table.insert(ct.cell, tmp)
    end
    ct.lng_lat_format = "string"
    -- 发送请求报文
    local code, head, data = http.request("POST", LBS_URL, timeout, nil, ct, 2, LBS_KEY .. ":" .. LBS_SECRET)
    if code == "200" then
        -- data = json.decode(data)
        log.info("agps.lbs length,file:", io.writefile(LBS_FILE, data))
        return data
    end
end
--- 获取基站坐标
-- @return string,基站定位的坐标字符串
-- @usage agps.getLBS()
function getLBS()
    return io.readfile(LBS_FILE) or cellTrack(30000)
end
