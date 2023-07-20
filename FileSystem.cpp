#include <iostream>
#include <vector>
#include <fstream>
#include <fcntl.h>
#include <filesystem>
#include <limits.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <dirent.h>
namespace fs = std::filesystem;

class File;

class Directory {
    std::string name;
    static std::vector<Directory*> created_directories;
    static std::vector<File*> created_files;
    friend class File;
    static fs::path pwd;
    public:
    	Directory(std::string name) :name(name) {};
	static void setPwd(const char s, std::string directoryName) {
	    fs::path currentPath = fs::current_path();
	    fs::path parentPath;

	    switch (s) {
		case '0':
		    std::cout << "Current Working Directory: " << currentPath << std::endl;
		    pwd = currentPath;
		    break;

		case '1':
		    parentPath = currentPath.parent_path();  // Get the parent directory

		    if (fs::exists(parentPath) && fs::is_directory(parentPath)) {
		        fs::current_path(parentPath);  // Change the working directory to the parent directory
		        std::cout << "Changed directory to: " << fs::current_path() << std::endl;
		        pwd = fs::current_path();
		    } else {
		        std::cout << "Failed to change directory. Parent directory doesn't exist or is not a directory." << std::endl;
		    }
		    break;

		case '2':
		    fs::path targetDirectory = pwd / directoryName;

		    if (fs::exists(targetDirectory) && fs::is_directory(targetDirectory)) {
		        pwd = targetDirectory;
		        std::cout << "Changed directory to: " << pwd << std::endl;
		    } else {
		        std::cout << "Failed to change directory. Directory doesn't exist or is not a directory." << std::endl;
		    }
		    break;
	    }
	}

    	static fs::path getPwd(){
    		return pwd;	
    	}
    	
    	void createDir()
    	{
		fs::path newDirectoryPath = pwd / (this->name);

		if (fs::exists(newDirectoryPath) && fs::is_directory(newDirectoryPath)) {
			std::cout << "Directory already exists!" << std::endl;
		} else if (fs::create_directory(newDirectoryPath)) {
			std::cout << "Directory created successfully!" <<"\n"<< newDirectoryPath <<std::endl;
		} else {
			std::cout << "Failed to create the directory." << std::endl;
		}
		created_directories.push_back(this);
    	}
	static void deleteDir(std::string directoryName)
	{
		fs::path directoryPath = pwd / directoryName;  // Construct the full path to the directory

		// Check if the directory exists
		if (fs::exists(directoryPath) && fs::is_directory(directoryPath)) {
			// Delete the directory and its contents
			if (fs::remove_all(directoryPath)) {
				std::cout << "Directory deleted successfully." << std::endl;
			} else {
				std::cout << "Failed to delete the directory." << std::endl;
			}
		} else {
			std::cout << "Directory not found or is not a directory." << std::endl;
		}

	}
	static void list(fs::path pathname)
	{
	    //fs::path path{fs::current_path()};
	    std::string path_string{pathname.u8string()};
	    //std::cout<<path_string<<std::endl;
	    const char* ch=path_string.c_str();
	    DIR *dir;
	    struct dirent *ent;
	    if ((dir = opendir(ch)) != nullptr) 
	    {
		while ((ent = readdir(dir)) != nullptr) 
		{
		    if (ent->d_name[0] != '.') 
		    {
		        std::cout << ent->d_name << std::endl;
		    }
		}
		closedir(dir);
	    } 
	    else 
	    {
		std::cout << "Directory doesn't exist or permissions weren't granted" << std::endl;
	    }
	}	
};
fs::path Directory::pwd;
std::vector<Directory*> Directory::created_directories;	 //To help the linker  (omitting these 3 lines throws some weird linker error)
std::vector<File*> Directory::created_files;


class File {
    int size;
    std::string name;
    std::string contents;
    friend class Directory;
    public:
    
    File(std::string name) :name(name) {};
    
    void createFile()
    {
    	fs::path directoryName = Directory::pwd;
	fs::path filePath = directoryName / (this->name);
	std::ofstream file(filePath);

	if (file) {
		std::cout << "File created successfully: " << filePath << std::endl;
	} else {
		std::cout << "Failed to create file: " << filePath << std::endl;
	}

	file.close();
	Directory::created_files.push_back(this);
    }
    static void deleteFile(std::string filename)
    {
	fs::path pwd = fs::current_path();  // Get the current directory path

	fs::path filePath = pwd / filename;  // Construct the full path to the file

	// Check if the file exists and is a regular file
	if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
		// Delete the file
		if (fs::remove(filePath)) {
			std::cout << "File deleted successfully." << std::endl;
		} else {
			std::cout << "Failed to delete the file." << std::endl;
		}
	} else {
		std::cout << "File not found or is not a regular file." << std::endl;
	}
    }
	static void readFile(const std::string& filename)
	{
	    std::string fullPath = (Directory::pwd) /  filename;
	    if (fs::exists(fullPath)) {
		std::ifstream file(fullPath);
		if (file.is_open()) {
		    std::cout << "File content:" << std::endl;
		    std::string line;
		    while (std::getline(file, line)) {
		        std::cout << line << std::endl;
		    }
		    file.close();
		} else {
		    std::cout << "Failed to open file: " << fullPath << std::endl;
		}
	    } else {
		std::cout << "File does not exist: " << fullPath << std::endl;
	    }
	}
	static void writeFile(const std::string& fileName)
	{
	fs::path filePath = (Directory::pwd) / fileName;  // Construct the full path to the file

	if (fs::exists(filePath) && fs::is_regular_file(filePath)) {
		std::ofstream outputFile(filePath, std::ios::app);

		if (outputFile) {
			std::string content;
			std::cout << "Enter the content to write to the file: ";
			std::cin.ignore();  // Ignore any remaining newline characters in the input stream
			std::getline(std::cin, content);
			outputFile << content;
			outputFile.close();

			// Re-open the file for reading
			std::ifstream inputFile(filePath);

			if (inputFile) {
				// Determine the file size by seeking to the end of the file and getting the position
				inputFile.seekg(0, std::ios::end);
				int fileSize = inputFile.tellg();
				inputFile.close();

				std::cout << "File written successfully. Size: " << fileSize << " bytes." << std::endl;
			} else {
				std::cout << "Failed to open the file for reading." << std::endl;
			}
		} else {
			std::cout << "Failed to open the file for writing." << std::endl;
		}
	} else {
		std::cout << "File not found or is not a regular file." << std::endl;
	}
	}
};


int main(){
	Directory::setPwd('0',"");
	char ch='g';
	char s='0';
	std::string d_name="";
	std::cout<<"This VFS initializes in the directory in which the program resides"<<"\n"<<std::endl;
	while(true){
		std::cout<<std::endl;
		std::cout<<"PWD:"<<std::endl;
		std::cout<<Directory::getPwd()<<std::endl;
		std::cout<<std::endl;
		std::cout<<"MENU:"<<std::endl;
		std::cout<<"a) Change PWD"<<std::endl;
		std::cout<<"b) Create a file"<<std::endl;
		std::cout<<"c) Create a directory"<<std::endl;
		std::cout<<"d) Delete a file or directory"<<std::endl;
		std::cout<<"e) List files and directories"<<std::endl;
		std::cout<<"f) Read a file"<<std::endl;
		std::cout<<"g) Write to a file"<<std::endl;
		std::cout<<"h) Exit the program"<<std::endl;
		std::cout<<"\n"<<"Enter Your Choice:"<<std::endl;
		std::cin>>ch;
		switch (ch){
			case 'A':
			case 'a':
				std::cout<<"Options for cd:"<<std::endl;
		                std::cout<<"0 - Get Present Working Directory"<<std::endl;
		                std::cout<<"1 - Change pwd to Parent Directory"<<std::endl;
		                std::cout<<"2 - Enter a Child directory"<<std::endl;
		                std::cout<<"Enter your choice(0/1/2):"<<std::endl;
		                std::cin>>s;
				switch(s){
					case '0':std::cout<<Directory::getPwd()<<std::endl;
						break;
					case '1':Directory::setPwd('1',"");
						break;
					case '2':
						std::cout<<"Enter child directory name:"<<std::endl;
						std::cin>>d_name;
						Directory::setPwd('2',d_name);
						break;
					default: std::cout<<"Invalid Choice!"<<std::endl;
						break;
				}
				break;
			case 'B':
			case 'b':
				std::cout<<"You will be creating the file in "<<Directory::getPwd()<<std::endl;
				std::cout<<"Enter the filename you want to create (Enter 'X' to exit from this MENU):"<<std::endl;
				std::cin>>d_name;
				if(d_name!="X")
				{
					File new_file(d_name);
					new_file.createFile();
				}						
				break;
			case 'C':
			case 'c':
				std::cout<<"You will be creating the directory in "<<Directory::getPwd()<<std::endl;
				std::cout<<"Enter the name of the directory you want to create (Enter 'X' to exit from this MENU):"<<std::endl;
				std::cin>>d_name;
				if(d_name!="X")
				{
					Directory new_directory(d_name);
					new_directory.createDir();
				}
				break;
			case 'D':
			case 'd':
				std::cout<<"Do you want to delete a file(0) OR a directory(1) in "<<Directory::getPwd()<<"?"<< " (Press 2 to exit this menu)"<<"\n"<<"Enter(0/1/2):"<<std::endl;
				std::cin>>s;
				switch(s){
				case '0':
					std::cout<<"Enter the name of the file you want to delete:"<<std::endl;
					std::cin>>d_name;
					File::deleteFile(d_name);
					break;
					
				case '1':
					std::cout<<"Enter the name of the directory you want to delete:"<<std::endl;
					std::cin>>d_name;
					Directory::deleteDir(d_name);
					break;
				}
				break;
			case 'E':
			case 'e':
				std::cout<<"Listing all Files and Directories within "<<Directory::getPwd()<<"(ls):\n"<<std::endl;
				Directory::list(Directory::getPwd());
				break;
			case 'F':
			case 'f':
				std::cout<<"Enter the name of the file you want to read:"<<std::endl;
				std::cin>>d_name;
				File::readFile(d_name);
				break;
			case 'G':
			case 'g':
				std::cout<<"Enter the name of the file you want to write to:"<<std::endl;
				std::cin>>d_name;
				File::writeFile(d_name);
				break;
			case 'H':
			case 'h':
				return 0;
			default: std::cout<<"Invalid Choice"<<std::endl;
		}
	}
}





























