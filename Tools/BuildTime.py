# 自动生成构建信息(构建序列号、构建时间等)并写入指定头文件，在构建项目之前调用。
# 用法1：命令行 python BuildTime.py [<target_file>.h]。
# 用法2：脚本 BuildTime([<target_file>.h])。
# Powered by feallee@hotmail.com on 2026/02/11.
import datetime  
import sys  
def BuildTime(target_file='BuildTime.h'): 
    build_number = 0
    try:  
        with open(target_file, 'r') as file:  
            for line in file:
                if '#define BUILD_TIME_NUMBER' in line:  
                    build_number_str = line.split()[-1]  
                    build_number = int(build_number_str) + 1  
                    if build_number > 255:  
                        build_number = 0  
                    break  
    except :       
        build_number = 0  
              
    now = datetime.datetime.now().strftime('%Y%m%d%H%M%S%f')   
    print("[BuildTime]Number: %d, Time: %s" % (build_number, now[:-3])) 
      
    try:  
        with open(target_file, 'w') as file:             
            file.write('#pragma once\n#ifdef __cplusplus\nextern "C"\n{\n#endif\n')  
            file.write('#define BUILD_TIME_NUMBER %d\n' % (build_number))
            file.write('#define BUILD_TIME_YEAR_4 %s\n' % (now[0]))
            file.write('#define BUILD_TIME_YEAR_3 %s\n' % (now[1]))
            file.write('#define BUILD_TIME_YEAR_2 %s\n' % (now[2]))  
            file.write('#define BUILD_TIME_YEAR_1 %s\n' % (now[3]))  
            file.write('#define BUILD_TIME_MONTH_2 %s\n' % (now[4]))
            file.write('#define BUILD_TIME_MONTH_1 %s\n' % (now[5]))
            file.write('#define BUILD_TIME_DAY_2 %s\n' % (now[6]))
            file.write('#define BUILD_TIME_DAY_1 %s\n' % (now[7]))  
            file.write('#define BUILD_TIME_HOUR_2 %s\n' % (now[8]))
            file.write('#define BUILD_TIME_HOUR_1 %s\n' % (now[9]))  
            file.write('#define BUILD_TIME_MINUTE_2 %s\n' % (now[10]))
            file.write('#define BUILD_TIME_MINUTE_1 %s\n' % (now[11]))              
            file.write('#define BUILD_TIME_SECOND_2 %s\n' % (now[12]))  
            file.write('#define BUILD_TIME_SECOND_1 %s\n' % (now[13])) 
            file.write('#define BUILD_TIME_MILLISECOND_3 %s\n' % (now[14]))
            file.write('#define BUILD_TIME_MILLISECOND_2 %s\n' % (now[15]))
            file.write('#define BUILD_TIME_MILLISECOND_1 %s\n' % (now[16])) 
            file.write('#ifdef __cplusplus\n}\n#endif\n')
    except Exception as e:  
        print(f'[BuildTime]Write file error: {target_file}, {e}')  

if __name__ == '__main__':  
    if len(sys.argv)>1:
        BuildTime(sys.argv[1])
    else:  
        BuildTime()  