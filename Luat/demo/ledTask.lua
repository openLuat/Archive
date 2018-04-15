--- 模块功能：testLed
-- @module ledTask
-- @author openLuat
-- @license MIT
-- @copyright openLuat
-- @release 2018.02.04
require "led"
require "pins"
module(..., package.seeall)
--- 闪烁灯任务
-- @return 无
-- @usage 每10秒自动切换到下一种指示灯状态
-- @usage 1、正常闪烁 500ms 亮灭
-- @usage 2、心跳灯,100ms亮，1.5秒灭
-- @usage 3、等级指示灯 闪4次，间隔1秒
-- @usage 4、呼吸灯
function ledTaskDemo()
    local ledpin = pins.setup(pio.P1_1, 0)
    while true do
        -- 正常闪烁指示灯 500ms亮 500ms灭
        for i = 1, 10 do
            led.blinkPwm(ledpin, 500, 500)
            sys.wait(1)
        end
        -- 心跳灯
        for i = 1, 10 do
            led.blinkPwm(ledpin, 100, 1500)
            sys.wait(1)
        end
        -- 等级指示灯
        for i = 1, 10 do
            led.leveled(ledpin, 250, 250, 4, 1000)
            sys.wait(1)
        end
         -- 呼吸灯
        for i = 1, 10 do
            led.breateLed(ledpin)
            sys.wait(1)
        end     
    end
end
sys.taskInit(ledTaskDemo)

