#include <iostream>
#include <fstream>
#include <string>



// Just to be clear; this function parses and builds the image
void parse(std::string tokens[], int counter, std::string imageName) {
    std::string containerName = imageName + "tmp";
    std::string baseImage = "";

    // Put code here next!!!!
    if (tokens[0] == "FROM") {
        if (tokens[1] == "RUN" || tokens[1] == "COPY" || tokens[1] == "FROM" || tokens[1] == "CONFIGURE") { std::cout << "Please specify the base container image!\n"; exit(1); }
        else {
            baseImage = tokens[1];

            // Make a container
            std::cout << "Pulling base image and getting things ready...\n";
            std::string setupCmd = "lxc launch images:" + tokens[1] + " " + containerName;
            system(setupCmd.c_str());
        }

        for (int i = 2; i < counter; i++)
        {
            if (tokens[i] == "CONFIGURE") {

            }
            else if (tokens[i] == "RUN") {
                std::string runCmd = "lxc exec " + containerName + " --";
                for (int j = i+1; j < counter; j++) {
                    if (tokens[j] == "RUN" || tokens[j] == "COPY" || tokens[j] == "FROM" || tokens[j] == "CONFIGURE") {
                        break;
                    }
                    else {
                        runCmd += " ";
                        runCmd += tokens[j];
                    }
                }
                system(runCmd.c_str());
            }
        }
        
    }
    else {
        std::cout << "The first token should be FROM (so lbs can know what distro to use as the base system; do -h for help)\n";
    }

}



int getCounter(std::string path) {

    // Find out how many tokens are in the lxdfile
    int counter = 0;
    std::ifstream file;

    if (path == ".") {
        file.open("lxdfile");
    }
    else {
        file.open(path + "/lxdfile");
    }

    if (!file.good()) {
        std::cout << "lxdfile not found!\n";
        exit(1);
    }


    if (file.is_open()) {
        std::string token;
        file >> token;
        std::string prevToken = "";
        
        while (token != prevToken) {
            counter++;

            // Make sure we don't read more tokens then there actually are
            prevToken = token;
            file >> token;
        }
    }
    file.close();

    return counter;
}


void makeList(std::string path, std::string imageName) {
    std::string tokenList[getCounter(path) - 1];
    std::ifstream file;

    if (path == ".") {
        file.open("lxdfile");
    }
    else {
        file.open(path + "/lxdfile");
    }
        

    if (file.is_open()) {
        std::string token;
        file >> token;
        std::string prevToken = "";
        
        for (int i = 0; i < getCounter(path); i++) {
            tokenList[i] = token;
            file >> token;
        }
    }

    file.close();

    parse(tokenList, getCounter(path), imageName);

    return;
}

int main(int argc, char* argv[]) {
    std::string arg1 = argv[1];
    if (argc == 4 && arg1 == "-t") {
        std::string imageName = argv[2];
        std::string path = argv[3];
        makeList(path, imageName);

        // Run lxc publish here, okay?


    }
    else if (arg1 == "-h" || arg1 == "--help") {
        std::cout << "\nHelp menu:\n-h/--help - Shows this help screen\n-t - Is used to name the built image - lbs -t <image_name>\nThe 3rd argument should be the path to the folder containing the lxdfile or . (for current directory)\nIMPORTANT NOTE: Please remember that this tool requires excatly 3 arguments\n\n";
    }
    else {
        std::cout << "\nHelp menu:\n-h/--help - Shows this help screen\n-t - Is used to name the built image - lbs -t <image_name>\nThe 3rd argument should be the path to the folder containing the lxdfile or . (for current directory)\nIMPORTANT NOTE: Please remember that this tool requires excatly 3 arguments\n\n";
    }
}