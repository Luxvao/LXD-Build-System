# DEV branch
This is a work in progress branch that has the latest features. The features here are not 100% finished.
Supports:
- Making a container
- Choosing the base image
- Running commands in the container
- Building images
- Comments! I actually implemented comments. They are very basic (`/*` to start a comment and `*/` to end one, but be aware, this syntax won't work: `/*comment*/`, there MUST be a space between the symbol and the comment like so: `/* comment */`. Sorry for the inconvenience, will be fixed later, but is not the main priority currently) 

# LBS - LXD Build System
This is a project meant for making LXD "images". It's just an over-complicated shell script, but it has the basics of the Dockerfile syntax.
# Functions
Right now it supports:
- Making a container
- Choosing the base image
- Running commands in the container
- Building images
- Copying files from disk to container
# Stuff I'll add
- Volume management
- Some way to configure containers
