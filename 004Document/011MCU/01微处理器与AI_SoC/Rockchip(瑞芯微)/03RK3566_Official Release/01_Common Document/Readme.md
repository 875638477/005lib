RK3566_PinOut_V10_20210601.xlsx sheet covers the pinout details, iolist, pinlength of package(pin_delay), usage of unused pins, IO strength level, and more.

[█████████████████████████████]
----------Important Notes 重要提醒----------	
		
RK3566/RK3568共有10个独立的IO电源域，分别为PMUIO[0:2]和VCCIO[1:7]。其中:
There are 10 independent IO power domains in RK3566/RK3568, they are PMUIO[0:2] and VCCIO[1:7].

1.PMUIO0、PMUIO1为固定电平电源域，不可配置；
1.PMUIO0 and PMUIO1 are fixed-level power domains which cannot be configured;

2.PMUIO2和VCCIO1,VCCIO[3:7]电源域均要求硬件供电电压与软件的配置（即如下所示的dts配置信息）相匹配:
2.PMUIO2 and VCCIO1,VCCIO[3:7]power domains require that their hardware power supply voltages must be consistent with the software configuration correspondingly (Like the dts configuration information shown below);
1) 当硬件IO电平接1.8V，软件电压配置也要相应配成1.8V;
1) When the hardware IO level is connected to 1.8V, the software voltage configuration should also be configured to 1.8V accordingly;
2) 当硬件IO电平接3.3V，软件电压配置也要相应配成3.3V
2) When the hardware IO level is connected to 3.3V, the software voltage configuration should also be configured to 3.3V accordingly

&pmu_io_domains {
status = "okay";
pmuio2-supply = <&vcc_3v3>;
vccio1-supply = <&vcc_3v3>;
vccio3-supply = <&vcc_3v3>;
vccio4-supply = <&vcc_3v3>;
vccio5-supply = <&vcc_3v3>;
vccio6-supply = <&vcc_3v3>;
vccio7-supply = <&vcc_3v3>;
};

3.VCCIO2电源域软件不需要配置，但是其硬件供电电压与FLASH_VOL_SEL状态必须相匹配：
3. There is no need to configure VCCIO2 power domain by software, but its hardware power supply and FLASH_VOL_SEL status must be matched:
1) 当VCCIO2供电是1.8V，则FLASH_VOL_SEL管脚必须保持为高电平；
1) When VCCIO2 voltage is connected to 1.8V, FLASH_VOL_SEL must be high;
2) 当VCCIO2供电是3.3V，则FLASH_VOL_SEL管脚必须保持为低电平
2) When VCCIO2 voltage is connected to 3.3V, FLASH_VOL_SEL must be low;

否则Otherwise:
1.软件配置为1.8V，硬件供电3.3V，会使得IO处于过压状态，长期工作IO会损坏；
1.If the software configuration is 1.8V, but the hardware power supply is 3.3V, it will cause the low withstand voltage circuit working in overvoltage state, and the chipset will be damaged after long time working.
2.软件配置为3.3V，硬件供电1.8V，IO功能会异常；
2.If the software configuration is 3.3V, but the hardware power supply is 1.8V, the circuit will work abnormally;

说明 Note:
1.上述软件dts配置文件及路径，参考选择如下（以下选择仅供参考，客户需要根据实际项目的需求自行选择合适的配置文件进行修改，切记不可直接复制！）：
1.The reference choices of software dts configuration files and their path are shown as follows (customers have to choose the appropriate configuration files to modify by themselves according to their projects，Remember not to copy them directly!)

1) Linux和Android通用产品 General products：
    arch/arm64/boot/dts/rockchip/rk3568-evb.dtsi
 2) 电子书产品 eBOOK products：
    arch/arm64/boot/dts/rockchip/rk3566-rk817-eink.dts
 3) 行业平板产品 Tablet products：
    arch/arm64/boot/dts/rockchip/rk3566-rk817-tablet.dts
 4) NVR产品 NVR products：
    arch/arm64/boot/dts/rockchip/rk3568-nvr.dtsi
 5) BOX产品 BOX products
    arch/arm64/boot/dts/rockchip/rk3566-box-demo-v10.dtsi
2.若客户项目的电源域有变动，必须相应更新匹配。我司各类文档均已强调此注意项，请客户软硬件人员重新审核各自项目电压配置的正确性！
2. If there is a change in the power domain of the customer project, it must be updated accordingly.
Various documents released by Rockchip have emphasized this notice, please review the voltage configuration and hardware power supply of your projects as soon as possible!

3.参考文档Reference documents：
1）https://redmine.rock-chips.com/documents/106

[█████████████████████████████]	
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			
