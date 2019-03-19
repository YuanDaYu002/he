Compile method:
	Use "make liteos_image" to compile.
	If you want to clean the sample, use "make liteos_image_clean".
Attention:
	Before exiting this directory to compile other sample, please
	use the command "make liteos_image_clean" to restore ld  files in this directory,
	otherwise other sample would fail while compiling.
	This sample is working on spi nor flash. If you want to make it work on other flash such as nand/spi nand,
	you need to modify the function image_read()
	

