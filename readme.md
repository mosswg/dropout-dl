# dropout-dl
a tool to download [dropout.tv](dropout.tv) episodes.


## how to build
```
cmake -S <source-dir> -B <build-dir>
cd <build-dir>
make
```

## setup
the sqlite and curl libraries are required \
additionally [cookies](#cookies) are needed

### dependency installation
#### void linux
```
sudo xbps-install -S libcurl
```

#### debian
```
sudo apt install libcurl
```

## cookies
### firefox
#### option 1 (requires sqlite-devel)
create a file named `firefox_profile` in the build directory and paste in your [firefox profile folder path](https://support.mozilla.org/en-US/kb/profiles-where-firefox-stores-user-data)
#### option 2 (requires sqlite)
close firefox and go to your [firefox profile folder](https://support.mozilla.org/en-US/kb/profiles-where-firefox-stores-user-data)
```
sqlite3 cookies.sqlite "SELECT value FROM moz_cookies WHERE host LIKE '%dropout.tv%' AND name='__cf_bm';" > <build-dir>/auth_cookie
sqlite3 cookies.sqlite "SELECT value FROM moz_cookies WHERE host LIKE '%dropout.tv%' AND name='_session';" > <build-dir>/session_cookie
```
this needs to be redone every time the cookies expire (~30 minutes)
#### option 3
open firefox and go to any dropout.tv episode \
open the dev tools and go to network then refresh \
search for `?api` and select the top request \
copy the `__cf_bm` cookie from the cookies section \
create a file called `auth_cookie` and paste the cookie in the file \
go back to firefox and copy the `_session` cookie into a file named `session_cookie` \
this needs to be redone everytime the cookie expires (~30 minutes)
### chrome
#### option 1 (requires sqlite-devel and libgcrypt) NOT CURRENTLY FUNCTIONAL
create a file named `chrome_profile` in the build directory and paste in your chrome profile folder path (found on [chrome://version](chrome://version))
#### option 2
go to settings > privacy and security > cookies > see all cookies > vhx.tv > __cf_bm \
copy the `content` and paste it into the `cookie` file \
this needs to be redone every time the cookies expire (~30 minutes)

## How to Use
```
./dropout-dl <url> [OPTIONS]
```

### Options
```
--help                   Display this message
--quality                Set the quality of the downloaded video. Quality can be set to 'all' which
                             will download all qualities and place them into separate folders
--output                 Set the output filename
--output-directory       Set the directory where files are output
--verbose                Display debug information while running
--force-cookies          Interpret the next to arguments as authentication cookie and session cookie
```
dropout-dl will download the episode into a folder with the name of the series in the format
`S<season-num>E<episode-num><name>.mp4`