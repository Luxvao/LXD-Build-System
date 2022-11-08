/*
     This file is part of LBS.

    LBS is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    LBS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with LBS. If not, see <https://www.gnu.org/licenses/>. 
*/

#include <iostream>
#include <fstream>
#include <string>


void parse(std::string tokens[], int counter, std::string imageName, bool buildImage) {
    std::string containerName = "";
    if (buildImage == true) {
        containerName = imageName + "tmp";
    }
    else {
        containerName = imageName;
    }
    std::string baseImage = "";

    if (tokens[counter-1] != "END") {
        std::cout << "You must end the lxdfile with the END instruction.\n";
        for (int i = 0; i < counter; i++) {
            std::cout << tokens[i] << "\n";
        }
        exit(1);
    }

    if (tokens[0] == "FROM") {
        if (tokens[1] == "RUN" || tokens[1] == "COPY" || tokens[1] == "FROM" || tokens[1] == "CONFIGURE") { std::cout << "Please specify the base container image!\n"; exit(1); }
        else {
            baseImage = tokens[1];

            std::cout << "Pulling base image and getting things ready...\n";
            std::string setupCmd = "lxc launch images:" + tokens[1] + " " + containerName;
            system(setupCmd.c_str());
        }

        for (int i = 2; i < counter; i++)
        {
            if (tokens[i] == "CONFIGUE") {
                std::string configCmd = "lxc config set " + containerName + " ";
                for (int j = i+1; j < counter; j++) {
                    if (tokens[j] == "RUN" || tokens[j] == "COPY" || tokens[j] == "FROM" || tokens[j] == "CONFIGURE" || tokens[j] == "END") {
                        break;
                    }
                    else {
                        configCmd += " ";
                        configCmd += tokens[j];
                    }
                }
                system(configCmd.c_str());
            }
            else if (tokens[i] == "VOLUME") {

            }
            else if (tokens[i] == "DEVICEADD") {
                std::string deviceCmd = "lxc config device add " + containerName + " ";
                for (int j = i+1; j < counter; j++) {
                    if (tokens[j] == "RUN" || tokens[j] == "COPY" || tokens[j] == "FROM" || tokens[j] == "CONFIGURE" || tokens[j] == "END") {
                        break;
                    }
                    else {
                        deviceCmd += " ";
                        deviceCmd += tokens[j];
                    }
                }
                system(deviceCmd.c_str());
            }
            else if (tokens[i] == "RUN") {
                std::string runCmd = "lxc exec " + containerName + " --";
                for (int j = i+1; j < counter; j++) {
                    if (tokens[j] == "RUN" || tokens[j] == "COPY" || tokens[j] == "FROM" || tokens[j] == "CONFIGURE" || tokens[j] == "END") {
                        break;
                    }
                    else {
                        runCmd += " ";
                        runCmd += tokens[j];
                    }
                }
                system(runCmd.c_str());
            }
            else if (tokens[i] == "COPY") {
                std::string copyCmd = "lxc file push ";
                std::string file = "";
                std::string dest = " " + containerName + "/";
                if (tokens[i+3] == "RUN" || tokens[i+3] == "COPY" || tokens[i+3] == "FROM" || tokens[i+3] == "CONFIGURE" || tokens[i+3] == "END") {
                    file = tokens[i+1];
                    dest += tokens[i+2];
                    copyCmd += file;
                    copyCmd += dest;
                    system(copyCmd.c_str());
                }
                else {
                    std::cout << "ERROR -> " + tokens[i] + " " + tokens[i+1] + " " + tokens[i+2] + " <-\n";
                }
            }
        }
        
    }
    else {
        std::cout << "The first token should be FROM (so lbs can know what image to use as the base system; do -h for help)\n";
    }

}



int getCounter(std::string path) {
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

            prevToken = token;
            file >> token;
        }
    }
    file.close();

    return counter;
}


void makeList(std::string path, std::string imageName, bool buildImage) {
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

    parse(tokenList, getCounter(path), imageName, buildImage);

    return;
}

int main(int argc, char* argv[]) {
    std::string path = "";
    bool continueThrough = false;
    std::string imageName = "";
    bool buildImage = true;

    if (argv[1] == NULL) { std::cout << "\nPlease ceeck out lbs -h.\n\n"; exit(1); }

    std::string arg1 = argv[1];
    std::string lArg = argv[argc-1];

    for (int i = 1; i < argc; i++) {
        std::string tmpArgs = argv[i];
        if (tmpArgs == "-c") {
            buildImage = false;
        }
        else if (tmpArgs == "-t") {
            if (argc-1 == i+1 || argc-1 == i) {
                std::cout << "You must specify the lxdfile location or must enter the container/image name\n";
                exit(1);
            }
            imageName = argv[i+1];
        }
    }
    continueThrough = true;

    if (lArg != "-c" && lArg != "-t") {
        path = lArg;
    }
    else {
        std::cout << "The last argument should be the lxdfile location\n";
        exit(1);
    }

    if (arg1 == "-h" || arg1 == "--help") {
        std::cout << "\nHelp menu:\n-h/--help - Shows this help screen\n-t - Is used to name the built image - lbs -t <image_name>\n-c - Is used when you want to just create a container from the lxdfile\nThe last argument should be the path to the folder containing the lxdfile or . (for current directory)\n\n";
        exit(1);
    }
    else if (continueThrough != true) {
        std::cout << "\nUnknows command. Please check out lbs -h\n\n";
        exit(1);
    }

    makeList(path, imageName, buildImage);

    if (buildImage == true) {
        std::string buildCmd = "lxc publish " + imageName + "tmp " + "--alias=" + imageName;
        std::string stopCmd = "lxc stop " + imageName + "tmp";
        std::string cleanupCmd = "lxc rm " + imageName + "tmp";
        system(stopCmd.c_str());
        system(buildCmd.c_str());
        std::cout << "Cleaning up...\n";
        system(cleanupCmd.c_str());
        std::cout << "The image is ready!\n";
    }
    else {
        std::cout << "The container was created and set up!\n";
    }
}