--- 模块功能：GPIO 功能配置，包括输入输出IO和上升下降沿中断IO
-- @module pins
-- @author 稀饭放姜
-- @license MIT
-- @copyright openLuat
-- @release 2017.09.23 11:34
module(..., package.seeall)
local interruptCallbacks = {}

--- 自适应GPIO模式
-- @param pin ，参数为pio.P0_1-31 和 pio_P1_1-31 (IO >= 32 and IO - 31)
-- @param val，输出模式默认电平：0 是低电平1是高电平，中断模式为回调函数
-- @return function ,返回一个函数，该函数接受一个参数用来设置IO的电平
-- @return nil,中断模式捆绑回调函数返回空
-- @usage pins.setup(pio.P1_1,0,key_int) ，配置Key的IO为pio.32,中断模式,自动触发key_int(msg)函数
-- @usage led = pins.setup(pio.P1_1,0) ,配置LED脚的IO为pio.32，输出模式，默认输出低电平。led(1)即可输出高电平
-- @usage key = pins.setup(pio.P1_1),配置key的IO为pio.32，输入模式,用key()即可获得当前电平
function setup(pin, val)
    -- 关闭该IO
    pio.pin.close(pin)
    -- 中断模式配置
    if type(val) == "function" then
        pio.pin.setdir(pio.INT, pin)
        --注册引脚中断的处理函数
        interruptCallbacks[pin] = val
        return
    end
    -- 输出模式初始化默认配置
    if val ~= nil then
        pio.pin.setdir(pio.OUTPUT, pin)
        pio.pin.setval(val, pin)
    -- 输入模式初始化默认配置
    else
        pio.pin.setdir(pio.INPUT, pin)
    end
    -- 返回一个自动切换输入输出模式的函数
    return function(val)
        pio.pin.close(pin)
        if val ~= nil then
            pio.pin.setdir(pio.OUTPUT, pin)
            pio.pin.setval(val, pin)
        else
            pio.pin.setdir(pio.INPUT, pin)
            return pio.pin.getval(pin)
        end
    end
end

rtos.on(rtos.MSG_INT, function(msg)
    if interruptCallbacks[msg.int_resnum] == nil then
        log.warn('pins.rtos.on', 'warning:rtos.MSG_INT callback nil', msg.int_resnum)
    end
    interruptCallbacks[msg.int_resnum](msg.int_id)
end)
