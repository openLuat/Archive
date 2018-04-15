module(...,package.seeall)

require"pins"

GSENSOR = {name="GSENSOR",pin=pio.P1_4,dir=pio.INT,valid=0}
LIGHTB = {pin=pio.P0_25}

pins.reg(GSENSOR,LIGHTB)
