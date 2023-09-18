<div align="center">
    <img src="https://raw.githubusercontent.com/mosswg/dropout-dl/main/assets/dropout_dl_logo.png" width="50%" />
</div>

dropout-dl is tool to download [dropout.tv](https://www.dropout.tv) episodes. It can download single episodes, seasons, or full series.


* [Installation](#installation)
  * [Docker](#docker)
  * [How to Build](#how-to-build)
  * [Dependencies](#Dependencies)
* [Usage](#how-to-use)
  
  * [Options](#options)
  * [Login](#login)
  * [Cookies](#cookies)



# Installation
## Docker
A docker image was created that makes it easier to build and use dropout-dl. You can simply build the docker image without worrying about installing any system dependencies:
```shell
docker build -t dropout-dl:latest .
```
After its done building, you can use it by adding your arguments to the end of the `docker run` command:
```shell
docker run --rm -it -v $PWD/login:/app/login -v $PWD/out:/Downloads dropout-dl:latest --output-directory /Downloads --captions -e https://www.dropout.tv/dimension-20/season:10/videos/the-chosen-ones
```
**Note:** The docker image expects the `login` file to be at `/app/login`.\
You must specify an output directory and mount that directory to the host so that you can retrieve the files from the docker container. In the above command I tell dropout-dl to output everything in `/Downloads` inside the container, which is mounted to a folder named `out` inside the current directory (`$PWD` is current directory).

## How to Build
```
cmake -S <source-dir> -B <build-dir>
cd <build-dir>
make
```

### Dependencies

### Required
* [cURL](https://curl.se/libcurl/) - Required for downloading pages and videos.
### Optional
* [SQLite](https://www.sqlite.org/index.html) - Required for retrieving cookies from browsers.
* [libgcrypt](https://www.gnupg.org/software/libgcrypt/index.html) - Used for decrypting chrome cookies retrieved from the sqlite database.

#### Void
```
sudo xbps-install -S libcurl
```
To install the optional dependencies, run:
```
sudo xbps-install -S sqlite-devel libgcrypt-devel
```

#### Debian/Ubuntu
```
sudo apt install libcurl4-gnutls-dev
```
To install the optional dependencies, run:
```
sudo apt install libsqlite3-dev libgcrypt-dev
```

## How to Use
```
./dropout-dl [options] <url>
```
By default, dropout-dl will download episodes in a season with the format `<series>/<season>/<series> - S<season-num>E<episode-num> - <episode-name>.mp4` and single episodes with the format `<series>/<season>/<series> - <season> - <episode-name>.mp4`.

### Options
```
--help              -h   Display this message
--quality           -q   Set the quality of the downloaded video. Quality can be set to 'all' which
                            will download all qualities and place them into separate folders
--output            -o   Set the output filename. Only works for single episode downloads
--output-directory  -d   Set the directory where files are output
--verbose           -v   Display debug information while running
--browser-cookies   -bc  Use cookies from the browser placed in 'firefox_profile' or 'chrome_profile'
--force-cookies          Interpret the next to argument as the session cookie
--series            -S   Interpret the url as a link to a series and download all episodes from all seasons
--season            -s   Interpret the url as a link to a season and download all episodes from all seasons
--episode           -e   Interpret the url as a link to a single episode
--captions          -c   Download the captions along with the episode. Overridden by --captions-only if set.
--captions-only     -co  Download the captions only, without the episode.";
```

If series, season, or episode is not used, the type will be inferred based on the link format.

### Login
Login in information must be placed in a file called `login` in the same directory as the executable. The file must be email then on a new line password. For example if your email is `email@example.com` and password `password123` the file would be:
```
email@example.com
password123
```

### Cookies
If you would like to avoid putting logging in for any reason cookies can be used. The option `browser-cookies` must be provided.
#### Firefox
Create a file named `firefox_profile` in the build directory and paste in your [firefox profile folder path](https://support.mozilla.org/en-US/kb/profiles-where-firefox-stores-user-data)
#### Chrome
Install libgcrypt and create a file named `chrome_profile` in the build directory and paste in your chrome profile folder path (found on [chrome://version](chrome://version))
#### Other/No Sqlite
Use the `--force-cookies` program option to manually input cookies.

## TODO
- [x] Create tests
- [x] Handle non-alphanumeric characters
- [ ] Test build process on other setups with other OSs.




## Contributing
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg?style=flat-square)](https://makeapullrequest.com)
### Issues
If you have any issues or would like a feature to be added please don't hesitate to submit an issue after checking to make sure it hasn't already been submitted. Using the templates is a good place to start, but sometimes they're overkill. For example, if the program segfaults for you, you don't need to state that the intended behaviour is to not segfault. \
\
If you'd like to contribute a good place to start is looking at open issues and trying to fix one with a pull request. \
**Working on your first Pull Request?** You can learn how from this *free* series [How to Contribute to an Open Source Project on GitHub](https://kcd.im/pull-request)

## Contributors
- [mosswg](https://github.com/mosswg)
- [SeanOMik](https://github.com/SeanOMik) - Docker support
- [Hello-User](https://github.com/Hello-User) - Skip downloading if file exists
