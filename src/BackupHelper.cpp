#include "BackupHelper.h"

std::map<ino_t, std::string> inoToStringMap;

void BackupHelper::doBackup(){

    doFilter();
    if(all_files.empty() | all_files.size() == 1){
        std::cout << "[!] Bad parameter, please check your input path.\n";
        exit(-1);
    }

    // 创建新文件
    initBackupFile();
    
    // 打包
    doPack();

    std::cout << "[+] Finish packing into file: " << this->bkfile_path << std::endl;



}

BackupHelper::BackupHelper(const Paras &p): _compress(p.compress), _encrypt(p.encrypt), re_name(p.re_name), size(p.size), ctime(p.ctime), mtime(p.mtime), passwd(p.password), typenum(p.typenum){
    input_path = p.input_path;
    output_path = p.output_path;
}

void BackupHelper::doFilter(){
    processPath(input_path);
    // std::cout << "筛选前：" << std::endl;
    // for (const auto &element : all_files){
    //     std::cout << element.name << " ";
    // }
    
    if(!typenum.empty()){
        for (auto it = all_files.begin(); it != all_files.end(); ) {
        bool flag_type = checkFilesType(it->metadata);
        if (flag_type) {
            ++it;  
        } else {
            it = all_files.erase(it); 
        }
     }
    }

    if(!size.empty()){
        for (auto it = all_files.begin(); it != all_files.end(); ) {
        bool flag_size = checkFilesSize(it->metadata);
        if (flag_size) {
            ++it;  
        } else {
            it = all_files.erase(it); 
        }
     }
    }

    if(!ctime.empty()){
        for (auto it = all_files.begin(); it != all_files.end(); ) {
        bool flag_ctime = checkFilesChangeTime(it->metadata);
        if (flag_ctime) {
            ++it;  
        } else {
            it = all_files.erase(it); 
        }
     }
    }
    
     if(!mtime.empty()){
        for (auto it = all_files.begin(); it != all_files.end(); ) {
        bool flag_mtime = checkFilesModifyTime(it->metadata);
        if (flag_mtime) {
            ++it;  
        } else {
            it = all_files.erase(it); 
        }
     }
    } 
    // std::cout << "\n筛选后：" << std::endl;
    // for (const auto &element : all_files){
    //     std::cout <<  element.name << " ";
    // }

}

void BackupHelper::processPath(const std::string& current_path) {
   try {
            // 处理给定路径的逻辑
            struct stat current_metadata;
            if (stat(current_path.c_str(), &current_metadata) == 0) {
                File current_file;
                current_file.name = std::filesystem::path(current_path).filename(); // 获取给定路径的文件名
                current_file.metadata = current_metadata;
                all_files.push_back(current_file);
            } else {
                std::cerr << "无法获取给定路径的文件信息：" << current_path << std::endl;
            }

            // 遍历给定路径下的文件和子目录
            for (const auto& entry : std::filesystem::recursive_directory_iterator(current_path)) {
                std::string entry_path = entry.path().string();
                std::string relative_path = std::filesystem::path(current_path).filename().string() + '/' + entry.path().lexically_relative(current_path).string();
                // 创建 File 结构体
                struct stat file_metadata;
                if (stat(entry_path.c_str(), &file_metadata) == 0) {
                    File file;
                    file.name = relative_path;
                    file.metadata = file_metadata;
                    all_files.push_back(file);
                } else {
                    std::cerr << "无法获取文件信息：" << entry_path << std::endl;
                }
            }
        } catch (const std::exception& ex) {
            std::cerr << "遍历路径时发生异常: " << ex.what() << std::endl;
        }
    }


std::time_t convertStringToTime(const std::string& str_time) {
    struct tm tm;
    strptime(str_time.c_str(), "%Y-%m-%d-%H:%M:%S", &tm); // 修改时间格式
    return std::mktime(&tm);
}

bool BackupHelper::checkFilesChangeTime(const struct stat& metadata){
    std::time_t creation_time = metadata.st_ctim.tv_sec;
    std::cout << creation_time << std::endl ;
    std::time_t t1 = convertStringToTime(ctime[0]);
    std::time_t t2 = convertStringToTime(ctime[1]);
    if(creation_time >= t1 && creation_time <= t2){
        return true;
    }
    else{
        return false;
    }
}



bool BackupHelper::checkFilesModifyTime(const struct stat& metadata){
    std::time_t modify_time = metadata.st_mtim.tv_sec;
    std::cout << modify_time << std::endl ;
    std::time_t t1 = convertStringToTime(mtime[0]);
    std::time_t t2 = convertStringToTime(mtime[1]);
    if(modify_time >= t1 && modify_time <= t2){
        return true;
    }
    else{
        return false;
    }
}

bool BackupHelper::checkFilesSize(const struct stat& metadata){
    off_t f_size = metadata.st_size;
    std::cout << "size value: " << f_size << std::endl;
    if(f_size >= size[0] && f_size <= size[1]){
        return true;
    }
    else{
        return false;
    }
}

bool BackupHelper::checkFilesAddress(const struct stat& metadata){

}

bool BackupHelper::checkFilesName(const struct stat& metadata){

}

bool BackupHelper::checkFilesType(const struct stat& metadata){
    int flag = 0;
    for(const auto &t_num : typenum){
        if(t_num !=1 && t_num !=2 && t_num !=3 && t_num !=4){
            std::cerr << "输入了未知的文件类型：" << t_num << std::endl;
        }
        if(t_num == 1){
            if(S_ISREG(metadata.st_mode)){
                flag += 1;
            }
            else{
                ;
            }
        }
        if(t_num == 2){
            if(S_ISDIR(metadata.st_mode)){
                flag += 1;
            }
            else{
                ;
            }
        }
        if(t_num == 3){
            if(S_ISLNK(metadata.st_mode)){
                flag += 1;
            }
            else{
                ;
            }
        }
        if(t_num == 4){
            if(S_ISFIFO(metadata.st_mode)){
                flag += 1;
            }
            else{
                ;
            }
        }
        
    }
    if(flag == typenum.size()){
        return true;
    }
    else{
        return false;
    }
}

std::string generateFileName(void){
    auto currentTime = std::chrono::system_clock::now();

    // 将时间点转换为 time_t
    std::time_t time = std::chrono::system_clock::to_time_t(currentTime);

    // 使用 std::put_time 将 time_t 格式化为字符串
    std::tm timeInfo = *std::localtime(&time);
    std::stringstream ss;
    ss << std::put_time(&timeInfo, "%Y-%m-%d-%H:%M:%S");
    return ss.str();
}

void BackupHelper::initBackupFile(){
    if(output_path.empty()){
        std::cout << "[!] ERROR: Can not get output dir.\n";
        exit(-1);
    }

    std::string tmpfilename = generateFileName();
    tmpfilename = output_path + "/" + tmpfilename + ".bup";

    // 创建新文件
    std::ofstream outputFile(tmpfilename);
    if (!outputFile.is_open()) {
        std::cout << "[!] Can not create new file. \n";
        exit(-1);
    }

    outputFile.close();

    this->bkfile_path = tmpfilename;

    // TODO: 写入压缩文件的头

}

std::filesystem::path changeRELtoABS(std::filesystem::path &path, char *rel_path){

    // 如果rel_path已经是绝对路径，不进行转换
    std::string str(rel_path);
    if(str.front() != '.'){
        std::filesystem::path tmp1(str);
        return tmp1;
    }

    std::istringstream iss(rel_path);
    std::string token;

    std::filesystem::path tmp = path.parent_path();
    
    while (std::getline(iss, token, '/')) {
        if(token == "."){
            tmp = tmp;
        }
        else if(token == ".."){
            tmp = tmp.parent_path();
        }
        else{
            tmp /= token;
        };
    }

    return tmp;
}

void BackupHelper::doPack(){

    // check output file
    if(access(bkfile_path.c_str(), W_OK) != 0){
        std::cerr << "[!] Can not write to bk file. \n";
        exit(-1);
    }

    std::filesystem::path i_path = this->input_path;
    std::filesystem::path parentPath = i_path.parent_path();

    for(const File &elem : all_files){

        std::cout << "[*] Packing file: " << elem.name << std::endl;

        std::filesystem::path backPath = parentPath / elem.name;

        if (std::filesystem::is_symlink(backPath)) {
            // 处理sym link文件
            char targetPath[100];
            // 这里可能获得相对路径，比如./，我们需要转换为绝对路径
            ssize_t bytesRead = readlink(backPath.c_str(), targetPath, 100 - 1);
            if (bytesRead != -1) {
                targetPath[bytesRead] = '\0';
            }

            // 相对路径转换为绝对路径，备份symlink的思路是保持链接对象
            std::filesystem::path abs_p = changeRELtoABS(backPath, targetPath);
            // std::cout << abs_p << std::endl;

            // elem的metadata要更新，用lstat重新获取
            lstat(backPath.c_str(), &elem.metadata);
            Pack pack(elem, backPath);
            pack.write_one_bkfile_into(bkfile_path, abs_p.c_str());
        }
        else if(elem.metadata.st_nlink > 1){
            // 处理hard link文件
            ino_t linkedInode = elem.metadata.st_ino;
            auto it = inoToStringMap.find(linkedInode);

            if (it != inoToStringMap.end()) {
                // 源文件已备份
                const char * link_t = it->second.c_str();

                Pack pack(elem, backPath);
                pack.write_one_bkfile_into(bkfile_path, link_t);
            } else{
                // 源文件未备份，该文件即为源文件
                inoToStringMap[linkedInode] = elem.name;

                // 注，默认该文件为普通文件，不处理嵌套链接
                Pack pack(elem, backPath);
                pack.write_one_bkfile_into(bkfile_path);
            }
        }
        else if(S_ISREG(elem.metadata.st_mode)){

            if(access(backPath.c_str(), R_OK) != 0){
                std::cerr << "[!] Can not read from file : " << elem.name << " .\n";
                exit(-1);
            }
            Pack pack(elem, backPath);
            pack.write_one_bkfile_into(bkfile_path);
        }
    }

}

void BackupHelper::getctime(){
    for(const auto &ctime : ctime){
        std::cout << ctime << std::endl;
    }
}

void BackupHelper::getctimetrans(){
    for(const auto &ctime : ctime){
        std::time_t t = convertStringToTime(ctime);
        std::cout << t << std::endl;
    }
}

void BackupHelper::getmtimetrans(){
    for(const auto &mtime : mtime){
        std::time_t t = convertStringToTime(mtime);
        std::cout << t << std::endl;
    }
}

void BackupHelper::gettype(){
    for(const auto &ftype : typenum){
        std::cout << ftype << std::endl;
    }
}