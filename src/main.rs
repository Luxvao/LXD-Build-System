use std::fs;
use std::env;
use std::process::Command;
use std::process::exit;

enum states {
    START,
    LOOP,
    COMMENT,
    FROM,
    RUN,
    COPY
}



#[allow(non_snake_case)]

fn main() {
    let cmdargs: Vec<String> = env::args().collect();
    let mut container: bool = false;
    let mut path: String = String::from("");
    let mut imageName: String = String::from("");
    let mut containerName: String = String::from("");
    
    if cmdargs.len() < 3 {
        println!("*HELP*\nbuild - pass it the path to the lxdfile and it will build you a container or image. Possible flags: -n - specify the name of the image/container, -c - make a container and don't create an image");
        exit(1);
    }

    if cmdargs[cmdargs.len() - 1].trim() == "-c" || cmdargs[cmdargs.len() - 1].trim() == "-n" || cmdargs[cmdargs.len() - 2].trim() == "-n" {
        println!("The last parameter should be the path to the lxdfile!");
        exit(1);
    }

    let mut counter: usize = 0;
    for i in &cmdargs {
        if counter == 0 {
            counter += 1;
            continue;
        }
        match i.trim() {
            "-n" => {
                name(&mut imageName, &cmdargs, &counter);
            }
            "-c" => {
                container = true;
            }
            _ => {
                if counter == cmdargs.len() - 1 {
                    continue;
                }
                if cmdargs[counter - 1].trim() == "-n" {
                    counter += 1;
                    continue;
                }
                exit(1);
            }
        }
        counter += 1;
    }

    path = cmdargs[cmdargs.len() - 1].clone();

    if !container {
        containerName = imageName.clone() + "tmp";
    }
    else {
        containerName = imageName.clone();
    }

    if path.trim() == "." {
        path = "".to_owned();
    }
    else {
        path += "/";
    }

    execute(&path, &containerName, container);

    if container == false {
        let publishCmd: Vec<String> = vec!["publish".to_owned(), containerName, "--alias=".to_owned() + &imageName];
        Command::new("lxc").args(&["stop", &(imageName.clone() + "tmp")]).status();
        let output = Command::new("lxc").args(&publishCmd).status();
        Command::new("lxc").args(&["rm", &(imageName.clone() + "tmp")]).status();
        println!("{:?}", output);
        exit(0);
    }
}

#[allow(non_snake_case)]

fn name(imageName: &mut String, cmdargs: &Vec<String>, counter: &usize) {
    if cmdargs[counter + 1].trim() == "-c" || cmdargs[counter + 1].trim() == "-n" {
        println!("The name of the image should not be {}", cmdargs[counter + 1]);
        exit(1);
    }

    *imageName = cmdargs[counter + 1].clone();
}

#[allow(non_snake_case)]

fn execute(path: &String, containerName: &String, container: bool) {
    let file = fs::read_to_string(path.to_owned() + "lxdfile");

    let binding = file.unwrap().to_owned();
    let tokens: Vec<&str> = binding.split(|c| c == ' ' || c == '\n').collect();

    let mut state: states = states::START;

    let mut runArgs: Vec<&str> = vec!["exec", containerName, "--", ""];
    let mut copyArgs: Vec<String> = vec!["file".to_owned(), "push".to_owned(), "".to_owned()];

    if tokens[tokens.len() - 1] != "END" {
        println!("You must end the lxdfile with END !");
    }

    for token in tokens {
        println!("{token}\n");
        match state {
            states::START => {
                if token.trim() == "FROM" {
                    state = states::FROM;
                }
                else {
                    println!("The first instruction must be FROM !");
                    exit(1);
                }
            }
            states::LOOP => {
                state = doLoop(&token);
            }
            states::COMMENT => {
                if token.trim() == "*/" {
                    state = states::LOOP;
                    continue;
                }
                else {
                    continue;
                }
            }
            states::FROM => {
                let baseImage = "images:".to_owned() + &token;
                let output = Command::new("lxc").args(&["launch", baseImage.trim(), containerName]).status();
                println!("{:?}", output);
                state = states::LOOP;
            }
            states::RUN => {
                let mut argSize = runArgs.len();
                if checkToken(&token) {
                    runArgs.resize(runArgs.len() - 1, "");
                    let output = Command::new("lxc").args(&runArgs).status();
                    println!("{:?}", output);
                    runArgs.clear();
                    runArgs = vec!["exec", containerName, "--", ""];
                    state = doLoop(&token);
                }
                else {
                    runArgs[argSize - 1] = token.trim();
                    runArgs.resize(runArgs.len() + 1, "");
                }
            }
            states::COPY => {
                let mut argSize = runArgs.len();
                if checkToken(&token) {
                    copyArgs.resize(copyArgs.len() - 1, "".to_owned());
                    let output = Command::new("lxc").args(&copyArgs).status();
                    println!("{:?}", output);
                    copyArgs.clear();
                    copyArgs = vec!["file".to_owned(), "push".to_owned(), "".to_owned()];
                    state = doLoop(&token);
                }
                else {
                    if copyArgs.len() == 4{
                        copyArgs[argSize - 1] = containerName.to_owned() + &"/".to_owned() + &token.to_owned();
                    }
                    else {
                        copyArgs[argSize - 1] = token.trim().to_owned();
                        copyArgs.resize(copyArgs.len() + 1, "".to_owned());
                    }
                }
            }
        }
    }
}


fn checkToken(token: &str) -> bool {
    token == "RUN" || token == "COPY" || token == "END" || token == "/*"
}

fn doLoop(token: &str) -> states {
    let mut state: states = states::LOOP;

    if token == "RUN" {
        state = states::RUN;
    }
    else if token == "COPY" {
        state = states::COPY;
    }
    else if token == "/*" {
        state = states::COMMENT;
    }

    return state;
}
