# dropout-dl
a tool to download [dropout.tv](dropout.tv) episodes.


## how to build
```
cmake -S <source-dir> -B <build-dir>
cd <build-dir>
make
```

### Dependency Installation
sqlite-devel is optional but highly recommended.

#### Void
```
sudo xbps-install -S libcurl sqlite-devel
```

#### Debian
```
sudo apt install libcurl sqlite-devel
```

## cookies
### Firefox
#### Option 1 (requires sqlite-devel)
Create a file named `firefox_profile` in the build directory and paste in your [firefox profile folder path](https://support.mozilla.org/en-US/kb/profiles-where-firefox-stores-user-data)
#### Option 2 (requires sqlite)
Close firefox and go to your [firefox profile folder](https://support.mozilla.org/en-US/kb/profiles-where-firefox-stores-user-data)
```
sqlite3 cookies.sqlite "SELECT value FROM moz_cookies WHERE host LIKE '%dropout.tv%' AND name='__cf_bm';" > <build-dir>/auth_cookie
sqlite3 cookies.sqlite "SELECT value FROM moz_cookies WHERE host LIKE '%dropout.tv%' AND name='_session';" > <build-dir>/session_cookie
```
This needs to be redone every time the cookies expire (~30 minutes)
#### Option 3
Open firefox and go to any dropout.tv episode \
Open the dev tools and go to network then refresh \
Search for `?api` and select the top request \
Copy the `__cf_bm` cookie from the cookies section \
Create a file called `auth_cookie` and paste the cookie in the file \
Go back to firefox and copy the `_session` cookie into a file named `session_cookie` \
This needs to be redone everytime the cookie expires (~30 minutes)
### chrome
#### Option 1 (requires sqlite-devel and libgcrypt) NOT CURRENTLY FUNCTIONAL
Create a file named `chrome_profile` in the build directory and paste in your chrome profile folder path (found on [chrome://version](chrome://version))
#### Option 2
Go to settings > privacy and security > cookies > see all cookies > vhx.tv > __cf_bm \
Copy the `content` and paste it into the `cookie` file \
This needs to be redone every time the cookies expire (~30 minutes)

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
--series                 Interpret the url as a link to a series and download all episodes from all seasons
--season                 Interpret the url as a link to a season and download all episodes from all seasons
```
By default, dropout-dl will download the episode in the format `<series>/S<season-num>E<episode-num><name>.mp4`