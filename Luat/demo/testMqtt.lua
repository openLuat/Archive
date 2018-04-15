--- testMqtt
-- @module testMqtt
-- @author ??
-- @license MIT
-- @copyright openLuat.com
-- @release 2017.10.24
require "mqtt"

sys.taskInit(function()
    while not socket.isReady() do sys.wait(1000) end
    local mqttc = mqtt.client(misc.getimei())
    while true do
        while not mqttc:connect("180.97.80.55", 1883) do
            sys.wait(2000)
        end
        if mqttc:subscribe(string.format("/device/%s/req", misc.getimei())) then
            local count = 0
            
            if mqttc:publish(string.format("/device/%s/report", misc.getimei()), "test publish " .. count) then
                while true do
                    local r, data = mqttc:receive(2000)
                    if r then
                        r = mqttc:publish(string.format("/device/%s/resp", misc.getimei()), "response " .. data.payload)
                    elseif data == "timeout" then
                        count = count + 1
                        r = mqttc:publish(string.format("/device/%s/report", misc.getimei()), "test publish " .. count, count%3)
                        log.info('testMqtt.publish', count, count%3, r)
                    end
                    if not r then
                        break
                    end
                end
            end
        end
        mqttc:disconnect()
    end
end)
