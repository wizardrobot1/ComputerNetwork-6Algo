targets:=sender1_network receiver1_network sender1_datalink receiver1_datalink sender1_physical receiver1_physical

common_dir := ../common
local_dir := .

common_src := $(wildcard $(common_dir)/*.c)#取common文件夹所有.c 文件名 (带目录)
common_obj := $(patsubst %.c,%.o,$(common_src))#把 common文件夹所有.c 文件名 换成 .o 文件名 (带目录)

local_src_a := $(wildcard *.c)#取本文件夹所有.c 文件名
targetsc :=$(patsubst %,%.c,$(targets))#targets的c文件
local_src :=$(filter-out $(targetsc),$(local_src_a))#去targets的c文件
local_obj := $(patsubst %.c,%.o,$(local_src))#把本文件夹所有.c 文件名 换成 .o 文件名 

objs:=$(common_obj)
objs+=$(local_obj)



all:$(targets)

.PHONY:all clean

$(objs):%.o:%.c#在各自目录下生成obj文件
	gcc -c $< -o $@

$(targets):$(objs)

$(targets):%:%.o
	gcc -o $@ $< $(objs)

clean:
	-rm -f $(targets) *.o $(common_dir)/*.o network_datalink.share.* datalink_network.share.* physical_datalink.share.* datalink_physical.share.* 接收到的文件

