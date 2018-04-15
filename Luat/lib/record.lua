--- 录音处理
-- @module record
-- @author 朱天华、小强
-- @license MIT
-- @copyright openLuat.com
-- @release 2017.11.23

require "log"
require "ril"
module(..., package.seeall)

local ID, FILE = 1, '/RecDir/rec001'
local recording
local stoping
local duration
local recordCallback

--- record.getdata 读取录音数据
-- @param offset 偏移位置
-- @param len 长度
-- @return data 录音数据
-- @usage data = record.getdata(0, 1024)
function getdata(offset, len)
    local f = io.open(FILE, "rb")
    if not f then log.error('record.getdata', 'open failed') return "" end
    if not f:seek("set", offset) then log.error('record.getdata', 'seek failed') f:close() return "" end
    local data = f:read(len)
    f:close()
    log.info("record.getdata", data and data:len() or 0)
    return data or ""
end

--- record.getsize 读取录音文件总长度，录音时长
-- @return filesize 录音文件大小
-- @return duration 录音时长
-- @usage filesize, duration = record.getsize()
function getsize()
    return io.filesize(FILE), duration and (duration-1)/1000+1 or 0
end

--- record.delete 删除录音
-- @usage record.delete()
function delete()
    os.remove(FILE)
end

--- record.exists 判断是否存在录音
-- @return result true - 有录音 false - 无录音
-- @usage result = record.exists()
function exists()
    return io.exists(FILE)
end

--- record.isBusy 是否正在处理录音
-- @return result true - 正在处理 false - 空闲
-- @usage result = record.isBusy()
function isBusy()
    return recording or stoping
end

--- record.start 开始录音
-- @param seconds 录音时长，单位：秒
-- @param cb 录音结果回调
-- @return result true - 开始录音 其他 - 失败
-- @usage result = record.start()
function start(seconds, cb)
    if recording or stoping or seconds <= 0 or seconds > 50 then
        log.error('record.start', recording, stoping, seconds)
        return
    end
    delete()
    duration = seconds * 1000
    ril.request("AT+AUDREC=0,0,1," .. ID .. "," .. duration)
    recording = true
    recordCallback = cb
    return true
end

--- record.stop 停止录音
-- @usage record.stop()
function stop()
    if not recording or stoping then return end
    ril.request("AT+AUDREC=0,0,0," .. ID .. "," .. duration)
    stoping = true
end

ril.regurc("+AUDREC", function(data)
    local action, size = data:match("(%d),(%d+)")
    if action and size then
        size = tonumber(size)
        if action == "1" then
            local result = size > 0 and recording
            if not result then os.remove(FILE) size = 0 end
            duration = size
            if recordCallback then recordCallback(result, size) recordCallback = nil end
            recording = false
            stoping = false
        end
    end
end)
ril.regrsp("+AUDREC", function(cmd, success)
    local action = cmd:match("AUDREC=%d,%d,(%d)")
    if action == "1" then
        if not success then
            if recordCallback then
                recordCallback(false, 0)
                recordCallback = nil
            end
            recording = false
        end
    elseif action == '0' then
        if stoping and not success then stoping = false end -- 失败直接结束，成功则等到+AUDREC上报才判定停止录音成功
    end
end)
