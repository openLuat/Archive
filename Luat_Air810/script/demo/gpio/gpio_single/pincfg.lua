require"pins"
module(...,package.seeall)

--如下配置了开源模块中所有可用作GPIO的引脚，每个配置只是演示需要
--用户最终需根据自己的需求自行修改

--pin值定义如下：
--pio.P0_XX：表示GPIOXX，例如pio.P0_15，表示GPIO15
--pio.P1_XX：表示GPIO(XX+32)，例如pio.P1_8，表示GPIO40

--dir值定义如下（默认值为pio.OUTPUT）：
--pio.OUTPUT：表示输出，初始化是输出低电平
--pio.OUTPUT1：表示输出，初始化是输出高电平
--pio.INPUT：表示输入，需要轮询输入的电平状态
--pio.INT：表示中断，电平状态发生变化时会上报消息，进入本模块的intmsg函数

--valid值定义如下（默认值为1）：
--valid的值跟pins.lua中的set、get接口配合使用
--dir为输出时，配合pins.set接口使用，pins.set的第一个参数如果为true，则会输出valid值表示的电平，0表示低电平，1表示高电平
--dir为输入或中断时，配合get接口使用，如果引脚的电平和valid的值一致，get接口返回true；否则返回false
--dir为中断时，cb为中断引脚的回调函数，有中断产生时，如果配置了cb，会调用cb，如果产生中断的电平和valid的值相同，则cb(true)，否则cb(false)

--等价于PIN31 = {pin=pio.P0_10,dir=pio.OUTPUT,valid=1}
--第31个引脚：GPIO54，配置为输出，初始化输出低电平；valid=1，调用pins.set(true,PIN31),则输出高电平，调用pins.set(false,PIN31),则输出低电平
PIN31 = {pin=pio.P1_22}

--第32个引脚：GPIO55；配置为输出，初始化输出高电平；valid=0，调用pins.set(true,PIN32),则输出低电平，调用pins.set(false,PIN32),则输出高电平
PIN32 = {pin=pio.P1_23,dir=pio.OUTPUT1,valid=0}

--如下三个配置含义和PIN31相似
PIN28 = {pin=pio.P0_16}
PIN27 = {pin=pio.P0_31}
PIN26 = {pin=pio.P1_1}


local function pin25cb(v)
	print("pin25cb",v)
end
--第25个引脚：GPIO36；配置为中断；valid=1
--intcb表示中断管脚的中断处理函数，产生中断时，如果为高电平，则回调intcb(true)；如果为低电平，则回调intcb(false)
--调用get(PIN25)时，如果为高电平，则返回true；如果为低电平，则返回false
PIN25 = {name="PIN25",pin=pio.P1_4,dir=pio.INT,valid=1,intcb=pin25cb}

--第38个引脚：GPIO21；配置为输入；valid=0
--调用get(PIN38)时，如果为高电平，则返回false；如果为低电平，则返回true
PIN38 = {pin=pio.P0_21,dir=pio.INPUT,valid=0}

--如下三个配置含义和PIN31相似
PIN39 = {pin=pio.P0_4}
PIN40 = {pin=pio.P0_2}
PIN46 = {pin=pio.P0_14}
PIN47 = {pin=pio.P0_15}

pins.reg(PIN31,PIN32,PIN28,PIN27,PIN26,PIN25,PIN38,PIN39,PIN40,PIN46,PIN47)

