## Synopsis

This project is an implementation of the FAT12 file system for Champlain College's Operating Systems Architecture course (CSI-385). The project implements the following commands:
* pbs
* pfe {start} {end}
* ls {path}
* cd {path}
* touch {path}
* mkdir {path}
* rm {path}
* rmdir {path}
* cat

Note that these commands are implemented without support for option flags.

## Installation

To get the project running, open the root project directory in the terminal and run `make`. Then do `cd ./build` and `./shell {floppy}` to run the shell with a specified floppy name. For testing purposes, it is suggested that you add any additional floppy images to src/disk_images.

## References

Inspiration was drawn from Alexander Apmann's implementation of the FAT12 file system as well as various un-cited online resources. All external references are cited appropriate where they occur in code.

## Tests

Automated tests scripts stored in the 'Test Scripts' directory are executed using a third party application.
If you wish to execute them, use the following steps.

	* `sudo apt-get install actionaz`
	* Open the script in the application.
	* Click the 'gear' icon and click within a terminal window open from the build directory within 5 seconds.

## Authors
* Matthew Dean (https://www.linkedin.com/in/matthew-dean-b804b485)
* John Ryan (https://www.linkedin.com/in/john-ryan-5b704687)

## License
The code authors release this code as public domain, and only request that references and use of this code be properly cited in anywhere that it is explicitly or implicitly used.