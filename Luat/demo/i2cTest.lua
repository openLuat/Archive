require "AM2320"
require "pins"

module(..., package.seeall)
-- pmd.ldoset(7, pmd.LDO_VLCD)
-- pmd.ldoset(7, pmd.LDO_VMMC)

-- pins.setup(pio.P0_10,1)

sys.taskInit(function()
    while true do
        local tmp, hum = AM2320.read(2, 0x5c)
        print("tastTask.AM2320 data is : ", tmp, hum, ccnt)
        sys.wait(10000)
    end
end)
