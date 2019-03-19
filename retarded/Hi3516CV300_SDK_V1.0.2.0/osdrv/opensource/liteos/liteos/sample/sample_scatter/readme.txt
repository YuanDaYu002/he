编译方法：
	执行make liteos_image即可。
	清除则使用make liteos_image_clean。
注意：
	在退出本目录去编译其他的sample之前，请在本目录下，使用make liteos_image_clean命令来还原liteos的ld脚本文件，
	否则其他sample会编译不过。
	本示例仅以spi nor为例，若需要在其它flash上运行，需要自行修改image_read函数实现

