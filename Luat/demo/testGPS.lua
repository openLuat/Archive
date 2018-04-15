--- 模块功能：testTask
-- @module test
-- @author openLuat
-- @license MIT
-- @copyright openLuat
-- @release 2017.02.17
require "gps"
require "agps"
module(..., package.seeall)
gps.setup()
gps.open()
-- demo任务演示更新星历，本地如果没有就从网上下载星历，每隔6小时自动更新1次星历。
sys.taskInit(function()
    local upTag = true
    local cnt = 0
    local function writegpd()
        local lbs = agps.getLBS()
        local data = agps.getGPD()
        if not gps.update(data) then log.info("AGPS update-gpd status:", "Error") end
        log.info("AGPS update-gpd status:", "Success")
        agps.refresh(10000)
        agps.cellTrack(10000)
    end
    while true do
        if upTag then writegpd(); upTag = false end
        sys.wait(10)
        print("test:", gps.read())
        cnt = cnt + 1
        if cnt == 6 * 3600 * 100 then upTag = true end
    end
end)
