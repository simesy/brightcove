; Brightcove Make
; Allows recursive downloading of brightcove dependencies.

api = 2
core = 6

projects[cck][version] = "2.9"
projects[cck][subdir] = "contrib"

projects[jquery_ui][version] = "1.5"
projects[jquery_ui][subdir] = "contrib"

projects[modalframe][version] = "1.7"
projects[modalframe][subdir] = "contrib"

libraries[jquery-ui][download][type] = "get"
libraries[jquery-ui][download][url] = "http://jquery-ui.googlecode.com/files/jquery-ui-1.7.3.zip"
libraries[jquery-ui][directory_name] = "jquery.ui"
libraries[jquery-ui][destination] = "modules/contrib/jquery_ui"

libraries[lib-bcmapi][download][type] = "get"
libraries[lib-bcmapi][download][url] = "https://github.com/downloads/BrightcoveOS/PHP-MAPI-Wrapper/BrightcoveOS-PHP-MAPI-Wrapper-2.0.5.zip"
libraries[lib-bcmapi][directory_name] = "bcmapi"

