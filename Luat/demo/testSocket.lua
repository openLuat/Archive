--- testSocket
-- @module testSocket
-- @author ??
-- @license MIT
-- @copyright openLuat.com
-- @release 2017.9.27
require "socket"
module(..., package.seeall)
local wdgcnt = 60
-- tcp test
sys.taskInit(function()
    local r, s
    
    while true do
        while not socket.isReady() do sys.wait(1000) end
        local c = socket.tcp()
        while not c:connect("120.27.222.26", 60000) do
            sys.wait(2000)
        end
        while true do
            r, s = c:recv(1000)
            if r then
                log.info("test.socket.tcp: recv", s)
                if not c:send(s) then
                    break
                end
            elseif r == 1000 then
                wdgcnt = 60
            end

        end
        c:close()
    end
end)

-- wdg
sys.taskInit(function()
    while true do
        wdgcnt = wdgcnt - 1
        if wdgcnt <= 0 then 
            sys.restart("我重启了！")
        end
        sys.wait(1000)
    end
end)
