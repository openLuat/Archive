---模块名称：SH1106驱动芯片命令配置
-- @module SH1106
-- @author 稀饭放姜
-- @license MIT
-- @copyright openLuat
-- @release 2017.10.17
module(..., package.seeall)
--[[
注意：此文件的配置，硬件上使用的是LCD专用的SPI引脚，不是标准的SPI引脚
disp库目前仅支持SPI接口的屏，硬件连线图如下：
Air模块			LCD
GND             地
LCD_CS          片选
LCD_CLK         时钟
LCD_DATA        数据
LCD_DC          数据/命令选择
VDDIO           电源
LCD_RST         复位
]]
--- 显示屏驱动初始化
-- @param rst, LCD复位引脚对应pio
-- @param dc, LCD 数据和指令引脚对应pio
-- @param cs, LCD 片选 对应pio
-- @param bus, LCD 使用的总线接口
-- @return 无
-- @usage init(pio.P0_28,pio.P0_29,pio.P0_30)
function init(rst, dc, cs, bus)
    local para = {
        width = 128, --分辨率宽度，128像素；用户根据屏的参数自行修改
        height = 64, --分辨率高度，64像素；用户根据屏的参数自行修改
        bpp = 1, --位深度，1表示单色。单色屏就设置为1，不可修改
        bus = bus or disp.BUS_SPI4LINE, --LCD专用SPI引脚接口，不可修改
        xoffset = 2, --Y轴偏移
        hwfillcolor = 0xFFFF, --填充色，黑色
        pinrst = rst or pio.P0_14, --reset，复位引脚
        pinrs = dc or pio.P0_18, --rs，命令/数据选择引脚
        pincs = cs or pio.p0_15, -- cs ，片选引脚
        --初始化命令
        initcmd = {
            0xAE, -- 关闭显示面板
            0x02, -- 设置列地址低4位
            0x10, -- 设置列地址高4位
            0x40, -- 设置显示开始行 Set Mapping RAM Display Start Line (0x00~0x3F)
            0x81, -- 设置对比度控制模式
            0xCF, -- 设置对比数寄存器(亮度电流大小)
            0xA1, -- Set SEG/Column Mapping     0xa0 从右到左 0xa1从左到右
            0xC8, -- Set COM/Row Scan Direction   0xc0 从下到上 0xc8 从上到下
            0xA6, -- set normal display
            0xA8, -- set multiplex ratio(1 to 64)
            0x3f, -- 1/64 duty
            0xD3, -- set display offset	Shift Mapping RAM Counter (0x00~0x3F)
            0x00, -- not offset
            0xd5, -- set display clock divide ratio/oscillator frequency
            0x80, -- set divide ratio, Set Clock as 100 Frames/Sec
            0xD9, -- set pre-charge period
            0xF1, -- Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
            0xDA, -- set com pins hardware configuration
            0x12,
            0xDB, -- set vcomh
            0x40, -- Set VCOM Deselect Level
            0x20, -- Set Page Addressing Mode (0x00/0x01/0x02)
            0x02,
            0x8D, -- set Charge Pump enable/disable
            0x14, -- set(0x10) disable
            0xA4, -- Disable Entire Display On (0xa4/0xa5)
            0xA6, -- Disable Inverse Display On (0xa6/a7)
            0xAF, -- turn on oled panel
        },
        --休眠命令
        sleepcmd = {
            0xAE,
        },
        --唤醒命令
        wakecmd = {
            0xAF,
        }
    }
    --控制SPI引脚的电压域
    pmd.ldoset(6, pmd.LDO_VLCD)
    disp.init(para)
    disp.clear()
    disp.update()
end
