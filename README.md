xibaluoma  
发现了一个bug  
大概率是两个插件的冲突，导致我打不开md文件  

先ctrl+shift+p打开命令面板  

输入Disable All Installed Extensions  

额禁用了所有的扩展，甚至连接不了服务器了，当然也看不了文件  

慢慢的一个一个打开，慢慢排查到了Office Viewer(Markdown Editor)  
因为我是先打开的Markdown Preview Enhanced  
所以是可以打开md并且有那个白色的预览界面的，到了Office Viewer(Markdown Editor)之后就出问题了  

故而判断出这两个插件存在冲突mlgbd  

Back to the threadpool:  
version 1:  

>usage:
./gen_test.sh 10000  
cmake -Bbuild   
cmake --build ./build  
./build/src/main data/ sorted.txt  

>output:
file list write to filelist.txt.  
delete tmp dir: /tmp/sort.  
create tmp dir: /tmp/sort.  
sort file chunk in /tmp/sort.  
sort partial sorted file chunk.  
copy result to sorted.txt.  